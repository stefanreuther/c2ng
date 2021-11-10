/**
  *  \file interpreter/savevisitor.cpp
  */

#include <algorithm>
#include <math.h>               // import into global namespace
#include "interpreter/savevisitor.hpp"
#include "afl/bits/int16le.hpp"
#include "afl/bits/int32le.hpp"
#include "afl/bits/value.hpp"
#include "afl/io/internalsink.hpp"
#include "interpreter/basevalue.hpp"
#include "interpreter/context.hpp"
#include "interpreter/error.hpp"
#include "interpreter/tagnode.hpp"
#include "util/io.hpp"

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

        // This line throws a conversion warning with g++ 4.9, glibc 2.19.
        // Internally, isnan is defined as if-float-then-__isnanf-else__isnan,
        // which makes the compiler see a double-to-float conversion even if it is not used.
        // We could get out the big guns (#define __isnanf __isnan), but for now, just live with it.
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
                sv.tag   = uint16_t(exp | (256*bits2));
                sv.value = bits1 | sign;
            }
        }
    }

}

interpreter::SaveVisitor::SaveVisitor(TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, SaveContext& ctx)
    : m_out(out),
      m_aux(aux),
      m_charset(cs),
      m_context(ctx)
{ }

void
interpreter::SaveVisitor::visitString(const String_t& str)
{
    // ex IntStringValue::store
    afl::base::GrowableBytes_t converted = m_charset.encode(afl::string::toMemory(str));

    // In theory, a script could build a 10G string. We can only save 4G max,
    // Given that it's unlikely that anyone ever successfully does this, and that PCC1 truncates
    // to 256 without comment, let's truncate here as well.
    // PCC 1.x also causes strings >2G to be misinterpreted. Thus, truncate at 2G.
    // To all those guys with your 128G RAM, keep building big strings,
    // but you won't cause bad file formats to be written :)
    converted.trim(0x7FFFFFFF);

    // \change We now always use Long String format.
    // PCC2 would have tried to use Short String format (Tag_String, PCC 1.0.8, January 2001) when saving a chart.cc file.
    // We don't know here whether we're saving a chart.cc file.
    // However, since all versions of PCC2 since 1.0.18 (April 2002) can read Long String format, let's keep the code simple.
    m_out.tag   = TagNode::Tag_LongString;
    m_out.value = uint32_t(converted.size());
    m_aux.handleFullData(converted);
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
    bv->store(m_out, m_aux, m_context);
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
                               afl::charset::Charset& cs, SaveContext& ctx)
{
    // ex IntDataSegment::save
    // ex ccexpr.pas:CVariables.SaveToStream
    // Collect headers in one sink, aux data in another
    afl::io::InternalSink headers;
    afl::io::InternalSink aux;

    for (size_t i = 0; i < slots; ++i) {
        // Generate single entry
        TagNode node;
        SaveVisitor(node, aux, cs, ctx).visit(data[i]);

        // Serialize tag node
        afl::bits::Value<afl::bits::Int16LE> packedTag;
        afl::bits::Value<afl::bits::Int32LE> packedValue;
        packedTag = node.tag;
        packedValue = node.value;
        headers.handleFullData(packedTag.m_bytes);
        headers.handleFullData(packedValue.m_bytes);
    }

    // Generate output
    out.handleFullData(headers.getContent());
    out.handleFullData(aux.getContent());
}

// Save contexts.
void
interpreter::SaveVisitor::saveContexts(afl::io::DataSink& out,
                                       const afl::container::PtrVector<interpreter::Context>& contexts,
                                       SaveContext& ctx)
{
    // ex int/contextio.h:saveContexts
    // Collect headers in one sink, aux data in another
    afl::io::InternalSink headers;
    afl::io::InternalSink aux;

    for (size_t i = 0, n = contexts.size(); i < n; ++i) {
        // Generate single entry
        TagNode node;
        contexts[i]->store(node, aux, ctx);

        // Serialize tag node
        afl::bits::Value<afl::bits::Int16LE> packedTag;
        afl::bits::Value<afl::bits::Int32LE> packedValue;
        packedTag = node.tag;
        packedValue = node.value;
        headers.handleFullData(packedTag.m_bytes);
        headers.handleFullData(packedValue.m_bytes);
    }

    // Generate output
    out.handleFullData(headers.getContent());
    out.handleFullData(aux.getContent());
}

// Save name list.
void
interpreter::SaveVisitor::saveNames(afl::io::DataSink& out, const afl::data::NameMap& names, size_t slots, afl::charset::Charset& cs)
{
    // ex IntVariableNames::save
    // ex ccexpr.pas:CNameList.SaveToStream
    size_t todo = std::min(slots, names.getNumNames());
    for (size_t i = 0; i < todo; ++i) {
        util::storePascalStringTruncate(out, names.getNameByIndex(i), cs);
    }
    for (size_t i = todo; i < slots; ++i) {
        util::storePascalStringTruncate(out, String_t(), cs);
    }
}
