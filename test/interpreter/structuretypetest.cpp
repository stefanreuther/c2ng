/**
  *  \file test/interpreter/structuretypetest.cpp
  *  \brief Test for interpreter::StructureType
  */

#include "interpreter/structuretype.hpp"

#include <stdexcept>
#include <memory>
#include "afl/io/internalsink.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/savecontext.hpp"

/** Simple test. */
AFL_TEST("interpreter.StructureType", a)
{
    interpreter::StructureTypeData::Ref_t sd(*new interpreter::StructureTypeData());
    interpreter::StructureType testee(sd);

    // Verify
    a.checkEqual("01. getType", &*testee.getType(), &*sd);
    a.checkEqual("02. toString", testee.toString(false).substr(0, 2), "#<");

    // Clone
    std::auto_ptr<interpreter::StructureType> copy(testee.clone());
    a.checkNonNull("11. clone", copy.get());
    a.checkEqual("12. getType", &*copy->getType(), &*sd);

    // Serialize
    {
        class TestSaveContext : public interpreter::SaveContext {
         public:
            virtual uint32_t addBCO(const interpreter::BytecodeObject& /*bco*/)
                { throw std::runtime_error("addBCO unexpected"); }
            virtual uint32_t addHash(const afl::data::Hash& /*hash*/)
                { throw std::runtime_error("addHash unexpected"); }
            virtual uint32_t addArray(const interpreter::ArrayData& /*array*/)
                { throw std::runtime_error("addArray unexpected"); }
            virtual uint32_t addStructureType(const interpreter::StructureTypeData& /*type*/)
                { return 42; }
            virtual uint32_t addStructureValue(const interpreter::StructureValueData& /*value*/)
                { throw std::runtime_error("addStructureValue unexpected"); }
            virtual bool isCurrentProcess(const interpreter::Process* /*p*/)
                { return false; }
        };

        interpreter::TagNode out;
        afl::io::InternalSink aux;
        TestSaveContext ctx;
        AFL_CHECK_SUCCEEDS(a("21. store"), testee.store(out, aux, ctx));
        a.checkEqual("22. tag", out.tag, interpreter::TagNode::Tag_StructType);
        a.checkEqual("23. value", out.value, 42U);
    }
}
