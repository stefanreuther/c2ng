/**
  *  \file gfx/context.hpp
  *  \brief Template class gfx::Context
  */
#ifndef C2NG_GFX_CONTEXT_HPP
#define C2NG_GFX_CONTEXT_HPP

#include "gfx/basecontext.hpp"
#include "gfx/colorscheme.hpp"

namespace gfx {

    /** Graphics context (state).
        A graphics context stores state for complex graphics operations to reduce the number of per-call parameters.
        Context is templated upon a color index type and allows use of colors from a color scheme. */
    template<typename Index>
    class Context : public BaseContext {
     public:
        typedef Index Index_t;

        /** Constructor.
            \param canvas Canvas to use
            \param colorScheme Color scheme to use */
        Context(Canvas& canvas, ColorScheme<Index>& colorScheme);

        /** Set color.
            Set the color as provided by the color scheme.
            \param color Color index
            \return *this */
        Context& setColor(Index color);

        /** Use color scheme.
            \param colorScheme New color scheme to use
            \return *this */
        Context& useColorScheme(ColorScheme<Index>& colorScheme);

        /** Access color scheme.
            \return color scheme */
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
