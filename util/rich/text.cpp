/**
  *  \file util/rich/text.cpp
  */

#include "util/rich/text.hpp"
#include "util/rich/attribute.hpp"
#include "util/rich/visitor.hpp"
#include "util/rich/colorattribute.hpp"


util::rich::Text::Text()
    : m_text(),
      m_attributes()
{
    // ex RichText::RichText
}

util::rich::Text::Text(const char* text)
    : m_text(text),
      m_attributes()
{
    // ex RichText::RichText
}

util::rich::Text::Text(String_t text)
    : m_text(text),
      m_attributes()
{
    // ex RichText::RichText
}

// /** Construct colored text from C string.
//     \param color Color to assign to whole string
//     \param text  Text */
util::rich::Text::Text(SkinColor::Color color, const char* text)
    : m_text(text),
      m_attributes()
{
    withNewAttribute(new ColorAttribute(color));
}

// /** Construct colored text from C++ string.
//     \param color Color to assign to whole string
//     \param text  Text */
util::rich::Text::Text(SkinColor::Color color, String_t text)
    : RefCounted(),
      m_text(text),
      m_attributes()
{
    withNewAttribute(new ColorAttribute(color));
}

util::rich::Text::Text(const Text& other)
    : RefCounted(),
      m_text(other.m_text),
      m_attributes()
{
    // ex RichText::RichText
    m_attributes.reserve(other.m_attributes.size());
    for (size_t i = 0, n = other.m_attributes.size(); i < n; ++i) {
        Attribute* att = other.m_attributes[i]->clone();
        att->m_start = other.m_attributes[i]->m_start;
        att->m_end = other.m_attributes[i]->m_end;
        m_attributes.pushBackNew(att);
    }
}

util::rich::Text::Text(const Text& other, size_type start, size_type length)
    : m_text(other.m_text, start, length),
      m_attributes()
{
    // ex RichText::RichText
    const size_type end = length > String_t::npos - start ? String_t::npos : start + length;

    for (size_t i = 0; i < other.m_attributes.size(); ++i) {
        const Attribute* attr = other.m_attributes[i];

        // starting position of new attribute
        size_type nstart = attr->m_start > start ? attr->m_start : start;
        /* ending position of new attribute */
        size_type nend = attr->m_end < end ? attr->m_end : end;
        /* attribute effective in this section? */
        if (nstart < nend) {
            Attribute* nattr = attr->clone();
            nattr->m_start = nstart - start;
            nattr->m_end   = nend - start;
            m_attributes.pushBackNew(nattr);
        }
    }
}
        
util::rich::Text::~Text()
{ }

util::rich::Text&
util::rich::Text::withNewAttribute(Attribute* attr)
{
    // ex RichText::withAttribute
    if (!m_text.empty() && attr != 0) {
        attr->m_start = 0;
        attr->m_end   = m_text.size();
        m_attributes.insertNew(m_attributes.begin(), attr);
    } else {
        delete attr;
    }
    return *this;
}

util::rich::Text&
util::rich::Text::withColor(SkinColor::Color color)
{
    return withNewAttribute(new ColorAttribute(color));
}

util::rich::Text&
util::rich::Text::withStyle(StyleAttribute::Style style)
{
    // ex RichText::withAttribute
    return withNewAttribute(new StyleAttribute(style));
}

String_t
util::rich::Text::getText() const
{
    // ex RichText::getText
    return m_text;
}

// /** Get substring of a rich-text object.
//     \param start  first position
//     \param length number of characters to extract */
util::rich::Text
util::rich::Text::substr(size_type start, size_type length) const
{
    // ex RichText::substr
    return Text(*this, start, length);
}

// /** Erase part of a rich-text object. */
void
util::rich::Text::erase(size_type start, size_type length)
{
    // ex RichText::erase
    // FIXME: slow q+d solution
    if (start < m_text.size()) {
        if (length >= m_text.size() - start) {
            /* delete till eos */
            *this = Text(*this, 0, start);
        } else {
            /* delete piece from middle */
            Text tmp(*this, 0, start);
            tmp.append(Text(*this, start + length, String_t::npos));
            *this = tmp;
        }
    }
}

// /** Append other rich text object.
//     \param other [in] text to append
//     \return *this */
util::rich::Text&
util::rich::Text::append(const Text& other)
{
    // ex RichText::append
    size_t offset = m_text.size();
    m_text.append(other.m_text);
    for (size_t i = 0; i < other.m_attributes.size(); ++i) {
        Attribute* att = other.m_attributes[i]->clone();
        att->m_start = other.m_attributes[i]->m_start + offset;
        att->m_end   = other.m_attributes[i]->m_end   + offset;
        m_attributes.pushBackNew(att);
    }
    return *this;
}

// /** Append attribute-less text.
//     \param text [in] text to append
//     \return *this */
util::rich::Text&
util::rich::Text::append(const char* text)
{
    // ex RichText::append
    m_text.append(text);
    return *this;
}

// /** Append attribute-less text.
//     \param text [in] text to append
//     \return *this */
util::rich::Text&
util::rich::Text::append(String_t text)
{
    // ex RichText::append
    m_text.append(text);
    return *this;
}

// /** Append colored text.
//     \param color [in] color to use
//     \param text [in] text to append
//     \return *this */
util::rich::Text&
util::rich::Text::append(SkinColor::Color color, const char* text)
{
    // ex RichText::append
    size_type start = m_text.size();
    m_text.append(text);

    Attribute* att = new ColorAttribute(color);
    att->m_start = start;
    att->m_end = m_text.size();
    m_attributes.pushBackNew(att);
    return *this;
}

// /** Append colored text.
//     \param color [in] color to use
//     \param text [in] text to append
//     \return *this */
util::rich::Text&
util::rich::Text::append(SkinColor::Color color, String_t text)
{
    size_type start = m_text.size();
    m_text.append(text);

    Attribute* att = new ColorAttribute(color);
    att->m_start = start;
    att->m_end = m_text.size();
    m_attributes.pushBackNew(att);
    return *this;
}

// /** Assignment operator. */
util::rich::Text&
util::rich::Text::operator=(const Text& other)
{
    // ex RichText::operator=
    if (&other != this) {
        Text tmp(other);
        tmp.swap(*this);
    }
    return *this;
}

// /** Swap two rich-text objects. */
void
util::rich::Text::swap(Text& other)
{
    // ex RichText::swap
    m_text.swap(other.m_text);
    m_attributes.swap(other.m_attributes);
}

// /** Clear this rich-text object. */
void
util::rich::Text::clear()
{
    // ex RichText::clear
    m_text.clear();
    m_attributes.clear();
}

// /** Visit this text item. */
util::rich::Visitor&
util::rich::Text::visit(Visitor& visitor) const
{
    // ex RichText::visit
    size_type index = 0;
    std::vector<Attribute*> att_stack;
    size_t att_index = 0;

    while (index < m_text.size()) {
        size_type next_event = m_text.size();
        if (att_stack.size() && att_stack.back()->m_end < next_event) {
            next_event = att_stack.back()->m_end;
        }
        if (att_index < m_attributes.size() && m_attributes[att_index]->m_start < next_event) {
            next_event = m_attributes[att_index]->m_start;
        }

        if (next_event > index) {
            /* we have some text */
            if (!visitor.handleText(m_text.substr(index, next_event - index))) {
                goto out;
            }
        } else {
            /* leave some attributes */
            while (att_stack.size() && att_stack.back()->m_end <= index) {
                if (!visitor.endAttribute(*att_stack.back())) {
                    goto out;
                }
                att_stack.pop_back();
            }

            /* enter some attributes */
            while (att_index < m_attributes.size() && m_attributes[att_index]->m_start <= index) {
                if (!visitor.startAttribute(*m_attributes[att_index])) {
                    goto out;
                }
                att_stack.push_back(m_attributes[att_index]);
                ++att_index;
            }
        }

        index = next_event;
    }

    /* just to be sure... */
    while (att_stack.size()) {
        if (!visitor.endAttribute(*att_stack.back())) {
            goto out;
        }
        att_stack.pop_back();
    }

 out:
    return visitor;
}

util::rich::Text
util::rich::operator+(const Text& lhs, const Text& rhs)
{
    Text tmp(lhs);
    tmp += rhs;
    return tmp;
}

util::rich::Text
util::rich::operator+(const char* lhs, const Text& rhs)
{
    Text tmp(lhs);
    tmp += rhs;
    return tmp;
}

util::rich::Text
util::rich::operator+(const Text& lhs, const char* rhs)
{
    Text tmp(lhs);
    tmp += rhs;
    return tmp;
}
