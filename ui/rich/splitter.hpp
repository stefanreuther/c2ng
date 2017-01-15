/**
  *  \file ui/rich/splitter.hpp
  *  \brief Class ui::rich::Splitter
  */
#ifndef C2NG_UI_RICH_SPLITTER_HPP
#define C2NG_UI_RICH_SPLITTER_HPP

#include <vector>
#include "util/rich/visitor.hpp"
#include "gfx/fontrequest.hpp"
#include "afl/string/string.hpp"
#include "util/skincolor.hpp"

namespace ui { namespace rich {

    /** Rich text splitter.
        This class implements util::rich::Visitor and handles standard attributes:
        - util::rich::StyleAttribute (font selection, underlining, keycaps)
        - util::rich::ColorAttribute (colors)

        Each segment of text is converted into a handlePart() function call.

        Additional attributes can be handled by implementing handleOtherAttribute(). */
    class Splitter : public util::rich::Visitor {
     public:
        /** Constructor. */
        Splitter();

        /** Handle part of text.
            This function is called with the resolved effective attributes.
            \param text Text
            \param font Effective font
            \param isUnderlined Effective underline attribute (util::rich::StyleAttribute::Underline)
            \param isKey Effective key attribute (util::rich::StyleAttribute::Key)
            \param color Effective color */
        virtual void handlePart(const String_t& text, gfx::FontRequest font, bool isUnderlined, bool isKey, util::SkinColor::Color color) = 0;

        /** Handle other attribute.
            \param att Attribute
            \param start true: we are entering this attribute (e.g. start of link); false: we are exiting this attribute (e.g. end of link). */
        virtual void handleOtherAttribute(const util::rich::Attribute& att, bool start) = 0;

        // Visitor virtuals:
        virtual bool handleText(String_t text);
        virtual bool startAttribute(const util::rich::Attribute& att);
        virtual bool endAttribute(const util::rich::Attribute& att);

     private:
        void handleAttribute(const util::rich::Attribute& att, bool start);

        std::vector<util::SkinColor::Color> m_colors;
        int m_bold;
        int m_big;
        int m_fixed;
        int m_underline;
        int m_key;
    };

} }

#endif
