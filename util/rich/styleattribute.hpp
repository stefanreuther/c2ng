/**
  *  \file util/rich/styleattribute.hpp
  *  \brief Class util::rich::StyleAttribute
  */
#ifndef C2NG_UTIL_RICH_STYLEATTRIBUTE_HPP
#define C2NG_UTIL_RICH_STYLEATTRIBUTE_HPP

#include "util/rich/attribute.hpp"

namespace util { namespace rich {

    /** Style attribute.
        The content of this tag is formatted in a particular style. */
    class StyleAttribute : public Attribute {
     public:
        /** Style. */
        enum Style {
            Bold,               ///< Make bold text.
            Italic,             ///< Make italic text.
            Underline,          ///< Underline.
            Big,                ///< Make text bigger.
            Small,              ///< Make text smaller.
            Fixed,              ///< Use fixed-width font.
            Key                 ///< Format as key-caps.
        };

        /** Constructor.
            \param style Style */
        explicit StyleAttribute(Style style);

        /** Destructor. */
        ~StyleAttribute();

        /** Clone.
            \return newly-allocated identical copy of this attribute */
        StyleAttribute* clone() const;

        /** Get style.
            \return style */
        Style getStyle() const;
     private:
        const Style m_style;
    };

} }

inline util::rich::StyleAttribute::Style
util::rich::StyleAttribute::getStyle() const
{
    return m_style;
}

#endif
