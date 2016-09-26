/**
  *  \file gfx/fontlist.hpp
  */
#ifndef C2NG_GFX_FONTLIST_HPP
#define C2NG_GFX_FONTLIST_HPP

#include "gfx/font.hpp"
#include "afl/base/ptr.hpp"
#include "gfx/fontrequest.hpp"

namespace gfx {

    class FontList {
     public:
        FontList();

        ~FontList();

        void addFont(FontRequest defn, afl::base::Ptr<Font> font);

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
