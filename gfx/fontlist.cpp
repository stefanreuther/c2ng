/**
  *  \file gfx/fontlist.cpp
  *  \brief Class gfx::FontList
  */

#include "gfx/fontlist.hpp"

// Default constructor.
gfx::FontList::FontList()
    : m_fonts()
{ }

// Destructor.
gfx::FontList::~FontList()
{ }

// Add font.
void
gfx::FontList::addFont(FontRequest defn, afl::base::Ptr<Font> font)
{
    m_fonts.push_back(Element(defn, font));
}

// Find a font.
afl::base::Ptr<gfx::Font>
gfx::FontList::findFont(FontRequest req) const
{
    afl::base::Ptr<Font> result = find(req);

    if (result.get() == 0) {
        // Not found. Try again with different slant (replace italic by regular).
        req.setSlant(afl::base::Nothing);
        result = find(req);
    }

    if (result.get() == 0) {
        // Not found. Try again with different weight (replace bold by thin).
        req.setWeight(afl::base::Nothing);
        result = find(req);
    }

    if (result.get() == 0) {
        // Not found. Try again with different size (replace big by normal).
        req.setSize(afl::base::Nothing);
        result = find(req);
    }

    if (result.get() == 0) {
        // Not found. Try again with different style (replace roman by sans, mono, etc.).
        // At this point, the request is all-wildcard.
        // If find still fails, this means we don't have any font at all.
        req.setStyle(afl::base::Nothing);
        result = find(req);
    }
    return result;
}

afl::base::Ptr<gfx::Font>
gfx::FontList::find(FontRequest req) const
{
    for (size_t i = 0, n = m_fonts.size(); i < n; ++i) {
        if (req.match(m_fonts[i].definition)) {
            return m_fonts[i].font;
        }
    }
    return afl::base::Ptr<Font>();
}
