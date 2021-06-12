/**
  *  \file ui/icons/vbox.hpp
  *  \brief Class ui::icons::VBox
  */
#ifndef C2NG_UI_ICONS_VBOX_HPP
#define C2NG_UI_ICONS_VBOX_HPP

#include <vector>
#include "ui/icons/icon.hpp"

namespace ui { namespace icons {

    /** Container for multiple icons, vertical arrangement.
        Displays multiple icons atop each other. */
    class VBox : public Icon {
     public:
        /** Constructor. */
        VBox();
        ~VBox();

        /** Add an icon.
            Call before using the VBox.
            \param icon Icon. Lifetime must exceed that of the VBox. */
        void add(Icon& icon);

        /** Set alignment.
            Call before using the VBox.

            Alignment determines how children are aligned atop each other if their widths differ.
            - LeftAlign: align left borders
            - CenterAlign: align centers
            - RightAlign: align right borders

            \param align Horizontal alignment */
        void setAlign(gfx::HorizontalAlignment align);

        /** Set padding.
            Padding is inserted between icons.
            Call before using the VBox.
            \param pad Padding in pixels (default: 0) */
        void setPad(int pad);

        // Icon:
        virtual gfx::Point getSize() const;
        virtual void draw(gfx::Context<SkinColor::Color>& ctx, gfx::Rectangle area, ButtonFlags_t flags) const;

     private:
        std::vector<Icon*> m_children;
        gfx::HorizontalAlignment m_align;
        int m_pad;
    };

} }


#endif
