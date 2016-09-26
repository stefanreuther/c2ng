/**
  *  \file ui/layout/vbox.hpp
  */
#ifndef C2NG_UI_LAYOUT_VBOX_HPP
#define C2NG_UI_LAYOUT_VBOX_HPP

#include "ui/layout/manager.hpp"

namespace ui { namespace layout {
    
    class VBox : public Manager {
     public:
        VBox(int space = 0, int outer = 0);

        virtual void doLayout(Widget& container, gfx::Rectangle area);
        virtual Info getLayoutInfo(const Widget& container);

        static VBox instance0, instance5;
     private:
        int space, outer;
    };

} }

#endif
