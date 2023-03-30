/**
  *  \file interpreter/test/valueverifier.cpp
  *  \brief Class interpreter::test::ValueVerifier
  */

#include <memory>
#include <cmath>
#include "interpreter/test/valueverifier.hpp"
#include "afl/data/booleanvalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/nullstream.hpp"
#include "interpreter/error.hpp"
#include "interpreter/tagnode.hpp"
#include "interpreter/vmio/nullsavecontext.hpp"

interpreter::test::ValueVerifier::ValueVerifier(BaseValue& v, afl::test::Assert as)
    : m_value(v), m_assert(as)
{ }

void
interpreter::test::ValueVerifier::verifyBasics()
{
    // Valid stringification
    m_assert.check("toString(false)", m_value.toString(false) != "");
    m_assert.check("toString(true)",  m_value.toString(true)  != "");

    // Clonable
    std::auto_ptr<BaseValue> clone(m_value.clone());
    m_assert.check("clone != null", clone.get() != 0);
    m_assert.check("clone != orig", clone.get() != &m_value);

    // Correct stringification
    m_assert.checkEqual("clone toString(false)", m_value.toString(false), clone->toString(false));
    m_assert.checkEqual("clone toString(true)", m_value.toString(true), clone->toString(true));
}

void
interpreter::test::ValueVerifier::verifyNotSerializable()
{
    bool ok = false;
    try {
        TagNode tag;
        afl::io::NullStream sink;
        interpreter::vmio::NullSaveContext saveContext;
        m_value.store(tag, sink, saveContext);
    }
    catch (Error& e) {
        ok = true;
    }
    catch (...) { }
    m_assert.check("save throws", ok);
}

void
interpreter::test::ValueVerifier::verifySerializable(uint16_t tag, uint32_t value, afl::base::ConstBytes_t data)
{
    TagNode n;
    afl::io::InternalStream sink;
    interpreter::vmio::NullSaveContext saveContext;
    try {
        m_value.store(n, sink, saveContext);
    }
    catch (...) {
        m_assert.fail("save failed");
    }
    m_assert.checkEqual("tag", n.tag, tag);
    m_assert.checkEqual("value", n.value, value);
    m_assert.checkEqualContent("data", sink.getContent(), data);
}

/*
 *  verifyNew functions
 */

void
interpreter::test::verifyNewInteger(const afl::test::Assert& as, afl::data::Value* value, int32_t expect)
{
    std::auto_ptr<afl::data::Value> p(value);
    as.check("expect non-null", p.get() != 0);

    afl::data::IntegerValue* iv = dynamic_cast<afl::data::IntegerValue*>(p.get());
    as.check("expect integer", iv != 0);
    as.checkEqual("expect value", iv->getValue(), expect);
}

void
interpreter::test::verifyNewFloat(const afl::test::Assert& as, afl::data::Value* value, double expect, double delta)
{
    std::auto_ptr<afl::data::Value> p(value);
    as.check("expect non-null", p.get() != 0);

    double found = 0;
    if (afl::data::IntegerValue* iv = dynamic_cast<afl::data::IntegerValue*>(p.get())) {
        found = iv->getValue();
    } else if (afl::data::FloatValue* fv = dynamic_cast<afl::data::FloatValue*>(p.get())) {
        found = fv->getValue();
    } else {
        as.fail("expect int or float");
    }
    as.check("expect value", std::abs(found - expect) <= delta);
}

void
interpreter::test::verifyNewBoolean(const afl::test::Assert& as, afl::data::Value* value, bool expect)
{
    std::auto_ptr<afl::data::Value> p(value);
    as.check("expect non-null", p.get() != 0);

    afl::data::BooleanValue* bv = dynamic_cast<afl::data::BooleanValue*>(p.get());
    as.check("expect boolean", bv != 0);
    as.checkEqual("expect value", int(bv->getValue()), int(expect));
}

String_t
interpreter::test::verifyNewString(const afl::test::Assert& as, afl::data::Value* value)
{
    std::auto_ptr<afl::data::Value> p(value);
    as.check("expect non-null", p.get() != 0);

    afl::data::StringValue* sv = dynamic_cast<afl::data::StringValue*>(p.get());
    as.check("expect string", sv != 0);
    return sv->getValue();
}

void
interpreter::test::verifyNewString(const afl::test::Assert& as, afl::data::Value* value, const char* expect)
{
    as.checkEqual("expect value", verifyNewString(as, value), expect);
}

void
interpreter::test::verifyNewNull(const afl::test::Assert& as, afl::data::Value* value)
{
    std::auto_ptr<afl::data::Value> p(value);
    as.check("expect null", p.get() == 0);
}
