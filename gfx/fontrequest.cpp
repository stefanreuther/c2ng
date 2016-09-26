/**
  *  \file gfx/fontrequest.cpp
  */

#include "gfx/fontrequest.hpp"

namespace {
    bool matchValue(gfx::FontRequest::Value_t requested, gfx::FontRequest::Value_t provided)
    {
        // Two values match if both are present or equal (exact match)
        // or either is unknown (dont-care matches anything, anything matches dont-care).
        int16_t r, p;
        if (requested.get(r) && provided.get(p)) {
            return r == p;
        } else {
            return true;
        }
    }
}

gfx::FontRequest::FontRequest()
    : m_size(0),
      m_weight(0),
      m_slant(0),
      m_style(0)
{ }

// gfx::FontRequest::FontRequest(afl::base::NothingType)
//     : m_size(),
//       m_weight(),
//       m_slant(),
//       m_style()
// { }

gfx::FontRequest&
gfx::FontRequest::addSize(int n)
{
    int16_t value = 0;
    if (m_size.get(value)) {
        n += value;
    }
    m_size = n;
    return *this;
}

gfx::FontRequest&
gfx::FontRequest::addWeight(int n)
{
    int16_t value = 0;
    if (m_weight.get(value)) {
        n += value;
    }
    m_weight = n;
    return *this;
}


gfx::FontRequest&
gfx::FontRequest::setSize(Value_t n)
{
    m_size = n;
    return *this;
}

gfx::FontRequest&
gfx::FontRequest::setWeight(Value_t n)
{
    m_weight = n;
    return *this;
}

gfx::FontRequest&
gfx::FontRequest::setSlant(Value_t n)
{
    m_slant = n;
    return *this;
}

gfx::FontRequest&
gfx::FontRequest::setStyle(Value_t n)
{
    m_style = n;
    return *this;
}

gfx::FontRequest::Value_t
gfx::FontRequest::getSize() const
{
    return m_size;
}

gfx::FontRequest::Value_t
gfx::FontRequest::getWeight() const
{
    return m_weight;
}

gfx::FontRequest::Value_t
gfx::FontRequest::getSlant() const
{
    return m_slant;
}

gfx::FontRequest::Value_t
gfx::FontRequest::getStyle() const
{
    return m_style;
}

bool
gfx::FontRequest::match(const FontRequest& provided)
{
    return matchValue(m_size, provided.m_size)
        && matchValue(m_weight, provided.m_weight)
        && matchValue(m_slant, provided.m_slant)
        && matchValue(m_style, provided.m_style);
}
