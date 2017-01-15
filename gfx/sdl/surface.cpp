/**
  *  \file gfx/sdl/surface.cpp
  */
#include "config.h"
#ifdef HAVE_SDL
# include <stdexcept>
# include "gfx/sdl/surface.hpp"
# include "afl/base/staticassert.hpp"
# include "afl/base/growablememory.hpp"
# include "gfx/graphicsexception.hpp"
# include "gfx/primitives.hpp"
# include "gfx/sdl/modetraits.hpp"

static_assert(SDL_ALPHA_OPAQUE == gfx::OPAQUE_ALPHA, "opaque polarity");


gfx::sdl::Surface::Surface(SDL_Surface* surface, bool owned)
    : m_surface(surface),
      m_owned(owned),
      m_locked(false),
      m_updateRegion()
{ }

gfx::sdl::Surface::Surface(int wi, int he, SDL_PixelFormat* format)
    : m_surface(SDL_CreateRGBSurface(SDL_SWSURFACE, wi, he,
                                     format->BitsPerPixel,
                                     format->Rmask, format->Gmask,
                                     format->Bmask, format->Amask)),
      m_owned(true),
      m_locked(false),
      m_updateRegion()
{
    if (!m_surface) {
        throw GraphicsException(SDL_GetError());
    }
}

gfx::sdl::Surface::~Surface()
{
    if (m_owned && m_surface != 0) {
        SDL_FreeSurface(m_surface);
    }
}

void
gfx::sdl::Surface::drawHLine(const Point& pt, int npix, Color_t color, LinePattern_t pat, Alpha_t alpha)
{
    // clipping
    int x1 = pt.getX();
    int y1 = pt.getY();
    int x2 = x1 + npix;
    if (y1 < 0 || y1 >= m_surface->h) {
        return;
    }
    if (x1 < 0) {
        x1 = 0;
    }
    if (x2 > m_surface->w) {
        x2 = m_surface->w;
    }
    if (x1 >= x2) {
        return;
    }

    ensureLocked();
    GFX_MODE_SWITCH(m_surface, doHLine(x1, y1, x2, color, pat, alpha));
    m_updateRegion.include(Rectangle(x1, y1, x2-x1, 1));
}

void
gfx::sdl::Surface::drawVLine(const Point& pt, int npix, Color_t color, LinePattern_t pat, Alpha_t alpha)
{
    // clipping
    int x1 = pt.getX();
    int y1 = pt.getY();
    int y2 = y1 + npix;
    if (x1 < 0 || x1 >= m_surface->w)
        return;
    if (y1 < 0)
        y1 = 0;
    if (y2 > m_surface->h)
        y2 = m_surface->h;
    if (y1 >= y2)
        return;

    ensureLocked();
    GFX_MODE_SWITCH(m_surface, doVLine(x1, y1, y2, color, pat, alpha));
    m_updateRegion.include(Rectangle(x1, y1, 1, y2-y1));
}

// FIXME: retire
void
gfx::sdl::Surface::drawPixel(const Point& pt, Color_t color, Alpha_t alpha)
{
    if (pt.getX() < 0 || pt.getX() >= m_surface->w || pt.getY() < 0 || pt.getY() >= m_surface->h) {
        return;
    }

    ensureLocked();
    Color_t c[1] = {color};
    GFX_MODE_SWITCH(m_surface, writePixels(pt.getX(), pt.getY(), c, alpha));
    m_updateRegion.include(Rectangle(pt, Point(1, 1)));
}

void
gfx::sdl::Surface::drawPixels(const Point& pt, afl::base::Memory<const Color_t> colors, Alpha_t alpha)
{
    int x = pt.getX();
    int y = pt.getY();
    if (y >= 0 && y < m_surface->h) {
        // Trim left side
        if (x < 0) {
            colors.split(-x);
            x = 0;
        }

        // Trim right side
        if (x < m_surface->w) {
            colors.trim(m_surface->w - x);
            if (!colors.empty()) {
                ensureLocked();
                GFX_MODE_SWITCH(m_surface, writePixels(x, y, colors, alpha));
                m_updateRegion.include(Rectangle(x, y, colors.size(), 1));
            }
        }
    }
}

void
gfx::sdl::Surface::drawBar(Rectangle rect, Color_t color, Color_t bg, const FillPattern& pat, Alpha_t alpha)
{
    rect.intersect(Rectangle(0, 0, m_surface->w, m_surface->h));
    if (!rect.exists()) {
        return;
    }

    ensureLocked();
    if (alpha == OPAQUE_ALPHA && pat.isBlack()) {
        SDL_Rect r;
        r.x = rect.getLeftX();
        r.y = rect.getTopY();
        r.w = rect.getWidth();
        r.h = rect.getHeight();
        SDL_FillRect(m_surface, &r, color);
    } else {
        GFX_MODE_SWITCH(m_surface, doBar(rect, color, bg, pat, alpha));
    }
    m_updateRegion.include(rect);
}

void
gfx::sdl::Surface::blit(const Point& pt, Canvas& src, Rectangle rect)
{
    if (Surface* s = dynamic_cast<Surface*>(&src)) {
        SDL_Rect dstrect;
        dstrect.x = pt.getX() + rect.getLeftX();
        dstrect.y = pt.getY() + rect.getTopY();
        dstrect.w = 0;
        dstrect.h = 0;

        SDL_Rect srcrect;
        srcrect.x = rect.getLeftX();
        srcrect.y = rect.getTopY();
        srcrect.w = rect.getWidth();
        srcrect.h = rect.getHeight();

        ensureUnlocked();
        SDL_BlitSurface(s->m_surface, &srcrect, m_surface, &dstrect);
    } else {
        defaultBlit(pt, src, rect);
    }
    m_updateRegion.include(Rectangle(pt.getX() + rect.getLeftX(),
                                     pt.getY() + rect.getTopY(),
                                     rect.getWidth(),
                                     rect.getHeight()));
}

void
gfx::sdl::Surface::blitPattern(Rectangle rect, const Point& pt, int bytesPerLine, const uint8_t* data, Color_t color, Color_t bg, Alpha_t alpha)
{
    // FIXME: if the rect has negative x/y, this will fail
    rect.intersect(Rectangle(0, 0, m_surface->w, m_surface->h));
    if (rect.exists()) {
        ensureLocked();
        GFX_MODE_SWITCH(m_surface, doBlitPattern(rect, pt, bytesPerLine, data, color, bg, alpha));
        m_updateRegion.include(rect);
    }
}

gfx::Rectangle
gfx::sdl::Surface::computeClipRect(Rectangle r)
{
    r.intersect(Rectangle(0, 0, m_surface->w, m_surface->h));
    return r;
}

void
gfx::sdl::Surface::getPixels(Point pt, afl::base::Memory<Color_t> colors)
{
    int x = pt.getX();
    int y = pt.getY();
    if (y >= 0 && y < m_surface->h) {
        // Handle initial out-of-range portion
        if (x < 0) {
            colors.split(-x).fill(0);
            x = 0;
        }

        // Read pixels
        if (x < m_surface->w) {
            afl::base::Memory<Color_t> actual = colors.split(m_surface->w - x);
            if (!actual.empty()) {
                ensureLocked();
                GFX_MODE_SWITCH(m_surface, readPixels(x, y, actual));
            }
        }

        // Fill remainder
        colors.fill(0);
    } else {
        colors.fill(0);
    }
}

gfx::Point
gfx::sdl::Surface::getSize()
{
    return Point(m_surface->w, m_surface->h);
}

int
gfx::sdl::Surface::getBitsPerPixel()
{
    return m_surface->format->BitsPerPixel;
}

bool
gfx::sdl::Surface::isVisible(Rectangle r)
{
    return computeClipRect(r).exists();
}

bool
gfx::sdl::Surface::isClipped(Rectangle r)
{
    return computeClipRect(r) == r;
}

void
gfx::sdl::Surface::setPalette(Color_t start, afl::base::Memory<const ColorQuad_t> colorDefinitions, afl::base::Memory<Color_t> colorHandles)
{
    // ensureUnlocked(); -- FIXME: why?
    if (m_surface->format->BytesPerPixel == 1) {
        // Palettized surface.
        // Fetch color key. Everything that has an alpha of 0 will be converted to color key.
        bool haveColorKey = false;
        uint32_t colorKey = 0;
        if ((m_surface->flags & SDL_SRCCOLORKEY) != 0) {
            // Surface already has a color key.
            // Accept that only if it is outside range we're configuring, to allow resetting color key by configuring an opaque color.
            colorKey = m_surface->format->colorkey;
            if (colorKey < start || colorKey >= start + colorDefinitions.size()) {
                haveColorKey = true;
            }
        }

        // Make sure we don't do stupid things.
        colorDefinitions.trim(256);

        // Convert.
        Color_t nr = start;
        afl::base::GrowableMemory<SDL_Color> palette;
        palette.ensureCapacity(colorDefinitions.size());
        while (const Color_t* in = colorDefinitions.eat()) {
            // Convert RGB
            SDL_Color c;
            c.r = RED_FROM_COLORQUAD(*in);
            c.g = GREEN_FROM_COLORQUAD(*in);
            c.b = BLUE_FROM_COLORQUAD(*in);
            c.unused = 0;
            palette.append(c);

            // Check color key
            if (ALPHA_FROM_COLORQUAD(*in) == TRANSPARENT_ALPHA) {
                // Color key candidate
                if (!haveColorKey) {
                    haveColorKey = true;
                    colorKey = nr;
                }
                if (Color_t* out = colorHandles.eat()) {
                    *out = colorKey;
                }
            } else {
                if (Color_t* out = colorHandles.eat()) {
                    *out = nr;
                }
            }
            ++nr;
        }

        // Update palette
        if (!palette.empty()) {
            SDL_SetPalette(m_surface, SDL_PHYSPAL|SDL_LOGPAL, palette.at(0), start, palette.size());
        }

        // Update color key
        if (haveColorKey) {
            SDL_SetColorKey(m_surface, SDL_SRCCOLORKEY, colorKey);
        } else {
            SDL_SetColorKey(m_surface, 0, 0);
        }

        // Flush output
        colorHandles.fill(0);
    } else {
        // RGB(A) surface.
        // FIXME: handle color keying for 16/24 bit
        while (Color_t* out = colorHandles.eat()) {
            if (const ColorQuad_t* in = colorDefinitions.eat()) {
                *out = SDL_MapRGBA(m_surface->format, RED_FROM_COLORQUAD(*in), GREEN_FROM_COLORQUAD(*in), BLUE_FROM_COLORQUAD(*in), ALPHA_FROM_COLORQUAD(*in));
            } else {
                *out = 0;
            }
        }
    }
}

void
gfx::sdl::Surface::decodeColors(afl::base::Memory<const Color_t> colorHandles, afl::base::Memory<ColorQuad_t> colorDefinitions)
{
    // ensureUnlocked(); -- FIXME: why?

    // Check color key
    bool haveColorKey = false;
    Color_t colorKey = 0;
    if ((m_surface->flags & SDL_SRCCOLORKEY) != 0) {
        colorKey = m_surface->format->colorkey;
        haveColorKey = true;
    }

    while (ColorQuad_t* out = colorDefinitions.eat()) {
        uint8_t r = 0, g = 0, b = 0, a = 0;
        if (const Color_t* in = colorHandles.eat()) {
            if (haveColorKey && *in == colorKey) {
                a = TRANSPARENT_ALPHA;
            } else {
                SDL_GetRGBA(*in, m_surface->format, &r, &g, &b, &a);
            }
        }
        *out = COLORQUAD_FROM_RGBA(r, g, b, a);
    }
}

void
gfx::sdl::Surface::encodeColors(afl::base::Memory<const ColorQuad_t> colorDefinitions, afl::base::Memory<Color_t> colorHandles)
{
    // ensureUnlocked(); -- FIXME: why?

    // Check color key
    bool haveColorKey = false;
    Color_t colorKey = 0;
    if ((m_surface->flags & SDL_SRCCOLORKEY) != 0) {
        colorKey = m_surface->format->colorkey;
        haveColorKey = true;
    }

    while (Color_t* out = colorHandles.eat()) {
        if (const ColorQuad_t* in = colorDefinitions.eat()) {
            if (haveColorKey && ALPHA_FROM_COLORQUAD(*in) == TRANSPARENT_ALPHA) {
                *out = colorKey;
            } else {
                *out = SDL_MapRGBA(m_surface->format, RED_FROM_COLORQUAD(*in), GREEN_FROM_COLORQUAD(*in), BLUE_FROM_COLORQUAD(*in), ALPHA_FROM_COLORQUAD(*in));
            }
        } else {
            *out = 0;
        }
    }
}

afl::base::Ref<gfx::Canvas>
gfx::sdl::Surface::convertCanvas(afl::base::Ref<Canvas> orig)
{
    // ex GfxPixmap::convertToScreenFormat, GfxPixmap::convertTo

    // FIXME: think about preserving alpha.
    // If input is RGBA8888 and screen is RGB565, this looses the alpha channel (I think).
    // SDL_DisplayFormatAlpha has some extra logic to avoid that.
    if (Surface* sfc = dynamic_cast<Surface*>(&orig.get())) {
        // I took a peep at SDL_DisplayFormat for these flag combinations:
        // - output is hardware if this is hardware
        // - preserve colorkey/alpha
        uint32_t flags = (m_surface->flags & SDL_HWSURFACE) | (sfc->m_surface->flags & (SDL_SRCCOLORKEY | SDL_SRCALPHA));
        SDL_Surface* copy = SDL_ConvertSurface(sfc->m_surface, m_surface->format, flags);
        if (copy) {
            try {
                return *new Surface(copy, true);
            }
            catch (...) {
                SDL_FreeSurface(copy);
                throw;
            }
        } else {
            throw GraphicsException(SDL_GetError());
        }
    } else {
        // FIXME: if orig is anything else, convert that to a SDL_Surface to speed up further blits.
        return orig;
    }
}

void
gfx::sdl::Surface::ensureLocked()
{
    if (!m_locked) {
        m_locked = true;
        if (SDL_MUSTLOCK(m_surface)) {
            SDL_LockSurface(m_surface);
        }
    }
}

void
gfx::sdl::Surface::ensureUnlocked()
{
    if (m_locked) {
        m_locked = false;
        if (SDL_MUSTLOCK(m_surface)) {
            SDL_UnlockSurface(m_surface);
        }
        SDL_UpdateRect(m_surface, m_updateRegion.getLeftX(), m_updateRegion.getTopY(), m_updateRegion.getWidth(), m_updateRegion.getHeight());
        m_updateRegion = Rectangle();
    }
}

#endif
