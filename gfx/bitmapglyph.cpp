/**
  *  \file gfx/bitmapglyph.cpp
  */

#include "gfx/bitmapglyph.hpp"
#include "gfx/context.hpp"
#include "gfx/canvas.hpp"

// /** Construct blank glyph. */
gfx::BitmapGlyph::BitmapGlyph()
    : m_width(0),
      m_height(0),
      m_data(),
      m_aaData()
{
    // ex GfxBitmapGlyph::GfxBitmapGlyph
}

// /** Construct blank glyph.
//     \param width  [in] Width in pixels
//     \param height [in] Height in pixels */
gfx::BitmapGlyph::BitmapGlyph(uint16_t width, uint16_t height)
    : m_width(width),
      m_height(height),
      m_data(getBytesForSize(width, height)),
      m_aaData()
{ }

// /** Construct glyph from input data.
//     \param width  [in] Width in pixels
//     \param height [in] Height in pixels
//     \param data   [in] Refers to getBytesForSize(width,height) bytes containing bitmap data */
// FIXME: convert to ConstBytes_t
gfx::BitmapGlyph::BitmapGlyph(uint16_t width, uint16_t height, const uint8_t* data)
    : m_width(width),
      m_height(height),
      m_data(data, data + getBytesForSize(width, height)),
      m_aaData()
{ }

// /** Destructor. */
gfx::BitmapGlyph::~BitmapGlyph()
{ }

/** Anti-aliasing hint. Specifies that the pixel at (x,y) should be
    drawn in half intensity.
    \param x X-coordinate [0,width)
    \param y Y-coordinate [0,height) */
void
gfx::BitmapGlyph::addAAHint(uint16_t x, uint16_t y)
{
    // ex GfxBitmapGlyph::addAAHint
    m_aaData.push_back(x);
    m_aaData.push_back(y);
    set(x, y, 0);
}

/** Get height of this glyph in pixels. */
int
gfx::BitmapGlyph::getHeight() const
{
    // ex GfxBitmapGlyph::getHeight
    return m_height;
}

/** Get width of this glyph in pixels. */
int
gfx::BitmapGlyph::getWidth() const
{
    // ex GfxBitmapGlyph::getWidth
    return m_width;
}

/** Draw this glyph.
    \param ctx Context to draw on
    \param x,y Position */
void
gfx::BitmapGlyph::draw(BaseContext& ctx, Point pt) const
{
    // ex GfxBitmapGlyph::draw
    // Do nothing if this glyph is blank
    if (m_width != 0 && m_height != 0) {
        // Main data
        ctx.canvas().blitPattern(Rectangle(pt, Point(m_width, m_height)),
                                 pt,
                                 getBytesPerLine(),
                                 &*m_data.begin(),
                                 ctx.getRawColor(),
                                 TRANSPARENT_COLOR,
                                 ctx.getAlpha());

        // AA hints
        const Alpha_t halfIntensity = (ctx.getAlpha()+1)/2;
        for (size_t i = 0, n = m_aaData.size(); i < n; i += 2) {
            ctx.canvas().drawPixel(pt + Point(m_aaData[i], m_aaData[i+1]), ctx.getRawColor(), halfIntensity);
        }
    }
}

/** Draw this glyph, with solid colors.
    \param can Canvas to draw on
    \param x,y Position
    \param pixel_color Color of regular pixels
    \param aa_color Color of half-intensity pixels */
void
gfx::BitmapGlyph::drawColored(Canvas& can, Point pt, Color_t pixel_color, Color_t aa_color) const
{
    // ex GfxBitmapGlyph::drawColored
    // Do nothing if this glyph is blank
    if (m_width != 0 && m_height != 0) {
        // Main data
        can.blitPattern(Rectangle(pt, Point(m_width, m_height)),
                        pt,
                        getBytesPerLine(),
                        &*m_data.begin(),
                        pixel_color,
                        TRANSPARENT_COLOR,
                        OPAQUE_ALPHA);

        // AA hints
        for (size_t i = 0, n = m_aaData.size(); i < n; i += 2) {
            can.drawPixel(pt + Point(m_aaData[i], m_aaData[i+1]), aa_color, OPAQUE_ALPHA);
        }
    }
}

/** Set pixel value.
    \param x,y   Position
    \param value true to set pixel, false to clear it */
void
gfx::BitmapGlyph::set(int x, int y, bool value)
{
    // ex GfxBitmapGlyph::set
    if (x >= 0 && y >= 0 && x < m_width && y < m_height) {
        if (value) {
            m_data[y * getBytesPerLine() + (x >> 3)] |=  (0x80 >> (x&7));
        } else {
            m_data[y * getBytesPerLine() + (x >> 3)] &= ~(0x80 >> (x&7));
        }
    }
}

/** Get pixel value.
    \param x,y   Position */
bool
gfx::BitmapGlyph::get(int x, int y) const
{
    // ex GfxBitmapGlyph::get
    return (x >= 0 && y >= 0 && x < m_width && y < m_height)
        && (m_data[y * getBytesPerLine() + (x >> 3)] & (0x80 >> (x&7))) != 0;
}


/** Get anti-aliasing data. */
const std::vector<uint16_t>&
gfx::BitmapGlyph::getAAData() const
{
    // ex GfxBitmapGlyph::getAAData
    return m_aaData;
}

/** Get pixel data. */
const std::vector<uint8_t>&
gfx::BitmapGlyph::getData() const
{
    // ex GfxBitmapGlyph::getData
    return m_data;
}


/** Compute number of bytes required for a glyph of the specified size.
    \param width Width in pixels
    \param height Height in pixels */
size_t
gfx::BitmapGlyph::getBytesForSize(uint16_t width, uint16_t height)
{
    // ex GfxBitmapGlyph::getBytesForSize
    return height * ((width + 7) / 8);
}

/** Get number of bytes per line. */
int
gfx::BitmapGlyph::getBytesPerLine() const
{
    // ex GfxBitmapGlyph::getBytesPerLine
    return (m_width+7)/8;
}
