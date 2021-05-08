/**
  *  \file ui/icons/colortile.hpp
  *  \brief Class ui::icons::ColorTile
  */
#ifndef C2NG_UI_ICONS_COLORTILE_HPP
#define C2NG_UI_ICONS_COLORTILE_HPP

#include "ui/icons/icon.hpp"
#include "ui/root.hpp"

namespace ui { namespace icons {

    /** Colored tile.
        Appears as a single-color area with an optional "up" frame.
        It will fill the area passed to draw() even if that differes from its preferred size. */
    class ColorTile : public Icon {
     public:
        /** Constructor.
            \param root Root (for palette)
            \param size Size
            \param color Color (palette index) */
        ColorTile(Root& root, gfx::Point size, uint8_t color);
        ~ColorTile();

        /** Set frame width.
            \param frameWidth width, in pixels. 0 for no frame. Default is 1.
            \return true if tile needs to be redrawn */
        bool setFrameWidth(int frameWidth);

        /** Set color.
            \param color Color (palette index)
            \return true if tile needs to be redrawn */
        bool setColor(uint8_t color);

        /** Set frame type.
            \param ft Frame type
            \return true if tile needs to be redrawn */
        bool setFrameType(FrameType ft);

        virtual gfx::Point getSize() const;
        virtual void draw(gfx::Context<SkinColor::Color>& ctx, gfx::Rectangle area, ButtonFlags_t flags) const;

     private:
        Root& m_root;
        gfx::Point m_size;
        uint8_t m_color;
        int m_frameWidth;
        FrameType m_frameType;
    };

} }

#endif
