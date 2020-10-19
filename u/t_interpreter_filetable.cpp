/**
  *  \file u/t_interpreter_filetable.cpp
  *  \brief Test for interpreter::FileTable
  */

#include "interpreter/filetable.hpp"

#include "t_interpreter.hpp"
#include "afl/io/nullstream.hpp"
#include "afl/data/integervalue.hpp"
#include "interpreter/filevalue.hpp"
#include "interpreter/error.hpp"
#include "afl/io/internalstream.hpp"

/** Simple test. */
void
TestInterpreterFileTable::testIt()
{
    interpreter::FileTable testee;

    // Initial state is no available file descriptors
    TS_ASSERT_EQUALS(testee.getFreeFile(), 0U);
    TS_ASSERT_THROWS(testee.openFile(0, *new afl::io::NullStream()), interpreter::Error);

    // Make some room
    testee.setMaxFiles(6);
    TS_ASSERT_EQUALS(testee.getFreeFile(), 1U);
    TS_ASSERT_EQUALS(testee.getFreeFile(), 1U);

    // Open files
    testee.openFile(0, *new afl::io::NullStream());
    testee.openFile(1, *new afl::io::NullStream());
    testee.openFile(5, *new afl::io::NullStream());
    TS_ASSERT_THROWS(testee.openFile(6, *new afl::io::NullStream()), interpreter::Error);
    TS_ASSERT_EQUALS(testee.getFreeFile(), 2U);

    // Check file arguments to file descriptors
    afl::data::IntegerValue one(1);     interpreter::FileValue fone(1);
    afl::data::IntegerValue four(4);    interpreter::FileValue ffour(4);
    afl::data::IntegerValue six(6);     interpreter::FileValue fsix(6);
    afl::data::IntegerValue neg(-1);    interpreter::FileValue fneg(-1);

    size_t fd;
    TS_ASSERT(!testee.checkFileArg(fd, 0,      false));
    TS_ASSERT( testee.checkFileArg(fd, &one,   false)); TS_ASSERT_EQUALS(fd, 1U);
    TS_ASSERT( testee.checkFileArg(fd, &fone,  false)); TS_ASSERT_EQUALS(fd, 1U);
    TS_ASSERT( testee.checkFileArg(fd, &four,  false)); TS_ASSERT_EQUALS(fd, 4U);
    TS_ASSERT( testee.checkFileArg(fd, &ffour, false)); TS_ASSERT_EQUALS(fd, 4U);
    TS_ASSERT_THROWS(testee.checkFileArg(fd, &six,  false), interpreter::Error);
    TS_ASSERT_THROWS(testee.checkFileArg(fd, &fsix, false), interpreter::Error);
    TS_ASSERT_THROWS(testee.checkFileArg(fd, &neg,  false), interpreter::Error);
    TS_ASSERT_THROWS(testee.checkFileArg(fd, &fneg, false), interpreter::Error);

    TS_ASSERT(!testee.checkFileArg(fd, 0,      true));
    TS_ASSERT( testee.checkFileArg(fd, &one,   true)); TS_ASSERT_EQUALS(fd, 1U);
    TS_ASSERT( testee.checkFileArg(fd, &fone,  true)); TS_ASSERT_EQUALS(fd, 1U);
    TS_ASSERT_THROWS(testee.checkFileArg(fd, &four,  true), interpreter::Error);
    TS_ASSERT_THROWS(testee.checkFileArg(fd, &ffour, true), interpreter::Error);
    TS_ASSERT_THROWS(testee.checkFileArg(fd, &six,   true), interpreter::Error);
    TS_ASSERT_THROWS(testee.checkFileArg(fd, &fsix,  true), interpreter::Error);
    TS_ASSERT_THROWS(testee.checkFileArg(fd, &neg,   true), interpreter::Error);
    TS_ASSERT_THROWS(testee.checkFileArg(fd, &fneg,  true), interpreter::Error);

    // Check file arguments to text files
    afl::io::TextFile* tf;
    TS_ASSERT(!testee.checkFileArg(tf, 0));    TS_ASSERT(tf == 0);
    TS_ASSERT( testee.checkFileArg(tf, &one)); TS_ASSERT(tf != 0);
    TS_ASSERT_THROWS(!testee.checkFileArg(tf, &ffour), interpreter::Error);
    TS_ASSERT_THROWS(!testee.checkFileArg(tf, &six),   interpreter::Error);
    TS_ASSERT_THROWS(!testee.checkFileArg(tf, &fneg),  interpreter::Error);

    // Close
    testee.closeFile(1);

    // Close non-open files
    TS_ASSERT_THROWS_NOTHING(testee.closeFile(0));
    TS_ASSERT_THROWS_NOTHING(testee.closeFile(2));
    TS_ASSERT_THROWS_NOTHING(testee.closeFile(size_t(-1)));
    TS_ASSERT_THROWS_NOTHING(testee.closeFile(6));
}

/** Test prepareForAppend(). */
void
TestInterpreterFileTable::testAppend()
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
        TS_ASSERT(testee.checkFileArg(tf, &one));
        tf->setSystemNewline(false);
        tf->writeLine("t\xc3\xa4xt");
    }
    {
        afl::data::IntegerValue two(2);
        TS_ASSERT(testee.checkFileArg(tf, &two));
        tf->setSystemNewline(false);
        tf->writeLine("t\xc3\xa4xt");
    }

    // Close
    testee.closeFile(1);
    testee.closeFile(2);

    // Verify
    TS_ASSERT_EQUALS(u8file->getSize(), 16U);
    TS_ASSERT_EQUALS(l1file->getSize(), 12U);
    TS_ASSERT_SAME_DATA(u8file->getContent().unsafeData(),
                        "\xEF\xBB\xBFu8file\n"
                        "t\xc3\xa4xt\n", 16);
    TS_ASSERT_SAME_DATA(l1file->getContent().unsafeData(),
                        "l1file\n"
                        "t\xe4xt\n", 12);
}
