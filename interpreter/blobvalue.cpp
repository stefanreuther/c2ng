/**
  *  \file interpreter/blobvalue.cpp
  *  \brief Class interpreter::BlobValue
  */

#include <memory>
#include "interpreter/blobvalue.hpp"
#include "afl/string/format.hpp"

interpreter::BlobValue::BlobValue()
    : BaseValue(),
      m_data()
{ }

interpreter::BlobValue::~BlobValue()
{ }

String_t
interpreter::BlobValue::toString(bool /*readable*/) const
{
    // ex IntBlobValue::toString
    return afl::string::Format("#<data:%d>", m_data.size());
}

void
interpreter::BlobValue::store(TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& /*cs*/, SaveContext& /*ctx*/) const
{
    // ex IntBlobValue::store
    // Limit to 4G to avoid messing up file formats.
    afl::base::ConstBytes_t data = m_data;
    if (sizeof(size_t) > sizeof(uint32_t)) {
        data.trim(0xFFFFFFFF);
    }

    out.tag = out.Tag_Blob;
    out.value = uint32_t(data.size());
    aux.handleFullData(data);
}

interpreter::BlobValue*
interpreter::BlobValue::clone() const
{
    // ex IntBlobValue::clone
    std::auto_ptr<BlobValue> result(new BlobValue());
    result->data().append(m_data);
    return result.release();
}

// BlobValue:
interpreter::BlobValue::Data_t&
interpreter::BlobValue::data()
{
    // ex IntBlobValue::getValue
    return m_data;
}
