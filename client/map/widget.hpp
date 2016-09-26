/**
  *  \file client/map/widget.hpp
  */
#ifndef C2NG_CLIENT_MAP_WIDGET_HPP
#define C2NG_CLIENT_MAP_WIDGET_HPP

#include "ui/simplewidget.hpp"
#include "client/map/renderer.hpp"
#include "client/map/proxy.hpp"
#include "game/map/renderlist.hpp"
#include "ui/root.hpp"

namespace client { namespace map {

    class Widget : public ui::SimpleWidget {
     public:
        Widget(util::RequestSender<game::Session> gameSender, ui::Root& root, gfx::Point preferredSize);
        ~Widget();

        void setCenter(game::map::Point pt);

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        void onUpdate(afl::base::Ptr<game::map::RenderList> renderList);
        void maybeRequestNewRange();

        Renderer m_renderer;
        Proxy m_proxy;
        ui::Root& m_root;
        gfx::Point m_preferredSize;

        game::map::Point m_min;
        game::map::Point m_max;
    };

} }

#endif
