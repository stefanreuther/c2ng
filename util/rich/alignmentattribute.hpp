/**
  *  \file util/rich/alignmentattribute.hpp
  */
#ifndef C2NG_UTIL_RICH_ALIGNMENTATTRIBUTE_HPP
#define C2NG_UTIL_RICH_ALIGNMENTATTRIBUTE_HPP

#include "util/rich/attribute.hpp"

namespace util { namespace rich {

    class AlignmentAttribute : public Attribute {
     public:
        AlignmentAttribute(int width, int alignment);
        ~AlignmentAttribute();
        AlignmentAttribute* clone() const;

        int getWidth() const;
        int getAlignment() const;
     private:
        int m_width;
        int m_alignment;
    };


} }

#endif
