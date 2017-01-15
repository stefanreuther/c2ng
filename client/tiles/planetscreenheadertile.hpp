/**
  *  \file client/tiles/planetscreenheadertile.hpp
  */
#ifndef C2NG_CLIENT_TILES_PLANETSCREENHEADERTILE_HPP
#define C2NG_CLIENT_TILES_PLANETSCREENHEADERTILE_HPP

#include "client/widgets/controlscreenheader.hpp"
#include "client/objectobserverproxy.hpp"
#include "ui/root.hpp"
#include "util/requestreceiver.hpp"

namespace client { namespace tiles {

    class PlanetScreenHeaderTile : public client::widgets::ControlScreenHeader {
     public:
        PlanetScreenHeaderTile(ui::Root& root, client::widgets::KeymapWidget& kmw);
        void attach(ObjectObserverProxy& oop);

     private:
        util::RequestReceiver<ControlScreenHeader> m_receiver;
    };

} }

#endif
