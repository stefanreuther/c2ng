/**
  *  \file ui/layoutablegroup.hpp
  */
#ifndef C2NG_UI_LAYOUTABLEGROUP_HPP
#define C2NG_UI_LAYOUTABLEGROUP_HPP

#include "ui/widget.hpp"
#include "ui/layout/manager.hpp"

namespace ui {

    class LayoutableGroup : public Widget {
     public:
        LayoutableGroup(ui::layout::Manager& mgr);
        // virtual void draw(gfx::Canvas& can);
        // virtual void handleStateChange(State st, bool enable);
        virtual void requestChildRedraw(Widget& child, const gfx::Rectangle& area);
        virtual void handleChildAdded(Widget& child);
        virtual void handleChildRemove(Widget& child);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual void handleChildPositionChange(Widget& child, gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;

        enum Transformation {
            OuterToInner,
            InnerToOuter
        };
        virtual gfx::Rectangle transformSize(gfx::Rectangle size, Transformation kind) const = 0;

        void add(Widget& child);
        void pack();
        void doLayout();

     private:
        ui::layout::Manager& m_manager;

        gfx::Point transformPoint(gfx::Point pt, Transformation kind) const;
    };

}

#endif
