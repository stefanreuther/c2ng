/**
  *  \file u/t_interpreter_test_contextverifier.cpp
  *  \brief Test for interpreter::test::ContextVerifier
  */

#include <memory>
#include "interpreter/test/contextverifier.hpp"

#include "t_interpreter_test.hpp"
#include "afl/data/booleanvalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "interpreter/arraydata.hpp"
#include "interpreter/arrayvalue.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/context.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "interpreter/values.hpp"

using afl::except::AssertionFailedException;

namespace {
    /*
     *  Simple context for testing
     *
     *  Primary objective: return a single predefined property under a predefined name with a predefined type hint.
     *
     *  Secondary objective: return additional properties in enumProperties() (these all cause verifyTypes to fail).
     */
    class TestContext : public interpreter::Context {
     public:
        TestContext(String_t name, interpreter::TypeHint th, afl::data::Value* value)
            : m_name(name), m_type(th), m_value(value), m_table()
            { }

        void setExtraTable(afl::base::Memory<const interpreter::NameTable> tab)
            { m_table = tab; }

        virtual interpreter::Context* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
            {
                if (name.match(m_name)) {
                    result = 42;
                    return this;
                } else {
                    return 0;
                }
            }

        virtual void set(PropertyIndex_t /*index*/, const afl::data::Value* /*value*/)
            { TS_FAIL("unexpected: set"); }

        virtual afl::data::Value* get(PropertyIndex_t index)
            {
                TS_ASSERT_EQUALS(index, 42U);
                return afl::data::Value::cloneOf(m_value.get());
            }

        virtual bool next()
            { TS_FAIL("unexpected: next"); return false; }

        virtual interpreter::Context* clone() const
            { TS_FAIL("unexpected: clone"); return 0; }

        virtual game::map::Object* getObject()
            { TS_FAIL("unexpected: getObject"); return 0; }

        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor)
            {
                if (!m_table.empty()) {
                    acceptor.enumTable(m_table);
                }
                acceptor.addProperty(m_name, m_type);
            }

        virtual String_t toString(bool /*readable*/) const
            { return "#<TestContext>"; }

        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
            { TS_FAIL("unexpected: store"); }
     private:
        String_t m_name;
        interpreter::TypeHint m_type;
        std::auto_ptr<afl::data::Value> m_value;
        afl::base::Memory<const interpreter::NameTable> m_table;
    };
}

/** Test verifyTypes, success cases.
    A: create a context with correct type mapping.
    E: verifyTypes() succeeds */
void
TestInterpreterTestContextVerifier::testVerifyTypesSuccess()
{
    {
        TestContext ctx("IV", interpreter::thInt, new afl::data::IntegerValue(2));
        interpreter::test::ContextVerifier testee(ctx, "testVerifyTypesSuccess: Int");
        TS_ASSERT_THROWS_NOTHING(testee.verifyTypes());
    }
    {
        TestContext ctx("BV", interpreter::thBool, new afl::data::BooleanValue(false));
        interpreter::test::ContextVerifier testee(ctx, "testVerifyTypesSuccess: Bool");
        TS_ASSERT_THROWS_NOTHING(testee.verifyTypes());
    }
    {
        TestContext ctx("FV", interpreter::thFloat, new afl::data::FloatValue(3.14));
        interpreter::test::ContextVerifier testee(ctx, "testVerifyTypesSuccess: Float");
        TS_ASSERT_THROWS_NOTHING(testee.verifyTypes());
    }
    {
        TestContext ctx("SV", interpreter::thString, new afl::data::StringValue("hi"));
        interpreter::test::ContextVerifier testee(ctx, "testVerifyTypesSuccess: String");
        TS_ASSERT_THROWS_NOTHING(testee.verifyTypes());
    }
    {
        TestContext ctx("ANY", interpreter::thNone, new afl::data::StringValue("hi"));
        interpreter::test::ContextVerifier testee(ctx, "testVerifyTypesSuccess: None");
        TS_ASSERT_THROWS_NOTHING(testee.verifyTypes());
    }
    {
        interpreter::BCORef_t bco = *new interpreter::BytecodeObject();
        bco->setIsProcedure(true);
        TestContext ctx("SUB", interpreter::thProcedure, new interpreter::SubroutineValue(bco));
        interpreter::test::ContextVerifier testee(ctx, "testVerifyTypesSuccess: Procedure");
        TS_ASSERT_THROWS_NOTHING(testee.verifyTypes());
    }
    {
        interpreter::BCORef_t bco = *new interpreter::BytecodeObject();
        bco->setIsProcedure(false);
        TestContext ctx("FXN", interpreter::thFunction, new interpreter::SubroutineValue(bco));
        interpreter::test::ContextVerifier testee(ctx, "testVerifyTypesSuccess: Function");
        TS_ASSERT_THROWS_NOTHING(testee.verifyTypes());
    }
    {
        TestContext ctx("ARR", interpreter::thArray, new interpreter::ArrayValue(*new interpreter::ArrayData()));
        interpreter::test::ContextVerifier testee(ctx, "testVerifyTypesSuccess: Function");
        TS_ASSERT_THROWS_NOTHING(testee.verifyTypes());
    }
}

/** Test verifyTypes, duplicate name.
    A: create a context with a duplicate type mapping.
    E: verifyTypes() fails */
void
TestInterpreterTestContextVerifier::testVerifyTypesDuplicate()
{
    // "V" is reported twice with same data
    TestContext ctx("V", interpreter::thInt, new afl::data::IntegerValue(2));
    static const interpreter::NameTable tab[] = {
        {"V", 42, 0, interpreter::thInt },
    };
    ctx.setExtraTable(tab);

    interpreter::test::ContextVerifier testee(ctx, "testVerifyTypesDuplicate");
    TS_ASSERT_THROWS(testee.verifyTypes(), AssertionFailedException);
}

/** Test verifyTypes, enumProperties/lookup mismatch.
    A: create a context that reports an unresolvable name in enumProperties.
    E: verifyTypes() fails */
void
TestInterpreterTestContextVerifier::testVerifyTypesMismatch()
{
    // "V" is reported correctly, "Q" is not resolvable.
    TestContext ctx("V", interpreter::thInt, new afl::data::IntegerValue(2));
    static const interpreter::NameTable tab[] = {
        {"Q", 42, 0, interpreter::thInt },
    };
    ctx.setExtraTable(tab);

    interpreter::test::ContextVerifier testee(ctx, "testVerifyTypesMismatch");
    TS_ASSERT_THROWS(testee.verifyTypes(), AssertionFailedException);
}

/** Test verifyTypes, type check fails.
    A: create a context that reports a wrong type hint.
    E: verifyTypes() fails */
void
TestInterpreterTestContextVerifier::testVerifyTypesTypeCheck()
{
    // Given integer, expect bool
    {
        TestContext ctx("V", interpreter::thBool, new afl::data::IntegerValue(2));
        interpreter::test::ContextVerifier testee(ctx, "testVerifyTypesTypeCheck: Bool");
        TS_ASSERT_THROWS(testee.verifyTypes(), AssertionFailedException);
    }

    // Given procedure, expect function
    {
        interpreter::BCORef_t bco = *new interpreter::BytecodeObject();
        bco->setIsProcedure(true);
        TestContext ctx("SUB", interpreter::thFunction, new interpreter::SubroutineValue(bco));
        interpreter::test::ContextVerifier testee(ctx, "testVerifyTypesTypeCheck: Function");
        TS_ASSERT_THROWS(testee.verifyTypes(), AssertionFailedException);
    }

    // Given function, expect procedure
    {
        interpreter::BCORef_t bco = *new interpreter::BytecodeObject();
        bco->setIsProcedure(false);
        TestContext ctx("FXN", interpreter::thProcedure, new interpreter::SubroutineValue(bco));
        interpreter::test::ContextVerifier testee(ctx, "testVerifyTypesTypeCheck: SubroutineValue");
        TS_ASSERT_THROWS(testee.verifyTypes(), AssertionFailedException);
    }
}

/** Test verifyTypes, null property.
    A: create a context that reports no no-null property.
    E: verifyTypes() fails */
void
TestInterpreterTestContextVerifier::testVerifyTypesNull()
{
    TestContext ctx("V", interpreter::thNone, 0);

    interpreter::test::ContextVerifier testee(ctx, "testVerifyTypesNull");
    TS_ASSERT_THROWS(testee.verifyTypes(), AssertionFailedException);
}

/** Test verifyInteger.
    A: create a context with an integer property.
    E: verifyInteger succeeds for that property, fails for others. Other type checks fail. */
void
TestInterpreterTestContextVerifier::testVerifyInteger()
{
    TestContext ctx("I", interpreter::thInt, new afl::data::IntegerValue(7));
    interpreter::test::ContextVerifier testee(ctx, "testVerifyInteger");

    TS_ASSERT_THROWS_NOTHING(testee.verifyInteger("I", 7));
    TS_ASSERT_THROWS(testee.verifyInteger("J", 7), AssertionFailedException);

    TS_ASSERT_THROWS(testee.verifyBoolean("I", true), AssertionFailedException);
    TS_ASSERT_THROWS(testee.verifyString("I", "s"), AssertionFailedException);
    TS_ASSERT_THROWS(testee.verifyNull("I"), AssertionFailedException);
}

/** Test verifyBoolean.
    A: create a context with a boolean property.
    E: verifyBoolean succeeds for that property, fails for others. Other type checks fail. */
void
TestInterpreterTestContextVerifier::testVerifyBoolean()
{
    TestContext ctx("B", interpreter::thInt, new afl::data::BooleanValue(true));
    interpreter::test::ContextVerifier testee(ctx, "testVerifyBoolean");

    TS_ASSERT_THROWS_NOTHING(testee.verifyBoolean("B", true));
    TS_ASSERT_THROWS(testee.verifyBoolean("C", true), AssertionFailedException);

    TS_ASSERT_THROWS(testee.verifyInteger("B", true), AssertionFailedException);
    TS_ASSERT_THROWS(testee.verifyString("B", "s"), AssertionFailedException);
    TS_ASSERT_THROWS(testee.verifyNull("B"), AssertionFailedException);
}

/** Test verifyString.
    A: create a context with a string property.
    E: verifyString succeeds for that property, fails for others. Other type checks fail. */
void
TestInterpreterTestContextVerifier::testVerifyString()
{
    TestContext ctx("S", interpreter::thString, new afl::data::StringValue("s"));
    interpreter::test::ContextVerifier testee(ctx, "testVerifyString");

    TS_ASSERT_THROWS_NOTHING(testee.verifyString("S", "s"));
    TS_ASSERT_THROWS(testee.verifyString("T", "s"), AssertionFailedException);

    TS_ASSERT_THROWS(testee.verifyInteger("S", 42), AssertionFailedException);
    TS_ASSERT_THROWS(testee.verifyBoolean("S", true), AssertionFailedException);
    TS_ASSERT_THROWS(testee.verifyNull("S"), AssertionFailedException);
}

/** Test verifyNull.
    A: create a context with a null property.
    E: verifyNull succeeds for that property, fails for others. Other type checks fail. */
void
TestInterpreterTestContextVerifier::testVerifyNull()
{
    TestContext ctx("N", interpreter::thNone, 0);
    interpreter::test::ContextVerifier testee(ctx, "testVerifyNull");

    TS_ASSERT_THROWS_NOTHING(testee.verifyNull("N"));
    TS_ASSERT_THROWS(testee.verifyNull("Q"), AssertionFailedException);

    TS_ASSERT_THROWS(testee.verifyInteger("N", 7), AssertionFailedException);
    TS_ASSERT_THROWS(testee.verifyBoolean("N", true), AssertionFailedException);
    TS_ASSERT_THROWS(testee.verifyString("N", "s"), AssertionFailedException);
}

