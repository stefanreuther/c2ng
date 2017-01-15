/**
  *  \file gfx/palettizedpixmap.cpp
  */

#include <cstdlib>
#include "gfx/palettizedpixmap.hpp"
#include "gfx/primitives.hpp"
#include "gfx/pixmapcanvasimpl.hpp"

class gfx::PalettizedPixmap::TraitsImpl {
 public:
    typedef uint8_t Pixel_t;
    typedef uint8_t Data_t;
        
    Data_t* get(int x, int y) const
        { return m_pix.row(y).at(x); }
    static inline Pixel_t peek(Data_t* ptr)
        { return *ptr; }
    static inline void poke(Data_t* ptr, Pixel_t val)
        { *ptr = val; }
    Pixel_t mix(Pixel_t a, Pixel_t b, Alpha_t balpha) const
        { return m_pix.findNearestColor(mixColor(m_pix.m_palette[a & 255], m_pix.m_palette[b & 255], balpha)); }
    inline Data_t* add(Data_t* ptr, int dx, int dy) const
        { return ptr + m_pix.getWidth()*dy + dx; }

    TraitsImpl(PalettizedPixmap& pix)
        : m_pix(pix)
        { }

 private:
    PalettizedPixmap& m_pix;
};

class gfx::PalettizedPixmap::CanvasImpl : public gfx::PixmapCanvasImpl<PalettizedPixmap, TraitsImpl> {
 public:
    CanvasImpl(afl::base::Ref<PalettizedPixmap> pix)
        : PixmapCanvasImpl<PalettizedPixmap, TraitsImpl>(pix)
        { }
    virtual int getBitsPerPixel()
        {
            return 8;
        }
    virtual void setPalette(Color_t start, afl::base::Memory<const ColorQuad_t> colorDefinitions, afl::base::Memory<Color_t> colorHandles)
        {
            pixmap().setPalette(start, colorDefinitions);
            while (Color_t* p = colorHandles.eat()) {
                *p = start++;
            }
        }
    virtual void decodeColors(afl::base::Memory<const Color_t> colorHandles, afl::base::Memory<ColorQuad_t> colorDefinitions)
        {
            while (const Color_t* color = colorHandles.eat()) {
                if (ColorQuad_t* def = colorDefinitions.eat()) {
                    *def = pixmap().m_palette[*color & 255];
                }
            }
            colorDefinitions.fill(COLORQUAD_FROM_RGBA(0,0,0,0));
        }
    virtual void encodeColors(afl::base::Memory<const ColorQuad_t> colorDefinitions, afl::base::Memory<Color_t> colorHandles)
        {
            while (const ColorQuad_t* def = colorDefinitions.eat()) {
                if (Color_t* color = colorHandles.eat()) {
                    *color = pixmap().findNearestColor(*def);
                }
            }
            colorHandles.fill(0);
        }
    virtual afl::base::Ref<Canvas> convertCanvas(afl::base::Ref<Canvas> orig)
        {
            // FIXME: can we do better?
            return orig;
        }
};


gfx::PalettizedPixmap::PalettizedPixmap(int w, int h)
    : Pixmap<uint8_t>(w, h)
{
    afl::base::Memory<ColorQuad_t>(m_palette).fill(COLORQUAD_FROM_RGBA(0,0,0,0));
}

afl::base::Ref<gfx::PalettizedPixmap>
gfx::PalettizedPixmap::create(int w, int h)
{
    return *new PalettizedPixmap(w, h);
}

void
gfx::PalettizedPixmap::setPalette(uint8_t start, afl::base::Memory<const ColorQuad_t> colorDefinitions)
{
    afl::base::Memory<ColorQuad_t>(m_palette).subrange(start).copyFrom(colorDefinitions);
}

void
gfx::PalettizedPixmap::setPalette(uint8_t slot, ColorQuad_t colorDefinition)
{
    m_palette[slot] = colorDefinition;
}

void
gfx::PalettizedPixmap::getPalette(uint8_t start, afl::base::Memory<ColorQuad_t> colorDefinitions) const
{
    colorDefinitions.copyFrom(afl::base::Memory<const ColorQuad_t>(m_palette).subrange(start));
}

uint8_t
gfx::PalettizedPixmap::findNearestColor(ColorQuad_t def) const
{
    uint8_t result = 0;
    int32_t resultDist = getColorDistance(def, m_palette[0]);
    for (int i = 1; i < 256 && resultDist != 0; ++i) {
        int32_t dist = getColorDistance(def, m_palette[i]);
        if (dist < i) {
            result = i;
            resultDist = dist;
        }
    }
    return result;
}

afl::base::Ref<gfx::Canvas>
gfx::PalettizedPixmap::makeCanvas()
{
    return *new CanvasImpl(*this);
}
