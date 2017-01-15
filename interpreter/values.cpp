/**
  *  \file interpreter/values.cpp
  */

#include <cmath>
#include "interpreter/values.hpp"
#include "afl/data/booleanvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/string/format.hpp"
#include "afl/data/visitor.hpp"
#include "interpreter/error.hpp"
#include "interpreter/basevalue.hpp"

/** Make a tristate-boolean value from integer.
    \param value Negative for empty, zero for false, positive for true */
afl::data::Value*
interpreter::makeBooleanValue(int value)
{
    // ex int/value.h:makeBoolValue(int value)
    if (value < 0) {
        return 0;
    } else {
        return new afl::data::BooleanValue(value != 0);
    }
}

/** Make integer value. Convenience function to save code.
    \param value Value to create */
afl::data::Value*
interpreter::makeIntegerValue(int32_t value)
{
    // ex int/value:makeIntValue(int32_t value)
    return new afl::data::IntegerValue(value);
}

/** Make float value. Convenience function to save code.
    \param value Value to create */
afl::data::Value*
interpreter::makeFloatValue(double value)
{
    // ex makeFloatValue(double value)
    return new afl::data::FloatValue(value);
}

/** Make string value. Convenience function to save code.
    \param str Value to create */
afl::data::Value*
interpreter::makeStringValue(String_t str)
{
    // ex makeStringValue(string_t str)
    return new afl::data::StringValue(str);
}

/** Make string value. Convenience function to save code.
    \param str Value to create */
afl::data::Value*
interpreter::makeStringValue(const char* str)
{
    // ex makeStringValue(const char* str)
    return new afl::data::StringValue(str);
}

afl::data::Value*
interpreter::makeOptionalStringValue(const afl::base::Optional<String_t>& value)
{
    if (const String_t* p = value.get()) {
        return makeStringValue(*p);
    } else {
        return 0;
    }
}

/** Get tristate-integer value from value. This is used whenever a value is used
    in a boolean context in a program.
    \retval -1 Input is EMPTY
    \retval 0  Input is False
    \retval +1 Input is True
    \change PCC2 <= 2.0.1 treats non-scalars as empty. We treat non-scalars as true. */
int
interpreter::getBooleanValue(const afl::data::Value* value)
{
    // What should this return for values that are neither int, nor float, nor string?
    // In PCC2 up to 2.0.1, we produced EMPTY which means out-of-range context accesses and correct accesses are indistinguishable
    // unless you resort to hacks such as "Ships(ID_TO_TEST) # ''" (produces a string for valid Ids, EMPTY otherwise).
    // Assuming that all non-empty values are somehow meaningful, it makes sense to treat them as true instead.
    // This is also consistent with other scripting languages that treat object references as true (JavaScript, Perl, Python...).

    // It is important to not generate an error.
    // This the convenient property of making ubool/unot/uzap/unot2/uisempty operations never fail,
    // which permits a number of useful optimisations.

    // Using a visitor is cleaner and about 10% faster than using a dynamic_cast type switch.
    class V : public afl::data::Visitor {
     public:
        V()
            : m_result(0)
            { }
        virtual void visitString(const String_t& str)
            { m_result = !str.empty(); }
        virtual void visitInteger(int32_t iv)
            { m_result = (iv != 0); }
        virtual void visitFloat(double fv)
            { m_result = (std::fabs(fv) > 1.0E-06); }
        virtual void visitBoolean(bool bv)
            { m_result = bv; }
        virtual void visitHash(const afl::data::Hash& /*hv*/)
            { m_result = 1; }
        virtual void visitVector(const afl::data::Vector& /*vv*/)
            { m_result = 1; }
        virtual void visitOther(const afl::data::Value& /*other*/)
            { m_result = 1; }
        virtual void visitNull()
            { m_result = -1; }
        virtual void visitError(const String_t& /*source*/, const String_t& /*str*/)
            { m_result = -1; }
        int get() const
            { return m_result; }
     private:
        int m_result;
    };
    V visi;
    visi.visit(value);
    return visi.get();
}

String_t
interpreter::toString(const afl::data::Value* value, bool readable)
{
    class ToStringVisitor : public afl::data::Visitor {
     public:
        ToStringVisitor(bool readable)
            : m_readable(readable),
              m_result()
            { }
        virtual void visitString(const String_t& str)
            {
                // ex IntStringValue::toString
                if (m_readable) {
                    m_result = quoteString(str);
                } else {
                    m_result = str;
                }
            }
        virtual void visitInteger(int32_t iv)
            {
                // ex IntIntIntValue::toString
                m_result = afl::string::Format("%d", iv);
            }
        virtual void visitFloat(double fv)
            {
                m_result = formatFloat(fv);
            }
        virtual void visitBoolean(bool bv)
            {
                // ex IntIntBoolValue::toString
                if (m_readable) {
                    m_result = bv ? "True" : "False";
                } else {
                    m_result = bv ? "YES" : "NO";
                }
            }
        virtual void visitHash(const afl::data::Hash& /*hv*/)
            {
                // ex IntHash::toString
                m_result = "#<hash>";
            }
        virtual void visitVector(const afl::data::Vector& /*vv*/)
            {
                // FIXME: does this appear in scripts? Our arrays behave differently (multi-dimension, callable, etc.)
                m_result = "#<vector>";
            }
        virtual void visitOther(const afl::data::Value& other)
            {
                // FIXME: relay to our value mix-in
                if (const BaseValue* bv = dynamic_cast<const BaseValue*>(&other)) {
                    m_result = bv->toString(m_readable);
                } else {
                    m_result = "#<unknown>";
                }
            }
        virtual void visitNull()
            {
                // ex IntValue::toStringOf
                if (m_readable) {
                    m_result = "Z(0)";
                }
            }
        virtual void visitError(const String_t& /*source*/, const String_t& str)
            {
                // FIXME: does this appear in scripts?
                throw Error(str);
            }
        bool m_readable;
        String_t m_result;
    };
    ToStringVisitor worker(readable);
    worker.visit(value);
    return worker.m_result;
}

/** Quote a string. Returns some piece of code that, when read, returns a string \c value. */
String_t
interpreter::quoteString(const String_t& value)
{
    // ex int/value.h:quoteString
    if (value.find_first_of("\"\\\n") == value.npos) {
        // No meta-characters, use unquoted double-quote string
        return "\"" + value + "\"";
    } else if (value.find_first_of("\'\n") == value.npos) {
        // Double-quotes or backslashes, but no apostrophes or newlines
        return "'" + value + "'";
    } else {
        // Sufficiently complicated, so add quotes.
        String_t output = "\"";
        String_t::size_type i = 0, j;
        while (((j = value.find_first_of("\"\\\n", i)) != value.npos)) {
            output.append(value, i, j-i);
            output.append(1, '\\');
            if (value[j] == '\n') {
                output.append(1, 'n');
            } else {
                output.append(value, j, 1);
            }
            i = j+1;
        }
        output.append(value, i, value.npos);
        output += "\"";
        return output;
    }
}

String_t
interpreter::formatFloat(double value)
{
    // ex IntFloatValue::toString
    String_t result = afl::string::Format("%f", value);
    if (result.find('.') != result.npos && result.find_first_of("Ee") == result.npos) {
        String_t::size_type cut = result.size();
        while (cut > 0 && result[cut-1] == '0')
            --cut;
        if (cut > 0 && result[cut-1] == '.')
            --cut;
        result.erase(cut);
    }
    return result;
}
