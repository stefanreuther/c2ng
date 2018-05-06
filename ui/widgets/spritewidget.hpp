/**
  *  \file ui/widgets/spritewidget.hpp
  */
#ifndef C2NG_UI_WIDGETS_SPRITEWIDGET_HPP
#define C2NG_UI_WIDGETS_SPRITEWIDGET_HPP

#include "ui/simplewidget.hpp"
#include "gfx/anim/controller.hpp"

namespace ui { namespace widgets {

    class SpriteWidget : public SimpleWidget {
     public:
        SpriteWidget();
        ~SpriteWidget();

        gfx::anim::Controller& controller();
        void tick();

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        gfx::anim::Controller m_controller;
    };

} }

#endif
