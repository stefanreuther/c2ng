/**
  *  \file u/t_interpreter_structuretype.cpp
  *  \brief Test for interpreter::StructureType
  */

#include <memory>
#include "interpreter/structuretype.hpp"

#include "t_interpreter.hpp"
#include "afl/io/internalsink.hpp"
#include "interpreter/savecontext.hpp"

/** Simple test. */
void
TestInterpreterStructureType::testIt()
{
    interpreter::StructureTypeData::Ref_t sd(*new interpreter::StructureTypeData());
    interpreter::StructureType testee(sd);

    // Verify
    TS_ASSERT_EQUALS(&*testee.getType(), &*sd);
    TS_ASSERT_EQUALS(testee.toString(false).substr(0, 2), "#<");

    // Clone
    std::auto_ptr<interpreter::StructureType> copy(testee.clone());
    TS_ASSERT(copy.get() != 0);
    TS_ASSERT_EQUALS(&*copy->getType(), &*sd);

    // Serialize
    {
        class TestSaveContext : public interpreter::SaveContext {
         public:
            virtual uint32_t addBCO(const interpreter::BytecodeObject& /*bco*/)
                { TS_FAIL("addBCO unexpected"); return 0; }
            virtual uint32_t addHash(const afl::data::Hash& /*hash*/)
                { TS_FAIL("addHash unexpected"); return 0; }
            virtual uint32_t addArray(const interpreter::ArrayData& /*array*/)
                { TS_FAIL("addArray unexpected"); return 0; }
            virtual uint32_t addStructureType(const interpreter::StructureTypeData& /*type*/)
                { return 42; }
            virtual uint32_t addStructureValue(const interpreter::StructureValueData& /*value*/)
                { TS_FAIL("addStructureValue unexpected"); return 0; }
            virtual bool isCurrentProcess(const interpreter::Process* /*p*/)
                { return false; }
        };

        interpreter::TagNode out;
        afl::io::InternalSink aux;
        TestSaveContext ctx;
        TS_ASSERT_THROWS_NOTHING(testee.store(out, aux, ctx));
        TS_ASSERT_EQUALS(out.tag, interpreter::TagNode::Tag_StructType);
        TS_ASSERT_EQUALS(out.value, 42U);
    }
}
