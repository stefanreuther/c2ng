/**
  *  \file test/interpreter/vmio/loadcontexttest.cpp
  *  \brief Test for interpreter::vmio::LoadContext
  */

#include "interpreter/vmio/loadcontext.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("interpreter.vmio.LoadContext")
{
    class Tester : public interpreter::vmio::LoadContext {
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
    Tester t;
}
