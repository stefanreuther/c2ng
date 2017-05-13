/**
  *  \file gfx/fontlist.hpp
  *  \brief Class gfx::FontList
  */
#ifndef C2NG_GFX_FONTLIST_HPP
#define C2NG_GFX_FONTLIST_HPP

#include "gfx/font.hpp"
#include "afl/base/ptr.hpp"
#include "gfx/fontrequest.hpp"

namespace gfx {

    /** Font list.
        Manages a list of fonts and definitions, and allows requesting fonts by FontRequest. */
    class FontList {
     public:
        /** Default constructor.
            Makes an empty list. */
        FontList();

        /** Destructor. */
        ~FontList();

        /** Add font.
            \param defn Font definition
            \param font Font */
        void addFont(FontRequest defn, afl::base::Ptr<Font> font);

        /** Find a font.
            If an exact match is not found, tries relaxing the search by dropping slant, weight, size, style requirements in this order.
            If multiple matches of identical quality exist, returns the one that was added first.
            \param req Request
            \return found font. Null if no font matches (only happens if the list is empty) */
        afl::base::Ptr<Font> findFont(FontRequest req) const;

     private:
        struct Element {
            FontRequest definition;
            afl::base::Ptr<Font> font;
            Element(FontRequest defn, afl::base::Ptr<Font> font)
                : definition(defn),
                  font(font)
                { }
        };
        std::vector<Element> m_fonts;

        afl::base::Ptr<Font> find(FontRequest req) const;
    };

}

#endif
