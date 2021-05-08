/**
  *  \file client/tiles/planetscreenheadertile.hpp
  */
#ifndef C2NG_CLIENT_TILES_PLANETSCREENHEADERTILE_HPP
#define C2NG_CLIENT_TILES_PLANETSCREENHEADERTILE_HPP

#include "client/widgets/controlscreenheader.hpp"
#include "game/proxy/objectobserver.hpp"
#include "ui/root.hpp"
#include "util/requestreceiver.hpp"

namespace client { namespace tiles {

    class PlanetScreenHeaderTile : public client::widgets::ControlScreenHeader {
     public:
        PlanetScreenHeaderTile(ui::Root& root, client::widgets::KeymapWidget& kmw, bool forTask);
        void attach(game::proxy::ObjectObserver& oop);

     private:
        util::RequestReceiver<ControlScreenHeader> m_receiver;
        bool m_forTask;
    };

} }

#endif
