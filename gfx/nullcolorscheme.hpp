/**
  *  \file gfx/nullcolorscheme.hpp
  */
#ifndef C2NG_GFX_NULLCOLORSCHEME_HPP
#define C2NG_GFX_NULLCOLORSCHEME_HPP

#include "gfx/colorscheme.hpp"

namespace gfx {

    class BaseContext;

    template<typename Index>
    class NullColorScheme : public ColorScheme<Index> {
     public:
        virtual ~NullColorScheme();

        virtual Color_t getColor(Index index);
        virtual void drawBackground(Canvas& can, const Rectangle& area);

        static NullColorScheme<Index> instance;
    };

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
