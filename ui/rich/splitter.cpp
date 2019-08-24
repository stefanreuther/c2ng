/**
  *  \file ui/rich/splitter.cpp
  *  \brief Class ui::rich::Splitter
  */

#include "ui/rich/splitter.hpp"
#include "util/rich/styleattribute.hpp"
#include "util/rich/colorattribute.hpp"

// Rich text splitter.
ui::rich::Splitter::Splitter()
    : m_colors(),
      m_bold(0),
      m_big(0),
      m_fixed(0),
      m_underline(0),
      m_key(0)
{ }

// Visitor virtuals:
bool
ui::rich::Splitter::handleText(String_t text)
{
    // ex RichTextSplitter::handleText
    gfx::FontRequest req;
    req.setSize(static_cast<int16_t>(m_big));
    req.setWeight(static_cast<int16_t>(m_bold));
    req.setStyle(m_fixed > 0 ? 1 : 0);

    util::SkinColor::Color color = !m_colors.empty() ? m_colors.back() : util::SkinColor::Static;

    handlePart(text, req, m_underline > 0, m_key > 0, color);
    return true;
}

bool
ui::rich::Splitter::startAttribute(const util::rich::Attribute& att)
{
    // ex RichTextSplitter::startAttribute
    handleAttribute(att, true);
    return true;
}

bool
ui::rich::Splitter::endAttribute(const util::rich::Attribute& att)
{
    // ex RichTextSplitter::endAttribute
    handleAttribute(att, false);
    return true;
}

void
ui::rich::Splitter::handleAttribute(const util::rich::Attribute& att, bool start)
{
    // ex RichTextSplitter::process
    if (const util::rich::StyleAttribute* sa = dynamic_cast<const util::rich::StyleAttribute*>(&att)) {
        int delta = start ? +1 : -1;
        switch (sa->getStyle()) {
         case util::rich::StyleAttribute::Bold:
            m_bold += delta;
            break;
         case util::rich::StyleAttribute::Big:
            m_big += delta;
            break;
         case util::rich::StyleAttribute::Small:
            m_big -= delta;
            break;
         case util::rich::StyleAttribute::Fixed:
            m_fixed += delta;
            break;
         case util::rich::StyleAttribute::Underline:
            m_underline += delta;
            break;
         case util::rich::StyleAttribute::Key:
            m_key += delta;
            break;
         default:
            break;
        }
    } else if (const util::rich::ColorAttribute* ca = dynamic_cast<const util::rich::ColorAttribute*>(&att)) {
        if (start) {
            m_colors.push_back(ca->getColor());
        } else {
            if (!m_colors.empty()) {
                m_colors.pop_back();
            }
        }
    } else {
        handleOtherAttribute(att, start);
    }
}
