/**
  *  \file ui/icons/hbox.hpp
  *  \brief Class ui::icons::HBox
  */
#ifndef C2NG_UI_ICONS_HBOX_HPP
#define C2NG_UI_ICONS_HBOX_HPP

#include <vector>
#include "ui/icons/icon.hpp"

namespace ui { namespace icons {

    /** Container for multiple icons, horizontal arrangement.
        Displays multiple icons next to each other. */
    class HBox : public Icon {
     public:
        /** Constructor. */
        HBox();
        ~HBox();

        /** Add an icon.
            Call before using the HBox.
            \param icon Icon. Lifetime must exceed that of the HBox. */
        void add(Icon& icon);

        /** Set alignment.
            Call before using the HBox.

            Horizontal alignment determines how children are arranged next to each other.
            - LeftAlign: left-to-right, starting at left boundary
            - CenterAlign: not supported
            - RightAlign: right-to-left, starting at right boundary

            Vertial alignment determines how children are arranged horizontally if their heights differ.
            - TopAlign: align top borders
            - MiddleAlign: align centers
            - BottomAlign: align bottoms

            \param x Horizontal alignment
            \param y Vertical alignment */
        void setAlign(gfx::HorizontalAlignment x, gfx::VerticalAlignment y);

        /** Set padding.
            Padding is inserted between icons.
            Call before using the HBox.
            \param pad Padding in pixels (default: 0) */
        void setPad(int pad);

        // Icon:
        virtual gfx::Point getSize() const;
        virtual void draw(gfx::Context<SkinColor::Color>& ctx, gfx::Rectangle area, ButtonFlags_t flags) const;

     private:
        std::vector<Icon*> m_children;
        gfx::HorizontalAlignment m_alignX;
        gfx::VerticalAlignment m_alignY;
        int m_pad;
    };

} }

#endif
