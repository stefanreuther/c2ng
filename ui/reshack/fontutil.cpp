/**
  *  \file ui/reshack/fontutil.cpp
  *  \brief Class ui::reshack::FontUtil
  */

#include "ui/reshack/fontutil.hpp"

#include <cstdio>
#include <cstring>
#include "afl/base/countof.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/bits/pack.hpp"
#include "afl/bits/uint16le.hpp"
#include "afl/bits/uint32le.hpp"
#include "afl/bits/value.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/char.hpp"
#include "afl/string/parse.hpp"
#include "gfx/bitmapglyph.hpp"
#include "gfx/codec/custom.hpp"
#include "ui/reshack/palette.hpp"
#include "util/io.hpp"
#include "util/string.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using gfx::PalettizedPixmap;
using ui::reshack::Palette;

namespace {
    void drawSolidBar(gfx::Canvas& can, const gfx::Rectangle& area, gfx::Color_t color)
    {
        can.drawBar(area, color, color, gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA);
    }

    bool checkOverlap(const PalettizedPixmap& pix, const gfx::BitmapGlyph& glyph, int dx, int dy)
    {
        for (int y = 0; y < glyph.getHeight(); ++y) {
            for (int x = 0; x < glyph.getWidth(); ++x) {
                if (glyph.get(x, y)) {
                    if (const uint8_t* p = pix.row(y+dy).at(x+dx)) {
                        if (*p != ui::reshack::Palette::FC_Black) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    /** Synthesize frame character. This can be used for U+2500 .. U+257F.
        The shape is described by an octal number:
        - 0Nxxxx = number of segments for dashed lines, 0 for solid lines
        - 0xNxxx = shape of "up" wing, 0=none, 1=light, 2=heavy, 3=double
        - 0xxNxx = shape of "right" wing
        - 0xxxNx = shape of "down" wing
        - 0xxxxN = shape of "left" wing
        \param painter Draw character here
        \param shape Shape description */
    void synthesizeFrameCharacter(PalettizedPixmap& pix, uint16_t shape)
    {
        // Figure out center
        int xs = pix.getWidth();
        int ys = pix.getHeight();
        int xc = xs / 2;
        int yc = (ys - 1) / 2;

        // Clear it
        pix.pixels().fill(Palette::FC_Black);

        // Thick lines for digit 2 or 3
        Ref<gfx::Canvas> can = pix.makeCanvas();
        if ((shape & 07000) > 01000) {
            can->drawVLine(gfx::Point(xc-1, 0), yc+1, Palette::FC_White, gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
            can->drawVLine(gfx::Point(xc+1, 0), yc+1, Palette::FC_White, gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
        }
        if ((shape & 00700) > 00100) {
            can->drawHLine(gfx::Point(xc, yc-1), xs-xc, Palette::FC_White, gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
            can->drawHLine(gfx::Point(xc, yc+1), xs-xc, Palette::FC_White, gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
        }
        if ((shape & 00070) > 00010) {
            can->drawVLine(gfx::Point(xc-1, yc), ys-yc, Palette::FC_White, gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
            can->drawVLine(gfx::Point(xc+1, yc), ys-yc, Palette::FC_White, gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
        }
        if ((shape & 00007) > 00001) {
            can->drawHLine(gfx::Point(0, yc-1), xc+1, Palette::FC_White, gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
            can->drawHLine(gfx::Point(0, yc+1), xc+1, Palette::FC_White, gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
        }

        // Re-blank centers for digit 3
        if ((shape & 07000) == 03000) {
            can->drawVLine(gfx::Point(xc, 0), yc+1, Palette::FC_Black, gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
        }
        if ((shape & 00700) == 00300) {
            can->drawHLine(gfx::Point(xc, yc), xs-xc, Palette::FC_Black, gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
        }
        if ((shape & 00070) == 00030) {
            can->drawVLine(gfx::Point(xc, yc), ys-yc, Palette::FC_Black, gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
        }
        if ((shape & 00007) == 00003) {
            can->drawHLine(gfx::Point(0, yc), xc+1, Palette::FC_Black, gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
        }

        // Draw center for digit 1 and 2
        if ((shape & 07000) != 03000 && (shape & 07000) != 0) {
            can->drawVLine(gfx::Point(xc, 0), yc+1, Palette::FC_White, gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
        }
        if ((shape & 00700) != 00300 && (shape & 00700) != 0) {
            can->drawHLine(gfx::Point(xc, yc), xs-xc, Palette::FC_White, gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
        }
        if ((shape & 00070) != 00030 && (shape & 00070) != 0) {
            can->drawVLine(gfx::Point(xc, yc), ys-yc, Palette::FC_White, gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
        }
        if ((shape & 00007) != 00003 && (shape & 00007) != 0) {
            can->drawHLine(gfx::Point(0, yc), xc+1, Palette::FC_White, gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
        }

        // Gaps for dashed lines
        int ngaps = (shape & 070000) >> 12;
        if (ngaps != 0) {
            int npieces = 2*ngaps - 1;
            if ((shape & 07070) != 0) {
                // We have vertical lines
                for (int i = 1; i < npieces; i += 2) {
                    int y1 = ys * i / npieces;
                    int y2 = ys * (i+1) / npieces;
                    drawSolidBar(*can, gfx::Rectangle(0, y1, xs, y2-y1), Palette::FC_Black);
                }
            } else {
                // We have horizontal lines
                for (int i = 1; i < npieces; i += 2) {
                    int x1 = xs * i / npieces;
                    int x2 = xs * (i+1) / npieces;
                    drawSolidBar(*can, gfx::Rectangle(x1, 0, x2-x1, ys), Palette::FC_Black);
                }
            }
        }
    }

    /** Synthesize block character. This can be used for U+2580 .. U+259FF.
        The shape is described by an octal number:
        - 0Nxx: kind, 1=vertical split, 2=horizontal split, 3=quadrant, 4=shade
        - 0xNx: first cell
        - 0xxN: number of cells
        For quadrant and shade, the last two digits give four bits of pattern
        that defines which quadrants / pixels are drawn.
        @param painter Draw character here
        @param shape Shape description */
    void synthesizeBlockCharacter(PalettizedPixmap& pix, uint16_t shape)
    {
        // Figure out center
        const int xs = pix.getWidth();
        const int ys = pix.getHeight();

        // Clear it
        pix.pixels().fill(Palette::FC_Black);

        Ref<gfx::Canvas> can = pix.makeCanvas();
        switch ((shape & 0700) >> 6) {
         case 1: {
            // Vertical
            int a = (shape & 070) >> 3;
            int b = (shape & 7);
            int y1 = ys * a / 8;
            int y2 = ys * (a+b+1) / 8;
            drawSolidBar(*can, gfx::Rectangle(0, y1, xs, y2-y1), Palette::FC_White);
            break;
         }

         case 2: {
            // Horizontal
            int a = (shape & 070) >> 3;
            int b = (shape & 7);
            int x1 = xs * a / 8;
            int x2 = xs * (a+b+1) / 8;
            drawSolidBar(*can, gfx::Rectangle(x1, 0, x2-x1, ys), Palette::FC_White);
            break;
         }

         case 3: {
            // Quadrant
            int xc = xs / 2;
            int yc = ys / 2;
            if ((shape & 1) != 0) {
                drawSolidBar(*can, gfx::Rectangle(0, 0, xc, yc), Palette::FC_White);
            }
            if ((shape & 2) != 0) {
                drawSolidBar(*can, gfx::Rectangle(xc, 0, xs-xc, yc), Palette::FC_White);
            }
            if ((shape & 4) != 0) {
                drawSolidBar(*can, gfx::Rectangle(0, yc, xc, ys-yc), Palette::FC_White);
            }
            if ((shape & 010) != 0) {
                drawSolidBar(*can, gfx::Rectangle(xc, yc, xs-xc, ys-yc), Palette::FC_White);
            }
            break;
         }

         case 4: {
            // Shade
            gfx::FillPattern p;
            p[0] = p[2] = p[4] = p[6] = 0x55 * (shape & 3);
            p[1] = p[3] = p[5] = p[7] = 0x55 * ((shape >> 2) & 3);
            can->drawBar(gfx::Rectangle(0, 0, xs, ys), Palette::FC_White, Palette::FC_Black, p, gfx::OPAQUE_ALPHA);
            break;
         }
        }
    }

    /*
     *  Saving
     *
     *  Structure definitions duplicated from gfx/bitmapfont for now,
     *  so we don't have the save code in production.
     */

    struct FileHeader {
        char signature[2];
        afl::bits::Value<afl::bits::UInt16LE> numFonts;
    };
    static_assert(sizeof(FileHeader) == 4, "sizeof FileHeader");

    struct IndexHeader {
        afl::bits::Value<afl::bits::UInt32LE> pos;
        uint8_t flags;
        uint8_t encoding;
        uint8_t reserved[2];
    };
    static_assert(sizeof(IndexHeader) == 8, "sizeof IndexHeader");

    struct FontHeader {
        afl::bits::Value<afl::bits::UInt16LE> type;
        afl::bits::Value<afl::bits::UInt16LE> height;
        afl::bits::Value<afl::bits::UInt16LE> numChars;
    };
    static_assert(sizeof(FontHeader) == 6, "sizeof FontHeader");

    struct CharacterHeader {
        afl::bits::Value<afl::bits::UInt32LE> pos;
        afl::bits::Value<afl::bits::UInt16LE> id;
        afl::bits::Value<afl::bits::UInt16LE> width;
    };
    static_assert(sizeof(CharacterHeader) == 8, "sizeof CharacterHeader");


    static void saveFontHeader(afl::io::Stream& out,
                               uint32_t& font_address,
                               const String_t& comment,
                               uint8_t encoding,
                               const gfx::BitmapFont& font,
                               std::vector<uint32_t>& character_ids,
                               std::vector<uint32_t>& character_addresses)
    {
        // Main header:
        //  FileHeader:
        //    2 bytes signature
        //    2 bytes number of fonts (=1)
        //  IndexHeader:
        //    4 bytes font address
        //    4 bytes flags
        //  1 byte font name (comment) length
        //  font name

        afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);

        // Header
        FileHeader header;
        header.signature[0] = 'F';
        header.signature[1] = 'N';
        header.numFonts = 1;
        out.fullWrite(afl::base::fromObject(header));

        IndexHeader index;
        index.pos = font_address;
        index.flags = 0;
        index.encoding = encoding;
        index.reserved[0] = 0;
        index.reserved[1] = 0;
        out.fullWrite(afl::base::fromObject(index));

        // Name
        util::storePascalStringTruncate(out, comment, cs);

        // Font data. font_address points here.
        font_address = static_cast<uint32_t>(out.getPos());

        // FontHeader
        // 2 bytes type (=3)
        // 2 bytes height
        // 2 bytes number of characters
        FontHeader fontHeader;
        fontHeader.type = 3;
        fontHeader.height = static_cast<uint16_t>(font.getHeight());
        fontHeader.numChars = static_cast<uint16_t>(character_ids.size());
        out.fullWrite(afl::base::fromObject(fontHeader));

        // Character headers
        for (size_t i = 0; i < character_ids.size(); ++i) {
            // 4 bytes address
            // 2 bytes id
            // 2 bytes width
            const gfx::BitmapGlyph* g = font.getGlyph(character_ids[i]);
            CharacterHeader ch;
            ch.pos   = character_addresses[i];
            ch.id    = static_cast<uint16_t>(character_ids[i]);
            ch.width = static_cast<uint16_t>(g != 0 ? g->getWidth() : 0);
            out.fullWrite(afl::base::fromObject(ch));
        }
    }
}

afl::base::Ptr<gfx::PalettizedPixmap>
ui::reshack::FontUtil::createPixmap(int w, int h)
{
    Ref<PalettizedPixmap> pix = PalettizedPixmap::create(w, h);
    pix->setPalette(0, gfx::codec::Custom::getPalette());
    return pix.asPtr();
}

afl::base::Ptr<gfx::PalettizedPixmap>
ui::reshack::FontUtil::createPixmapFromGlyph(const gfx::BitmapFont& fnt, afl::charset::Unichar_t ch)
{
    // ex glyphToPixmap(const GfxBitmapFont& fnt, uint32_t ch)
    const gfx::BitmapGlyph* g = fnt.getGlyph(ch);

    // Create pixmap
    Ptr<PalettizedPixmap> pix = createPixmap(g != 0 ? g->getWidth() : 0, fnt.getHeight());

    // Populate pixmap
    if (g != 0) {
        g->drawColored(*pix->makeCanvas(), gfx::Point(0, 0), ui::reshack::Palette::FC_White, ui::reshack::Palette::FC_Half);
    }
    return pix;
}

gfx::BitmapGlyph*
ui::reshack::FontUtil::createGlyphFromPixmap(afl::base::Ptr<const gfx::PalettizedPixmap> pix)
{
    // pixmapToGlyph(Ptr<GfxPixmap> pix)
    if (pix->getWidth() == 0) {
        // Empty image: return null glyph
        return 0;
    } else {
        // Non-empty image: build a glyph
        std::auto_ptr<gfx::BitmapGlyph> g(new gfx::BitmapGlyph(static_cast<uint16_t>(pix->getWidth()), static_cast<uint16_t>(pix->getHeight())));
        afl::base::Memory<const uint8_t> pixels = pix->pixels();
        for (int y = 0; y < pix->getHeight(); ++y) {
            for (int x = 0; x < pix->getWidth(); ++x) {
                if (const uint8_t* p = pixels.eat()) {
                    if (*p == ui::reshack::Palette::FC_Black) {
                        // Ignore
                    } else if (*p == ui::reshack::Palette::FC_Half) {
                        g->addAAHint(static_cast<uint16_t>(x), static_cast<uint16_t>(y));
                    } else {
                        g->set(x, y, true);
                    }
                }
            }
        }
        return g.release();
    }
}

afl::charset::Unichar_t
ui::reshack::FontUtil::findPreviousExistingCharacter(const gfx::BitmapFont& fnt, afl::charset::Unichar_t ch)
{
    while (ch > 0) {
        --ch;
        if (fnt.getGlyph(ch) != 0) {
            break;
        }
    }
    return ch;
}

afl::charset::Unichar_t
ui::reshack::FontUtil::findNextExistingCharacter(const gfx::BitmapFont& fnt, afl::charset::Unichar_t ch)
{
    while (ch < 0xFFFF) {
        ++ch;
        if (fnt.getGlyph(ch) != 0) {
            break;
        }
    }
    return ch;
}

int
ui::reshack::FontUtil::findTop(const gfx::BitmapGlyph& g)
{
    for (int y = 0; y < g.getHeight(); ++y) {
        for (int x = 0; x < g.getWidth(); ++x) {
            if (g.get(x, y)) {
                return y;
            }
        }
    }
    return -1;
}

int
ui::reshack::FontUtil::findBottom(const gfx::BitmapGlyph& g)
{
    for (int y = g.getHeight(); y > 0; --y) {
        for (int x = 0; x < g.getWidth(); ++x) {
            if (g.get(x, y-1)) {
                return y;
            }
        }
    }
    return -1;
}

std::pair<int,int>
ui::reshack::FontUtil::findFontMargins(const gfx::BitmapFont& font)
{
    int top = font.getHeight();
    int bot = 0;
    for (uint32_t i = 0, e = font.getCurrentCharacterLimit(); i < e; ++i) {
        if (const gfx::BitmapGlyph* g = font.getGlyph(i)) {
            for (int y = 0; y < g->getHeight(); ++y) {
                for (int x = 0; x < g->getWidth(); ++x) {
                    if (g->get(x, y)) {
                        if (y < top) {
                            top = y;
                        }
                        if (y > bot) {
                            bot = y+1;
                        }
                    }
                }
            }
        }
    }
    return std::make_pair(top, font.getHeight()-bot);
}

bool
ui::reshack::FontUtil::changeFontAlignment(gfx::BitmapFont& font, int addTop, int addBottom)
{
    int newHeight = font.getHeight() + addTop + addBottom;
    if (newHeight <= 0) {
        return false;
    }

    font.setHeight(newHeight);
    for (uint32_t i = 0, e = font.getCurrentCharacterLimit(); i < e; ++i) {
        if (const gfx::BitmapGlyph* g = font.getGlyph(i)) {
            std::auto_ptr<gfx::BitmapGlyph> ng(new gfx::BitmapGlyph(static_cast<uint16_t>(g->getWidth()), static_cast<uint16_t>(newHeight)));
            for (int y = 0; y < newHeight; ++y) {
                if (y - addTop >= 0 && y - addTop < g->getHeight()) {
                    for (int x = 0; x < g->getWidth(); ++x) {
                        if (g->get(x, y - addTop)) {
                            ng->set(x, y, true);
                        }
                    }
                }
            }
            const std::vector<uint16_t>& aa = g->getAAData();
            for (size_t z = 0; z < aa.size(); z += 2) {
                if (aa[z+1] + addTop >= 0 && aa[z+1] + addTop < newHeight) {
                    ng->addAAHint(aa[z], static_cast<uint16_t>(aa[z+1] + addTop));
                }
            }
            font.addNewGlyph(i, ng.release());
        }
    }
    return true;
}

afl::base::Ptr<gfx::PalettizedPixmap>
ui::reshack::FontUtil::synthesizeCombinedCharacter(const gfx::BitmapFont& font, afl::base::Memory<const afl::charset::Unichar_t> chars)
{
    // Figure out width
    int width = 0;
    for (size_t i = 0; i < chars.size(); ++i) {
        if (const gfx::BitmapGlyph* g = font.getGlyph(*chars.at(i))) {
            width = std::max(width, g->getWidth());
        }
    }

    // Build result
    Ptr<PalettizedPixmap> pix = createPixmap(width, font.getHeight());
    Ref<gfx::Canvas> can = pix->makeCanvas();
    for (size_t i = 0; i < chars.size(); ++i) {
        if (const gfx::BitmapGlyph* g = font.getGlyph(*chars.at(i))) {
            int dx = (width - g->getWidth())/2;
            int dy = 0;
            if (i != 0) {
                dy = -g->getHeight();
                while (!checkOverlap(*pix, *g, dx, dy) && dy < g->getHeight()) {
                    ++dy;
                }
                dy -= 2;
            }

            g->drawColored(*can, gfx::Point(dx, dy), Palette::FC_White, Palette::FC_Half);
        }
    }

    return pix;
}

afl::base::Ptr<gfx::PalettizedPixmap>
ui::reshack::FontUtil::synthesizeGraphicCharacter(const gfx::BitmapFont& font, afl::charset::Unichar_t ch)
{
    // synthesizeCharacter(RHPainter& painter, const GfxBitmapFont& font, uint32_t ch)
    static const uint16_t frameShapes[] = {
        000101, 000202, 001010, 002020, 030101, 030202, 031010, 032020,
        040101, 040202, 041010, 042020, 000110, 000210, 000120, 000220,
        000011, 000012, 000021, 000022, 001100, 001200, 002100, 002200,
        001001, 001002, 002001, 002002, 001110, 001210, 002110, 001120,
        002120, 002210, 001220, 002220, 001011, 001012, 002011, 001021,
        002021, 002012, 001022, 002022, 000111, 000112, 000211, 000212,
        000121, 000122, 000221, 000222, 001101, 001102, 001201, 001202,
        002101, 002102, 002201, 002202, 001111, 001112, 001211, 001212,
        002111, 001121, 002121, 002112, 002211, 001122, 001221, 002212,
        001222, 002122, 002221, 002222, 020101, 020202, 021010, 022020,
        000303, 003030, 000310, 000130, 000330, 000013, 000031, 000033,
        001300, 003100, 003300, 001003, 003001, 003003, 001310, 003130,
        003330, 001013, 003031, 003033, 000313, 000131, 000333, 001303,
        003101, 003303, 001313, 003131, 003333, 000000, 000000, 000000,
        000000, 000000, 000000, 000000, 000001, 001000, 000100, 000010,
        000002, 002000, 000200, 000020, 000201, 001020, 000102, 002010,
    };

    static const uint16_t blockShapes[] = {
        0103, 0170, 0161, 0152, 0143, 0134, 0125, 0116,
        0107, 0206, 0205, 0204, 0203, 0202, 0201, 0200,
        0243, 0401, 0411, 0407, 0100, 0270, 0304, 0310,
        0301, 0315, 0311, 0307, 0313, 0302, 0306, 0316,
    };

    /* We need the block glyph which determines the size of all pseudo-graphic characters */
    const gfx::BitmapGlyph* blockGlyph = font.getGlyph(0x2588);
    if (blockGlyph != 0) {
        if (ch >= 0x2500 && ch < 0x2500 + countof(frameShapes) && frameShapes[ch - 0x2500] != 0) {
            Ptr<PalettizedPixmap> pix = createPixmap(blockGlyph->getWidth(), blockGlyph->getHeight());
            synthesizeFrameCharacter(*pix, frameShapes[ch - 0x2500]);
            return pix;
        }
        if (ch >= 0x2580 && ch < 0x2580 + countof(blockShapes)) {
            Ptr<PalettizedPixmap> pix = createPixmap(blockGlyph->getWidth(), blockGlyph->getHeight());
            synthesizeBlockCharacter(*pix, blockShapes[ch - 0x2580]);
            return pix;
        }
    }

    return 0;
}

void
ui::reshack::FontUtil::changeFontEncoding(gfx::BitmapFont& font, const afl::charset::Codepage& fromCodepage)
{
    // changeToUnicode(GfxBitmapFont& font, CharacterSet cs)
    // Remember all the glyphs and delete from font
    afl::container::PtrVector<gfx::BitmapGlyph> codepageGlyphs;
    for (int i = 128; i < 256; ++i) {
        if (const gfx::BitmapGlyph* fontGlyph = font.getGlyph(i)) {
            codepageGlyphs.pushBackNew(new gfx::BitmapGlyph(*fontGlyph));
            font.addNewGlyph(i, 0);
        } else {
            codepageGlyphs.pushBackNew(0);
        }
    }

    // Now copy them to their proper place
    for (int i = 128; i < 256; ++i) {
        afl::charset::Unichar_t uni = fromCodepage.m_characters[i-128];
        if (codepageGlyphs[i-128] != 0 && uni >= 128) {
            font.addNewGlyph(uni, codepageGlyphs.extractElement(i-128));
        }
    }
}

void
ui::reshack::FontUtil::saveFont(afl::io::Stream& out, const gfx::BitmapFont& font, String_t comment, uint8_t encoding)
{
    // RHFontEditor::saveFont(string_t name, string_t comment, int flags)
    // Build list of characters
    std::vector<uint32_t> character_ids;
    uint32_t max = font.getCurrentCharacterLimit();
    if (max > 0xFFFF) {
        max = 0xFFFF;
    }
    if (max < 0x100) {
        max = 0x100;
    }
    for (uint32_t i = 0; i < max; ++i) {
        if (i < 0x100 || font.getGlyph(i) != 0) {
            character_ids.push_back(i);
        }
    }

    // Prepare header
    std::vector<uint32_t> character_addresses(character_ids.size());
    uint32_t font_address = 0;
    saveFontHeader(out, font_address, comment, encoding, font, character_ids, character_addresses);

    // Save characters
    for (size_t i = 0; i < character_ids.size(); ++i) {
        const gfx::BitmapGlyph* g = font.getGlyph(character_ids[i]);
        character_addresses[i] = static_cast<uint32_t>(out.getPos());
        if (g != 0) {
            // Glyph exists
            const std::vector<uint8_t>& data = g->getData();
            const std::vector<uint16_t>& aadata = g->getAAData();
            out.fullWrite(data);

            size_t naa = aadata.size() / 2;
            afl::base::GrowableBytes_t aabuf;
            aabuf.resize(naa*4 + 2);

            uint16_t num[] = {static_cast<uint16_t>(naa)};
            afl::bits::packArray<afl::bits::UInt16LE>(aabuf.subrange(0, 2), num);
            afl::bits::packArray<afl::bits::UInt16LE>(aabuf.subrange(2), aadata);
            out.fullWrite(aabuf);
        } else {
            // Glyph does not exist. Write just a blank number-of-AA-hints entry
            uint8_t tmp[2];
            tmp[0] = tmp[1] = 0;
            out.fullWrite(tmp);
        }
    }

    // Update header
    out.setPos(0);
    saveFontHeader(out, font_address, comment, encoding, font, character_ids, character_addresses);
}

afl::base::Ptr<gfx::BitmapFont>
ui::reshack::FontUtil::loadBDF(afl::io::Stream& in)
{
    // loadBDF(GfxBitmapFont& font, Stream& s)
    Ptr<gfx::BitmapFont> font = new gfx::BitmapFont();

    uint16_t fontWidth = 0;
    uint16_t fontHeight = 0;
    int fontBaseline = 0;

    uint16_t charWidth = 0;
    uint16_t charHeight = 0;
    int charBaseline = 0;

    long charCode = -1;
    bool did = false;

    afl::io::TextFile tf(in);
    String_t line;
    while (tf.readLine(line)) {
        String_t arg;
        afl::string::strSplit(line, line, arg, " ");
        if (line == "STARTCHAR" || line == "ENDCHAR") {
            /* STARTCHAR starts a new character. ENDCHAR ends a
               character, in case we've missed it. Both mean we have
               to forget properties of the previous character. */
            charWidth = fontWidth;
            charHeight = fontHeight;
            charBaseline = fontBaseline;
            charCode = -1;
        } else if (line == "ENCODING") {
            /* ENCODING n specifies the codepoint. We assume anything
               in Latin-1 or Unicode so far; users can remap using
               the font editor's remap function. */
            afl::string::strToInteger(arg, charCode);
        } else if (line == "BBX") {
            /* BBX x y dx dy specifies the character's bounding box.
               We interpret this as:
               - x is the actual glyph width
               - y is the bitmap height, which may be smaller than
                 the actual font height (a property which we cannot
                 handle, all our glyphs have the same height)
               - dy is the origin within the character cell, relative
                 to the font bbox. */
            int x, y, dx, dy;
            if (std::sscanf(arg.c_str(), "%d %d %d %d", &x, &y, &dx, &dy) == 4) {
                charWidth = static_cast<uint16_t>(x);
                charHeight = static_cast<uint16_t>(y);
                charBaseline = dy;
            }
        } else if (line == "FONTBOUNDINGBOX") {
            /* FONTBOUNDINGBOX x y dx dy specifies the font bounding box.
               - x just serves as a default width
               - y is the font height
               - dy is the default origin within the character cell */
            int x, y, dx, dy;
            if (std::sscanf(arg.c_str(), "%d %d %d %d", &x, &y, &dx, &dy) == 4) {
                fontWidth = static_cast<uint16_t>(x);
                fontHeight = static_cast<uint16_t>(y);
                fontBaseline = dy;
            }
        } else if (line == "BITMAP") {
            /* BITMAP starts the bitmap definition, one line per pixel
               line, MSB-first hex digits. */
            if (charCode >= 0 && charCode < 0x10000 && charWidth > 0 && charHeight > 0) {
                std::auto_ptr<gfx::BitmapGlyph> g(new gfx::BitmapGlyph(charWidth, fontHeight));
                for (int y = 0; y < charHeight; ++y) {
                    // Read a line
                    if (!tf.readLine(line)) {
                        break;
                    }
                    if (line == "ENDCHAR") {
                        break;
                    }

                    // Parse it
                    int x = 0;
                    String_t::size_type n = 0;
                    while (n < line.size() && x < charWidth) {
                        // Read a nibble
                        const char* dig = "0123456789ABCDEF";
                        const char* p = std::strchr(dig, afl::string::charToUpper(line[n]));
                        if (p == 0) {
                            break;
                        }
                        const int ch = static_cast<int>(p - dig);
                        // Read four bits
                        for (int i = 0; i < 4 && x < charWidth; ++i) {
                            if (((ch << i) & 8) != 0) {
                                int rx = x;
                                int ry = y + (fontHeight - charHeight) + (fontBaseline - charBaseline);
                                if (rx >= 0 && ry >= 0 && rx < g->getWidth() && ry < g->getHeight()) {
                                    g->set(rx, ry, true);
                                }
                            }
                            ++x;
                        }
                        ++n;
                    }
                }
                font->addNewGlyph(static_cast<afl::charset::Unichar_t>(charCode), g.release());
                did = true;
            }
            charCode = -1;
        } else {
            // ignore
        }
    }
    return did ? font : 0;
}
