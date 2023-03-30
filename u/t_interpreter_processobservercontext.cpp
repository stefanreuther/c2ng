/**
  *  \file u/t_interpreter_processobservercontext.cpp
  *  \brief Test for interpreter::ProcessObserverContext
  */

#include "interpreter/processobservercontext.hpp"

#include "t_interpreter.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/process.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/world.hpp"

namespace {
    class NullPA : public interpreter::PropertyAcceptor {
     public:
        virtual void addProperty(const String_t& /*name*/, interpreter::TypeHint /*th*/)
            { TS_ASSERT(0); }
    };

    int32_t getIntegerValue(interpreter::Process& proc, String_t name)
    {
        std::auto_ptr<afl::data::Value> v = proc.getVariable(name);
        int32_t iv = 0;
        interpreter::checkIntegerArg(iv, v.get());
        return iv;
    }
}

void
TestInterpreterProcessObserverContext::testIt()
{
    // Environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    // Create a byte-code object:
    //    local A = 42
    //    do while true: stop
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
    interpreter::BytecodeObject::Label_t loop = bco->makeLabel();
    uint16_t lv = bco->addLocalVariable("A");
    bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sInteger, 42);
    bco->addInstruction(interpreter::Opcode::maStore, interpreter::Opcode::sLocal, lv);
    bco->addLabel(loop);
    bco->addInstruction(interpreter::Opcode::maSpecial, interpreter::Opcode::miSpecialSuspend, 0);
    bco->addJump(interpreter::Opcode::jAlways, loop);

    // Create a process and run it
    interpreter::Process p1(world, "p1", 999);
    p1.pushFrame(bco, false);
    p1.run();
    TS_ASSERT_EQUALS(p1.getState(), interpreter::Process::Suspended);
    TS_ASSERT_EQUALS(getIntegerValue(p1, "A"), 42);

    // Create ProcessObserverContext and verify its basic properties
    std::auto_ptr<interpreter::ProcessObserverContext> testee(interpreter::ProcessObserverContext::create(p1));
    TS_ASSERT(testee.get() != 0);
    TS_ASSERT(testee->getObject() == 0);
    TS_ASSERT_EQUALS(testee->toString(false).substr(0, 1), "#");

    interpreter::test::ContextVerifier verif(*testee, "testIt");
    verif.verifyBasics();
    verif.verifyNotSerializable();

    {
        NullPA pa;
        testee->enumProperties(pa);
    }

    std::auto_ptr<interpreter::ProcessObserverContext> clone(testee->clone());
    TS_ASSERT(clone.get() != 0);
    TS_ASSERT_EQUALS(clone->toString(false), testee->toString(false));

    // Create a second process to observe the first one through the ProcessObserverContext
    interpreter::Process p2(world, "p2", 888);
    p2.pushNewContext(clone.release());
    TS_ASSERT_EQUALS(getIntegerValue(p2, "A"), 42);

    // Run the first process; this will disconnect the second one
    p1.run();
    TS_ASSERT_EQUALS(p1.getState(), interpreter::Process::Suspended);
    TS_ASSERT_EQUALS(getIntegerValue(p1, "A"), 42);

    TS_ASSERT(p2.getVariable("A").get() == 0);
}

