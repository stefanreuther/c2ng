/**
  *  \file test/interpreter/test/contextverifiertest.cpp
  *  \brief Test for interpreter::test::ContextVerifier
  */

#include "interpreter/test/contextverifier.hpp"

#include "afl/data/booleanvalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/arraydata.hpp"
#include "interpreter/arrayvalue.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/simplecontext.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "interpreter/values.hpp"
#include <memory>

using afl::except::AssertionFailedException;

namespace {
    /*
     *  Simple context for testing
     *
     *  Primary objective: return a single predefined property under a predefined name with a predefined type hint.
     *
     *  Secondary objective: return additional properties in enumProperties() (these all cause verifyTypes to fail).
     */
    class TestContext : public interpreter::SimpleContext, public interpreter::Context::PropertyAccessor {
     public:
        TestContext(afl::test::Assert a, String_t name, interpreter::TypeHint th, afl::data::Value* value)
            : m_assert(a), m_name(name), m_type(th), m_value(value), m_table()
            { }

        void setExtraTable(afl::base::Memory<const interpreter::NameTable> tab)
            { m_table = tab; }

        virtual interpreter::Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
            {
                if (name.match(m_name)) {
                    result = 42;
                    return this;
                } else {
                    return 0;
                }
            }

        virtual void set(PropertyIndex_t /*index*/, const afl::data::Value* /*value*/)
            { m_assert.fail("unexpected: set"); }

        virtual afl::data::Value* get(PropertyIndex_t index)
            {
                m_assert.checkEqual("TestContext::get", index, 42U);
                return afl::data::Value::cloneOf(m_value.get());
            }

        virtual bool next()
            { m_assert.fail("unexpected: next"); return false; }

        virtual interpreter::Context* clone() const
            { m_assert.fail("unexpected: clone"); return 0; }

        virtual afl::base::Deletable* getObject()
            { m_assert.fail("unexpected: getObject"); return 0; }

        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const
            {
                if (!m_table.empty()) {
                    acceptor.enumTable(m_table);
                }
                acceptor.addProperty(m_name, m_type);
            }

        virtual String_t toString(bool /*readable*/) const
            { return "#<TestContext>"; }

        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
            { m_assert.fail("unexpected: store"); }
     private:
        afl::test::Assert m_assert;
        String_t m_name;
        interpreter::TypeHint m_type;
        std::auto_ptr<afl::data::Value> m_value;
        afl::base::Memory<const interpreter::NameTable> m_table;
    };
}

/** Test verifyTypes, success cases.
    A: create a context with correct type mapping.
    E: verifyTypes() succeeds */
AFL_TEST("interpreter.test.ContextVerifier:verifyTypes:success:thInt", a)
{
    TestContext ctx(a, "IV", interpreter::thInt, new afl::data::IntegerValue(2));
    interpreter::test::ContextVerifier testee(ctx, a);
    AFL_CHECK_SUCCEEDS(a, testee.verifyTypes());
}

AFL_TEST("interpreter.test.ContextVerifier:verifyTypes:success:thBool", a)
{
    TestContext ctx(a, "BV", interpreter::thBool, new afl::data::BooleanValue(false));
    interpreter::test::ContextVerifier testee(ctx, a);
    AFL_CHECK_SUCCEEDS(a, testee.verifyTypes());
}

AFL_TEST("interpreter.test.ContextVerifier:verifyTypes:success:thFloat", a)
{
    TestContext ctx(a, "FV", interpreter::thFloat, new afl::data::FloatValue(3.14));
    interpreter::test::ContextVerifier testee(ctx, a);
    AFL_CHECK_SUCCEEDS(a, testee.verifyTypes());
}

AFL_TEST("interpreter.test.ContextVerifier:verifyTypes:success:thString", a)
{
    TestContext ctx(a, "SV", interpreter::thString, new afl::data::StringValue("hi"));
    interpreter::test::ContextVerifier testee(ctx, a);
    AFL_CHECK_SUCCEEDS(a, testee.verifyTypes());
}

AFL_TEST("interpreter.test.ContextVerifier:verifyTypes:success:thNone", a)
{
    TestContext ctx(a, "ANY", interpreter::thNone, new afl::data::StringValue("hi"));
    interpreter::test::ContextVerifier testee(ctx, a);
    AFL_CHECK_SUCCEEDS(a, testee.verifyTypes());
}

AFL_TEST("interpreter.test.ContextVerifier:verifyTypes:success:thProcedure", a)
{
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
    TestContext ctx(a, "SUB", interpreter::thProcedure, new interpreter::SubroutineValue(bco));
    interpreter::test::ContextVerifier testee(ctx, a);
    AFL_CHECK_SUCCEEDS(a, testee.verifyTypes());
}

AFL_TEST("interpreter.test.ContextVerifier:verifyTypes:success:thFunction", a)
{
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    TestContext ctx(a, "FXN", interpreter::thFunction, new interpreter::SubroutineValue(bco));
    interpreter::test::ContextVerifier testee(ctx, a);
    AFL_CHECK_SUCCEEDS(a, testee.verifyTypes());
}

AFL_TEST("interpreter.test.ContextVerifier:verifyTypes:success:thArray", a)
{
    TestContext ctx(a, "ARR", interpreter::thArray, new interpreter::ArrayValue(*new interpreter::ArrayData()));
    interpreter::test::ContextVerifier testee(ctx, a);
    AFL_CHECK_SUCCEEDS(a, testee.verifyTypes());
}

/** Test verifyTypes, duplicate name.
    A: create a context with a duplicate type mapping.
    E: verifyTypes() fails */
AFL_TEST("interpreter.test.ContextVerifier:verifyTypes:fail:duplicate", a)
{
    // "V" is reported twice with same data
    TestContext ctx(a, "V", interpreter::thInt, new afl::data::IntegerValue(2));
    static const interpreter::NameTable tab[] = {
        {"V", 42, 0, interpreter::thInt },
    };
    ctx.setExtraTable(tab);

    interpreter::test::ContextVerifier testee(ctx, a);
    AFL_CHECK_THROWS(a, testee.verifyTypes(), AssertionFailedException);
}

/** Test verifyTypes, enumProperties/lookup mismatch.
    A: create a context that reports an unresolvable name in enumProperties.
    E: verifyTypes() fails */
AFL_TEST("interpreter.test.ContextVerifier:verifyTypes:fail:unresolved-name", a)
{
    // "V" is reported correctly, "Q" is not resolvable.
    TestContext ctx(a, "V", interpreter::thInt, new afl::data::IntegerValue(2));
    static const interpreter::NameTable tab[] = {
        {"Q", 42, 0, interpreter::thInt },
    };
    ctx.setExtraTable(tab);

    interpreter::test::ContextVerifier testee(ctx, a);
    AFL_CHECK_THROWS(a, testee.verifyTypes(), AssertionFailedException);
}

/** Test verifyTypes, type check fails.
    A: create a context that reports a wrong type hint.
    E: verifyTypes() fails */

// Given integer, expect bool
AFL_TEST("interpreter.test.ContextVerifier:verifyTypes:fail:mismatch:thBool:int", a)
{
    TestContext ctx(a, "V", interpreter::thBool, new afl::data::IntegerValue(2));
    interpreter::test::ContextVerifier testee(ctx, a);
    AFL_CHECK_THROWS(a, testee.verifyTypes(), AssertionFailedException);
}

// Given procedure, expect function
AFL_TEST("interpreter.test.ContextVerifier:verifyTypes:fail:mismatch:thFunction:proc", a)
{
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
    TestContext ctx(a, "SUB", interpreter::thFunction, new interpreter::SubroutineValue(bco));
    interpreter::test::ContextVerifier testee(ctx, a);
    AFL_CHECK_THROWS(a, testee.verifyTypes(), AssertionFailedException);
}

// Given function, expect procedure
AFL_TEST("interpreter.test.ContextVerifier:verifyTypes:fail:mismatch:thProcedure:func", a)
{
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    TestContext ctx(a, "FXN", interpreter::thProcedure, new interpreter::SubroutineValue(bco));
    interpreter::test::ContextVerifier testee(ctx, a);
    AFL_CHECK_THROWS(a, testee.verifyTypes(), AssertionFailedException);
}

/** Test verifyTypes, null property.
    A: create a context that reports no no-null property.
    E: verifyTypes() fails */
AFL_TEST("interpreter.test.ContextVerifier:verifyTypes:fail:all-null", a)
{
    TestContext ctx(a, "V", interpreter::thNone, 0);

    interpreter::test::ContextVerifier testee(ctx, a);
    AFL_CHECK_THROWS(a, testee.verifyTypes(), AssertionFailedException);
}

/** Test verifyInteger.
    A: create a context with an integer property.
    E: verifyInteger succeeds for that property, fails for others. Other type checks fail. */
AFL_TEST("interpreter.test.ContextVerifier:verifyInteger", a)
{
    TestContext ctx(a, "I", interpreter::thInt, new afl::data::IntegerValue(7));
    interpreter::test::ContextVerifier testee(ctx, a);

    AFL_CHECK_SUCCEEDS(a("01. ok"), testee.verifyInteger("I", 7));
    AFL_CHECK_THROWS(a("02. fail: name"), testee.verifyInteger("J", 7), AssertionFailedException);

    AFL_CHECK_THROWS(a("11. verifyBoolean"), testee.verifyBoolean("I", true), AssertionFailedException);
    AFL_CHECK_THROWS(a("12. verifyString"),  testee.verifyString("I", "s"), AssertionFailedException);
    AFL_CHECK_THROWS(a("13. verifyNull"),    testee.verifyNull("I"), AssertionFailedException);
}

/** Test verifyBoolean.
    A: create a context with a boolean property.
    E: verifyBoolean succeeds for that property, fails for others. Other type checks fail. */
AFL_TEST("interpreter.test.ContextVerifier:verifyBoolean", a)
{
    TestContext ctx(a, "B", interpreter::thInt, new afl::data::BooleanValue(true));
    interpreter::test::ContextVerifier testee(ctx, a);

    AFL_CHECK_SUCCEEDS(a("01. ok"), testee.verifyBoolean("B", true));
    AFL_CHECK_THROWS(a("02. fail: name"), testee.verifyBoolean("C", true), AssertionFailedException);

    AFL_CHECK_THROWS(a("11. verifyInteger"), testee.verifyInteger("B", true), AssertionFailedException);
    AFL_CHECK_THROWS(a("12. verifyString"),  testee.verifyString("B", "s"), AssertionFailedException);
    AFL_CHECK_THROWS(a("13. verifyNull"),    testee.verifyNull("B"), AssertionFailedException);
}

/** Test verifyString.
    A: create a context with a string property.
    E: verifyString succeeds for that property, fails for others. Other type checks fail. */
AFL_TEST("interpreter.test.ContextVerifier:verifyString", a)
{
    TestContext ctx(a, "S", interpreter::thString, new afl::data::StringValue("s"));
    interpreter::test::ContextVerifier testee(ctx, a);

    AFL_CHECK_SUCCEEDS(a("01. ok"), testee.verifyString("S", "s"));
    AFL_CHECK_THROWS(a("02. fail: name"), testee.verifyString("T", "s"), AssertionFailedException);

    AFL_CHECK_THROWS(a("11. verifyInteger"), testee.verifyInteger("S", 42), AssertionFailedException);
    AFL_CHECK_THROWS(a("12. verifyBoolean"), testee.verifyBoolean("S", true), AssertionFailedException);
    AFL_CHECK_THROWS(a("13. verifyNull"),    testee.verifyNull("S"), AssertionFailedException);
}

/** Test verifyNull.
    A: create a context with a null property.
    E: verifyNull succeeds for that property, fails for others. Other type checks fail. */
AFL_TEST("interpreter.test.ContextVerifier:verifyNull", a)
{
    TestContext ctx(a, "N", interpreter::thNone, 0);
    interpreter::test::ContextVerifier testee(ctx, a);

    AFL_CHECK_SUCCEEDS(a("01. verifyNull"), testee.verifyNull("N"));
    AFL_CHECK_THROWS(a("02. fail: name"), testee.verifyNull("Q"), AssertionFailedException);

    AFL_CHECK_THROWS(a("11. verifyInteger"), testee.verifyInteger("N", 7), AssertionFailedException);
    AFL_CHECK_THROWS(a("12. verifyBoolean"), testee.verifyBoolean("N", true), AssertionFailedException);
    AFL_CHECK_THROWS(a("13. verifyString"),  testee.verifyString("N", "s"), AssertionFailedException);
}

AFL_TEST("interpreter.test.ContextVerifier:set", a)
{
    class Tester : public interpreter::SimpleContext, public interpreter::Context::PropertyAccessor {
     public:
        Tester(afl::test::Assert a)
            : m_assert(a), m_value()
            { }
        const String_t& getLastValue() const
            { return m_value; }
        virtual interpreter::Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
            {
                if (name.match("V")) {
                    result = 42;
                    return this;
                } else {
                    return 0;
                }
            }

        virtual void set(PropertyIndex_t index, const afl::data::Value* value)
            {
                m_assert.checkEqual("Tester::set", index, 42U);
                m_value = interpreter::toString(value, true);
            }

        virtual afl::data::Value* get(PropertyIndex_t /*index*/)
            { m_assert.fail("unexpected: get"); return 0; }
        virtual bool next()
            { m_assert.fail("unexpected: next"); return false; }
        virtual interpreter::Context* clone() const
            { m_assert.fail("unexpected: clone"); return 0; }
        virtual afl::base::Deletable* getObject()
            { m_assert.fail("unexpected: getObject"); return 0; }
        virtual void enumProperties(interpreter::PropertyAcceptor& /*acceptor*/) const
            { m_assert.fail("unexpected: enumProperties"); }
        virtual String_t toString(bool /*readable*/) const
            { return "#<TestContext>"; }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
            { m_assert.fail("unexpected: store"); }
     private:
        afl::test::Assert m_assert;
        String_t m_value;
    };

    Tester ctx(a);
    interpreter::test::ContextVerifier testee(ctx, a);

    // Success cases
    AFL_CHECK_SUCCEEDS(a("11. setIntegerValue"), testee.setIntegerValue("V", 55));
    a.checkEqual        ("12. getLastValue",     ctx.getLastValue(), "55");
    AFL_CHECK_SUCCEEDS(a("13. setStringValue"),  testee.setStringValue("V", "x"));
    a.checkEqual        ("14. getLastValue",     ctx.getLastValue(), "\"x\"");
    AFL_CHECK_SUCCEEDS(a("15. setValue"),        testee.setValue("V", 0));
    a.checkEqual        ("16. getLastValue",     ctx.getLastValue(), "Z(0)");
    AFL_CHECK_SUCCEEDS(a("17. setValue"),        testee.setValue("V", &ctx));
    a.checkEqual        ("18. getLastValue",     ctx.getLastValue(), "#<TestContext>");

    // Failure cases (bad name)
    AFL_CHECK_THROWS(a("21. setIntegerValue"), testee.setIntegerValue("X", 55), AssertionFailedException);
    AFL_CHECK_THROWS(a("22. setStringValue"),  testee.setStringValue("X", "x"), AssertionFailedException);
    AFL_CHECK_THROWS(a("23. setValue"),        testee.setValue("X", 0),         AssertionFailedException);
}
