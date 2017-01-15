/**
  *  \file interpreter/vmio/valueloader.cpp
  *  \brief Class interpreter::vmio::ValueLoader
  */

#include <cstring>
#include <cmath>
#include "interpreter/vmio/valueloader.hpp"
#include "afl/base/growablememory.hpp"
#include "afl/bits/value.hpp"
#include "afl/bits/int16le.hpp"
#include "afl/bits/int32le.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/except/fileformatexception.hpp"
#include "util/translation.hpp"
#include "interpreter/values.hpp"
#include "interpreter/filevalue.hpp"
#include "interpreter/blobvalue.hpp"
#include "interpreter/vmio/loadcontext.hpp"
#include "interpreter/vmio/structures.hpp"
#include "util/io.hpp"

namespace {
    using interpreter::vmio::structures::Tag;

    afl::data::Value* failInvalidValue(afl::io::Stream& aux)
    {
        throw afl::except::FileFormatException(aux, _("Invalid value in data segment; file probably written by newer version of PCC"));
    }

    afl::data::Value* checkNull(afl::data::Value* value, afl::io::Stream& aux)
    {
        if (value == 0) {
            failInvalidValue(aux);
        }
        return value;
    }
}

// Constructor.
interpreter::vmio::ValueLoader::ValueLoader(afl::charset::Charset& cs, LoadContext& ctx)
    : m_charset(cs),
      m_context(ctx)
{ }


// Load data segment.
void
interpreter::vmio::ValueLoader::load(afl::data::Segment& data, afl::io::Stream& in, size_t firstIndex, size_t slots)
{
    // ex IntDataSegment::load
    afl::base::GrowableMemory<Tag> headers;
    headers.resize(slots);
    in.fullRead(headers.toBytes());

    // Read elements
    afl::base::Memory<const Tag> reader(headers);
    size_t slot = firstIndex;
    while (const Tag* p = reader.eat()) {
        // Generate node
        TagNode node;
        node.tag = p->packedTag;
        node.value = p->packedValue;
        data.setNew(slot, loadValue(node, in));
        ++slot;
    }
}

// Load single value.
afl::data::Value*
interpreter::vmio::ValueLoader::loadValue(const TagNode& tag, afl::io::Stream& aux)
{
    // ex IntValue::load
    switch (tag.tag) {
     case TagNode::Tag_Empty:
        return 0;

     case TagNode::Tag_Integer:
        return makeIntegerValue(tag.value);

     case TagNode::Tag_Boolean:
        return makeBooleanValue(tag.value != 0);

     case TagNode::Tag_String:
        return makeStringValue(loadPascalString(tag.value, aux));

     case TagNode::Tag_LongString:
        return makeStringValue(loadLongString(tag.value, aux));

     case TagNode::Tag_32bitFP:
        return makeFloatValue(loadFloat(tag.value));

     case TagNode::Tag_FPZero:
        return makeFloatValue(0.0);

     case TagNode::Tag_Blob:
        return makeBlobValue(tag.value, aux);

     case TagNode::Tag_FileHandle:
        return new FileValue(tag.value);

     case TagNode::Tag_BCO:
        return checkNull(m_context.loadBCO(tag.value), aux);

     case TagNode::Tag_Array:
        return checkNull(m_context.loadArray(tag.value), aux);

     case TagNode::Tag_Hash:
        return checkNull(m_context.loadHash(tag.value), aux);

     case TagNode::Tag_Struct:
        return checkNull(m_context.loadStructureValue(tag.value), aux);

     case TagNode::Tag_StructType:
        return checkNull(m_context.loadStructureType(tag.value), aux);

     default:
        if ((tag.tag & 0x00FF) == 0) {
            if (Context* ctx = m_context.loadContext(tag, aux)) {
                return ctx;
            } else {
                return failInvalidValue(aux);
            }
        } else {
            return makeFloatValue(loadFloat48(tag));
        }
    }
}

// Load a name table.
void
interpreter::vmio::ValueLoader::loadNames(afl::data::NameMap& names, afl::io::Stream& in, uint32_t n)
{
    // ex IntVariableNames::load
    while (n > 0) {
        names.add(util::loadPascalString(in, m_charset));
        --n;
    }
}

// Load Pascal string.
// A Pascal string consists of a length byte followed by the string data.
String_t
interpreter::vmio::ValueLoader::loadPascalString(uint32_t flag, afl::io::Stream& aux)
{
    // ex IntStringValue::IntStringValue (part), getPascalStringT
    if (flag != 0) {
        return util::loadPascalString(aux, m_charset);
    } else {
        return String_t();
    }
}

// Load a string of a given length.
String_t
interpreter::vmio::ValueLoader::loadLongString(uint32_t length, afl::io::Stream& aux)
{
    // ex IntStringValue::IntStringValue (part)
    afl::base::GrowableMemory<char> buffer;
    buffer.resize(length);
    aux.fullRead(buffer.toBytes());

    return m_charset.decode(buffer);
}

// Load 32-bit float.
double
interpreter::vmio::ValueLoader::loadFloat(uint32_t value)
{
    // ex IntFloatValue::IntFloatValue (part)
    // FIXME: portability. We now assume that floats have the same endianness as int32s.
    static_assert(sizeof(value) == sizeof(float), "sizeof float");
    float tmp;
    std::memcpy(&tmp, &value, sizeof(tmp));
    return tmp;
}

// Load 48-bit float.
double
interpreter::vmio::ValueLoader::loadFloat48(const TagNode& tag)
{
    // ex int/value.cc:getFP48

    // REAL format:                                      FLOAT format:
    //   8 bit exponent "e"                               23 bit mantissa "m"
    //  39 bit mantissa "m"                                8 bit exponent "e"
    //   1 bit sign "s"                                    1 bit sign "s"
    // Value is e=0 => 0.0                               e=0   => (-1)^s * (2^-126) * (0.m)
    //          e>0 => (-1)^s * 2^(e-129) * (1.m)        e=255 => Inf, NaN
    //                                                   sonst => (-1)^s * 2^(e-127) * (1.m)

    int e = tag.tag & 255;
    if (e == 0) {
        return 0.0;
    } else {
        int s    = (tag.value & 0x80000000U) ? -1 : +1;
        /* Generate the mantissa 1.m times 2**39 */
        double m = (tag.value & 0x7FFFFFFF) * 256.0 + (tag.tag >> 8) + 549755813888.0 /* 2**39 */;
        return s * std::ldexp(m, e - (129 + 39));
    }

#if 0
    // The following converts the REAL into a FLOAT by manipulating the bit
    // representation. It is less precise, as it generates only 23 mantissa
    // bits (i.e. loses 16 bits).
    uint32 e = tag.tag & 255;           // REAL exponent
    uint32 m = tag.value & 0x7FFFFFFF;  // REAL mantissa, 31 bits (ignoring 8 LSBs)
    uint32 s = tag.value >> 31;         // REAL sign

    // Convert to FLOAT mantissa. We have 31 bits, want 23, so shift by 8.
    m >>= 8;

    if (e <= 2) {
        // Result will be FLOAT zero
        return 0.0;
    } else {
        // Assemble new value
        uint32 nv = m | ((e - 2) << 23) | (s << 31);
        float fv;
        memcpy(&fv, &nv, sizeof(nv));
        return fv;
    }
#endif
}

// Load Blob value.
afl::data::Value*
interpreter::vmio::ValueLoader::makeBlobValue(uint32_t size, afl::io::Stream& aux)
{
    // ex IntBlobValue::IntBlobValue
    std::auto_ptr<BlobValue> p(new BlobValue());
    p->data().resize(size);
    aux.fullRead(p->data());
    return p.release();
}
