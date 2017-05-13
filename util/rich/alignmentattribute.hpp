/**
  *  \file util/rich/alignmentattribute.hpp
  *  \brief Class util::rich::AlignmentAttribute
  */
#ifndef C2NG_UTIL_RICH_ALIGNMENTATTRIBUTE_HPP
#define C2NG_UTIL_RICH_ALIGNMENTATTRIBUTE_HPP

#include "util/rich/attribute.hpp"

namespace util { namespace rich {

    /** Alignment attribute.
        The content of an AlignmentAttribute is aligned to fit within a box of the given width.
        This can be used to build tables. */
    class AlignmentAttribute : public Attribute {
     public:
        /** Constructor.
            \param width Width of box, in pixels
            \param alignment Alignment (0=left, 1=center, 2=right) */
        AlignmentAttribute(int width, int alignment);

        /** Destructor. */
        ~AlignmentAttribute();

        /** Clone.
            \return newly-allocated identical copy of this attribute */
        AlignmentAttribute* clone() const;

        /** Get width.
            \return width. */
        int getWidth() const;

        /** Get alignment.
            \return alignment */
        int getAlignment() const;
     private:
        const int m_width;
        const int m_alignment;
    };


} }

#endif
