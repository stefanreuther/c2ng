/**
  *  \file util/rich/colorattribute.hpp
  *  \brief Class util::rich::ColorAttribute
  */
#ifndef C2NG_UTIL_RICH_COLORATTRIBUTE_HPP
#define C2NG_UTIL_RICH_COLORATTRIBUTE_HPP

#include "util/rich/attribute.hpp"
#include "util/skincolor.hpp"

namespace util { namespace rich {

    /** Color attribute.
        The content of a ColorAttribute is formatted in a given color. */
    class ColorAttribute : public Attribute {
     public:
        /** Constructor.
            \param color Color */
        explicit ColorAttribute(SkinColor::Color color);

        /** Destructor. */
        ~ColorAttribute();

        /** Clone.
            \return newly-allocated identical copy of this attribute */
        ColorAttribute* clone() const;

        /** Get color.
            \return color */
        SkinColor::Color getColor() const;
     private:
        const SkinColor::Color m_color;
    };

} }

// Get color.
inline util::SkinColor::Color
util::rich::ColorAttribute::getColor() const
{
    return m_color;
}

#endif
