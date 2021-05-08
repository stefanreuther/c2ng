/**
  *  \file gfx/bitmapfont.cpp
  *  \brief Class gfx::BitmapFont
  */

#include <memory>
#include "gfx/bitmapfont.hpp"
#include "afl/base/growablememory.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/bits/int16le.hpp"
#include "afl/bits/uint16le.hpp"
#include "afl/bits/uint32le.hpp"
#include "afl/bits/value.hpp"
#include "afl/charset/utf8reader.hpp"
#include "afl/except/fileformatexception.hpp"
#include "gfx/bitmapglyph.hpp"

namespace {
    uint16_t mapCharacterId(int /*encoding*/, uint16_t chid)
    {
        /* We would remap character Ids here:
           encoding = 0(cp437/pcc1), 1(cp866), 2(unicode) */
        return chid;
    }
}

// Construct an empty font.
gfx::BitmapFont::BitmapFont()
    : glyphs(),
      height(0)
{
    // ex GfxBitmapFont::GfxBitmapFont
}

// Destructor.
gfx::BitmapFont::~BitmapFont()
{ }

// Add new glyph.
void
gfx::BitmapFont::addNewGlyph(afl::charset::Unichar_t id, BitmapGlyph* g)
{
    // ex GfxBitmapFont::addNewGlyph
    size_t outerIndex = id >> 8;
    size_t innerIndex = (id & 255);

    /* Do not enlarge arrays to store null in it. */
    if (g == 0
        && (outerIndex >= glyphs.size()
            || glyphs[outerIndex] == 0
            || innerIndex >= glyphs[outerIndex]->size()))
    {
        return;
    }

    try {
        /* Make sure outer array is big enough */
        while (glyphs.size() <= outerIndex) {
            glyphs.pushBackNew(0);
        }

        /* Make sure inner array exists */
        if (glyphs[outerIndex] == 0) {
            glyphs.replaceElementNew(outerIndex, new afl::container::PtrVector<BitmapGlyph>());
        }

        /* Make sure inner array is big enough */
        while (glyphs[outerIndex]->size() <= innerIndex) {
            glyphs[outerIndex]->pushBackNew(0);
        }
    }
    catch (...) {
        delete g;
        throw;
    }

    /* Store glyph */
    glyphs[outerIndex]->replaceElementNew(innerIndex, g);

    /* Update height */
    if (g != 0) {
        int h = g->getHeight();
        if (h > height) {
            height = h;
        }
    }
}

// Get glyph for a character.
const gfx::BitmapGlyph*
gfx::BitmapFont::getGlyph(afl::charset::Unichar_t id) const
{
    // ex GfxBitmapFont::getGlyph
    size_t outerIndex = id >> 8;
    size_t innerIndex = (id & 255);
    if (outerIndex < glyphs.size()
        && glyphs[outerIndex] != 0
        && innerIndex < glyphs[outerIndex]->size())
    {
        return (*glyphs[outerIndex])[innerIndex];
    } else {
        return 0;
    }
}

// Get current upper bound of character Ids.
uint32_t
gfx::BitmapFont::getCurrentCharacterLimit() const
{
    // ex GfxBitmapFont::getCurrentCharacterLimit
    size_t limit = 256 * glyphs.size();
    if (glyphs.size() != 0 && glyphs.back() != 0) {
        limit -= 256;
        limit += glyphs.back()->size();
    }
    return static_cast<uint32_t>(limit);
}

// Get font height.
int
gfx::BitmapFont::getHeight() const
{
    // ex GfxBitmapFont::getHeight
    return height;
}

namespace {
    struct FileHeader {
        char signature[2];
        afl::bits::Value<afl::bits::Int16LE> numFonts;
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

    struct Hint {
        afl::bits::Value<afl::bits::UInt16LE> x;
        afl::bits::Value<afl::bits::UInt16LE> y;       
    };
    static_assert(sizeof(Hint) == 4, "sizeof Hint");
}

// Load bitmap font from "FN" file.
void
gfx::BitmapFont::load(afl::io::Stream& s, int index, afl::string::Translator& tx)
{
    afl::io::Stream::FileSize_t base = s.getPos();

    // Read file header
    FileHeader header;
    s.fullRead(afl::base::fromObject(header));
    if (header.signature[0] != 'F' || header.signature[1] != 'N') {
        throw afl::except::FileFormatException(s, tx("File is missing required signature"));
    }

    const int numFonts = header.numFonts;
    if (numFonts <= 0 || index >= numFonts) {
        throw afl::except::FileFormatException(s, tx("File does not contain required font"));
    }

    // Read font index
    IndexHeader indexHeader;
    s.setPos(base + 4 + 8*index);
    s.fullRead(afl::base::fromObject(indexHeader));
    const uint8_t encoding = indexHeader.encoding;

    // Read font header
    FontHeader fontHeader;
    s.setPos(base + indexHeader.pos);
    s.fullRead(afl::base::fromObject(fontHeader));
    const uint16_t type   = fontHeader.type;
    const uint16_t height = fontHeader.height;
    const uint16_t nchars = fontHeader.numChars;

    // Read character headers
    afl::base::GrowableMemory<CharacterHeader> charHeaders;
    charHeaders.resize(nchars);
    s.fullRead(charHeaders.toBytes());

    // Read characters
    afl::base::Memory<const CharacterHeader> cch(charHeaders);
    while (const CharacterHeader* pch = cch.eat()) {
        const uint32_t chpos   = pch->pos;
        const uint16_t chid    = mapCharacterId(encoding, pch->id);
        const uint16_t chwidth = pch->width;

        if (chwidth != 0) {
            // Read bitmap
            const uint32_t chsize = (chwidth+7)/8 * height;
            afl::base::GrowableMemory<uint8_t> bits;
            bits.resize(chsize);

            s.setPos(base + chpos);
            s.fullRead(bits);
            std::auto_ptr<BitmapGlyph> glyph(new BitmapGlyph(chwidth, height, bits));

            // Read anti-aliasing hints
            if (type == 3) {
                // Read number of hints
                afl::bits::Value<afl::bits::UInt16LE> numHints;
                s.fullRead(afl::base::fromObject(numHints));

                // Read the hints
                afl::base::GrowableMemory<Hint> hints;
                hints.resize(numHints);
                s.fullRead(hints.toBytes());

                // Process them
                afl::base::Memory<const Hint> hh(hints);
                while (const Hint* h = hh.eat()) {
                    glyph->addAAHint(h->x, h->y);
                }
            }

            addNewGlyph(chid, glyph.release());
        } else {
            addNewGlyph(chid, 0);
        }
    }
}


// Font virtuals:
void
gfx::BitmapFont::outText(BaseContext& ctx, Point pt, String_t text)
{
    // ex GfxBitmapFont::outText
    afl::charset::Utf8Reader rdr(afl::string::toBytes(text), 0);
    while (rdr.hasMore()) {
        afl::charset::Unichar_t ch = rdr.eat();
        if (const BitmapGlyph* g = getGlyph(ch)) {
            // Render this character
            g->draw(ctx, pt);
            pt.addX(g->getWidth());
        } else if (afl::charset::isErrorCharacter(ch)) {
            // Try to render two digits
            uint8_t base = afl::charset::getErrorCharacterId(ch);
            const BitmapGlyph* a = getGlyph(0xE100 + ((base >> 4) & 15));
            const BitmapGlyph* b = getGlyph(0xE130 + (base & 15));
            if (a) {
                a->draw(ctx, pt);
                if (b) {
                    b->draw(ctx, pt);
                }
                pt.addX(a->getWidth());
            }
        } else {
            // Try to render four digits
            const BitmapGlyph* a = getGlyph(0xE100 + ((ch >> 12) & 15));
            const BitmapGlyph* b = getGlyph(0xE110 + ((ch >> 8) & 15));
            const BitmapGlyph* c = getGlyph(0xE120 + ((ch >> 4) & 15));
            const BitmapGlyph* d = getGlyph(0xE130 + (ch & 15));
            if (a) {
                a->draw(ctx, pt);
                if (b) {
                    b->draw(ctx, pt);
                }
                if (c) {
                    c->draw(ctx, pt);
                }
                if (d) {
                    d->draw(ctx, pt);
                }
                pt.addX(a->getWidth());
            }
        }
    }
}

int
gfx::BitmapFont::getTextWidth(String_t text)
{
    // ex GfxBitmapFont::getTextWidth
    afl::charset::Utf8Reader rdr(afl::string::toBytes(text), 0);
    int total = 0;
    while (rdr.hasMore()) {
        afl::charset::Unichar_t ch = rdr.eat();
        if (const BitmapGlyph* g = getGlyph(ch)) {
            total += g->getWidth();
        } else if (afl::charset::isErrorCharacter(ch)) {
            if (const BitmapGlyph* g = getGlyph(0xE100 + ((afl::charset::getErrorCharacterId(ch) >> 4) & 15))) {
                total += g->getWidth();
            }
        } else {
            if (const BitmapGlyph* g = getGlyph(0xE100 + ((ch >> 12) & 15))) {
                total += g->getWidth();
            }
        }
    }
    return total;
    
}

int
gfx::BitmapFont::getTextHeight(String_t /*text*/)
{
    return height;
}

