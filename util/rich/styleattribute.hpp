/**
  *  \file util/rich/styleattribute.hpp
  */
#ifndef C2NG_UTIL_RICH_STYLEATTRIBUTE_HPP
#define C2NG_UTIL_RICH_STYLEATTRIBUTE_HPP

#include "util/rich/attribute.hpp"

namespace util { namespace rich {

    class StyleAttribute : public Attribute {
     public:
        enum Style {
            Bold,
            Italic,
            Underline,
            Big,
            Small,
            Fixed,
            Key
        };
        StyleAttribute(Style style);
        ~StyleAttribute();
        StyleAttribute* clone() const;
        Style getStyle() const
            { return m_style; }
     private:
        Style m_style;
    };

} }

#endif
