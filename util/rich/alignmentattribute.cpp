/**
  *  \file util/rich/alignmentattribute.cpp
  */

#include "util/rich/alignmentattribute.hpp"

util::rich::AlignmentAttribute::AlignmentAttribute(int width, int alignment)
    : Attribute(),
      m_width(width),
      m_alignment(alignment)
{ }

util::rich::AlignmentAttribute::~AlignmentAttribute()
{ }

util::rich::AlignmentAttribute*
util::rich::AlignmentAttribute::clone() const
{
    return new AlignmentAttribute(m_width, m_alignment);
}

int
util::rich::AlignmentAttribute::getWidth() const
{
    return m_width;
}

int
util::rich::AlignmentAttribute::getAlignment() const
{
    return m_alignment;
}
