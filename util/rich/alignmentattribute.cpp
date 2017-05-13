/**
  *  \file util/rich/alignmentattribute.cpp
  *  \brief Class util::rich::AlignmentAttribute
  */

#include "util/rich/alignmentattribute.hpp"

// Constructor.
util::rich::AlignmentAttribute::AlignmentAttribute(int width, int alignment)
    : Attribute(),
      m_width(width),
      m_alignment(alignment)
{ }

// Destructor.
util::rich::AlignmentAttribute::~AlignmentAttribute()
{ }

// Clone.
util::rich::AlignmentAttribute*
util::rich::AlignmentAttribute::clone() const
{
    return new AlignmentAttribute(m_width, m_alignment);
}

// Get width.
int
util::rich::AlignmentAttribute::getWidth() const
{
    return m_width;
}

// Get alignment.
int
util::rich::AlignmentAttribute::getAlignment() const
{
    return m_alignment;
}
