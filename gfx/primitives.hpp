/**
  *  \file gfx/primitives.hpp
  *  \brief Template class gfx::Primitives
  */
#ifndef C2NG_GFX_PRIMITIVES_HPP
#define C2NG_GFX_PRIMITIVES_HPP

#include "gfx/types.hpp"
#include "afl/bits/rotate.hpp"
#include "gfx/rectangle.hpp"
#include "gfx/point.hpp"
#include "afl/base/types.hpp"
#include "afl/base/memory.hpp"

namespace gfx {

    /** Graphics primitives on framebuffers.
        This class implements basic operations on framebuffers.
        It is used to implement framebuffer-based drawing.
        It implements all applicable drawing options.
        However, it assumes that clipping has been performed by the caller; there is no range checking.

        \tparam T traits class.
        - <tt>typedef Pixel_t</tt>: data type for pixels
        - <tt>typedef Data_t</tt>: base type for data pointers
        - <tt>Data_t* get(int x, int y)</tt>: create pointer into framebuffer at x,y
        - <tt>Pixel_t peek(Data_t*)</tt>: read a pixel
        - <tt>void poke(Data_t*, Pixel_t)</tt>: write a pixel
        - <tt>Pixel(Pixel_t a, Pixel_b b, Alpha_t balpha)</tt>: alpha blending
        - <tt>Data_t* add(Data_t*, int dx, int dy)</tt>: update data pointer */
    template<class T>
    class Primitives {
     public:
        /** Data type for pixel values.
            Convenience typedef. */
        typedef typename T::Pixel_t Pixel_t;

        /** Base type for data pointers.
            Convenience typedef. */
        typedef typename T::Data_t Data_t;

        /** Constructor.
            \param traits Traits object */
        Primitives(const T& traits)
            : m_traits(traits)
            { }

        /** Write pixels.
            \param x,y Location
            \param pixels Pixels
            \param alpha Transparency */
        void writePixels(int x, int y, afl::base::Memory<const Color_t> pixels, Alpha_t alpha);

        /** Read pixels.
            \param x,y Location
            \param pixels Pixels
            \param alpha Transparency */
        void readPixels(int x, int y, afl::base::Memory<Color_t> pixels);

        /** Draw horizontal Line.
            \param x1,y1 Origin position
            \param x2 Ending X coordinate, exclusive
            \param color Desired color
            \param pat Line pattern
            \param alpha Transparency */
        void doHLine(int x1, int y1, int x2, Pixel_t color, LinePattern_t pat, Alpha_t alpha);

        /** Draw vertical Line.
            \param x1,y1 Origin position
            \param y2 Ending Y coordinate, exclusive
            \param color Desired color
            \param pat Line pattern
            \param alpha Transparency */
        void doVLine(int x1, int y1, int y2, Pixel_t color, uint8_t pat, Alpha_t alpha);

        /** Draw bar (filled rectangle).
            \param rect Area
            \param color Foreground color
            \param bg Background color or GFX_TRANSPARENT
            \param pat Fill pattern
            \param alpha Transparency */
        void doBar(Rectangle rect, Color_t color, Color_t bg, const FillPattern& pat, Alpha_t alpha);

        /** Blit pattern.
            \param rect Area
            \param pt Position of bit 7, data[0] on screen.
            \param bytesPerLine Bytes per line
            \param data Pixel data. Bit 7 = left, 0 = right.
            \param color Foreground color
            \param bg Background color or GFX_TRANSPARENT
            \param alpha Transparency */
        void doBlitPattern(Rectangle rect, const Point& pt, int bytesPerLine, const uint8_t* data, Color_t color, Color_t bg, Alpha_t alpha);

     private:
        /* Blit pattern for transparent patterns, solid (no background color, no alpha). */
        inline void doBlitPatternTransp(Rectangle rect, Point pt, int bytesPerLine, const uint8_t* data, Pixel_t color);

        /* Blit pattern with background color, solid (no alpha). */
        inline void doBlitPatternOpaque(Rectangle rect, Point pt, int bytesPerLine, const uint8_t* data, Pixel_t color, Pixel_t bg);

        /* Blit pattern with transparency. */
        inline void doBlitPatternAlpha(Rectangle rect, Point pt, int bytesPerLine, const uint8_t* data, Pixel_t color, Color_t bg, Alpha_t alpha);

        const T& m_traits;
    };

}

// Write pixels.
template<typename T>
void
gfx::Primitives<T>::writePixels(int x, int y, afl::base::Memory<const Color_t> pixels, Alpha_t alpha)
{
    Data_t* p = m_traits.get(x, y);
    if (alpha == OPAQUE_ALPHA) {
        while (const Color_t* pix = pixels.eat()) {
            m_traits.poke(p, *pix);
            p = m_traits.add(p, 1, 0);
        }
    } else {
        while (const Color_t* pix = pixels.eat()) {
            m_traits.poke(p, m_traits.mix(m_traits.peek(p), *pix, alpha));
            p = m_traits.add(p, 1, 0);
        }
    }
}

// Read pixels.
template<typename T>
void
gfx::Primitives<T>::readPixels(int x, int y, afl::base::Memory<Color_t> pixels)
{
    Data_t* p = m_traits.get(x, y);
    while (Color_t* pix = pixels.eat()) {
        *pix = m_traits.peek(p);
        p = m_traits.add(p, 1, 0);
    }
}

// Draw horizontal Line.
template<typename T>
void
gfx::Primitives<T>::doHLine(int x1, int y1, int x2, Pixel_t color, LinePattern_t pat, Alpha_t alpha)
{
    if (pat == 0 || alpha == TRANSPARENT_ALPHA) {
        /* nothing */
    } else {
        Data_t* p = m_traits.get(x1, y1);
        if (alpha == OPAQUE_ALPHA) {
            if (pat == 255) {
                /* solid line */
                while (x1 < x2) {
                    m_traits.poke(p, color);
                    p = m_traits.add(p, 1, 0);
                    ++x1;
                }
            } else {
                /* pattern line */
                uint8_t mask = afl::bits::rotateRight8(0x80, x1);
                while (x1 < x2) {
                    if (mask & pat) {
                        m_traits.poke(p, color);
                    }
                    p = m_traits.add(p, 1, 0);
                    ++x1;
                    mask = afl::bits::rotateRight8(mask, 1);
                }
            }
        } else {
            if (pat == 255) {
                /* solid line */
                while (x1 < x2) {
                    m_traits.poke(p, m_traits.mix(m_traits.peek(p), color, alpha));
                    p = m_traits.add(p, 1, 0);
                    ++x1;
                }
            } else {
                /* pattern line */
                uint8_t mask = afl::bits::rotateRight8(0x80, x1);
                while (x1 < x2) {
                    if (mask & pat) {
                        m_traits.poke(p, m_traits.mix(m_traits.peek(p), color, alpha));
                    }
                    p = m_traits.add(p, 1, 0);
                    ++x1;
                    mask = afl::bits::rotateRight8(mask, 1);
                }
            }
        }
    }
}

// Draw vertical Line.
template<typename T>
void
gfx::Primitives<T>::doVLine(int x1, int y1, int y2, Pixel_t color, uint8_t pat, Alpha_t alpha)
{
    if (pat == 0 || alpha == TRANSPARENT_ALPHA) {
        /* nothing */
    } else {
        Data_t* p = m_traits.get(x1, y1);
        if (alpha == OPAQUE_ALPHA) {
            if (pat == 255) {
                while (y1 < y2) {
                    m_traits.poke(p, color);
                    p = m_traits.add(p, 0, 1);
                    ++y1;
                }
            } else {
                uint8_t mask = afl::bits::rotateRight8(0x80, y1);
                while (y1 < y2) {
                    if (mask & pat) {
                        m_traits.poke(p, color);
                    }
                    p = m_traits.add(p, 0, 1);
                    ++y1;
                    mask = afl::bits::rotateRight8(mask, 1);
                }
            }
        } else {
            if (pat == 255) {
                while (y1 < y2) {
                    m_traits.poke(p, m_traits.mix(m_traits.peek(p), color, alpha));
                    p = m_traits.add(p, 0, 1);
                    ++y1;
                }
            } else {
                uint8_t mask = afl::bits::rotateRight8(0x80, y1);
                while (y1 < y2) {
                    if (mask & pat) {
                        m_traits.poke(p, m_traits.mix(m_traits.peek(p), color, alpha));
                    }
                    p = m_traits.add(p, 0, 1);
                    ++y1;
                    mask = afl::bits::rotateRight8(mask, 1);
                }
            }
        }
    }
}

// Draw bar (filled rectangle).
template<typename T>
void
gfx::Primitives<T>::doBar(Rectangle rect, Color_t color, Color_t bg, const FillPattern& pat, Alpha_t alpha)
{
    int x1 = rect.getLeftX(), y1 = rect.getTopY(), x2 = rect.getRightX(), h = rect.getHeight();
    if (bg == TRANSPARENT_COLOR) {
        while (h > 0) {
            doHLine(x1, y1, x2, color, pat[y1], alpha);
            ++y1; --h;
        }
    } else {
        while (h > 0) {
            doHLine(x1, y1, x2, color, pat[y1], alpha);
            doHLine(x1, y1, x2, bg, ~pat[y1], alpha);
            ++y1; --h;
        }
    }
}

// Blit pattern.
template<typename T>
void
gfx::Primitives<T>::doBlitPattern(Rectangle rect, const Point& pt, int bytesPerLine, const uint8_t* data, Color_t color, Color_t bg, Alpha_t alpha)
{
    if (alpha == OPAQUE_ALPHA) {
        if (bg == TRANSPARENT_COLOR) {
            doBlitPatternTransp(rect, pt, bytesPerLine, data, color);
        } else {
            doBlitPatternOpaque(rect, pt, bytesPerLine, data, color, bg);
        }
    } else {
        doBlitPatternAlpha(rect, pt, bytesPerLine, data, color, bg, alpha);
    }
}

// Blit pattern for transparent patterns, solid (no background color, no alpha).
template<typename T>
void
gfx::Primitives<T>::doBlitPatternTransp(Rectangle rect, Point pt, int bytesPerLine, const uint8_t* data, Pixel_t color)
{
    int x = rect.getLeftX() - pt.getX();                // columns to skip in data image
    int y = rect.getTopY() - pt.getY();                 // lines to skip in data image
    data += bytesPerLine * y + x/8;

    uint8_t zmask = 0x80 >> (x & 7);
    Data_t* zmem = m_traits.get(rect.getLeftX(), rect.getTopY());
    int h = rect.getHeight();
    while (h > 0) {
        uint8_t mask = zmask;
        Data_t* mem = zmem;
        const uint8_t* pat = data;
        for (int k = 0; k < rect.getWidth(); ++k) {
            if (*pat & mask) {
                m_traits.poke(mem, color);
            }
            mem = m_traits.add(mem, 1, 0);
            mask >>= 1;
            if (!mask) {
                mask = 0x80;
                ++pat;
            }
        }
        --h;
        data += bytesPerLine;
        zmem = m_traits.add(zmem, 0, 1);
    }
}

// Blit pattern with background color, solid (no alpha).
template<typename T>
void
gfx::Primitives<T>::doBlitPatternOpaque(Rectangle rect, Point pt, int bytesPerLine, const uint8_t* data, Pixel_t color, Pixel_t bg)
{
    int x = rect.getLeftX() - pt.getX();                // columns to skip in data image
    int y = rect.getTopY() - pt.getY();                 // lines to skip in data image
    data += bytesPerLine * y + x/8;

    uint8_t zmask = 0x80 >> (x & 7);
    Data_t* zmem = m_traits.get(rect.getLeftX(), rect.getTopY());
    int h = rect.getHeight();
    while (h > 0) {
        uint8_t mask = zmask;
        Data_t* mem = zmem;
        const uint8_t* pat = data;
        for (int k = 0; k < rect.getWidth(); ++k) {
            if (*pat & mask) {
                m_traits.poke(mem, color);
            } else {
                m_traits.poke(mem, bg);
            }
            mem = m_traits.add(mem, 1, 0);
            mask >>= 1;
            if (!mask) {
                mask = 0x80;
                ++pat;
            }
        }
        --h;
        data += bytesPerLine;
        zmem = m_traits.add(zmem, 0, 1);
    }
}

// Blit pattern with transparency.
// Note that 'bg' must be Color_t here to test for TRANSPARENT_COLOR.
template<typename T>
void
gfx::Primitives<T>::doBlitPatternAlpha(Rectangle rect, Point pt, int bytesPerLine, const uint8_t* data, Pixel_t color, Color_t bg, Alpha_t alpha)
{
    int x = rect.getLeftX() - pt.getX();                // columns to skip in data image
    int y = rect.getTopY() - pt.getY();                 // lines to skip in data image
    data += bytesPerLine * y + x/8;

    uint8_t zmask = 0x80 >> (x & 7);
    Data_t* zmem = m_traits.get(rect.getLeftX(), rect.getTopY());
    int h = rect.getHeight();
    while (h > 0) {
        uint8_t mask = zmask;
        Data_t* mem = zmem;
        const uint8_t* pat = data;
        for (int k = 0; k < rect.getWidth(); ++k) {
            if (*pat & mask) {
                m_traits.poke(mem, m_traits.mix(m_traits.peek(mem), color, alpha));
            } else if (bg != TRANSPARENT_COLOR) {
                m_traits.poke(mem, m_traits.mix(m_traits.peek(mem), bg, alpha));
            }
            mem = m_traits.add(mem, 1, 0);
            mask >>= 1;
            if (!mask) {
                mask = 0x80;
                ++pat;
            }
        }
        --h;
        data += bytesPerLine;
        zmem = m_traits.add(zmem, 0, 1);
    }
}

#endif
