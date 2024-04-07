/**
  *  \file test/interpreter/vmio/processloadcontexttest.cpp
  *  \brief Test for interpreter::vmio::ProcessLoadContext
  */

#include "interpreter/vmio/processloadcontext.hpp"

#include <memory>
#include "afl/io/constmemorystream.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/mutexcontext.hpp"
#include "interpreter/process.hpp"
#include "interpreter/singlecontext.hpp"
#include "interpreter/world.hpp"

using interpreter::MutexContext;

/** Test deserialisation for loadMutex().
    There used to be a typo in there. */
AFL_TEST("interpreter.vmio.ProcessLoadContext:loadMutex", a)
{
    // Environment classes
    class TestParent : public interpreter::vmio::LoadContext {
     public:
        virtual afl::data::Value* loadBCO(uint32_t /*id*/)
            { return 0; }
        virtual afl::data::Value* loadArray(uint32_t /*id*/)
            { return 0; }
        virtual afl::data::Value* loadHash(uint32_t /*id*/)
            { return 0; }
        virtual afl::data::Value* loadStructureValue(uint32_t /*id*/)
            { return 0; }
        virtual afl::data::Value* loadStructureType(uint32_t /*id*/)
            { return 0; }
        virtual interpreter::Context* loadContext(const interpreter::TagNode& /*tag*/, afl::io::Stream& /*aux*/)
            { return 0; }
        virtual interpreter::Process* createProcess()
            { return 0; }
        virtual void finishProcess(interpreter::Process& /*proc*/)
            { }
    };

    // Environment
    TestParent parent;
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);
    interpreter::Process proc(world, a.getLocation(), 99);

    // Test
    interpreter::vmio::ProcessLoadContext testee(parent, proc);

    // Load
    static const uint8_t DATA[] = {
        2,0,0,0, 3,0,0,0,
        'h','i', 'm','o','m'
    };

    // - With "not this process" flag
    {
        afl::io::ConstMemoryStream aux(DATA);
        interpreter::TagNode tag;
        tag.tag = tag.Tag_Mutex;
        tag.value = 0;
        std::auto_ptr<interpreter::Context> result(testee.loadContext(tag, aux));
        MutexContext* ctx = dynamic_cast<MutexContext*>(result.get());
        a.check("01. context", ctx != 0);
        a.checkEqual("02. toString", ctx->toString(true), "Lock(\"hi\",\"mom\")");
    }

    // - With "is this process" flag [as of 20220801, no longer different from above]
    {
        afl::io::ConstMemoryStream aux(DATA);
        interpreter::TagNode tag;
        tag.tag = tag.Tag_Mutex;
        tag.value = 1;
        std::auto_ptr<interpreter::Context> result(testee.loadContext(tag, aux));
        MutexContext* ctx = dynamic_cast<MutexContext*>(result.get());
        a.check("01. context", ctx != 0);
        a.checkEqual("02. toString", ctx->toString(true), "Lock(\"hi\",\"mom\")");
    }
}
