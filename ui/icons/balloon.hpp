/**
  *  \file ui/icons/balloon.hpp
  *  \brief Class ui::icons::Balloon
  */
#ifndef C2NG_UI_ICONS_BALLOON_HPP
#define C2NG_UI_ICONS_BALLOON_HPP

#include "ui/icons/icon.hpp"
#include "ui/root.hpp"

namespace ui { namespace icons {

    /** Speech balloon.
        Displays a rectangular speech balloon with the tail centered on the bottom edge.
        The balloon has a single-pixel, single-color frame and is filled with 50% opaque black (100% opaque in palettized mode). */
    class Balloon : public Icon {
     public:
        /** Constructor.
            @param content Content to display within the bubble
            @param root    Root (for colors)
            @param color   Color index */
        Balloon(Icon& content, Root& root, uint8_t color);

        /** Set frame color.
            @param color New color
            @return true if this is a change and the icon needs to be redrawn */
        bool setColor(uint8_t color);

        // Icon:
        virtual gfx::Point getSize() const;
        virtual void draw(gfx::Context<util::SkinColor::Color>& ctx, gfx::Rectangle area, ButtonFlags_t flags) const;

     private:
        Icon& m_content;
        Root& m_root;
        uint8_t m_color;
    };

} }

#endif
