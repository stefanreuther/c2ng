/**
  *  \file client/tiles/errortile.hpp
  */
#ifndef C2NG_CLIENT_TILES_ERRORTILE_HPP
#define C2NG_CLIENT_TILES_ERRORTILE_HPP

#include "ui/simplewidget.hpp"
#include "ui/root.hpp"

namespace client { namespace tiles {

    class ErrorTile : public ui::SimpleWidget {
     public:
        ErrorTile(String_t text, ui::Root& root);
        ~ErrorTile();

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        String_t m_text;
        ui::Root& m_root;
    };

} }

#endif
