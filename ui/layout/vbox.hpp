/**
  *  \file ui/layout/vbox.hpp
  *  \brief Class ui::layout::VBox
  */
#ifndef C2NG_UI_LAYOUT_VBOX_HPP
#define C2NG_UI_LAYOUT_VBOX_HPP

#include "ui/layout/manager.hpp"

namespace ui { namespace layout {

    /** Vertical Box Layout

        Widgets will be arranged vertically, all the same width, below each other from top to bottom.
        This layout will completely cover the container with widgets (subject to space/outer settings). */
    class VBox : public Manager {
     public:
        /** Create a Vertical Box layout.
            @param space space to leave between widgets, in pixels.
            @param outer space to leave at top/bottom */
        VBox(int space = 0, int outer = 0);

        // Manager:
        virtual void doLayout(Widget& container, gfx::Rectangle area) const;
        virtual Info getLayoutInfo(const Widget& container) const;

        /** Predefined instance with space=0, outer=0. */
        static const VBox instance0;

        /** Predefined instance with space=5, outer=0.
            Use for most vertical widget arrangements (e.g. buttons). */
        static const VBox instance5;
     private:
        int space, outer;
    };

} }

#endif
