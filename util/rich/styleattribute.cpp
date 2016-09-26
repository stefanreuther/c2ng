/**
  *  \file util/rich/styleattribute.cpp
  */

#include "util/rich/styleattribute.hpp"

util::rich::StyleAttribute::StyleAttribute(Style style)
    : m_style(style)
{
    // RichTextStyleAttribute::RichTextStyleAttribute
}

util::rich::StyleAttribute::~StyleAttribute()
{ }

util::rich::StyleAttribute*
util::rich::StyleAttribute::clone() const
{
    return new StyleAttribute(m_style);
}
