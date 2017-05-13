/**
  *  \file u/t_interpreter_structurevalue.cpp
  *  \brief Test for interpreter::StructureValue
  */

#include <memory>
#include "interpreter/structurevalue.hpp"

#include "t_interpreter.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/io/internalsink.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/savecontext.hpp"
#include "interpreter/values.hpp"

/** Simple basic tests. */
void
TestInterpreterStructureValue::testIt()
{
    // Create a type
    interpreter::StructureTypeData::Ref_t type(*new interpreter::StructureTypeData());
    type->names().addMaybe("A");
    type->names().addMaybe("X");

    // Create a value
    interpreter::StructureValueData::Ref_t value(*new interpreter::StructureValueData(type));

    // Testee
    interpreter::StructureValue testee(value);
    TS_ASSERT_EQUALS(testee.toString(false).substr(0, 2), "#<");
    TS_ASSERT_EQUALS(&*testee.getValue(), &*value);
    TS_ASSERT(testee.getObject() == 0);

    // Clone
    std::auto_ptr<interpreter::StructureValue> copy(testee.clone());
    TS_ASSERT(copy.get() != 0);
    TS_ASSERT_EQUALS(&*copy->getValue(), &*value);

    // Element access
    interpreter::Context::PropertyIndex_t index;
    interpreter::Context* ctx = testee.lookup("A", index);
    TS_ASSERT(ctx != 0);

    // Set a value
    std::auto_ptr<afl::data::Value> newValue(interpreter::makeIntegerValue(33));
    ctx->set(index, newValue.get());

    // Read it again
    std::auto_ptr<afl::data::Value> readValue(ctx->get(index));
    TS_ASSERT(readValue.get() != 0);
    TS_ASSERT(readValue.get() != newValue.get());
    int32_t iv = 0;
    TS_ASSERT(interpreter::checkIntegerArg(iv, readValue.get()));
    TS_ASSERT_EQUALS(iv, 33);

    // Lookup failure
    TS_ASSERT(testee.lookup("", index) == 0);
    TS_ASSERT(testee.lookup("AA", index) == 0);

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
                { TS_FAIL("addStructureType unexpected"); return 0; }
            virtual uint32_t addStructureValue(const interpreter::StructureValueData& /*value*/)
                { return 777; }
            virtual bool isCurrentProcess(const interpreter::Process* /*p*/)
                { return false; }
        };
        
        interpreter::TagNode out;
        afl::io::InternalSink aux;
        afl::charset::Utf8Charset cs;
        TestSaveContext ctx;
        TS_ASSERT_THROWS_NOTHING(testee.store(out, aux, cs, ctx));
        TS_ASSERT_EQUALS(out.tag, interpreter::TagNode::Tag_Struct);
        TS_ASSERT_EQUALS(out.value, 777U);
    }

    // Enumerate
    {
        class TestProperyAcceptor : public interpreter::PropertyAcceptor {
         public:
            TestProperyAcceptor()
                : m_as(0), m_xs(0)
                { }
            virtual void addProperty(const String_t& name, interpreter::TypeHint th)
                {
                    // No type hint expected
                    TS_ASSERT_EQUALS(th, interpreter::thNone);

                    if (name == "A") {
                        ++m_as;
                    } else if (name == "X") {
                        ++m_xs;
                    } else {
                        TS_FAIL("unexpected name");
                    }
                }
            void verify()
                {
                    TS_ASSERT_EQUALS(m_as, 1);
                    TS_ASSERT_EQUALS(m_xs, 1);
                }
         private:
            int m_as;
            int m_xs;
        };
        TestProperyAcceptor acceptor;
        testee.enumProperties(acceptor);
        acceptor.verify();
    }
}

