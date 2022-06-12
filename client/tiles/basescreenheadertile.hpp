/**
  *  \file client/tiles/basescreenheadertile.hpp
  */
#ifndef C2NG_CLIENT_TILES_BASESCREENHEADERTILE_HPP
#define C2NG_CLIENT_TILES_BASESCREENHEADERTILE_HPP

#include "client/widgets/controlscreenheader.hpp"
#include "game/proxy/objectobserver.hpp"
#include "ui/root.hpp"
#include "util/requestreceiver.hpp"

namespace client { namespace tiles {

    class BaseScreenHeaderTile : public client::widgets::ControlScreenHeader {
     public:
        BaseScreenHeaderTile(ui::Root& root, gfx::KeyEventConsumer& kmw, bool forTask);
        void attach(game::proxy::ObjectObserver& oop);

     private:
        util::RequestReceiver<ControlScreenHeader> m_receiver;
        bool m_forTask;
    };

} }

#endif
