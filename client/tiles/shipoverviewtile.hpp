/**
  *  \file client/tiles/shipoverviewtile.hpp
  */
#ifndef C2NG_CLIENT_TILES_SHIPOVERVIEWTILE_HPP
#define C2NG_CLIENT_TILES_SHIPOVERVIEWTILE_HPP

#include "ui/simplewidget.hpp"
#include "ui/root.hpp"
#include "game/types.hpp"
#include "client/objectobserverproxy.hpp"
#include "util/requestreceiver.hpp"

namespace client { namespace tiles {

    class ShipOverviewTile : public ui::SimpleWidget {
     public:
        ShipOverviewTile(ui::Root& root);
        ~ShipOverviewTile();

        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        void setStatus(afl::base::Memory<String_t> strings, afl::base::Memory<game::IntegerProperty_t> ints);
        void attach(ObjectObserverProxy& oop);

     private:
        ui::Root& m_root;
        util::RequestReceiver<ShipOverviewTile> m_receiver;

        String_t m_strings[9];
        game::IntegerProperty_t m_ints[4];
    };

} }

#endif
