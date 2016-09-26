/**
  *  \file interpreter/filevalue.cpp
  */

#include "interpreter/filevalue.hpp"
#include "afl/string/format.hpp"

interpreter::FileValue::FileValue(int32_t fileNr)
    : m_fileNr(fileNr)
{ }

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
interpreter::FileValue::store(TagNode& out, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, SaveContext* /*ctx*/) const
{
    // ex IntFileValue::store(IntTagNode& sv, Stream& /*aux*/)
    out.tag   = TagNode::Tag_FileHandle;
    out.value = m_fileNr;
}

// FIXME: port this (load)
// IntFileValue::IntFileValue(const IntTagNode& sv, Stream& /*aux*/)
//     : file_nr(sv.value)
// { }

