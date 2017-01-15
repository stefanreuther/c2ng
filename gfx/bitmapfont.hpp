/**
  *  \file gfx/bitmapfont.hpp
  */
#ifndef C2NG_GFX_BITMAPFONT_HPP
#define C2NG_GFX_BITMAPFONT_HPP

#include "afl/charset/unicode.hpp"
#include "gfx/font.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/io/stream.hpp"

namespace gfx {

    class BitmapGlyph;

    /** Bitmap font. Contains a list of bitmaps. Can render Unicode text. */
    class BitmapFont : public Font {
     public:
        BitmapFont();
        ~BitmapFont();

        // BitmapFont methods:
        void addNewGlyph(afl::charset::Unichar_t id, BitmapGlyph* g);
        const BitmapGlyph* getGlyph(afl::charset::Unichar_t id) const;
        uint32_t getCurrentCharacterLimit() const;
        int getHeight() const;
        void load(afl::io::Stream& s, int index);

        // Font virtuals:
        virtual void outText(BaseContext& ctx, Point pt, String_t text);
        virtual int getTextWidth(String_t text);
        virtual int getTextHeight(String_t text);

     private:
        /** Nested array of glyphs. The inner vectors contain up to 256
            character glyphs corresponding to the lower 8 bits of an
            Unicode codepoint. The outer vector is indexed by the upper 8
            bits of a codepoint. */
        afl::container::PtrVector<afl::container::PtrVector<BitmapGlyph> > glyphs;

        /** Height of this font. */
        int height;
    };
}

#endif
