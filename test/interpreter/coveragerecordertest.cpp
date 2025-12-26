/**
  *  \file test/interpreter/coveragerecordertest.cpp
  *  \brief Test for interpreter::CoverageRecorder
  */

#include "interpreter/coveragerecorder.hpp"

#include "afl/io/internalstream.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/process.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "interpreter/world.hpp"

using afl::io::InternalStream;
using interpreter::BCORef_t;
using interpreter::BytecodeObject;
using interpreter::CoverageRecorder;
using interpreter::SubroutineValue;

namespace {
    void runTest(BytecodeObject& bco, CoverageRecorder& testee)
    {
        // Execution environment
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        interpreter::World world(log, tx, fs);
        interpreter::Process proc(world, "proc", 42);
        proc.pushFrame(bco, false);

        // Testee
        testee.addBCO(bco);
        proc.run(&testee);
    }
}

/** Test basics.
    A: create a simple piece of code. Run it with instrumentation.
    E: correct result produced */
AFL_TEST("interpreter.CoverageRecorder:basics", a)
{
    // Create some bytecode
    BCORef_t bco = BytecodeObject::create(true);
    bco->setFileName("file.q");
    bco->addLineNumber(10);
    bco->addPushLiteral(0);
    bco->addPushLiteral(0);
    bco->addLineNumber(12);
    bco->addPushLiteral(0);

    // Run
    CoverageRecorder testee;
    runTest(*bco, testee);

    // Output
    InternalStream out;
    testee.save(out, "theTest");

    a.checkEqual("result",
                 afl::string::fromBytes(out.getContent()),
                 "TN:theTest\n"
                 "SF:file.q\n"
                 "FN:10,anon_1\n"
                 "FNDA:1,anon_1\n"
                 "FNF:1\n"
                 "FNH:1\n"
                 "DA:10,1\n"
                 "DA:12,1\n"
                 "end_of_record\n");
}

/** Test recursive bytecode.
    Note that the compiler cannot create this.
    A: create two BytecodeObjects referring to each other. Run one with instrumentation.
    E: correct result produced; no infinite loop */
AFL_TEST("interpreter.CoverageRecorder:recursive", a)
{
    // Create some bytecode with recursive links
    BCORef_t bco1 = BytecodeObject::create(true);
    BCORef_t bco2 = BytecodeObject::create(true);
    SubroutineValue sub1(bco1);
    SubroutineValue sub2(bco2);

    bco1->setSubroutineName("FIRST");
    bco1->setFileName("file.q");
    bco1->addLineNumber(10);
    bco1->addPushLiteral(&sub2);

    bco2->setSubroutineName("SECOND");
    bco2->setFileName("file.q");
    bco2->addLineNumber(20);
    bco2->addPushLiteral(&sub1);

    // Run
    CoverageRecorder testee;
    runTest(*bco1, testee);

    // Output
    InternalStream out;
    testee.save(out, "theTest");

    a.checkEqual("result",
                 afl::string::fromBytes(out.getContent()),
                 "TN:theTest\n"
                 "SF:file.q\n"
                 "FN:10,FIRST\n"
                 "FNDA:1,FIRST\n"
                 "FN:20,SECOND\n"
                 "FNDA:0,SECOND\n"
                 "FNF:2\n"
                 "FNH:1\n"
                 "DA:10,1\n"
                 "DA:20,0\n"
                 "end_of_record\n");

    // Clean up for valgrind
    bco1->literals().clear();
    bco2->literals().clear();
}

/** Test multiple files.
    A: create multiple BytecodeObjects with different file names. Run one with instrumentation.
    E: correct result produced; no infinite loop */
AFL_TEST("interpreter.CoverageRecorder:two-files", a)
{
    // Create some bytecode with recursive links
    BCORef_t bco1 = BytecodeObject::create(true);
    BCORef_t bco2 = BytecodeObject::create(true);
    SubroutineValue sub2(bco2);

    bco1->setSubroutineName("FIRST");
    bco1->setFileName("file.q");
    bco1->addLineNumber(10);
    bco1->addPushLiteral(&sub2);

    bco2->setSubroutineName("SECOND");
    bco2->setFileName("other.q");
    bco2->addLineNumber(20);
    bco2->addPushLiteral(0);

    // Run
    CoverageRecorder testee;
    runTest(*bco1, testee);

    // Output
    InternalStream out;
    testee.save(out, "theTest");

    a.checkEqual("result",
                 afl::string::fromBytes(out.getContent()),
                 "TN:theTest\n"
                 "SF:file.q\n"
                 "FN:10,FIRST\n"
                 "FNDA:1,FIRST\n"
                 "FNF:1\n"
                 "FNH:1\n"
                 "DA:10,1\n"
                 "end_of_record\n"
                 "TN:theTest\n"
                 "SF:other.q\n"
                 "FN:20,SECOND\n"
                 "FNDA:0,SECOND\n"
                 "FNF:1\n"
                 "FNH:0\n"
                 "DA:20,0\n"
                 "end_of_record\n");
}

/** Test code without source position.
    A: create a simple piece of code. Run it with instrumentation.
    E: correct result produced */
AFL_TEST("interpreter.CoverageRecorder:no-source", a)
{
    // Create some bytecode
    BCORef_t bco = BytecodeObject::create(true);
    bco->addPushLiteral(0);
    bco->addPushLiteral(0);

    // Run
    CoverageRecorder testee;
    runTest(*bco, testee);

    // Output
    InternalStream out;
    testee.save(out, "theTest");

    a.checkEqual("result",
                 afl::string::fromBytes(out.getContent()),
                 "");
}
