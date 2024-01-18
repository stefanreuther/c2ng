/**
  *  \file test/interpreter/savecontexttest.cpp
  *  \brief Test for interpreter::SaveContext
  */

#include "interpreter/savecontext.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("interpreter.SaveContext")
{
    class Tester : public interpreter::SaveContext {
     public:
        virtual uint32_t addBCO(const interpreter::BytecodeObject& /*bco*/)
            { return 0; }
        virtual uint32_t addHash(const afl::data::Hash& /*hash*/)
            { return 0; }
        virtual uint32_t addArray(const interpreter::ArrayData& /*array*/)
            { return 0; }
        virtual uint32_t addStructureType(const interpreter::StructureTypeData& /*type*/)
            { return 0; }
        virtual uint32_t addStructureValue(const interpreter::StructureValueData& /*value*/)
            { return 0; }
        virtual bool isCurrentProcess(const interpreter::Process* /*p*/)
            { return false; }
    };
    Tester t;
}
