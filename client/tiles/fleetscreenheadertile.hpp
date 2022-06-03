/**
  *  \file client/tiles/fleetscreenheadertile.hpp
  *  \brief Class client::tiles::FleetScreenHeaderTile
  */
#ifndef C2NG_CLIENT_TILES_FLEETSCREENHEADERTILE_HPP
#define C2NG_CLIENT_TILES_FLEETSCREENHEADERTILE_HPP

#include "client/widgets/controlscreenheader.hpp"
#include "game/proxy/objectobserver.hpp"
#include "util/requestreceiver.hpp"

namespace client { namespace tiles {

    /** Fleet screen header tile.
        When attached to an ObjectObserver showing a ship,
        displays information about this ship in its fleet. */
    class FleetScreenHeaderTile : public client::widgets::ControlScreenHeader {
     public:
        /** Constructor.
            @param root   Root
            @param kmw    Keymap widget to receive keyboard input */
        FleetScreenHeaderTile(ui::Root& root, client::widgets::KeymapWidget& kmw);

        /** Attach to ObjectObserver.
            @param oop ObjectObserver */
        void attach(game::proxy::ObjectObserver& oop);

     private:
        util::RequestReceiver<ControlScreenHeader> m_receiver;
    };

} }

#endif
