/**
  *  \file util/syntax/nullhighlighter.cpp
  */

#include "util/syntax/nullhighlighter.hpp"
#include "util/syntax/segment.hpp"

util::syntax::NullHighlighter::NullHighlighter()
    : m_text()
{
    // ex NullSyntaxHighlighter::NullSyntaxHighlighter
}
util::syntax::NullHighlighter::~NullHighlighter()
{ }


void
util::syntax::NullHighlighter::init(afl::string::ConstStringMemory_t text)
{
    // ex NullSyntaxHighlighter::init
    m_text = text;
}

bool
util::syntax::NullHighlighter::scan(Segment& result)
{
    // ex NullSyntaxHighlighter::scan
    if (!m_text.empty()) {
        result.set(DefaultFormat, m_text);
        m_text.reset();
        return true;
    } else {
        return false;
    }
}
