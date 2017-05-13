/**
  *  \file gfx/bitmapglyph.cpp
  *  \brief Class gfx::BitmapGlyph
  */

#include "gfx/bitmapglyph.hpp"
#include "gfx/context.hpp"
#include "gfx/canvas.hpp"

// Construct an empty glyph of zero size.
gfx::BitmapGlyph::BitmapGlyph()
    : m_width(0),
      m_height(0),
      m_data(),
      m_aaData()
{
    // ex GfxBitmapGlyph::GfxBitmapGlyph
    // FIXME: do we need this signature?
}

// Construct a blank glyph of a given size.
gfx::BitmapGlyph::BitmapGlyph(uint16_t width, uint16_t height)
    : m_width(width),
      m_height(height),
      m_data(getBytesForSize(width, height)),
      m_aaData()
{ }

// Construct glyph from bitmap data.
// FIXME: convert to ConstBytes_t
gfx::BitmapGlyph::BitmapGlyph(uint16_t width, uint16_t height, const uint8_t* data)
    : m_width(width),
      m_height(height),
      m_data(data, data + getBytesForSize(width, height)),
      m_aaData()
{ }

// Destructor.
gfx::BitmapGlyph::~BitmapGlyph()
{ }

// Add anti-aliasing hint.
void
gfx::BitmapGlyph::addAAHint(uint16_t x, uint16_t y)
{
    // ex GfxBitmapGlyph::addAAHint
    m_aaData.push_back(x);
    m_aaData.push_back(y);
    set(x, y, 0);
}

// Get height of this glyph in pixels.
int
gfx::BitmapGlyph::getHeight() const
{
    // ex GfxBitmapGlyph::getHeight
    return m_height;
}

// Get width of this glyph in pixels.
int
gfx::BitmapGlyph::getWidth() const
{
    // ex GfxBitmapGlyph::getWidth
    return m_width;
}

// Draw this glyph.
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
        const Alpha_t halfIntensity = static_cast<Alpha_t>((ctx.getAlpha()+1)/2);
        for (size_t i = 0, n = m_aaData.size(); i < n; i += 2) {
            ctx.canvas().drawPixel(pt + Point(m_aaData[i], m_aaData[i+1]), ctx.getRawColor(), halfIntensity);
        }
    }
}

// Draw this glyph with defined colors.
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

// Set pixel value.
void
gfx::BitmapGlyph::set(int x, int y, bool value)
{
    // ex GfxBitmapGlyph::set
    if (x >= 0 && y >= 0 && x < m_width && y < m_height) {
        uint8_t& byte = m_data[y * getBytesPerLine() + (x >> 3)];
        uint8_t  mask = uint8_t(0x80 >> (x&7));
        if (value) {
            byte = uint8_t(byte |  mask);
        } else {
            byte = uint8_t(byte & ~mask);
        }
    }
}

// Get pixel value.
bool
gfx::BitmapGlyph::get(int x, int y) const
{
    // ex GfxBitmapGlyph::get
    return (x >= 0 && y >= 0 && x < m_width && y < m_height)
        && (m_data[y * getBytesPerLine() + (x >> 3)] & (0x80 >> (x&7))) != 0;
}

// Access anti-aliasing data.
const std::vector<uint16_t>&
gfx::BitmapGlyph::getAAData() const
{
    // ex GfxBitmapGlyph::getAAData
    return m_aaData;
}

// Access pixel data.
const std::vector<uint8_t>&
gfx::BitmapGlyph::getData() const
{
    // ex GfxBitmapGlyph::getData
    return m_data;
}

// Compute number of bytes required for a glyph of the specified size.
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
