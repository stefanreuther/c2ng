/**
  *  \file util/rich/colorattribute.cpp
  *  \brief Class util::rich::ColorAttribute
  */

#include "util/rich/colorattribute.hpp"

// Constructor.
util::rich::ColorAttribute::ColorAttribute(SkinColor::Color color)
    : m_color(color)
{ }

// Destructor.
util::rich::ColorAttribute::~ColorAttribute()
{ }

// Clone.
util::rich::ColorAttribute*
util::rich::ColorAttribute::clone() const
{
    // ex RichTextColorAttribute::clone
    return new ColorAttribute(m_color);
}

