/**
  *  \file interpreter/savevisitor.cpp
  */

#include <math.h>               // import into global namespace
#include "interpreter/savevisitor.hpp"
#include "interpreter/tagnode.hpp"
#include "interpreter/error.hpp"
#include "interpreter/basevalue.hpp"
#include "afl/io/internalsink.hpp"
#include "afl/bits/value.hpp"
#include "afl/bits/int16le.hpp"
#include "afl/bits/int32le.hpp"

/* Fallback implementation of isnan.
   If we have a macro, assume it works.
   Otherwise, define a function that defers to any other function definition in overload resolution. */
#ifndef isnan
namespace {
    struct Wrap {
        Wrap(double d)
            : d(d)
            { }
        double d;
    };
}
static inline bool isnan(Wrap d, ...)
{
    return d.d != d.d;
}
#endif


namespace {
    const char NAME[] = "VM Save";

    /** Store Pascal string into stream.
        \param s   Stream to write to from
        \param str String, should be <= 255 characters (or will be truncated) */
    void storePascalStringT(afl::io::DataSink& sink, const String_t& str)
    {
        const String_t name(NAME);
        uint8_t length = str.size();
        sink.handleFullData(name, afl::base::fromObject(length));
        sink.handleFullData(name, afl::string::toBytes(str).trim(length));
    }

    /** Store value in REAL (real48) format.
        REAL cannot store infinities, and has a smaller range than usual doubles.
        Those are stored as REAL-max.
        Underflows turn into REAL zeroes.
        REAL cannot store NaNs; those are converted into Tag_Empty.
        Although this does change the value in an observable way, it's probably the best we can do.
        The interpreter should try to avoid such values.
        \param value [in] Value
        \param sv [out] Stored value */
    void storeFP48(double value, interpreter::TagNode& sv)
    {
        // REAL format:
        //   8 bit exponent "e"
        //  39 bit mantissa "m"
        //   1 bit sign "s"
        // Value is e=0 => 0.0
        //          e>0 => (-1)^s * 2^(e-129) * (1.m)
        // Maximum REAL is 1.111111111111111111111111111111111111111 * 2^126
        //            i.e. 170141183460314489226776631181521715200
        //            i.e. 1.7e+38
        // We cannot store infinities and NaNs.
        // - convert infinities and overflows to max REAL
        // - convert underflows to 0.0
        // - convert NaNs to EMPTY (!)
        if (isnan(value)) {
            sv.tag   = sv.Tag_Empty;
            sv.value = 0;
            return;
        }

        // Find out sign
        uint32_t sign;
        if (value < 0) {
            sign  = 0x80000000U;
            value = -value;
        } else {
            sign = 0;
        }

        // First check for infinities or large values
        if (value > 1.0E+39) {
            sv.tag   = 0xFFFF;
            sv.value = 0x7FFFFFFF | sign;
        } else {
            // Reasonable value. Split into exponent and mantissa.
            int exp;
            double mant = frexp(value, &exp);
            exp -= 1;               // mant will be 0.1xxxxxxxxxxxxxx, we want 1.xxxxxxxxxx
            exp += 129;             // adjust for storage
            if (exp <= 0) {
                // Zero or underflow
                sv.tag   = 0;
                sv.value = 0;
            } else if (exp > 255) {
                // Overflow
                sv.tag   = 0xFFFF;
                sv.value = 0x7FFFFFFF | sign;
            } else {
                // It's a regular value. Extract mantissa bits.
                mant -= 0.5;           // 0.0yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyxxxxxxxx
                mant *= 4294967296.0;  // yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy.xxxxxxxx
                int32_t bits1 = int32_t(mant);
                mant -= bits1;         // 0.xxxxxxxx
                mant *= 256.0;
                int32_t bits2 = int32_t(mant);
                // Build result
                sv.tag   = exp | (256*bits2);
                sv.value = bits1 | sign;
            }
        }
    }

}

interpreter::SaveVisitor::SaveVisitor(TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, SaveContext* ctx)
    : m_out(out),
      m_aux(aux),
      m_charset(cs),
      m_pContext(ctx)
{ }

void
interpreter::SaveVisitor::visitString(const String_t& str)
{
    // ex IntStringValue::store
    String_t converted = m_charset.encode(afl::string::toMemory(str));
    if (converted.size() == 0) {
        // Empty string. Use Short String format, which is supported by PCC 1.0.8+.
        m_out.tag   = TagNode::Tag_String;
        m_out.value = 0;
    } else if (converted.size() <= 255 && m_pContext == 0) {
        // Use Short String format, which is supported by PCC 1.0.8+.
        // When we're saving a VM file (that is, when we have a SaveContext instance), we don't have to care for backward compatibility,
        // so let's save a few bytes by not using short string format there.
        m_out.tag   = TagNode::Tag_String;
        m_out.value = 0x11111111;
        storePascalStringT(m_aux, converted);
    } else {
        // Use Long String format, PCC 1.0.18+.
        m_out.tag   = TagNode::Tag_LongString;
        m_out.value = converted.size();
        m_aux.handleFullData(NAME, afl::string::toBytes(converted));
    }
}

void
interpreter::SaveVisitor::visitInteger(int32_t iv)
{
    // ex IntIntIntValue::store
    m_out.tag = TagNode::Tag_Integer;
    m_out.value = iv;
}

void
interpreter::SaveVisitor::visitFloat(double fv)
{
    // ex IntFloatValue::store
    if (fv == 0.0) {
        m_out.tag   = TagNode::Tag_FPZero;
        m_out.value = 0;
    } else {
        storeFP48(fv, m_out);
    }
}

void
interpreter::SaveVisitor::visitBoolean(bool bv)
{
    // ex IntIntBoolValue::store
    m_out.tag = TagNode::Tag_Boolean;
    m_out.value = bv;
}

void
interpreter::SaveVisitor::visitHash(const afl::data::Hash& /*hv*/)
{
    throw Error(Error::notSerializable());
}

void
interpreter::SaveVisitor::visitVector(const afl::data::Vector& /*vv*/)
{
    throw Error(Error::notSerializable());
}

void
interpreter::SaveVisitor::visitOther(const afl::data::Value& other)
{
    const BaseValue* bv = dynamic_cast<const BaseValue*>(&other);
    if (bv == 0) {
        throw Error(Error::notSerializable());
    }
    bv->store(m_out, m_aux, m_charset, m_pContext);
}

void
interpreter::SaveVisitor::visitNull()
{
    m_out.tag = TagNode::Tag_Empty;
    m_out.value = 0;
}

void
interpreter::SaveVisitor::visitError(const String_t& /*source*/, const String_t& /*str*/)
{
    throw Error(Error::notSerializable());
}

void
interpreter::SaveVisitor::save(afl::io::DataSink& out,
                               const afl::data::Segment& data, size_t slots,
                               afl::charset::Charset& cs, SaveContext* ctx)
{
    // ex IntDataSegment::save
    // Collect headers in one sink, aux data in another
    afl::io::InternalSink headers;
    afl::io::InternalSink aux;
    const String_t name(NAME);

    for (size_t i = 0; i < slots; ++i) {
        // Generate single entry
        TagNode node;
        SaveVisitor(node, aux, cs, ctx).visit(data[i]);

        // Serialize tag node
        afl::bits::Value<afl::bits::Int16LE> packedTag;
        afl::bits::Value<afl::bits::Int32LE> packedValue;
        packedTag = node.tag;
        packedValue = node.value;
        headers.handleFullData(name, packedTag.m_bytes);
        headers.handleFullData(name, packedValue.m_bytes);
    }

    // Generate output
    out.handleFullData(name, headers.getContent());
    out.handleFullData(name, aux.getContent());
}
