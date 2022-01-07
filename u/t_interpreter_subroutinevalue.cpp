/**
  *  \file u/t_interpreter_subroutinevalue.cpp
  *  \brief Test for interpreter::SubroutineValue
  */

#include <memory>
#include "interpreter/subroutinevalue.hpp"

#include "t_interpreter.hpp"
#include "interpreter/error.hpp"
#include "afl/io/internalsink.hpp"
#include "interpreter/savecontext.hpp"

void
TestInterpreterSubroutineValue::testIt()
{
    // Create testee
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
    bco->setSubroutineName("SUB");
    interpreter::SubroutineValue testee(bco);

    // Name mentions the sub name but is otherwise unparseable
    TS_ASSERT_EQUALS(testee.toString(false).substr(0, 2), "#<");
    TS_ASSERT(testee.toString(false).find("SUB") != String_t::npos);

    TS_ASSERT_EQUALS(&*testee.getBytecodeObject(), &*bco);
    TS_ASSERT_EQUALS(testee.getDimension(0), 0);
    TS_ASSERT_THROWS(testee.makeFirstContext(), interpreter::Error);

    // Cloning
    std::auto_ptr<interpreter::SubroutineValue> copy(testee.clone());
    TS_ASSERT(copy.get() != 0);
    TS_ASSERT_EQUALS(&*copy->getBytecodeObject(), &*bco);

    // Store
    {
        class TestSaveContext : public interpreter::SaveContext {
         public:
            virtual uint32_t addBCO(const interpreter::BytecodeObject& /*bco*/)
                { return 12345; }
            virtual uint32_t addHash(const afl::data::Hash& /*hash*/)
                { TS_FAIL("addHash unexpected"); return 0; }
            virtual uint32_t addArray(const interpreter::ArrayData& /*array*/)
                { TS_FAIL("addArray unexpected"); return 0; }
            virtual uint32_t addStructureType(const interpreter::StructureTypeData& /*type*/)
                { TS_FAIL("addStructureType unexpected"); return 0; }
            virtual uint32_t addStructureValue(const interpreter::StructureValueData& /*value*/)
                { TS_FAIL("addStructureValue unexpected"); return 0; }
            virtual bool isCurrentProcess(const interpreter::Process* /*p*/)
                { return false; }
        };

        interpreter::TagNode out;
        afl::io::InternalSink aux;
        TestSaveContext ctx;
        TS_ASSERT_THROWS_NOTHING(testee.store(out, aux, ctx));
        TS_ASSERT_EQUALS(out.tag, interpreter::TagNode::Tag_BCO);
        TS_ASSERT_EQUALS(out.value, 12345U);
    }
}
