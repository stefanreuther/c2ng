/**
  *  \file u/t_interpreter_savecontext.cpp
  *  \brief Test for interpreter::SaveContext
  */

#include "interpreter/savecontext.hpp"

#include "t_interpreter.hpp"

/** Interface test. */
void
TestInterpreterSaveContext::testInterface()
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

