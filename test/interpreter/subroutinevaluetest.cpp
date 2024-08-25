/**
  *  \file test/interpreter/subroutinevaluetest.cpp
  *  \brief Test for interpreter::SubroutineValue
  */

#include "interpreter/subroutinevalue.hpp"

#include <stdexcept>
#include <memory>
#include "afl/io/internalsink.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/error.hpp"
#include "interpreter/savecontext.hpp"

AFL_TEST("interpreter.SubroutineValue", a)
{
    // Create testee
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
    bco->setSubroutineName("SUB");
    interpreter::SubroutineValue testee(bco);

    // Name mentions the sub name but is otherwise unparseable
    a.checkEqual("01. toString", testee.toString(false).substr(0, 2), "#<");
    a.check("02. toString", testee.toString(false).find("SUB") != String_t::npos);

    a.checkEqual("11. getBytecodeObject", &*testee.getBytecodeObject(), &*bco);
    a.checkEqual("12. getDimension", testee.getDimension(0), 0U);
    AFL_CHECK_THROWS(a("13. makeFirstContext"), testee.makeFirstContext(), interpreter::Error);

    // Cloning
    std::auto_ptr<interpreter::SubroutineValue> copy(testee.clone());
    a.checkNonNull("21. clone", copy.get());
    a.checkEqual("22. getBytecodeObject", &*copy->getBytecodeObject(), &*bco);

    // Store
    {
        class TestSaveContext : public interpreter::SaveContext {
         public:
            virtual uint32_t addBCO(const interpreter::BytecodeObject& /*bco*/)
                { return 12345; }
            virtual uint32_t addHash(const afl::data::Hash& /*hash*/)
                { throw std::runtime_error("addHash unexpected"); }
            virtual uint32_t addArray(const interpreter::ArrayData& /*array*/)
                { throw std::runtime_error("addArray unexpected"); }
            virtual uint32_t addStructureType(const interpreter::StructureTypeData& /*type*/)
                { throw std::runtime_error("addStructureType unexpected"); }
            virtual uint32_t addStructureValue(const interpreter::StructureValueData& /*value*/)
                { throw std::runtime_error("addStructureValue unexpected"); }
            virtual bool isCurrentProcess(const interpreter::Process* /*p*/)
                { return false; }
        };

        interpreter::TagNode out;
        afl::io::InternalSink aux;
        TestSaveContext ctx;
        AFL_CHECK_SUCCEEDS(a("31. store"), testee.store(out, aux, ctx));
        a.checkEqual("32. tag", out.tag, interpreter::TagNode::Tag_BCO);
        a.checkEqual("33. value", out.value, 12345U);
    }
}
