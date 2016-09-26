/**
  *  \file ui/layout/hbox.hpp
  */
#ifndef C2NG_UI_LAYOUT_HBOX_HPP
#define C2NG_UI_LAYOUT_HBOX_HPP

#include "ui/layout/manager.hpp"

namespace ui { namespace layout {

    // horizontal box
    class HBox : public Manager {
     public:
        HBox(int space = 0, int outer = 0);

        virtual void doLayout(Widget& container, gfx::Rectangle area);
        virtual Info getLayoutInfo(const Widget& container);

        static HBox instance0, instance5;

     private:
        int space, outer;
    };

} }

#endif
