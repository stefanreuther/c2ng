/**
  *  \file ui/group.hpp
  */
#ifndef C2NG_UI_GROUP_HPP
#define C2NG_UI_GROUP_HPP

#include "ui/layoutablegroup.hpp"

namespace ui {

    class Group : public LayoutableGroup {
     public:
        Group(ui::layout::Manager& mgr);

        // LayoutableGroup:
        virtual gfx::Rectangle transformSize(gfx::Rectangle size, Transformation kind) const;

        // Widget:
        virtual void draw(gfx::Canvas& can);

        // EventConsumer:
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);
    };

}

#endif
