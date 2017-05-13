/**
  *  \file gfx/bitmapfont.hpp
  *  \brief Class gfx::BitmapFont
  */
#ifndef C2NG_GFX_BITMAPFONT_HPP
#define C2NG_GFX_BITMAPFONT_HPP

#include "afl/charset/unicode.hpp"
#include "gfx/font.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/io/stream.hpp"

namespace gfx {

    class BitmapGlyph;

    /** Bitmap font.
        A bitmap font contains a list of bitmaps (BitmapGlyph) it uses to render Unicode characters.
        (This is not a Unicode renderer and does not support combining characters and the like.)
        Such fonts can be created in a variety of ways; PCC2 uses a custom font file format. */
    class BitmapFont : public Font {
     public:
        /** Construct an empty font. */
        BitmapFont();

        /** Destructor. */
        ~BitmapFont();

        /*
         *  BitmapFont methods:
         */

        /** Add new glyph.
            If there already is a glyph with that Id, it is replaced.
            \param id Unicode character
            \param g Glyph. Can be null. BitmapFont assumes ownership. */
        void addNewGlyph(afl::charset::Unichar_t id, BitmapGlyph* g);

        /** Get glyph for a character.
            \param id Character id
            \return Glyph; 0 if none present. */
        const BitmapGlyph* getGlyph(afl::charset::Unichar_t id) const;

        /** Get current upper bound of character Ids.
            This value can be used as an upper limit for iteration over all characters.
            \return some n, such that getGlyph(m) returns null for all m >= n. */
        uint32_t getCurrentCharacterLimit() const;

        /** Get font height.
            \return height in pixels */
        int getHeight() const;

        /** Load bitmap font from "FN" file.
            This is a custom font file format, used by PCC 1.x as well as PCC2.
            \param s Stream
            \param index Font index (typically 0) */
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
