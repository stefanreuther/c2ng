/**
  *  \file util/syntax/segment.cpp
  *  \brief Class util::syntax::Segment
  */

#include "util/syntax/segment.hpp"

// Default constructor.
util::syntax::Segment::Segment()
    : m_format(DefaultFormat),
      m_text(),
      m_link(),
      m_info()
{ }

// Make specific segment.
util::syntax::Segment::Segment(Format fmt, afl::string::ConstStringMemory_t text)
    : m_format(fmt),
      m_text(text),
      m_link(),
      m_info()
{ }

// Set content.
void
util::syntax::Segment::set(Format fmt, afl::string::ConstStringMemory_t text)
{
    // ex SyntaxHighlighter::Segment::set
    m_format = fmt;
    m_text = text;
    m_link.clear();
    m_info.clear();
}

// Start a segment.
void
util::syntax::Segment::start(afl::string::ConstStringMemory_t tail)
{
    // ex SyntaxHighlighter::Segment::start (sort-of)
    set(DefaultFormat, tail);
}

// Finish a segment.
void
util::syntax::Segment::finish(Format fmt, afl::string::ConstStringMemory_t tail)
{
    // ex SyntaxHighlighter::Segment::finish (sort-of)
    m_format = fmt;

    // Trim m_text such that tail is NOT contained in it.
    // For a valid call, this 'if' condition always holds.
    if (tail.size() <= m_text.size()) {
        m_text.trim(m_text.size() - tail.size());
    }
}

// Set associated link.
void
util::syntax::Segment::setLink(const String_t& link)
{
    m_link = link;
}

// Set associated information text.
void
util::syntax::Segment::setInfo(const String_t& info)
{
    m_info = info;
}

// Set format.
void
util::syntax::Segment::setFormat(Format fmt)
{
    m_format = fmt;
}

// Get format.
util::syntax::Format
util::syntax::Segment::getFormat() const
{
    return m_format;
}

// Get text content.
afl::string::ConstStringMemory_t
util::syntax::Segment::getText() const
{
    return m_text;
}

// Get associated link.
const String_t&
util::syntax::Segment::getLink() const
{
    return m_link;
}

// Get associated information text.
const String_t&
util::syntax::Segment::getInfo() const
{
    return m_info;
}
