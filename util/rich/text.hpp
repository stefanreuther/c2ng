/**
  *  \file util/rich/text.hpp
  */
#ifndef C2NG_UTIL_RICH_TEXT_HPP
#define C2NG_UTIL_RICH_TEXT_HPP

#include "afl/string/string.hpp"
#include "afl/container/ptrvector.hpp"
#include "util/skincolor.hpp"
#include "util/rich/styleattribute.hpp"
#include "afl/base/refcounted.hpp"

namespace util { namespace rich {

    class Attribute;
    class Visitor;

    /** Text with Attributes.
        This class is similar to a std::string, but in addition allows to
        associate arbitrary attributes with runs of characters. Attributes
        can be nested, but there must be a subset ordering on the ranges:
        if two ranges overlap, one must be a subset of the other. With
        this behaviour, RichText is similar to XML tags.

        Attributes are descendants of RichTextAttribute. For convenience,
        a RichTextColorAttribute is provided and explicitly supported by
        our constructors.

        In addition to method calls, you can use RichTextVisitor to
        inspect a RichText object. */
    class Text : public afl::base::RefCounted {
     public:
        typedef String_t::size_type size_type;

        /** Construct blank object. */
        Text();

        /** Construct from C string. Creates unattributed text. */
        Text(const char* text);

        /** Construct from C++ string. Creates unattributed text. */
        Text(String_t text);
        Text(SkinColor::Color color, const char* text);
        Text(SkinColor::Color color, String_t text);

        /** Copy Constructor. */
        Text(const Text& other);

        /** Construct sub-string.
            \param other  Rich-text object to copy from
            \param start  Start at this position
            \param length Copy this many characters */
        Text(const Text& other, size_type start, size_type length = String_t::npos);
        
        ~Text();

        /** Apply attribute to whole text.
            \param attr attribute to apply. This object takes ownership of the object. On error, deletes the object.
            \return *this */
        Text& withNewAttribute(Attribute* attr);
        Text& withColor(SkinColor::Color color);
        Text& withStyle(StyleAttribute::Style style);
//     RichText& withAttribute(RichTextStyleAttribute::Style style);

        /** Get raw text without attributes. */
        String_t getText() const;

        /** Get number of attribute records. */
        size_t getNumAttributes() const
            { return m_attributes.size(); }

        Text substr(size_type start, size_type length = String_t::npos) const;
        void erase(size_type start, size_type length = String_t::npos);

        size_type find(char what, size_type start_at = 0) const
            { return m_text.find(what, start_at); }

        Text& append(const Text& other);
        Text& append(const char* text);
        Text& append(String_t text);
        Text& append(SkinColor::Color color, const char* text);
        Text& append(SkinColor::Color color, String_t text);

        Text& operator=(const Text& other);
        Text& operator+=(const Text& other)
            { return append(other); }
        Text& operator+=(const char* other)
            { return append(other); }
        Text& operator+=(String_t text)
            { return append(text); }

        size_type size() const
            { return m_text.size(); }
        size_type length() const
            { return m_text.size(); }
        bool empty() const
            { return m_text.empty(); }
        char operator[](size_type i) const
            { return m_text[i]; }

        void swap(Text& other);
        void clear();

        Visitor& visit(Visitor& visitor) const;

     private:
        /** The raw text. */
        String_t m_text;

        /** All attributes, sorted by start then end. Note that attributes cannot overlap. */
        afl::container::PtrVector<Attribute> m_attributes;
        };

    Text operator+(const Text& lhs, const Text& rhs);
    Text operator+(const char* lhs, const Text& rhs);
    Text operator+(const Text& lhs, const char* rhs);

} }

#endif
