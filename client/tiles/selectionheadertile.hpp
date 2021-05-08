/**
  *  \file client/tiles/selectionheadertile.hpp
  */
#ifndef C2NG_CLIENT_TILES_SELECTIONHEADERTILE_HPP
#define C2NG_CLIENT_TILES_SELECTIONHEADERTILE_HPP

#include "game/proxy/objectobserver.hpp"
#include "ui/root.hpp"
#include "ui/widget.hpp"
#include "ui/widgets/button.hpp"
#include "util/requestreceiver.hpp"

namespace client { namespace tiles {

    class SelectionHeaderTile : public ui::Widget {
     public:
        SelectionHeaderTile(ui::Root& root, ui::Widget& keys);
        ~SelectionHeaderTile();

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void requestChildRedraw(Widget& child, const gfx::Rectangle& area);
        virtual void handleChildAdded(Widget& child);
        virtual void handleChildRemove(Widget& child);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual void handleChildPositionChange(Widget& child, gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        void setStatus(String_t name, bool marked);
        void attach(game::proxy::ObjectObserver& oop);

     private:
        ui::Root& m_root;
        String_t m_name;
        bool m_marked;
        util::RequestReceiver<SelectionHeaderTile> m_receiver;
        ui::widgets::Button m_prev;
        ui::widgets::Button m_next;
    };


} }

#endif
