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
    out.tag = out.Tag_Blob;
    out.value = m_data.size();
    aux.handleFullData(toString(false), m_data);
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
