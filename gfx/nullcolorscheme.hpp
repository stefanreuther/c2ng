/**
  *  \file gfx/nullcolorscheme.hpp
  *  \brief Class gfx::NullColorScheme
  */
#ifndef C2NG_GFX_NULLCOLORSCHEME_HPP
#define C2NG_GFX_NULLCOLORSCHEME_HPP

#include "gfx/colorscheme.hpp"

namespace gfx {

    class BaseContext;

    /** Null color scheme.
        This color scheme can be used as a place-holder when the actual color scheme is not known.
        It implements a 1:1 mapping between indexes and canvas colors.
        \tparam Index color index */
    template<typename Index>
    class NullColorScheme : public ColorScheme<Index> {
     public:
        /** Destructor. */
        virtual ~NullColorScheme();

        // ColorScheme:
        virtual Color_t getColor(Index index);
        virtual void drawBackground(Canvas& can, const Rectangle& area);

        /** Static global instance. */
        static NullColorScheme<Index> instance;
    };

    /** Draw background in color 0.
        This is the common implementation of NullColorScheme<T>::drawBackground.
        \param can Canvas
        \param area Area to fill */
    void drawNullBackground(Canvas& can, const Rectangle& area);

}

template<typename Index>
gfx::NullColorScheme<Index> gfx::NullColorScheme<Index>::instance;

template<typename Index>
gfx::NullColorScheme<Index>::~NullColorScheme()
{ }

template<typename Index>
gfx::Color_t
gfx::NullColorScheme<Index>::getColor(Index index)
{
    return Color_t(index);
}

template<typename Index>
void
gfx::NullColorScheme<Index>::drawBackground(Canvas& can, const Rectangle& area)
{
    drawNullBackground(can, area);
}

#endif
