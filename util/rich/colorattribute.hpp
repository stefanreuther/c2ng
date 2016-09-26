/**
  *  \file util/rich/colorattribute.hpp
  */
#ifndef C2NG_UTIL_RICH_COLORATTRIBUTE_HPP
#define C2NG_UTIL_RICH_COLORATTRIBUTE_HPP

#include "util/rich/attribute.hpp"
#include "util/skincolor.hpp"

namespace util { namespace rich {

    class ColorAttribute : public Attribute {
     public:
        ColorAttribute(SkinColor::Color color);
        ~ColorAttribute();
        ColorAttribute* clone() const;

        SkinColor::Color getColor() const
            { return m_color; }
     private:
        SkinColor::Color m_color;
    };

} }

#endif
