/**
  *  \file ui/spacer.hpp
  *  \brief Class ui::Spacer
  */
#ifndef C2NG_UI_SPACER_HPP
#define C2NG_UI_SPACER_HPP

#include "ui/simplewidget.hpp"

namespace ui {

    /** Invisible spacer.
        This is a widget that has no behaviour or appearance, and just takes up space in layout.
        You can predefine its getLayoutInfo() value and therefore define its growth behaviour.
        Two constructor signatures offer common modes. */
    class Spacer : public SimpleWidget {
     public:
        /** Construct growable spacer.
            This spacer will grow to fill all available space. */
        Spacer();

        /** Construct fixed-size spacer.
            This spacer will occupy a fixed amount of space.
            \param size Size */
        Spacer(gfx::Point size);

        /** Construct custom spacer.
            This constructor defines the complete layout info object and therefore allows defining custom behaviour.
            \param info Layout info */
        Spacer(ui::layout::Info info);

        ~Spacer();

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        const ui::layout::Info m_info;
    };

}

#endif
