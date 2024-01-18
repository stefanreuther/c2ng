/**
  *  \file test/interpreter/structurevaluetest.cpp
  *  \brief Test for interpreter::StructureValue
  */

#include "interpreter/structurevalue.hpp"

#include <stdexcept>
#include <memory>
#include "afl/io/internalsink.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/savecontext.hpp"
#include "interpreter/values.hpp"

/** Simple basic tests. */
AFL_TEST("interpreter.StructureValue", a)
{
    // Create a type
    interpreter::StructureTypeData::Ref_t type(*new interpreter::StructureTypeData());
    type->names().addMaybe("A");
    type->names().addMaybe("X");

    // Create a value
    interpreter::StructureValueData::Ref_t value(*new interpreter::StructureValueData(type));

    // Testee
    interpreter::StructureValue testee(value);
    a.checkEqual("01. toString", testee.toString(false).substr(0, 2), "#<");
    a.checkEqual("02. getValue", &*testee.getValue(), &*value);
    a.checkNull("03. getObject", testee.getObject());

    // Clone
    std::auto_ptr<interpreter::StructureValue> copy(testee.clone());
    a.checkNonNull("11. clone", copy.get());
    a.checkEqual("12. getValue", &*copy->getValue(), &*value);

    // Element access
    interpreter::Context::PropertyIndex_t index;
    interpreter::Context::PropertyAccessor* ctx = testee.lookup("A", index);
    a.checkNonNull("21. lookup", ctx);

    // Set a value
    std::auto_ptr<afl::data::Value> newValue(interpreter::makeIntegerValue(33));
    ctx->set(index, newValue.get());

    // Read it again
    std::auto_ptr<afl::data::Value> readValue(ctx->get(index));
    a.checkNonNull("31. read value", readValue.get());
    a.check("32. read value", readValue.get() != newValue.get());
    int32_t iv = 0;
    a.check("33. read value", interpreter::checkIntegerArg(iv, readValue.get()));
    a.checkEqual("34. value", iv, 33);

    // Lookup failure
    a.checkNull("41. lookup", testee.lookup("", index));
    a.checkNull("42. lookup", testee.lookup("AA", index));

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
                { throw std::runtime_error("addStructureType unexpected"); }
            virtual uint32_t addStructureValue(const interpreter::StructureValueData& /*value*/)
                { return 777; }
            virtual bool isCurrentProcess(const interpreter::Process* /*p*/)
                { return false; }
        };

        interpreter::TagNode out;
        afl::io::InternalSink aux;
        TestSaveContext ctx;
        AFL_CHECK_SUCCEEDS(a("51. store"), testee.store(out, aux, ctx));
        a.checkEqual("52. tag", out.tag, interpreter::TagNode::Tag_Struct);
        a.checkEqual("53. value", out.value, 777U);
    }

    // Enumerate
    {
        class TestProperyAcceptor : public interpreter::PropertyAcceptor {
         public:
            TestProperyAcceptor(afl::test::Assert a)
                : m_assert(a), m_as(0), m_xs(0)
                { }
            virtual void addProperty(const String_t& name, interpreter::TypeHint th)
                {
                    // No type hint expected
                    m_assert.checkEqual("61. type hint", th, interpreter::thNone);

                    if (name == "A") {
                        ++m_as;
                    } else if (name == "X") {
                        ++m_xs;
                    } else {
                        throw std::runtime_error("unexpected name");
                    }
                }
            void verify()
                {
                    m_assert.checkEqual("71. seen A", m_as, 1);
                    m_assert.checkEqual("72. seen X", m_xs, 1);
                }
         private:
            afl::test::Assert m_assert;
            int m_as;
            int m_xs;
        };
        TestProperyAcceptor acceptor(a);
        testee.enumProperties(acceptor);
        acceptor.verify();
    }
}
