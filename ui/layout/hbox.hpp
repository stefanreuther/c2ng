/**
  *  \file ui/layout/hbox.hpp
  *  \brief Class ui::layout::HBox
  */
#ifndef C2NG_UI_LAYOUT_HBOX_HPP
#define C2NG_UI_LAYOUT_HBOX_HPP

#include "ui/layout/manager.hpp"

namespace ui { namespace layout {

    /** Horizontal Box Layout

        Widgets will be arranged horizontally, all the same height, from left to right.
        This layout will completely cover the container with widgets (subject to space/outer settings). */
    class HBox : public Manager {
     public:
        /** Create a Horizontal Box layout.
            @param space space to leave between widgets, in pixels.
            @param outer space to leave at left/right side. */
        HBox(int space = 0, int outer = 0);

        // Manager:
        virtual void doLayout(Widget& container, gfx::Rectangle area) const;
        virtual Info getLayoutInfo(const Widget& container) const;

        /** Predefined instance with space=0, outer=0. */
        static const HBox instance0;

        /** Predefined instance with space=5, outer=0.
            Use for most horizontal widget arrangements (e.g. buttons). */
        static const HBox instance5;

     private:
        int space, outer;
    };

} }

#endif
