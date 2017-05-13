/**
  *  \file util/rich/text.hpp
  *  \brief Class util::rich::Text
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
        This class is similar to a std::string, but in addition allows to associate arbitrary attributes with runs of characters.
        Attributes can be nested, but there must be a subset ordering on the ranges:
        if two ranges overlap, one must be a subset of the other.
        With this behaviour, Text is similar to XML tags.

        Attributes are descendants of Attribute.
        For convenience, ColorAttribute is explicitly supported by our constructors.

        In addition to method calls, you can use Visitor to inspect a Text object. */
    class Text : public afl::base::RefCounted {
     public:
        /** Type for a string index. */
        typedef String_t::size_type size_type;

        /*
         *  Construction
         */

        /** Construct blank object. */
        Text();

        /** Construct from C string.
            Creates unattributed text.
            \param text Text */
        Text(const char* text);

        /** Construct from C++ string.
            Creates unattributed text.
            \param text Text */
        Text(String_t text);

        /** Construct colored text from C string.
            \param color Color to assign to whole string
            \param text  Text */
        Text(SkinColor::Color color, const char* text);

        /** Construct colored text from C++ string.
            \param color Color to assign to whole string
            \param text  Text */
        Text(SkinColor::Color color, String_t text);

        /** Copy Constructor.
            \param text Other text */
        Text(const Text& other);

        /** Construct sub-string.
            \param other  Rich-text object to copy from
            \param start  Start at this position
            \param length Copy this many characters */
        Text(const Text& other, size_type start, size_type length = String_t::npos);

        /** Destructor. */
        ~Text();


        /*
         *  Operations
         */

        /** Apply attribute to whole text.
            \param attr attribute to apply. This Text object takes ownership of the Attribute object. On error, deletes the object.
            \return *this */
        Text& withNewAttribute(Attribute* attr);

        /** Apply color to whole text.
            \param color Color
            \return *this */
        Text& withColor(SkinColor::Color color);

        /** Apply style to whole text.
            \param style Style
            \return *this */
        Text& withStyle(StyleAttribute::Style style);

        /** Get raw text without attributes.
            \return text */
        String_t getText() const;

        /** Get number of attribute records.
            \return number of attribute records */
        size_t getNumAttributes() const;

        /** Get substring of a rich-text object.
            \param start  first position (0-based)
            \param length number of characters to extract
            \return substring */
        Text substr(size_type start, size_type length = String_t::npos) const;

        /** Erase part of a rich-text object.
            \param start First character to erase (0-based)
            \param length Number of characters to erase */
        void erase(size_type start, size_type length = String_t::npos);

        /** Find character in string.
            \param what Character to find
            \param startAt Start at this position (0-based)
            \return First position >= startAt containing the character, String_t::npos if not found */
        size_type find(char what, size_type startAt = 0) const;

        /** Append rich text.
            \param other Text to append
            \return *this */
        Text& append(const Text& other);

        /** Append C string.
            Appends the text with no attributes.
            \param text Text to append
            \return *this */
        Text& append(const char* text);

        /** Append C++ string.
            Appends the text with no attributes.
            \param text Text to append
            \return *this */
        Text& append(String_t text);

        /** Append colored C string.
            \param color Color
            \param text Text to append
            \return *this */
        Text& append(SkinColor::Color color, const char* text);

        /** Append colored C++ string.
            \param color Color
            \param text Text to append
            \return *this */
        Text& append(SkinColor::Color color, String_t text);

        /** Assignment operator.
            \param other Source
            \return *this */
        Text& operator=(const Text& other);

        /** Append rich text.
            \param other Text to append
            \return *this */
        Text& operator+=(const Text& other);

        /** Append C string.
            \param other Text to append
            \return *this */
        Text& operator+=(const char* other);

        /** Append C++ string.
            \param other Text to append
            \return *this */
        Text& operator+=(String_t text);

        /** Get length.
            \return length in bytes */
        size_type size() const;

        /** Get length.
            \return length in bytes */
        size_type length() const;

        /** Check emptiness.
            \return true if this string is empty (=length is zero) */
        bool empty() const;

        /** Character access.
            For indexes within the string [0,size()), returns the character.
            For one-past-the-end, returns '\0'.
            \param i Index [0,size()]
            \return character */
        char operator[](size_type i) const;

        /** Swap two rich-text objects.
            \param other [in/out] Other object */
        void swap(Text& other);

        /** Clear this rich-text object.
            Resets the object to the same state it had after default-constructon. */
        void clear();

        /** Visit this text.
            Calls visitor's methods to describe this Text's content.
            \param visitor Visitor
            \return visitor */
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

inline size_t
util::rich::Text::getNumAttributes() const
{
    return m_attributes.size();
}

inline util::rich::Text::size_type
util::rich::Text::find(char what, size_type startAt) const
{
    return m_text.find(what, startAt);
}

inline util::rich::Text&
util::rich::Text::operator+=(const Text& other)
{
    return append(other);
}

inline util::rich::Text&
util::rich::Text::operator+=(const char* other)
{
    return append(other);
}

inline util::rich::Text&
util::rich::Text::operator+=(String_t text)
{
    return append(text);
}

inline util::rich::Text::size_type
util::rich::Text::size() const
{
    return m_text.size();
}

inline
util::rich::Text::size_type
util::rich::Text::length() const
{
    return m_text.size();
}

inline bool
util::rich::Text::empty() const
{
    return m_text.empty();
}

inline char
util::rich::Text::operator[](size_type i) const
{
    // 'std::string::operator[] const' is guaranteed to return '\0' in C++98.
    // In C++14, it is also guaranteed to return '\0' for the non-const signature.
    return m_text[i];
}

#endif
