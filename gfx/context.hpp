/**
  *  \file gfx/context.hpp
  */
#ifndef C2NG_GFX_CONTEXT_HPP
#define C2NG_GFX_CONTEXT_HPP

#include "gfx/basecontext.hpp"
#include "gfx/colorscheme.hpp"

namespace gfx {

    template<typename Index>
    class Context : public BaseContext {
     public:
        typedef Index Index_t;

        Context(Canvas& canvas, ColorScheme<Index>& colorScheme);

        Context& setColor(Index color);
        Context& useColorScheme(ColorScheme<Index>& colorScheme);
        ColorScheme<Index>& colorScheme() const;

     private:
        ColorScheme<Index>* m_colorScheme;
    };

}

template<typename Index>
inline
gfx::Context<Index>::Context(Canvas& canvas, ColorScheme<Index>& colorScheme)
    : BaseContext(canvas),
      m_colorScheme(&colorScheme)
{ }

template<typename Index>
gfx::Context<Index>&
gfx::Context<Index>::setColor(Index color)
{
    // ex GfxContext::setColor
    setRawColor(colorScheme().getColor(color));
    return *this;
}

template<typename Index>
inline gfx::Context<Index>&
gfx::Context<Index>::useColorScheme(ColorScheme<Index>& colorScheme)
{
    // ex GfxContext::useColorScheme
    m_colorScheme = &colorScheme;
    return *this;
}

template<typename Index>
inline gfx::ColorScheme<Index>&
gfx::Context<Index>::colorScheme() const
{
    // ex GfxContext::getColorScheme
    return *m_colorScheme;
}
#endif
