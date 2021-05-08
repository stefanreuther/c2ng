/**
  *  \file client/tiles/shipscreenheadertile.hpp
  */
#ifndef C2NG_CLIENT_TILES_SHIPSCREENHEADERTILE_HPP
#define C2NG_CLIENT_TILES_SHIPSCREENHEADERTILE_HPP

#include "client/widgets/controlscreenheader.hpp"
#include "game/proxy/objectobserver.hpp"
#include "ui/root.hpp"
#include "util/requestreceiver.hpp"

namespace client { namespace tiles {

    class ShipScreenHeaderTile : public client::widgets::ControlScreenHeader {
     public:
        enum Kind {
            ShipScreen,
            HistoryScreen,
            ShipTaskScreen
        };

        ShipScreenHeaderTile(ui::Root& root, client::widgets::KeymapWidget& kmw, Kind k);
        void attach(game::proxy::ObjectObserver& oop);

     private:
        util::RequestReceiver<ControlScreenHeader> m_receiver;
        Kind m_kind;
    };

} }

#endif
