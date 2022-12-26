/**
  *  \file interpreter/filefunctions.cpp
  *  \brief Interpreter: File I/O Related Stuff
  *
  *  PCC2 comment:
  *
  *  This module implements all file I/O related functions. Some are
  *  implemented as builtin commands due to their special syntax,
  *  which internally call regular commands.
  *
  *  \todo This generates and evaluates <tt>ufilenr</tt> instructions.
  *  Those are actually not required at runtime; we know we're dealing
  *  with file numbers. We can, however, not remove them completely at
  *  compile time because many commands are called regularily, and the
  *  compiler makes no further assumptions about regular commands.
  *  Removing the <tt>ufilenr</tt> instructions completely would mean
  *  we accept file numbers in invalid places.
  */

#include <memory>
#include "interpreter/filefunctions.hpp"
#include "afl/bits/fixedstring.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/blobvalue.hpp"
#include "interpreter/error.hpp"
#include "interpreter/expr/node.hpp"
#include "interpreter/expr/parser.hpp"
#include "interpreter/opcode.hpp"
#include "interpreter/simplefunction.hpp"
#include "interpreter/simpleprocedure.hpp"
#include "interpreter/specialcommand.hpp"
#include "interpreter/statementcompilationcontext.hpp"
#include "interpreter/statementcompiler.hpp"
#include "interpreter/tokenizer.hpp"
#include "interpreter/values.hpp"
#include "interpreter/world.hpp"
#include "util/io.hpp"

using afl::io::TextFile;
using interpreter::Arguments;
using interpreter::BytecodeObject;
using interpreter::FileTable;
using interpreter::Opcode;
using interpreter::StatementCompilationContext;
using interpreter::Tokenizer;
using interpreter::World;
using interpreter::checkArgumentCount;
using interpreter::checkBooleanArg;
using interpreter::checkIntegerArg;
using interpreter::checkStringArg;
using interpreter::makeIntegerValue;
using interpreter::makeStringValue;
using interpreter::expr::Node;

namespace {
    /** Size limit for blobs.
        This is an artifical limit to avoid that errors overload the program too easily. */
    const int32_t BLOB_LIMIT = 65536;

    /** Open modes.
        We use those internally instead of afl::io::FileSystem::OpenMode
        because the numerical are an implementation detail to the afl::io module,
        but the values used here should remain portable across saved VMs. */
    const int32_t OpenForReading = 0;
    const int32_t OpenForWriting = 1;
    const int32_t OpenForRandom  = 2;
    const int32_t OpenForAppend  = 3;
    const int32_t OpenModeMax = OpenForAppend;


    /* @q Get #f:File, var:Blob, length:Int (Global Command)
       Read binary data.
       This command will read %length bytes from the file, and place them in the data block %var.
       @since PCC2 1.99.12, PCC 1.0.13, PCC2 2.40.1 */
    class SFGet : public interpreter::SpecialCommand {
     public:
        /** Compile "Get" command. Syntax is "Get #1, data, length". This is
            compiled into "data := CC$Get(data, #1, length)". */
        virtual void compileCommand(Tokenizer& line, BytecodeObject& bco, const StatementCompilationContext& scc)
            {
                // ex int/file.cc:SFGet
                // Read arguments
                afl::base::Deleter del;
                std::vector<const Node*> xn;
                line.readNextToken();
                parseCommandArgumentList(line, xn, del);
                checkArgumentCount(xn.size(), 3, 3);

                // Generate code. We must generate a read-modify-write cycle, because if
                // the fd or the length is null, data must remain unchanged.
                xn[1]->compileRead(bco, scc);
                xn[0]->compileValue(bco, scc);
                xn[2]->compileValue(bco, scc);
                bco.addInstruction(Opcode::maPush, Opcode::sNamedShared, bco.addName("CC$GET"));
                bco.addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 3);
                xn[1]->compileWrite(bco, scc);
                bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
            }
    };

    /* @q Input #f:File, var:Str, Optional flag:Bool (Global Command)
       Read line data.
       This command will read one line of text from the file, and place it in the variable %var.
       The %flag specifies what to do at the end of the file:
       - %False (default): generate an error
       - %True: set %var to EMPTY and continue normally
       @since PCC2 1.99.12, PCC 1.0.13, PCC2 2.40.1 */
    class SFInput : public interpreter::SpecialCommand {
     public:
        /** Compile "Input" command. Syntax is "Input #1, output, flag". This is
            compiled into "data := CC$Input(data, #1[, flag])". */
        virtual void compileCommand(Tokenizer& line, BytecodeObject& bco, const StatementCompilationContext& scc)
            {
                // ex int/file.cc:SFInput
                // Read arguments
                afl::base::Deleter del;
                std::vector<const Node*> xn;
                line.readNextToken();
                parseCommandArgumentList(line, xn, del);
                checkArgumentCount(xn.size(), 2, 3);

                // Generate code. We must generate a read-modify-write cycle, because if
                // the fd or the length is null, data must remain unchanged.
                xn[1]->compileRead(bco, scc);
                xn[0]->compileValue(bco, scc);
                if (xn.size() > 2) {
                    xn[2]->compileValue(bco, scc);
                }
                bco.addInstruction(Opcode::maPush, Opcode::sNamedShared, bco.addName("CC$INPUT"));
                bco.addInstruction(Opcode::maIndirect, Opcode::miIMLoad, static_cast<uint16_t>(xn.size()));
                xn[1]->compileWrite(bco, scc);
                bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
            }
    };

    /* @q Open name:Str For Input|Output|Random|Append As #file:File] (Global Command)
       Open a file.
       The %mode specifies what you intend to do with the file:
       - %Input: open existing file for reading
       - %Output: create new file for writing, overwrite existing file
       - %Random: open existing file for reading and writing
       - %Append: append to existing file or create new one

       Operations with this file will use <a href="int:index:type:file">file number</a> %file.
       If that number already referred to an existing file before, that one will be closed first.
       @since PCC2 1.99.12, PCC 1.0.13, PCC2 2.40.1 */
    class SFOpen : public interpreter::SpecialCommand {
     public:
        /** Compile "Open" command. Syntax is
            "Open FILENAME (For (Input|Output|Random|Append)|As #fd)*".
            This is compiled into "CC$Open FD, FILENAME, MODE". */
        virtual void compileCommand(Tokenizer& tok, BytecodeObject& bco, const StatementCompilationContext& scc)
            {
                // ex int/file.cc:SFOpen
                // ex fileint.pas:File_Open (part)
                // Read file name argument
                tok.readNextToken();
                afl::base::Deleter del;
                const Node& fileName(interpreter::expr::Parser(tok, del).parse());

                // Read keyword arguments
                const Node* fd = 0;
                int mode = -1;
                while (tok.getCurrentToken() != tok.tEnd) {
                    if (tok.checkAdvance("FOR")) {
                        if (mode != -1) {
                            throw interpreter::Error("Duplicate mode for 'Open'");
                        } else if (tok.checkAdvance("INPUT")) {
                            mode = OpenForReading;
                        } else if (tok.checkAdvance("OUTPUT")) {
                            mode = OpenForWriting;
                        } else if (tok.checkAdvance("RANDOM")) {
                            mode = OpenForRandom;
                        } else if (tok.checkAdvance("APPEND")) {
                            mode = OpenForAppend;
                        } else {
                            throw interpreter::Error("Invalid mode for 'Open'");
                        }
                    } else if (tok.checkAdvance("AS")) {
                        if (fd != 0) {
                            throw interpreter::Error("Duplicate file number for 'Open'");
                        }
                        fd = &interpreter::expr::Parser(tok, del).parse();
                    } else {
                        throw interpreter::Error("Syntax error");
                    }
                }

                // Do we have everything?
                if (mode == -1) {
                    throw interpreter::Error("Missing mode for 'Open'");
                }
                if (fd == 0) {
                    throw interpreter::Error("Missing file number for 'Open'");
                }

                // Generate code
                fd->compileValue(bco, scc);
                fileName.compileValue(bco, scc);
                bco.addInstruction(Opcode::maPush, Opcode::sInteger, static_cast<uint16_t>(mode));
                bco.addInstruction(Opcode::maPush, Opcode::sNamedShared, bco.addName("CC$OPEN"));
                bco.addInstruction(Opcode::maIndirect, Opcode::miIMCall, 3);
            }
    };

    /* @q SetByte v:Blob, pos:Int, value:Int... (Global Command)
       Store bytes into blob.
       Packs the %value arguments into the blob %v, one byte per element,
       starting at position %pos.
       The first position in the blob has index 0.
       @since PCC2 1.99.12, PCC 1.0.13, PCC2 2.40.1 */

    /* @q SetWord v:Blob, pos:Int, value:Int... (Global Command)
       Store words into blob.
       Packs the %value arguments into the blob %v, two bytes (16 bits) per element,
       starting at position %pos.
       The first position in the blob has index 0.
       @since PCC2 1.99.12, PCC 1.0.13, PCC2 2.40.1 */

    /* @q SetLong v:Blob, pos:Int, value:Int... (Global Command)
       Store longs into blob.
       Packs the %value arguments into the blob %v, four bytes (32 bits) per element,
       starting at position %pos.
       The first position in the blob has index 0.
       @since PCC2 1.99.12, PCC 1.0.13, PCC2 2.40.1 */
    class SFSetInt : public interpreter::SpecialCommand {
     public:
        SFSetInt(uint16_t size)
            : m_size(size)
            { }

        /** Compile a "SetXxx" command. The command is compiled into a
            read-modify-write cycle, using the CC$SETINT (IFCCSetInt)
            function. Parameters given by the user are the blob variable,
            an index, and the values.
            \param tok [in] Tokenizer to read user parameters from
            \param bco [in/out] Byte-code object to produce output on
            \param scc [in] Statement compilation context
            \param size [in] Number of bytes to pack each parameter in */
        virtual void compileCommand(Tokenizer& tok, BytecodeObject& bco, const StatementCompilationContext& scc)
            {
                // ex int/file.cc:compileSet
                // ex fileint.pas:setxxx_internal (part)

                // Read arguments
                afl::base::Deleter del;
                std::vector<const Node*> xn;
                tok.readNextToken();
                parseCommandArgumentList(tok, xn, del);
                checkArgumentCount(xn.size(), 3, 0xFFFE);

                // Read cycle for first arg
                xn[0]->compileRead(bco, scc);

                // Push size
                bco.addInstruction(Opcode::maPush, Opcode::sInteger, m_size);

                // Compile all other args
                for (size_t i = 1; i < xn.size(); ++i) {
                    xn[i]->compileValue(bco, scc);
                }

                // Call routine to do the work
                bco.addInstruction(Opcode::maPush, Opcode::sNamedShared, bco.addName("CC$SETINT"));
                bco.addInstruction(Opcode::maIndirect, Opcode::miIMLoad, static_cast<uint16_t>(xn.size() + 1));

                // Write cycle for first arg
                xn[0]->compileWrite(bco, scc);
                bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
            }
     private:
        const uint16_t m_size;
    };

    /* @q SetStr v:Blob, pos:Int, length:Int, str:Int (Global Command)
       Store string into blob.
       The string is converted to the game character set, padded with spaces or truncated to match the %length,
       and then stored into the blob %v starting at position %pos.
       The first position in the blob has index 0.
       @since PCC2 1.99.12, PCC 1.0.13, PCC2 2.40.1 */
    class SFSetStr : public interpreter::SpecialCommand {
     public:
        /** Compile a "SetStr" command. This command packs a string into a
            space-padded, game-charset string. It is compiled into a
            read-modify-write cycle using the CC$SETSTR (IFCCSetStr) function.
            Arguments given by the user are
            - data block (blob)
            - index
            - length of field
            - value */
        virtual void compileCommand(Tokenizer& tok, BytecodeObject& bco, const StatementCompilationContext& scc)
            {
                // ex int/file.cc:SFSetStr
                // ex fileint.pas:File_SetStr (part)
                // Read arguments
                afl::base::Deleter del;
                std::vector<const Node*> xn;
                tok.readNextToken();
                parseCommandArgumentList(tok, xn, del);
                checkArgumentCount(xn.size(), 4, 4);

                // Read cycle for first arg
                xn[0]->compileRead(bco, scc);

                // Compile all other args
                for (size_t i = 1; i < xn.size(); ++i) {
                    xn[i]->compileValue(bco, scc);
                }

                // Call routine to do the work
                bco.addInstruction(Opcode::maPush, Opcode::sNamedShared, bco.addName("CC$SETSTR"));
                bco.addInstruction(Opcode::maIndirect, Opcode::miIMLoad, static_cast<uint16_t>(xn.size())); // always 4, see checkArgumentCount

                // Write cycle for first arg
                xn[0]->compileWrite(bco, scc);
                bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
            }
    };


    /******************************* Utilities *******************************/

    /** Prepare blob value.
        \param newBV [out] Newly-allocated, empty blob value
        \param v     [in] Old value
        \param bytes [in] Minimum number of bytes the new blob must have */
    void prepareBlob(interpreter::BlobValue& newBV, afl::data::Value* v, size_t bytes)
    {
        interpreter::BlobValue::Data_t& newValue = newBV.data();
        if (interpreter::BlobValue* oldBV = dynamic_cast<interpreter::BlobValue*>(v)) {
            newValue.reset();
            newValue.append(oldBV->data());
        }
        if (newValue.size() < bytes) {
            newValue.resize(bytes);
        }
    }

    /** Check Blob-type argument.
        \param out [out] Blob returned here
        \param p [in] Argument received from user
        \return true iff valid blob found, false if user passed null
        \throw Error on type error */
    bool checkBlobArg(interpreter::BlobValue*& out, afl::data::Value* p)
    {
        if (!p) {
            return false;
        } else if (interpreter::BlobValue* bv = dynamic_cast<interpreter::BlobValue*>(p)) {
            out = bv;
            return true;
        } else {
            throw interpreter::Error::typeError(interpreter::Error::ExpectBlob);
        }
    }

    /** Execute GetByte/Word/Long function.
        \param args User-supplied args: blob, index
        \param size Number of bytes per integer, 1, 2 or 4.
        \return Return value for user, newly-allocated afl::data::Value object */
    afl::data::Value* extractInt(Arguments& args, size_t size)
    {
        // ex fileint.pas:op_GETxxx
        interpreter::BlobValue* bv;
        int32_t index;

        // Read args
        args.checkArgumentCount(2);
        if (!checkBlobArg(bv, args.getNext()) || !checkIntegerArg(index, args.getNext(), 0, BLOB_LIMIT)) {
            return 0;
        }

        // Do it
        uint32_t result = 0;
        for (size_t i = 0; i < size; ++i) {
            if (uint8_t* p = bv->data().at(index)) {
                result += uint32_t(*p) << (8*i);
            }
            ++index;
        }

        // Byte results are unsigned. For signed long results, conversion
        // uint32_t->int32_t does the right thing. We just have to fix up
        // signed word results.
        if (size == 2) {
            return makeIntegerValue(int16_t(result));
        } else {
            return makeIntegerValue(result);
        }
    }

    /** Convert file size into script-side value.
        The value is returned as integer or float if it fits, otherwise an error is generated. */
    afl::data::Value* makeFileSizeValue(afl::io::Stream::FileSize_t n)
    {
        if (n <= 0x7FFFFFFF) {
            return interpreter::makeIntegerValue(static_cast<int32_t>(n));
        } else if (n <= 0x20000000000000ULL) {
            return interpreter::makeFloatValue(static_cast<double>(n));
        } else {
            throw interpreter::Error::rangeError();
        }
    }


    /******************************* Functions *******************************/



    /* @q CC$Get(old:Blob, fd:File, length:Int):Blob (Internal)
       Backend to {Get}. */
    afl::data::Value* IFCCGet(World& world, Arguments& args)
    {
        // Read args
        TextFile* tf;
        int32_t size;
        args.checkArgumentCount(3);
        afl::data::Value* oldValue = args.getNext();
        if (!world.fileTable().checkFileArg(tf, args.getNext()) || !checkIntegerArg(size, args.getNext(), 0, BLOB_LIMIT)) {
            return afl::data::Value::cloneOf(oldValue);
        }

        // Do it
        std::auto_ptr<interpreter::BlobValue> bv(new interpreter::BlobValue());
        bv->data().resize(size);
        if (size != 0) {
            int32_t got = static_cast<int32_t>(tf->read(bv->data()));
            if (got != size) {
                throw interpreter::Error("Premature end of file");
            }
        }
        return bv.release();
    }

    /* @q CC$Input(old:Str, fd:File, Optional flag:Bool):Str (Internal)
       Backend to {Input}. */
    afl::data::Value* IFCCInput(World& world, Arguments& args)
    {
        // Read args
        TextFile* tf;
        bool flag = false;
        args.checkArgumentCount(2, 3);
        afl::data::Value* oldValue = args.getNext();
        if (!world.fileTable().checkFileArg(tf, args.getNext())) {
            return afl::data::Value::cloneOf(oldValue);
        }
        checkBooleanArg(flag, args.getNext());

        // Do it
        String_t line;
        if (tf->readLine(line)) {
            return makeStringValue(line);
        } else if (flag) {
            return 0;
        } else {
            throw interpreter::Error("Premature end of file");
        }
    }

    /* @q CC$Open(fd:File, name:Str, mode:Int):Any (Internal)
       Backend to {Open}. */

    /** Execute "Open" command. Parameters are
        - file number
        - file name
        - mode

        Open takes named arguments, which are currently encoded into the
        mode. Future expansions to the Open command will add bits to mode
        ("feature X present"), and additional arguments ("this is the
        value of feature X"). */
    afl::data::Value* IFCCOpen(World& world, Arguments& args)
    {
        // ex int/file.cc:IFCCOpen
        // ex fileint.pas:File_Open (part)
        // Read args
        size_t fd;
        String_t filename;
        int32_t mode;

        FileTable& table = world.fileTable();

        args.checkArgumentCount(3);
        if (!table.checkFileArg(fd, args.getNext(), false)
            || !checkStringArg(filename, args.getNext())
            || !checkIntegerArg(mode, args.getNext(), 0, OpenModeMax))
        {
            return 0;
        }

        // Do it
        if (mode == OpenForAppend) {
            // Append is special
            try {
                // Try to open existing file
                table.openFile(fd, world.fileSystem().openFile(filename, afl::io::FileSystem::OpenWrite));
                table.prepareForAppend(fd);
            }
            catch (afl::except::FileProblemException&) {
                // File does not exist. Try creating it.
                table.openFile(fd, world.fileSystem().openFile(filename, afl::io::FileSystem::Create));
            }
        } else if (mode == OpenForWriting) {
            // Write only
            table.openFile(fd, world.fileSystem().openFile(filename, afl::io::FileSystem::Create));
        } else if (mode == OpenForRandom) {
            // Read+Write
            table.openFile(fd, world.fileSystem().openFile(filename, afl::io::FileSystem::OpenWrite));
        } else {
            // Read only
            table.openFile(fd, world.fileSystem().openFile(filename, afl::io::FileSystem::OpenRead));
        }
        return 0;
    }

    /* @q CC$Print(fd:File, text:Str):Any (Internal)
       Backend to {Print} to a file. */
    afl::data::Value* IFCCPrint(World& world, Arguments& args)
    {
        TextFile* tf;
        String_t text;
        args.checkArgumentCount(2);
        if (world.fileTable().checkFileArg(tf, args.getNext()) && checkStringArg(text, args.getNext())) {
            tf->writeLine(text);
        }
        return 0;
    }


    /* @q CC$SetInt(v:Blob, size:Int, pos:Int, value:Int...):Blob (Internal)
       Backend to {SetByte}, {SetWord}, {SetLong}. */
    afl::data::Value* IFCCSetInt(World& /*world*/, Arguments& args)
    {
        // ex fileint.pas:setxxx_internal (part)

        /* Implementation notes: PCC 1.x does not modify the output value
           if one of the parameters is null. This means we have to return
           a copy of our first argument for the same effect.

           In addition, PCC 1.x blanks the blob (a string, actually) if it
           has the wrong type. So let's do the same. It's quite convenient
           to implement anyway, and keeps code which resets the blob using
           things like "blob := ''" working. */
        int32_t size, index;

        // At least four args:
        // - blob
        // - size
        // - index
        // - one or more data elements
        args.checkArgumentCountAtLeast(4);

        afl::data::Value* first = args.getNext();
        if (!checkIntegerArg(size, args.getNext(), 1, 4) || !checkIntegerArg(index, args.getNext(), 0, BLOB_LIMIT)) {
            // size or index are null, ignore this command
            return afl::data::Value::cloneOf(first);
        }

        // prepare the blob
        size_t bytesNeeded = args.getNumArgs() * size;
        std::auto_ptr<interpreter::BlobValue> blob(new interpreter::BlobValue());
        prepareBlob(*blob, first, index + bytesNeeded);

        // execute
        while (args.getNumArgs() != 0) {
            // read argument
            // FIXME: PCC1 checks ranges (0..255 for SetByte, -32768..+32767 for SetWord)
            int32_t theValue;
            if (!checkIntegerArg(theValue, args.getNext())) {
                return afl::data::Value::cloneOf(first);
            }
            // encode it
            for (int32_t i = 0; i < size; ++i) {
                if (uint8_t* p = blob->data().at(index)) {
                    *p = static_cast<uint8_t>(theValue & 255);
                }
                ++index;
                theValue >>= 8;
            }
        }

        return blob.release();
    }

    /* @q CC$SetStr(v:Blob, pos:Int, size:Int, value:Str):Blob (Internal)
       Backend to {SetStr}. */
    afl::data::Value* IFCCSetStr(World& world, Arguments& args)
    {
        // ex fileint.pas:File_SetStr (part)
        int32_t index, size;
        String_t value;

        // Four args:
        // - blob
        // - index
        // - size
        // - string
        args.checkArgumentCount(4);

        afl::data::Value* first = args.getNext();
        if (!checkIntegerArg(index, args.getNext(), 0, BLOB_LIMIT)
            || !checkIntegerArg(size, args.getNext(), 0, BLOB_LIMIT)
            || !checkStringArg(value, args.getNext()))
        {
            // some parameter is null, ignore this command
            return afl::data::Value::cloneOf(first);
        }

        // Convert to game character set
        afl::base::GrowableBytes_t encodedValue(world.fileTable().getFileCharset().encode(afl::string::toMemory(value)));

        // prepare the blob
        std::auto_ptr<interpreter::BlobValue> blob(new interpreter::BlobValue());
        prepareBlob(*blob, first, index + size);

        // execute
        if (size != 0) {
            afl::bits::packFixedString(blob->data().subrange(index, size), encodedValue);
        }

        return blob.release();
    }

    /* @q FPos(#fd:File):Int (Function)
       Get current position within a file.

       @diff If the file is larger than 2 GByte, the file position can be too large to be expressed as an integer.
       PCC2 2.40.3 or later will return a floating-point value for positions between 2 GiB and 8 PiB (9 PB),
       and fail with a range error for even larger positions.
       Older versions will truncate the value (remainder modulo 4 GiB).

       @see Seek, FSize
       @since PCC2 1.99.12, PCC 1.0.13, PCC2 2.40.1 */
    afl::data::Value* IFFPos(World& world, Arguments& args)
    {
        // ex fileint.pas:op_FPOS
        TextFile* tf;
        args.checkArgumentCount(1);
        if (!world.fileTable().checkFileArg(tf, args.getNext())) {
            return 0;
        }
        return makeFileSizeValue(tf->getPos());
    }

    /* @q FreeFile():Int (Function)
       Get an unused file number.
       If there is no unused file number, fails with an error.
       Note that this function will always return the same value until you {Open} it
       (or {Close} another file).
       It is therefore normally used in the form
       | Dim fd = FreeFile()
       | Open "file" For Input As #fd
       | Dim fd2 = FreeFile()
       | Open "anotherfile" For Input As #fd2
       @since PCC2 1.99.12, PCC 1.0.13, PCC2 2.40.1 */
    afl::data::Value* IFFreeFile(World& world, Arguments& args)
    {
        // ex fileint.pas:op_FREEFILE_func
        args.checkArgumentCount(0);
        size_t result = world.fileTable().getFreeFile();
        if (result == 0) {
            throw interpreter::Error("No free file number");
        }
        return makeIntegerValue(static_cast<int32_t>(result));
    }

    /* @q FSize(#fd:File):Int (Function)
       Get size of the file, in bytes.

       @diff If the file is larger than 2 GByte, the file size cannot be expressed as an integer.
       PCC2 2.40.3 or later will return a floating-point value for files between 2 GiB and 8 PiB (9 PB),
       and fail with a range error for even larger files.
       Older versions will truncate the value (remainder modulo 4 GiB).

       @see FPos
       @since PCC2 1.99.12, PCC 1.0.13, PCC2 2.40.1 */
    afl::data::Value* IFFSize(World& world, Arguments& args)
    {
        // ex fileint.pas:op_FSIZE
        TextFile* tf;
        args.checkArgumentCount(1);
        if (!world.fileTable().checkFileArg(tf, args.getNext())) {
            return 0;
        }
        return makeFileSizeValue(tf->getSize());
    }

    /* @q GetByte(v:Blob, pos:Int):Int (Function)
       Extract byte.
       Returns the byte stored at position %pos in the given data block.
       @since PCC2 1.99.12, PCC 1.0.13, PCC2 2.40.1 */
    afl::data::Value* IFGetByte(World& /*world*/, Arguments& args)
    {
        // ex fileint.pas:op_GETBYTE
        return extractInt(args, 1);
    }

    /* @q GetDirectoryName(n:Str):Str (Function)
       Get directory name.
       The parameter is a full file name, possibly including a directory path.
       This function removes the final file name component and returns just the directories.
       @since PCC2 1.99.12, PCC 1.1.20, PCC2 2.40.1 */
    afl::data::Value* IFGetDirectoryName(World& world, Arguments& args)
    {
        // ex ccexpr.pas:op_GETDIRECTORYNAME_func
        String_t a;
        args.checkArgumentCount(1);
        if (!checkStringArg(a, args.getNext())) {
            return 0;
        } else {
            return makeStringValue(world.fileSystem().getDirectoryName(a));
        }
    }

    /* @q GetFileName(n:Str):Str (Function)
       Get file name.
       The parameter is a full file name, possibly including a directory path.
       This function removes all directory names, and returns just the file name.
       @since PCC2 1.99.12, PCC 1.1.20, PCC2 2.40.1 */
    afl::data::Value* IFGetFileName(World& world, Arguments& args)
    {
        // ex ccexpr.pas:op_GETFILENAME_func
        String_t a;
        args.checkArgumentCount(1);
        if (!checkStringArg(a, args.getNext())) {
            return 0;
        } else {
            return makeStringValue(world.fileSystem().getFileName(a));
        }
    }

    /* @q AppendFileNameExtension(n:Str, ext:Str, Optional force:Bool):Str (Function)
       Append a file name extension.
       The parameter %n is a full file name, possibly including a directory path.
       If it does not already contain an extension, or %force is specified, extension %ext is appended.
       For example,
       |  AppendFileNameExtension("readme", "txt")
       will produce "readme.txt".
       @since PCC2 2.40.9 */
    afl::data::Value* IFAppendFileNameExtension(World& world, Arguments& args)
    {
        String_t pathName, ext;
        args.checkArgumentCount(2, 3);
        if (!checkStringArg(pathName, args.getNext()) || !checkStringArg(ext, args.getNext())) {
            return 0;
        } else {
            bool force = interpreter::getBooleanValue(args.getNext()) > 0;
            return makeStringValue(util::appendFileNameExtension(world.fileSystem(), pathName, ext, force));
        }
    }

    /* @q GetLong(v:Blob, pos:Int):Int (Function)
       Extract long.
       Returns the long (4 bytes, 32 bits) stored at position %pos in the given data block.
       @since PCC2 1.99.12, PCC 1.0.13, PCC2 2.40.1 */
    afl::data::Value* IFGetLong(World& /*world*/, Arguments& args)
    {
        // ex fileint.pas:op_GETWORD
        return extractInt(args, 4);
    }

    /* @q GetStr(v:Blob, pos:Int, length:Int):Str (Function)
       Extract string.
       Returns the string that is stored at position %pos in the data block in a field of size %length.
       The string is converted from the game character set, and trailing space is removed.
       @since PCC2 1.99.12, PCC 1.0.13, PCC2 2.40.1 */
    afl::data::Value* IFGetStr(World& world, Arguments& args)
    {
        // ex fileint.pas:op_GETSTR_func
        interpreter::BlobValue* bv;
        int32_t index, size;

        // Read args
        args.checkArgumentCount(3);
        if (!checkBlobArg(bv, args.getNext()) || !checkIntegerArg(index, args.getNext(), 0, BLOB_LIMIT) || !checkIntegerArg(size, args.getNext(), 0, BLOB_LIMIT)) {
            return 0;
        }

        // Do it
        String_t result = world.fileTable().getFileCharset().decode(afl::bits::unpackFixedString(bv->data().subrange(index, size)));
        return makeStringValue(result);
    }

    /* @q GetWord(v:Blob, pos:Int):Int (Function)
       Extract word.
       Returns the word (2 bytes, 16 bits) stored at position %pos in the given data block.
       @since PCC2 1.99.12, PCC 1.0.13, PCC2 2.40.1 */
    afl::data::Value* IFGetWord(World& /*world*/, Arguments& args)
    {
        // ex fileint.pas:op_GETLONG
        return extractInt(args, 2);
    }

    /* @q MakeFileName(n:Str...):Str (Function)
       Create a file name.
       Parameters are file name fragments, i.e. directories, until the last fragment which is a file name.
       This function builds a file name from these, using operating-system dependant rules.
       For example,
       | MakeFileName("a", "b", "c.txt")
       will return "a\b\c.txt" or "a/b/c.txt", depending on the operating system.
       @since PCC2 1.99.12, PCC 1.1.20, PCC2 2.40.1 */
    afl::data::Value* IFMakeFileName(World& world, Arguments& args)
    {
        // ex ccexpr.pas:op_MAKEFILENAME_func
        String_t a, b;
        args.checkArgumentCountAtLeast(1);
        if (!checkStringArg(a, args.getNext())) {
            return 0;
        }

        while (args.getNumArgs() != 0) {
            if (!checkStringArg(b, args.getNext())) {
                return 0;
            }
            a = world.fileSystem().makePathName(a, b);
        }
        return makeStringValue(a);
    }


    /******************************* Procedures ******************************/

    /* @q Close #fd:File (Global Command)
       Close a file.
       If some data is still in the write buffer, it will be written to disk now.
       The file number will become available for re-use.
       @since PCC2 1.99.12, PCC 1.0.13, PCC2 2.40.1 */
    void IFClose(World& world, interpreter::Process& /*proc*/, Arguments& args)
    {
        size_t fd;
        args.checkArgumentCount(1);
        if (world.fileTable().checkFileArg(fd, args.getNext(), true)) {
            world.fileTable().closeFile(fd);
        }
    }

    /* @q Put #fd:File, v:Blob, Optional length:Int (Global Command)
       Write binary data.
       Writes the data block %v into the file at the current position.
       If the %length is specified (recommended), it determines the number of bytes to write.
       If the %length is not specified, PCC writes as many bytes as the block contains.

       @since PCC2 1.99.12, PCC 1.0.13, PCC2 2.40.1 */
    void IFPut(World& world, interpreter::Process& /*proc*/, Arguments& args)
    {
        // Read args
        TextFile* tf;
        interpreter::BlobValue* bv;
        int32_t size;
        args.checkArgumentCount(2, 3);
        if (!world.fileTable().checkFileArg(tf, args.getNext()) || !checkBlobArg(bv, args.getNext())) {
            return;
        }
        if (!checkIntegerArg(size, args.getNext(), 0, BLOB_LIMIT)) {
            size = static_cast<int32_t>(bv->data().size());
        }

        // Write the blob
        size_t todo = size;
        afl::base::ConstBytes_t bytes(bv->data().subrange(0, size));
        tf->fullWrite(bytes);
        todo -= bytes.size();

        // Write some nulls if blob was too short
        while (todo > 0) {
            static const uint8_t zeroes[256] = {};
            bytes = zeroes;
            bytes.trim(todo);
            tf->fullWrite(bytes);
            todo -= bytes.size();
        }
    }

    /* @q Seek #fd:File, pos:Int (Global Command)
       Go to position in file.
       @see FPos(), FSize()
       @since PCC2 1.99.12, PCC 1.0.13, PCC2 2.40.1 */
    void IFSeek(World& world, interpreter::Process& /*proc*/, Arguments& args)
    {
        TextFile* tf;
        int32_t pos;
        args.checkArgumentCount(2);
        if (world.fileTable().checkFileArg(tf, args.getNext()) && checkIntegerArg(pos, args.getNext(), 0, 0x7FFFFFFF)) {
            tf->setPos(pos);
        }
    }
}

void
interpreter::registerFileFunctions(World& world)
{
    // ex int/file.h:initInterpreterFileInterface
    world.addNewSpecialCommand("GET",   new SFGet());
    world.addNewSpecialCommand("INPUT", new SFInput());
    world.addNewSpecialCommand("OPEN",  new SFOpen());
    world.addNewSpecialCommand("SETBYTE", new SFSetInt(1));
    world.addNewSpecialCommand("SETLONG", new SFSetInt(4));
    world.addNewSpecialCommand("SETSTR",  new SFSetStr());
    world.addNewSpecialCommand("SETWORD", new SFSetInt(2));

    typedef SimpleFunction<World&> FunctionValue_t;

    world.setNewGlobalValue("APPENDFILENAMEEXTENSION", new FunctionValue_t(world, IFAppendFileNameExtension));
    world.setNewGlobalValue("CC$GET",                  new FunctionValue_t(world, IFCCGet));
    world.setNewGlobalValue("CC$INPUT",                new FunctionValue_t(world, IFCCInput));
    world.setNewGlobalValue("CC$OPEN",                 new FunctionValue_t(world, IFCCOpen));
    world.setNewGlobalValue("CC$PRINT",                new FunctionValue_t(world, IFCCPrint));
    world.setNewGlobalValue("CC$SETINT",               new FunctionValue_t(world, IFCCSetInt));
    world.setNewGlobalValue("CC$SETSTR",               new FunctionValue_t(world, IFCCSetStr));
    world.setNewGlobalValue("FPOS",                    new FunctionValue_t(world, IFFPos));
    world.setNewGlobalValue("FREEFILE",                new FunctionValue_t(world, IFFreeFile));
    world.setNewGlobalValue("FSIZE",                   new FunctionValue_t(world, IFFSize));
    world.setNewGlobalValue("GETBYTE",                 new FunctionValue_t(world, IFGetByte));
    world.setNewGlobalValue("GETDIRECTORYNAME",        new FunctionValue_t(world, IFGetDirectoryName));
    world.setNewGlobalValue("GETFILENAME",             new FunctionValue_t(world, IFGetFileName));
    world.setNewGlobalValue("GETLONG",                 new FunctionValue_t(world, IFGetLong));
    world.setNewGlobalValue("GETSTR",                  new FunctionValue_t(world, IFGetStr));
    world.setNewGlobalValue("GETWORD",                 new FunctionValue_t(world, IFGetWord));
    world.setNewGlobalValue("MAKEFILENAME",            new FunctionValue_t(world, IFMakeFileName));

    world.setNewGlobalValue("CLOSE", new SimpleProcedure<World&>(world, IFClose));
    world.setNewGlobalValue("PUT",   new SimpleProcedure<World&>(world, IFPut));
    world.setNewGlobalValue("SEEK",  new SimpleProcedure<World&>(world, IFSeek));
}
