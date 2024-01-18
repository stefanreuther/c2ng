/**
  *  \file test/interpreter/filetabletest.cpp
  *  \brief Test for interpreter::FileTable
  */

#include "interpreter/filetable.hpp"

#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/multiplexablestream.hpp"
#include "afl/io/nullstream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/loglistener.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/error.hpp"
#include "interpreter/filevalue.hpp"

namespace {
    class FailStream : public afl::io::MultiplexableStream {
     public:
        virtual size_t read(Bytes_t /*m*/)
            { return 0; }
        virtual size_t write(ConstBytes_t /*m*/)
            { throw afl::except::FileProblemException(*this, "no write"); }
        virtual void flush()
            { throw afl::except::FileProblemException(*this, "no flush"); }
        virtual void setPos(FileSize_t /*pos*/)
            { }
        virtual FileSize_t getPos()
            { return 0; }
        virtual FileSize_t getSize()
            { return 0; }
        virtual uint32_t getCapabilities()
            { return CanRead | CanWrite; }
        virtual String_t getName()
            { return "FailStream"; }
        virtual afl::base::Ptr<afl::io::FileMapping> createFileMapping(FileSize_t /*limit*/)
            { return 0; }
    };
}

/** Simple test. */
AFL_TEST("interpreter.FileTable:basics", a)
{
    interpreter::FileTable testee;

    // Initial state is no available file descriptors
    a.checkEqual("01. getFreeFile", testee.getFreeFile(), 0U);
    AFL_CHECK_THROWS(a("02. openFile"), testee.openFile(0, *new afl::io::NullStream()), interpreter::Error);

    // Make some room
    testee.setMaxFiles(6);
    a.checkEqual("11. getFreeFile", testee.getFreeFile(), 1U);
    a.checkEqual("12. getFreeFile", testee.getFreeFile(), 1U);

    // Open files
    testee.openFile(0, *new afl::io::NullStream());
    testee.openFile(1, *new afl::io::NullStream());
    testee.openFile(5, *new afl::io::NullStream());
    AFL_CHECK_THROWS(a("21. openFile"), testee.openFile(6, *new afl::io::NullStream()), interpreter::Error);
    a.checkEqual("22. getFreeFile", testee.getFreeFile(), 2U);

    // Check file arguments to file descriptors
    afl::data::IntegerValue one(1);     interpreter::FileValue fone(1);
    afl::data::IntegerValue four(4);    interpreter::FileValue ffour(4);
    afl::data::IntegerValue six(6);     interpreter::FileValue fsix(6);
    afl::data::IntegerValue neg(-1);    interpreter::FileValue fneg(-1);
    afl::data::StringValue str("str");

    size_t fd;
    a.check("31. checkFileArg", !testee.checkFileArg(fd, 0,      false));
    a.check("32. checkFileArg",  testee.checkFileArg(fd, &one,   false)); a.checkEqual("32. fd value", fd, 1U);
    a.check("33. checkFileArg",  testee.checkFileArg(fd, &fone,  false)); a.checkEqual("33. fd value", fd, 1U);
    a.check("34. checkFileArg",  testee.checkFileArg(fd, &four,  false)); a.checkEqual("34. fd value", fd, 4U);
    a.check("35. checkFileArg",  testee.checkFileArg(fd, &ffour, false)); a.checkEqual("35. fd value", fd, 4U);
    AFL_CHECK_THROWS(a("36. fd value"), testee.checkFileArg(fd, &six,  false), interpreter::Error);
    AFL_CHECK_THROWS(a("37. fd value"), testee.checkFileArg(fd, &fsix, false), interpreter::Error);
    AFL_CHECK_THROWS(a("38. fd value"), testee.checkFileArg(fd, &neg,  false), interpreter::Error);
    AFL_CHECK_THROWS(a("39. fd value"), testee.checkFileArg(fd, &fneg, false), interpreter::Error);
    AFL_CHECK_THROWS(a("40. fd value"), testee.checkFileArg(fd, &str,  false), interpreter::Error);

    a.check("41. checkFileArg", !testee.checkFileArg(fd, 0,      true));
    a.check("42. checkFileArg",  testee.checkFileArg(fd, &one,   true)); a.checkEqual("42. fd value", fd, 1U);
    a.check("43. checkFileArg",  testee.checkFileArg(fd, &fone,  true)); a.checkEqual("43. fd value", fd, 1U);
    AFL_CHECK_THROWS(a("44. checkFileArg"), testee.checkFileArg(fd, &four,  true), interpreter::Error);
    AFL_CHECK_THROWS(a("45. checkFileArg"), testee.checkFileArg(fd, &ffour, true), interpreter::Error);
    AFL_CHECK_THROWS(a("46. checkFileArg"), testee.checkFileArg(fd, &six,   true), interpreter::Error);
    AFL_CHECK_THROWS(a("47. checkFileArg"), testee.checkFileArg(fd, &fsix,  true), interpreter::Error);
    AFL_CHECK_THROWS(a("48. checkFileArg"), testee.checkFileArg(fd, &neg,   true), interpreter::Error);
    AFL_CHECK_THROWS(a("49. checkFileArg"), testee.checkFileArg(fd, &fneg,  true), interpreter::Error);
    AFL_CHECK_THROWS(a("50. checkFileArg"), testee.checkFileArg(fd, &str,   true), interpreter::Error);

    // Check file arguments to text files
    afl::io::TextFile* tf;
    a.check("51. checkFileArg", !testee.checkFileArg(tf, 0));    a.checkNull   ("51. fd value", tf);
    a.check("52. checkFileArg",  testee.checkFileArg(tf, &one)); a.checkNonNull("52. fd value", tf);
    AFL_CHECK_THROWS(a("53. checkFileArg"), testee.checkFileArg(tf, &ffour), interpreter::Error);
    AFL_CHECK_THROWS(a("54. checkFileArg"), testee.checkFileArg(tf, &six),   interpreter::Error);
    AFL_CHECK_THROWS(a("55. checkFileArg"), testee.checkFileArg(tf, &fneg),  interpreter::Error);
    AFL_CHECK_THROWS(a("56. checkFileArg"), testee.checkFileArg(tf, &str),   interpreter::Error);

    // Close
    testee.closeFile(1);

    // Close non-open files
    AFL_CHECK_SUCCEEDS(a("61. closeFile"), testee.closeFile(0));
    AFL_CHECK_SUCCEEDS(a("62. closeFile"), testee.closeFile(2));
    AFL_CHECK_SUCCEEDS(a("63. closeFile"), testee.closeFile(size_t(-1)));
    AFL_CHECK_SUCCEEDS(a("64. closeFile"), testee.closeFile(6));
}

/** Test prepareForAppend(). */
AFL_TEST("interpreter.FileTable:prepareForAppend", a)
{
    // Prepare a UTF-8 file
    afl::base::Ref<afl::io::InternalStream> u8file(*new afl::io::InternalStream());
    u8file->fullWrite(afl::string::toBytes("\xEF\xBB\xBFu8file\n"));
    u8file->setPos(0);

    // Prepare a Latin-1 file
    afl::base::Ref<afl::io::InternalStream> l1file(*new afl::io::InternalStream());
    l1file->fullWrite(afl::string::toBytes("l1file\n"));
    l1file->setPos(0);

    // Prepare a file table
    interpreter::FileTable testee;
    testee.setMaxFiles(10);
    testee.openFile(1, u8file);
    testee.openFile(2, l1file);

    // Prepare
    testee.prepareForAppend(1);
    testee.prepareForAppend(2);

    // Write
    afl::io::TextFile* tf = 0;
    {
        afl::data::IntegerValue one(1);
        a.check("01. checkFileArg", testee.checkFileArg(tf, &one));
        tf->setSystemNewline(false);
        tf->writeLine("t\xc3\xa4xt");
    }
    {
        afl::data::IntegerValue two(2);
        a.check("02. checkFileArg", testee.checkFileArg(tf, &two));
        tf->setSystemNewline(false);
        tf->writeLine("t\xc3\xa4xt");
    }

    // Close
    testee.closeFile(1);
    testee.closeFile(2);

    // Verify
    a.checkEqual("11. getSize", u8file->getSize(), 16U);
    a.checkEqual("12. getSize", l1file->getSize(), 12U);
    a.checkEqualContent<uint8_t>("21. u8file", u8file->getContent(), afl::string::toBytes("\xEF\xBB\xBFu8file\nt\xc3\xa4xt\n"));
    a.checkEqualContent<uint8_t>("22. l1file", l1file->getContent(), afl::string::toBytes("l1file\nt\xe4xt\n"));
}

/** Test closing file when an error occurs.
    A: Open a stream that fails on flush/write. Write something into it. Close file.
    E: closeFile() must throw, but file must be closed afterwards. */
AFL_TEST("interpreter.FileTable:closeFile:error", a)
{
    const int FILE_NR = 1;

    // Open a file
    interpreter::FileTable testee;
    testee.setMaxFiles(6);
    testee.openFile(FILE_NR, *new FailStream());

    // Write
    afl::io::TextFile* tf = testee.getFile(FILE_NR);
    a.checkNonNull("01. getFile", tf);
    tf->writeLine("hi there");

    // Close
    AFL_CHECK_THROWS(a("11. closeFile"), testee.closeFile(FILE_NR), afl::except::FileProblemException);
    a.checkNull("12. getFile", testee.getFile(FILE_NR));
}

/** Test closeAllFiles(), success case.
    A: open some files. Call closeAllFiles().
    E: files closed, no log messages generated. */
AFL_TEST("interpreter.FileTable:closeAllFiles:success", a)
{
    // Prepare
    interpreter::FileTable testee;
    testee.setMaxFiles(6);
    testee.openFile(1, *new afl::io::NullStream());
    testee.openFile(2, *new afl::io::NullStream());
    testee.openFile(3, *new afl::io::NullStream());

    // Test
    afl::test::LogListener log;
    afl::string::NullTranslator tx;
    testee.closeAllFiles(log, tx);

    // Verify
    a.checkEqual("01. getNumMessages", log.getNumMessages(), 0U);
    a.checkNull("02. getFile", testee.getFile(1));
    a.checkNull("03. getFile", testee.getFile(2));
    a.checkNull("04. getFile", testee.getFile(3));
}

/** Test closeAll, error case.
    A: open some files. Call closeAllFiles().
    E: files closed, some log messages generated. */
AFL_TEST("interpreter.FileTable:closeAllFiles:error", a)
{
    // Prepare
    interpreter::FileTable testee;
    testee.setMaxFiles(6);
    testee.openFile(1, *new afl::io::NullStream());
    testee.openFile(2, *new FailStream());
    testee.openFile(3, *new afl::io::NullStream());
    testee.getFile(2)->writeLine("hi");

    // Test
    afl::test::LogListener log;
    afl::string::NullTranslator tx;
    testee.closeAllFiles(log, tx);

    // Verify
    a.checkGreaterEqual("01. getNumMessages", log.getNumMessages(), 1U);
    a.checkNull("02. getFile", testee.getFile(1));
    a.checkNull("03. getFile", testee.getFile(2));
    a.checkNull("04. getFile", testee.getFile(3));
}
