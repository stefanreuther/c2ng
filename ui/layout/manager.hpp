/**
  *  \file ui/layout/manager.hpp
  */
#ifndef C2NG_UI_LAYOUT_MANAGER_HPP
#define C2NG_UI_LAYOUT_MANAGER_HPP

#include "ui/layout/info.hpp"
#include "afl/base/deletable.hpp"
#include "ui/widget.hpp"

namespace ui { namespace layout {

    /** Basic Layout Manager Interface.
        Encapsulates the functionality of arranging a widget container's children. */
    class Manager : public afl::base::Deletable {
     public:
        /** Arrange a container's children.
            This does not resize the container itself. */
        virtual void doLayout(Widget& container, gfx::Rectangle area) = 0;

        /** Get layout information.
            Compute preferred and minimum sizes, just like Widget::getLayoutInfo(). */
        virtual Info getLayoutInfo(const Widget& container) = 0;
    };

} }

#endif
