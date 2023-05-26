/**
  *  \file ui/layout/manager.hpp
  *  \brief Base class ui::layout::Manager
  */
#ifndef C2NG_UI_LAYOUT_MANAGER_HPP
#define C2NG_UI_LAYOUT_MANAGER_HPP

#include "afl/base/deletable.hpp"
#include "ui/layout/info.hpp"
#include "ui/widget.hpp"

namespace ui { namespace layout {

    /** Basic Layout Manager Interface.
        Encapsulates the functionality of arranging a widget container's children.
        A layout manager can compute a compound widget's preferred size,
        and arrange children according to a given size.

        @see ui::LayoutableGroup */
    class Manager : public afl::base::Deletable {
     public:
        /** Arrange a container's children.
            This does not resize the container itself.
            @param container Container
            @param area      Area available to child widgets (can differ from container size
                             in case container has extra margins like a window title) */
        virtual void doLayout(Widget& container, gfx::Rectangle area) const = 0;

        /** Get layout information.
            Compute preferred and minimum sizes, just like Widget::getLayoutInfo().

            @param container Container
            @return Area required for child widgets (container needs to add its own margin info if needed) */
        virtual Info getLayoutInfo(const Widget& container) const = 0;
    };

} }

#endif
