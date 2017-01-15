/**
  *  \file gfx/canvas.hpp
  */
#ifndef C2NG_GFX_CANVAS_HPP
#define C2NG_GFX_CANVAS_HPP

#include "afl/base/deletable.hpp"
#include "gfx/point.hpp"
#include "gfx/fillpattern.hpp"
#include "gfx/rectangle.hpp"
#include "gfx/types.hpp"
#include "afl/base/memory.hpp"
#include "afl/base/ref.hpp"

namespace gfx {

    /** Drawing area.

        Offers an interface for simple graphics primitives.
        Descendants have to actually implement them.
        Based on the simple graphics primitives, we offer a few more complex ones.
        The complex primitives use a gfx::Context object to store the common parameters.
        This permits "stateful" programming.
        The simple graphics primitives are stateless.

        Unlike in PCC2, there is no need to do lock/unlock/update.

        <b>Colors:</b>

        Each color is defined by a ColorQuad_t in RGBA format.
        Pixels are internally represented as Color_t using a mapping (encode/decode).
        If the canvas is palettized, available colors must be defined using setPalette().

        Non-opaque pixel alpha only affects when this canvas is used as a source in blitting.
        For example, a pixel defined as COLORQUAD_FROM_RGBA(x,x,x,TRANSPARENT_ALPHA) will not modify the target canvas in a blit operation.

        Operation alpha affects how the new pixel is copied into this source.
        An operation with TRANSPARENT_ALPHA will not modify this canvas,
        an operation with OPAQUE_ALPHA will copy the new color into the canvas unmodified,
        and anything between will perform blending.

        Canvas implementations have different restrictions.
        Minimum assumptions:
        - 8-bit canvas needs a palette.
        - On all but 32-bit canvases, all colors need to be opaque, but one color can be defined transparent and is used for color-keying.
          Whereas on an 8-bit canvas, the color number uniquely identifies the color-key slot,
          care must be taken that the color does not result from a blending operation in 16- and 24-bit canvases. */
    class Canvas : public afl::base::Deletable, public afl::base::RefCounted {
     public:
        /** Draw horizontal line.
            \param pt    origin
            \param npix  number of pixels (width)
            \param color color, in target format.
            \param pat   pattern. Only draw pixels set in this pattern.
            \param alpha alpha value for transparency. */
        virtual void drawHLine(const Point& pt, int npix, Color_t color, LinePattern_t pat, Alpha_t alpha) = 0;

        /** Draw vertical line.
            \param pt    origin
            \param npix  number of pixels (height)
            \param color color, in target format.
            \param pat   pattern. Only draw pixels set in this pattern.
            \param alpha alpha value for transparency. */
        virtual void drawVLine(const Point& pt, int npix, Color_t color, LinePattern_t pat, Alpha_t alpha) = 0;

        /** Draw single pixel.
            \param pt    position.
            \param color color, in target format.
            \param alpha alpha value for transparency. */
        virtual void drawPixel(const Point& pt, Color_t color, Alpha_t alpha) = 0;

        /** Draw sequence of pixels.
            \param pt position.
            \param colors color, in target format.
            \param alpha alpha value for transparency. */
        virtual void drawPixels(const Point& pt, afl::base::Memory<const Color_t> colors, Alpha_t alpha) = 0;

        /** Draw filled rectangle.
            \param rect  area
            \param color color, in target format.
            \param bg    background color, or GFX_TRANSPARENT.
            \param pat   fill pattern.
            \param alpha alpha value for transparency. */
        // FIXME: change this signature to something better:
        // - drawSolidRectangle
        // - transparent background explicit
        virtual void drawBar(Rectangle rect, Color_t color, Color_t bg, const FillPattern& pat, Alpha_t alpha) = 0;

        /** Copy other surface (pixmap).
            \param pt    anchor point. src's (0,0) is here.
            \param src   input data
            \param rect  display this part of src, relative coordinates.

            `blit(pt, src, GfxRect(src))' will display the whole
            surface. `blitSurface(pt, src, GfxRect(2, 3, 4, 5))' will
            display a 4x5 area from surface src, at (x+2, y+3) up to but not
            including (x+2+4, y+3+5). */
        virtual void blit(const Point& pt, Canvas& src, Rectangle rect) = 0;

        /** Display pattern (monochrome pixmap).
            \param rect    position, screen coordinates;
            \param pt      position of bit 7, data[0], on the screen;
            \param bytes_per_line  bytes per line in data;
            \param data    pixel data. bit 7 = left, bit 0 = right (VGA bit order);
            \param color   color of "on" bits;
            \param bg      color of "off" bits, or GFX_TRANSPARENT;
            \param alpha   alpha channel.

            \pre pt.x <= rect.x, pt.y <= rect.y

            Note that this function has no constraints on the bytes_per_line
            parameter. bytes_per_line can also be zero or negative for
            interesting effects. */
        // FIXME: can we fit Memory<> in here?
        virtual void blitPattern(Rectangle rect, const Point& pt, int bytesPerLine, const uint8_t* data, Color_t color, Color_t bg, Alpha_t alpha) = 0;

        /** Compute clipping rectangle. This can be used to optimize
            output. This computes the smallest rectangle p such that
            drawBar(p) and drawBar(r) have the same effect. */
        virtual Rectangle computeClipRect(Rectangle r) = 0;

        // FIXME: retire
        // /** Get offset transformation to another surface.
        //     Adjusts #pt such that other.drawPixel(pt) afterwards
        //     accesses the same pixel as this->drawPixel(pt) before.
        //     \param other Reference surface
        //     \param result [in/out] Point to compute
        //     \return true if transformation can be computed, false otherwise.
        //     \inv computeOffset(*this,pt) returns true and does not modify pt */
        // virtual bool computeOffset(GfxCanvas& other, GfxPoint& pt) = 0;

        /** Read one pixel. \return pixel value in target format. */
        virtual void getPixels(Point pt, afl::base::Memory<Color_t> colors) = 0;

        /** Get size of this surface */
        virtual Point getSize() = 0;

        /** Get number of bits used per pixel. */
        virtual int getBitsPerPixel() = 0;

        virtual bool isVisible(Rectangle r) = 0;
        virtual bool isClipped(Rectangle r) = 0;

        /** Define palette colors.
            This makes sure you can display graphics with the defined colors (ColorQuad_t), and produce the Color_t values to refer to them.

            If this is a palettized canvas, this will actually modify the palette.
            Existing pixels on the screen will change their appearance.
            The color handles will be palette indexes.

            If this is a truecolor canvas, this will compute pixel values and return them as color handles.

            \param start            [in] First slot to use
            \param colorDefinitions [in] Colors to define
            \param colorHandles     [out] Handles to defined colors

            If colorDefinitions and colorHandles do not have the same number of elements, the behaviour is undefined. */
        virtual void setPalette(Color_t start, afl::base::Memory<const ColorQuad_t> colorDefinitions, afl::base::Memory<Color_t> colorHandles) = 0;

        /** Decode colors.
            This takes a list of color handles and returns their meaning as ColorQuad_t.
            \param colorHandles     [in] Colors to query
            \param colorDefinitions [out] Colors

            If colorDefinitions and colorHandles do not have the same number of elements, the behaviour is undefined. */
        virtual void decodeColors(afl::base::Memory<const Color_t> colorHandles, afl::base::Memory<ColorQuad_t> colorDefinitions) = 0;

        /** Encode colors.
            This finds out the color handles you can use to produce the desired colors.
            Unlike setPalette(), this will not modify a possible palette, but find the closest matching palette entries.

            \param colorDefinitions [in] Colors to query
            \param colorHandles     [out] Color handles

            If colorDefinitions and colorHandles do not have the same number of elements, the behaviour is undefined. */
        virtual void encodeColors(afl::base::Memory<const ColorQuad_t> colorDefinitions, afl::base::Memory<Color_t> colorHandles) = 0;

        virtual afl::base::Ref<Canvas> convertCanvas(afl::base::Ref<Canvas> orig) = 0;

        void defaultBlit(Point pt, Canvas& src, Rectangle rect);
    };

}

// FIXME: move elsewhere?
// /** Check if rectangle is clipped. If it is not clipped, it is completely
//     visible. */
// bool
// GfxCanvas::isClipped(Rectangle r)
// {
//     return r != computeClipRect(r);
// }

// /** Check if rectangle is visible. A rectangle is visible if at least
//     one pixel of it can be seen. */
// bool
// GfxCanvas::isVisible(Rectangle r)
// {
//     return computeClipRect(r).exists();
// }

#endif
