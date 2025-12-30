/**
  *  \file ui/reshack/fontutil.hpp
  *  \brief Class ui::reshack::FontUtil
  */
#ifndef C2NG_UI_RESHACK_FONTUTIL_HPP
#define C2NG_UI_RESHACK_FONTUTIL_HPP

#include <utility>
#include "afl/charset/codepage.hpp"
#include "afl/io/stream.hpp"
#include "gfx/bitmapfont.hpp"
#include "gfx/palettizedpixmap.hpp"

namespace ui { namespace reshack {

    /** Aggregation of assorted utilities related to font editing. */
    class FontUtil {
     public:
        /** Create a pixmap.
            @param w Width
            @param h Height
            @return new pixmap */
        static afl::base::Ptr<gfx::PalettizedPixmap> createPixmap(int w, int h);

        /** Convert a character glyph to an editable pixmap.
            @param fnt Font
            @param ch  Character number
            @return New pixmap. Never null. */
        static afl::base::Ptr<gfx::PalettizedPixmap> createPixmapFromGlyph(const gfx::BitmapFont& fnt, afl::charset::Unichar_t ch);

        /** Convert pixmap to character glyph.
            @param pix Pixmap
            @return newly-allocated glyph. Can be 0 if pixmap is empty (zero-size). */
        static gfx::BitmapGlyph* createGlyphFromPixmap(afl::base::Ptr<const gfx::PalettizedPixmap> pix);

        /** Find previous existing (non-null) character from font.
            @param fnt Font
            @param ch  Starting character
            @return New character number of non-null or first character */
        static afl::charset::Unichar_t findPreviousExistingCharacter(const gfx::BitmapFont& fnt, afl::charset::Unichar_t ch);

        /** Find next existing (non-null) character from font.
            @param fnt Font
            @param ch  Starting character
            @return New character number of non-null or last character  */
        static afl::charset::Unichar_t findNextExistingCharacter(const gfx::BitmapFont& fnt, afl::charset::Unichar_t ch);

        /** Find first used line in glyph.
            @param g Glyph
            @return 0-based line number (Y coordinate) or top-most set pixel; -1 if glyph is empty */
        static int findTop(const gfx::BitmapGlyph& g);

        /** Find last used line in glyph.
            @param g Glyph
            @return 0-based line number (Y coordinate) or bottom-most set pixel; -1 if glyph is empty */
        static int findBottom(const gfx::BitmapGlyph& g);

        /** Find font margins.
            Returns a pair, where
            - first = number of lines that are empty on top of every character
            - second = number of lines that are empty on bottom of every character
            @param font Font
            @return result pair */
        static std::pair<int,int> findFontMargins(const gfx::BitmapFont& font);

        /** Change font alignment.
            @param font       Font, updated in-place
            @param addTop     Number of lines to add on top of every character (negative to remove)
            @param addBottom  Number of lines to add at bottom of every character (negative to remove)
            @return true on success, false if parameters make the new font have zero size */
        static bool changeFontAlignment(gfx::BitmapFont& font, int addTop, int addBottom);

        /** Synthesize character by combining multiple glyphs.
            This can be used to synthesize, for example, LATIN CAPITAL LETTER A WITH ACUTE
            from LATIN CAPITAL LETTER A and COMBINING ACUTE ACCENT.
            @param font   Font
            @param chars  Input characters (base character first)
            @return newly-allocated combined character */
        static afl::base::Ptr<gfx::PalettizedPixmap> synthesizeCombinedCharacter(const gfx::BitmapFont& font, afl::base::Memory<const afl::charset::Unichar_t> chars);

        /** Synthesize graphic (block) character from scratch.
            @param font   Font, must have U+2588 to define size of all characters
            @param ch     Character to generate
            @return newly-allocated character */
        static afl::base::Ptr<gfx::PalettizedPixmap> synthesizeGraphicCharacter(const gfx::BitmapFont& font, afl::charset::Unichar_t ch);

        /** Change encoding of a font.
            Assumes that this font (first 256 characters) are encoded according to a codepage,
            and rearranges them at their proper Unicode positions.
            @param font   Font
            @param fromCodepage Codepage to convert from */
        static void changeFontEncoding(gfx::BitmapFont& font, const afl::charset::Codepage& fromCodepage);

        /** Save font.
            @param out       Output file
            @param font      Font
            @param comment   Comment/font name to place in header
            @param encoding  Encoding to place in header */
        static void saveFont(afl::io::Stream& out, const gfx::BitmapFont& font, String_t comment, uint8_t encoding);

        /** Load BDF font.
            BDF is Adobe's Glyph Bitmap Distribution File Format, which is used (among others) as portable storage for X11 fonts.

            This loader is only in the resource editor, not in the main program,
            because most BDF fonts need a little manual work before being actually usable in PCC2.

            This file format doesn't seem to be fully documented anywhere; a partial spec is on
            https://adobe-type-tools.github.io/font-tech-notes/pdfs/5005.BDF_Spec.pdf
            (formerly http://partners.adobe.com/public/developer/en/font/5005.BDF_Spec.pdf).

            @param in Input file to read from
            @return newly-allocated font if any characters were successfully loaded */
        static afl::base::Ptr<gfx::BitmapFont> loadBDF(afl::io::Stream& in);
    };

} }

#endif
