/**
  *  \file gfx/colorquantizer.cpp
  *  \brief Class gfx::ColorQuantizer
  */

#include <algorithm>
#include <memory>
#include "gfx/colorquantizer.hpp"
#include "afl/base/countof.hpp"
#include "afl/base/inlinememory.hpp"
#include "util/math.hpp"

namespace {
    using afl::base::InlineMemory;
    using afl::base::Memory;
    using gfx::Canvas;
    using gfx::ColorQuad_t;
    using gfx::Color_t;
    using gfx::PalettizedPixmap;
    using gfx::Point;

    /* Local state */
    struct LocalState {
        ColorQuad_t palette[256];
        uint8_t firstUsable;
        uint8_t firstDynamic;
        size_t numUsable;
        size_t numDynamic;

        uint32_t colorStats[32*32*32];
    };

    const size_t nil = size_t(-1);

    /*
     *  Internal pre-quantisation of colors.
     *
     *  We're doing statistics with 5 bit per color component (32K colors, 128K temp buffer).
     *  Though this is 1 bit worse then possible output quality for PCC output formats,
     *  and 3 bit worse than input (or general output) formats, it gives good results
     */

    uint8_t redFromIndex(size_t index)
    {
        return ((index >> 10) & 31) << 3;
    }

    uint8_t greenFromIndex(size_t index)
    {
        return ((index >> 5) & 31) << 3;
    }

    uint8_t blueFromIndex(size_t index)
    {
        return (index & 31) << 3;
    }

    ColorQuad_t colorQuadFromIndex(size_t index)
    {
        return COLORQUAD_FROM_RGB(redFromIndex(index), greenFromIndex(index), blueFromIndex(index));
    }

    size_t indexFromRGB(size_t r, size_t g, size_t b)
    {
        return ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3);
    }

    size_t indexFromColorQuad(ColorQuad_t c)
    {
        return indexFromRGB(RED_FROM_COLORQUAD(c), GREEN_FROM_COLORQUAD(c), BLUE_FROM_COLORQUAD(c));
    }

    /* Count colors. */
    void countColors(LocalState& st, Canvas& can)
    {
        // ex replaceb.pas:CountColors
        Memory<ColorQuad_t>(st.colorStats).fill(0);

        const Point size = can.getSize();
        for (int y = 0; y < size.getY(); ++y) {
            const int N = 1024;
            int x = 0;
            while (x < size.getX()) {
                int n = std::min(N, size.getX() - x);

                InlineMemory<Color_t,N> colorBuffer;
                InlineMemory<ColorQuad_t,N> quadBuffer;
                colorBuffer.trim(n);
                quadBuffer.trim(n);

                can.getPixels(Point(x, y), colorBuffer);
                can.decodeColors(colorBuffer, quadBuffer);

                while (const ColorQuad_t* p = quadBuffer.eat()) {
                    ++st.colorStats[indexFromColorQuad(*p)];
                }

                x += n;
            }
        }
    }

    /* Find most frequent color, and return its index.
       If none found, returns nil.
       For now, slow search. */
    size_t findMostFrequentColor(const LocalState& st)
    {
        // ex replaceb.pas:GetMostFrequentColor, sort-of
        size_t result = nil;
        uint32_t cnt = 0;
        for (size_t i = 0; i < countof(st.colorStats); ++i) {
            if (st.colorStats[i] > cnt) {
                cnt = st.colorStats[i];
                result = i;
            }
        }
        return result;
    }

    /* Distance between two colors. */
    size_t getColorDistance(ColorQuad_t a, ColorQuad_t b)
    {
        // Euclidean distance, extra weight on green
        size_t n = util::squareInteger(RED_FROM_COLORQUAD(a) - RED_FROM_COLORQUAD(b))
            + 2*util::squareInteger(GREEN_FROM_COLORQUAD(a) - GREEN_FROM_COLORQUAD(b))
            + util::squareInteger(BLUE_FROM_COLORQUAD(a) - BLUE_FROM_COLORQUAD(b));

        // Normalize to our quantisation
        n /= 64;

        return n;
    }

    /* Find closest color. */
    size_t findClosestColor(const LocalState& st, ColorQuad_t c)
    {
        size_t index = st.firstUsable;
        size_t last = index + st.numUsable;
        size_t result = nil;
        size_t dif = 0;

        while (index < last) {
            if (index == st.firstDynamic && st.numDynamic > 0) {
                // Skip over dynamic colors
                index += st.numDynamic;
            } else {
                // Check current
                size_t d = getColorDistance(c, st.palette[index]);
                if (result == nil || d < dif) {
                    dif = d;
                    result = index;
                }
                ++index;
            }
        }

        return result;
    }

    /** Assign colors. */
    void assignColors(LocalState& st)
    {
        // ex replaceb.pas:AssignColors
        // Do we have dynamic colors at all?
        if (st.numDynamic == 0) {
            return;
        }

        // Determine threshold for allocating a color.
        // PCC1 uses threshold 0 for 256-color pictures (64 dynamic colors), 20 for 16-color (5 dynamic colors).
        // This produces threshold 1 for the former.
        const size_t threshold = 100 / st.numDynamic;

        while (st.numDynamic > 0) {
            // Find most frequent color
            const size_t index = findMostFrequentColor(st);
            if (index == nil) {
                break;
            }

            // Strike it out
            st.colorStats[index] = 0;

            // Find closest color
            const ColorQuad_t c = colorQuadFromIndex(index);
            const size_t closest = findClosestColor(st, c);

            // If not close enough to an existing color, allocate one
            if (closest == nil || getColorDistance(c, st.palette[closest]) > threshold) {
                st.palette[st.firstDynamic++] = c;
                --st.numDynamic;
            }
        }
    }

    int clampComponent(int n)
    {
        return (n > 255 ? 255 : n < 0 ? 0 : n);
    }

    void addError(ColorQuad_t& c, int rdiff, int gdiff, int bdiff)
    {
        c = COLORQUAD_FROM_RGB(clampComponent(RED_FROM_COLORQUAD(c) + rdiff),
                               clampComponent(GREEN_FROM_COLORQUAD(c) + gdiff),
                               clampComponent(BLUE_FROM_COLORQUAD(c) + bdiff));
    }

    void ditherImage(LocalState& st, PalettizedPixmap& out, Canvas& in)
    {
        // ex replaceb.pas:DitherImage
        size_t width = in.getSize().getX();
        size_t height = in.getSize().getY();
        std::vector<Color_t> colorBuffer;
        std::vector<ColorQuad_t> thisLine, nextLine;
        colorBuffer.resize(width);
        thisLine.resize(width);
        nextLine.resize(width);

        // Clear colorStats; we're re-using it as quantisation buffer
        Memory<uint32_t>(st.colorStats).fill(0);

        // Operate
        for (size_t y = 0; y < height; ++y) {
            // Update RGBA data
            if (y == 0) {
                in.getPixels(Point(0, int(y)), colorBuffer);
                in.decodeColors(colorBuffer, thisLine);
            } else {
                thisLine.swap(nextLine);
            }
            if (y+1 < height) {
                in.getPixels(Point(0, int(y)+1), colorBuffer);
                in.decodeColors(colorBuffer, nextLine);
            }

            // Process line
            for (size_t x = 0; x < width; ++x) {
                // Pick pixel, using cache
                ColorQuad_t sourceColor = thisLine[x];
                size_t cacheIndex = indexFromColorQuad(sourceColor);
                uint32_t cached = st.colorStats[cacheIndex];
                size_t colorIndex;
                if (cached != 0) {
                    colorIndex = cached-1;
                } else {
                    colorIndex = findClosestColor(st, sourceColor);
                    if (colorIndex == nil) {
                        // fail-safe; cannot happen
                        colorIndex = 0;
                    }
                    st.colorStats[cacheIndex] = uint32_t(1+colorIndex);
                }

                if (uint8_t* p = out.row(int(y)).at(x)) {
                    *p = static_cast<uint8_t>(colorIndex);
                }

                // Dithering
                // (Same algorithm as in replaceb, very simplified.)
                ColorQuad_t chosenColor = st.palette[colorIndex];
                int rdiff = (RED_FROM_COLORQUAD(sourceColor)   - RED_FROM_COLORQUAD(chosenColor)) / 4;
                int gdiff = (GREEN_FROM_COLORQUAD(sourceColor) - GREEN_FROM_COLORQUAD(chosenColor)) / 4;
                int bdiff = (BLUE_FROM_COLORQUAD(sourceColor)  - BLUE_FROM_COLORQUAD(chosenColor)) / 4;
                if (x+1 < width) {
                    addError(thisLine[x+1], rdiff, gdiff, bdiff);
                }
                addError(nextLine[x], rdiff, gdiff, bdiff);
                if (x > 0) {
                    addError(nextLine[x-1], rdiff, gdiff, bdiff);
                }
            }
        }
    }
}


gfx::ColorQuantizer::ColorQuantizer()
    : m_firstUsable(0), m_firstDynamic(0),
      m_numUsable(256), m_numDynamic(256)
{
    Memory<ColorQuad_t>(m_palette).fill(0);
}

afl::base::Ref<gfx::PalettizedPixmap>
gfx::ColorQuantizer::quantize(Canvas& can) const
{
    // Build local state
    // (on heap, it's 129 kiB)
    std::auto_ptr<LocalState> st(new LocalState());
    Memory<ColorQuad_t>(st->palette).copyFrom(m_palette);

    st->firstUsable = m_firstUsable;
    st->numUsable = std::min(size_t(256 - m_firstUsable), m_numUsable);

    if (m_firstDynamic < m_firstUsable || m_firstDynamic > st->firstUsable + st->numUsable) {
        st->firstDynamic = 0;
        st->numDynamic = 0;
    } else {
        st->firstDynamic = m_firstDynamic;
        st->numDynamic = std::min(size_t(st->firstUsable + st->numUsable - m_firstDynamic), m_numDynamic);
    }

    // Determine palette
    countColors(*st, can);
    assignColors(*st);

    // Build result
    afl::base::Ref<PalettizedPixmap> result = PalettizedPixmap::create(can.getSize().getX(), can.getSize().getY());
    result->setPalette(0, st->palette);
    ditherImage(*st, *result, can);

    return result;
}

gfx::ColorQuantizer&
gfx::ColorQuantizer::setUsablePaletteRange(uint8_t from, size_t count)
{
    m_firstUsable = from;
    m_numUsable = count;
    return *this;
}

gfx::ColorQuantizer&
gfx::ColorQuantizer::setDynamicPaletteRange(uint8_t from, size_t count)
{
    m_firstDynamic = from;
    m_numDynamic = count;
    return *this;
}

gfx::ColorQuantizer&
gfx::ColorQuantizer::setPalette(uint8_t start, afl::base::Memory<const ColorQuad_t> colorDefinitions)
{
    while (!colorDefinitions.empty()) {
        size_t amountCopied = afl::base::Memory<ColorQuad_t>(m_palette).subrange(start).copyFrom(colorDefinitions).size();
        colorDefinitions.split(amountCopied);
        start = 0;
    }
    return *this;
}

gfx::ColorQuantizer&
gfx::ColorQuantizer::setPalette(uint8_t slot, ColorQuad_t colorDefinition)
{
    m_palette[slot] = colorDefinition;
    return *this;
}
