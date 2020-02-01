/**
  *  \file gfx/fontrequest.cpp
  *  \brief Class gfx::FontRequest
  */

#include "gfx/fontrequest.hpp"
#include "afl/string/char.hpp"

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

    void parseString(gfx::FontRequest& req, afl::string::ConstStringMemory_t m)
    {
        while (const char* p = m.eat()) {
            switch (afl::string::charToLower(*p)) {
             case '+': req.addSize(+1);   break;
             case '-': req.addSize(-1);   break;
             case 'b': req.addWeight(+1); break;
             case 'l': req.addWeight(-1); break;
             case 'i': req.setSlant(1);   break;
             case 'u': req.setSlant(0);   break;
             case 'f': req.setStyle(1);   break;
             case 'p': req.setStyle(0);   break;
             default:                     break;
            }
        }
    }
}

// Default constructor.
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

gfx::FontRequest::FontRequest(const char* str)
    : m_size(0),
      m_weight(0),
      m_slant(0),
      m_style(0)
{
    parse(str);
}

gfx::FontRequest::FontRequest(const String_t& str)
    : m_size(0),
      m_weight(0),
      m_slant(0),
      m_style(0)
{
    parse(str);
}

// Add size.
gfx::FontRequest&
gfx::FontRequest::addSize(int n)
{
    m_size = static_cast<RawValue_t>(m_size.orElse(0) + n);
    return *this;
}

// Add weight.
gfx::FontRequest&
gfx::FontRequest::addWeight(int n)
{
    m_weight = static_cast<RawValue_t>(m_weight.orElse(0) + n);
    return *this;
}

// Set size.
gfx::FontRequest&
gfx::FontRequest::setSize(Value_t n)
{
    m_size = n;
    return *this;
}

// Set weight.
gfx::FontRequest&
gfx::FontRequest::setWeight(Value_t n)
{
    m_weight = n;
    return *this;
}

// Set slant.
gfx::FontRequest&
gfx::FontRequest::setSlant(Value_t n)
{
    m_slant = n;
    return *this;
}

// Set style.
gfx::FontRequest&
gfx::FontRequest::setStyle(Value_t n)
{
    m_style = n;
    return *this;
}

// Parse request string.
gfx::FontRequest&
gfx::FontRequest::parse(const char* str)
{
    parseString(*this, afl::string::toMemory(str));
    return *this;
}

// Parse request string.
gfx::FontRequest&
gfx::FontRequest::parse(const String_t& str)
{
    parseString(*this, afl::string::toMemory(str));
    return *this;
}

// Get size.
gfx::FontRequest::Value_t
gfx::FontRequest::getSize() const
{
    return m_size;
}

// Get weight.
gfx::FontRequest::Value_t
gfx::FontRequest::getWeight() const
{
    return m_weight;
}

// Get slant.
gfx::FontRequest::Value_t
gfx::FontRequest::getSlant() const
{
    return m_slant;
}

// Get style.
gfx::FontRequest::Value_t
gfx::FontRequest::getStyle() const
{
    return m_style;
}

// Match another FontRequest.
bool
gfx::FontRequest::match(const FontRequest& provided)
{
    return matchValue(m_size, provided.m_size)
        && matchValue(m_weight, provided.m_weight)
        && matchValue(m_slant, provided.m_slant)
        && matchValue(m_style, provided.m_style);
}

// Compare for equality.
bool
gfx::FontRequest::operator==(const FontRequest& other) const
{
    return m_size.isSame(other.m_size)
        && m_weight.isSame(other.m_weight)
        && m_slant.isSame(other.m_slant)
        && m_style.isSame(other.m_style);
}

// Compare for inequality.
bool
gfx::FontRequest::operator!=(const FontRequest& other) const
{
    return !operator==(other);
}
