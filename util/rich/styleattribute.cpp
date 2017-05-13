/**
  *  \file util/rich/styleattribute.cpp
  *  \brief Class util::rich::StyleAttribute
  */

#include "util/rich/styleattribute.hpp"

// Constructor.
util::rich::StyleAttribute::StyleAttribute(Style style)
    : m_style(style)
{
    // RichTextStyleAttribute::RichTextStyleAttribute
}

// Destructor.
util::rich::StyleAttribute::~StyleAttribute()
{ }

// Clone.
util::rich::StyleAttribute*
util::rich::StyleAttribute::clone() const
{
    return new StyleAttribute(m_style);
}
