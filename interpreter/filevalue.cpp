/**
  *  \file interpreter/filevalue.cpp
  *  \brief Class interpreter::FileValue
  */

#include "interpreter/filevalue.hpp"
#include "afl/string/format.hpp"

// Constructor.
interpreter::FileValue::FileValue(int32_t fileNr)
    : m_fileNr(fileNr)
{ }

// Destructor.
interpreter::FileValue::~FileValue()
{ }

String_t
interpreter::FileValue::toString(bool /*readable*/) const
{
    // ex IntFileValue::toString
    return afl::string::Format("#%d", m_fileNr);
}

interpreter::FileValue*
interpreter::FileValue::clone() const
{
    // ex IntFileValue::clone
    return new FileValue(m_fileNr);
}

void
interpreter::FileValue::store(TagNode& out, afl::io::DataSink& /*aux*/, SaveContext& /*ctx*/) const
{
    // ex IntFileValue::store(IntTagNode& sv, Stream& /*aux*/)
    out.tag   = TagNode::Tag_FileHandle;
    out.value = m_fileNr;
}
