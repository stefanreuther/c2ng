/**
  *  \file ui/widgets/panel.hpp
  */
#ifndef C2NG_UI_WIDGETS_PANEL_HPP
#define C2NG_UI_WIDGETS_PANEL_HPP

#include "ui/layoutablegroup.hpp"

namespace ui { namespace widgets {

    class Panel : public LayoutableGroup {
     public:
        Panel(ui::layout::Manager& mgr, int padding = 0);

        void setPadding(int padding);

        // LayoutableGroup:
        virtual gfx::Rectangle transformSize(gfx::Rectangle size, Transformation kind) const;

        // Widget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);

        // EventConsumer:
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        int m_padding;
    };


} }

#endif
