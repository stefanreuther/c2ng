/**
  *  \file ui/icons/stylableicon.hpp
  */
#ifndef C2NG_UI_ICONS_STYLABLEICON_HPP
#define C2NG_UI_ICONS_STYLABLEICON_HPP

#include "ui/icons/icon.hpp"
#include "ui/colorscheme.hpp"
#include "ui/draw.hpp"
#include "gfx/context.hpp"
#include "gfx/types.hpp"

namespace ui { namespace icons {

    class StylableIcon : public Icon {
     public:
        StylableIcon(Icon& content, ColorScheme& colors);
        ~StylableIcon();

        bool setPaddingBefore(gfx::Point p);
        bool setPaddingAfter(gfx::Point p);
        bool setMarginBefore(gfx::Point p);
        bool setMarginAfter(gfx::Point p);
        bool setBackgroundColor(gfx::Color_t color);
        bool setFrameWidth(int width);
        bool setFrameType(FrameType type);

        virtual gfx::Point getSize() const;
        virtual void draw(gfx::Context<util::SkinColor::Color>& ctx, gfx::Rectangle area, ButtonFlags_t flags) const;

     private:
        Icon& m_content;
        ColorScheme& m_colors;
        gfx::Point m_paddingBefore;
        gfx::Point m_paddingAfter;
        gfx::Point m_marginBefore;
        gfx::Point m_marginAfter;
        bool m_hasBackgroundColor;
        gfx::Color_t m_backgroundColor;
        FrameType m_frameType;
        int m_frameWidth;
    };

} }

#endif
