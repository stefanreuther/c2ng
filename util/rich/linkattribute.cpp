/**
  *  \file util/rich/linkattribute.cpp
  */

#include "util/rich/linkattribute.hpp"

util::rich::LinkAttribute::LinkAttribute(String_t target)
    : m_target(target)
{
    // RichTextLinkAttribute::RichTextLinkAttribute
}

util::rich::LinkAttribute::~LinkAttribute()
{ }

util::rich::LinkAttribute*
util::rich::LinkAttribute::clone() const
{
    return new LinkAttribute(m_target);
}
