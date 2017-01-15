/**
  *  \file gfx/sdl/surface.hpp
  */
#ifndef C2NG_GFX_SDL_SURFACE_HPP
#define C2NG_GFX_SDL_SURFACE_HPP

#include <SDL_video.h>
#include "gfx/canvas.hpp"

namespace gfx { namespace sdl {

    /** SDL surface.
        This wraps an SDL_Surface into the Canvas interface. */
    class Surface : public Canvas {
     public:
        /** Use existing surface.
            We'll draw on the specified surface.
            If \c owned is true, the surface will be deleted when this object is destroyed.

            The surface must be unlocked; noone else must play with the lock state.

            This is used to get a drawing surface for the screen after SDL_SetVideoMode(). */
        Surface(SDL_Surface* surface, bool owned);

        /** Memory GfxSurface.
            Creates a surface with the specified pixel format and the specified dimensions. */
        Surface(int wi, int he, SDL_PixelFormat* format);
        ~Surface();

        virtual void drawHLine(const Point& pt, int npix, Color_t color, LinePattern_t pat, Alpha_t alpha);
        virtual void drawVLine(const Point& pt, int npix, Color_t color, LinePattern_t pat, Alpha_t alpha);
        virtual void drawPixel(const Point& pt, Color_t color, Alpha_t alpha);
        virtual void drawPixels(const Point& pt, afl::base::Memory<const Color_t> colors, Alpha_t alpha);
        virtual void drawBar(Rectangle rect, Color_t color, Color_t bg, const FillPattern& pat, Alpha_t alpha);
        virtual void blit(const Point& pt, Canvas& src, Rectangle rect);
        virtual void blitPattern(Rectangle rect, const Point& pt, int bytesPerLine, const uint8_t* data, Color_t color, Color_t bg, Alpha_t alpha);
        virtual Rectangle computeClipRect(Rectangle r);
        virtual void getPixels(Point pt, afl::base::Memory<Color_t> colors);
        virtual Point getSize();
        virtual int getBitsPerPixel();
        virtual bool isVisible(Rectangle r);
        virtual bool isClipped(Rectangle r);
        virtual void setPalette(Color_t start, afl::base::Memory<const ColorQuad_t> colorDefinitions, afl::base::Memory<Color_t> colorHandles);
        virtual void decodeColors(afl::base::Memory<const Color_t> colorHandles, afl::base::Memory<ColorQuad_t> colorDefinitions);
        virtual void encodeColors(afl::base::Memory<const ColorQuad_t> colorDefinitions, afl::base::Memory<Color_t> colorHandles);
        virtual afl::base::Ref<Canvas> convertCanvas(afl::base::Ref<Canvas> orig);

        void ensureLocked();
        void ensureUnlocked();

     private:
        SDL_Surface* m_surface;
        bool m_owned;
        bool m_locked;
        Rectangle m_updateRegion;
    };

} }

#endif
