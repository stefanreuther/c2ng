/**
  *  \file ui/widgets/alignedcontainer.hpp
  *  \brief Class ui::widgets::AlignedContainer
  */
#ifndef C2NG_UI_WIDGETS_ALIGNEDCONTAINER_HPP
#define C2NG_UI_WIDGETS_ALIGNEDCONTAINER_HPP

#include "gfx/types.hpp"
#include "ui/widget.hpp"

namespace ui { namespace widgets {

    /** Aligned container for a widget.
        Contains a single widget and aligns it (left/center/right, top/middle/bottom) within available space.
        By default, also provides a 10px padding around it.

        The effect of AlignedContainer can also be achieved by clever use of spacers; AlignedContainer makes it simpler. */
    class AlignedContainer : public Widget {
     public:
        /** Constructor.
            \param content Contained widget
            \param alignX  Horizontal alignment
            \param alignY  Vertical alignment */
        AlignedContainer(Widget& content, gfx::HorizontalAlignment alignX, gfx::VerticalAlignment alignY);

        /** Set padding.
            \param padX  Horizontal padding in pixels; added at both sides
            \param padY  Vertical padding in pixels; added at both sides */
        void setPadding(int padX, int padY);

        // Widget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void requestChildRedraw(Widget& child, const gfx::Rectangle& area);
        virtual void handleChildAdded(Widget& child);
        virtual void handleChildRemove(Widget& child);
        virtual void handlePositionChange();
        virtual void handleChildPositionChange(Widget& child, const gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        gfx::HorizontalAlignment m_alignX;
        gfx::VerticalAlignment m_alignY;
        int m_padX;
        int m_padY;
    };

} }

#endif
