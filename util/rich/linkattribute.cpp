/**
  *  \file util/rich/linkattribute.cpp
  *  \brief Class util::rich::LinkAttribute
  */

#include "util/rich/linkattribute.hpp"

// Constructor.
util::rich::LinkAttribute::LinkAttribute(String_t target)
    : m_target(target)
{
    // RichTextLinkAttribute::RichTextLinkAttribute
}

// Destructor.
util::rich::LinkAttribute::~LinkAttribute()
{ }

// Clone.
util::rich::LinkAttribute*
util::rich::LinkAttribute::clone() const
{
    return new LinkAttribute(m_target);
}
