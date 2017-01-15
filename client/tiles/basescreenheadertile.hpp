/**
  *  \file client/tiles/basescreenheadertile.hpp
  */
#ifndef C2NG_CLIENT_TILES_BASESCREENHEADERTILE_HPP
#define C2NG_CLIENT_TILES_BASESCREENHEADERTILE_HPP

#include "client/widgets/controlscreenheader.hpp"
#include "client/objectobserverproxy.hpp"
#include "ui/root.hpp"
#include "util/requestreceiver.hpp"

namespace client { namespace tiles {

    class BaseScreenHeaderTile : public client::widgets::ControlScreenHeader {
     public:
        BaseScreenHeaderTile(ui::Root& root, client::widgets::KeymapWidget& kmw);
        void attach(ObjectObserverProxy& oop);

     private:
        util::RequestReceiver<ControlScreenHeader> m_receiver;
    };

} }

#endif
