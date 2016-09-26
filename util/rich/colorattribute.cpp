/**
  *  \file util/rich/colorattribute.cpp
  */

#include "util/rich/colorattribute.hpp"

util::rich::ColorAttribute::ColorAttribute(SkinColor::Color color)
    : m_color(color)
{ }

util::rich::ColorAttribute::~ColorAttribute()
{ }

util::rich::ColorAttribute*
util::rich::ColorAttribute::clone() const
{
    // ex RichTextColorAttribute::clone
    return new ColorAttribute(m_color);
}

