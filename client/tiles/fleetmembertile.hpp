/**
  *  \file client/tiles/fleetmembertile.hpp
  *  \brief Class client::tiles::FleetMemberTile
  */
#ifndef C2NG_CLIENT_TILES_FLEETMEMBERTILE_HPP
#define C2NG_CLIENT_TILES_FLEETMEMBERTILE_HPP

#include "afl/base/deleter.hpp"
#include "afl/base/signalconnection.hpp"
#include "afl/string/translator.hpp"
#include "client/widgets/fleetmemberlistbox.hpp"
#include "game/proxy/fleetproxy.hpp"
#include "gfx/keyeventconsumer.hpp"
#include "ui/group.hpp"
#include "ui/root.hpp"
#include "ui/skincolorscheme.hpp"

namespace client { namespace tiles {

    /** Fleet member tile.
        Displays a list of fleet members retrieved from a FleetProxy, and forwards selection back into it.
        To use,
        - construct
        - call attach() to connect the FleetProxy */
    class FleetMemberTile : public ui::Group {
     public:
        FleetMemberTile(ui::Root& root, gfx::KeyEventConsumer& keyHandler, afl::string::Translator& tx);

        ~FleetMemberTile();

        void attach(game::proxy::FleetProxy& proxy);

     private:
        afl::base::Deleter m_deleter;
        ui::SkinColorScheme m_internalColorScheme;
        client::widgets::FleetMemberListbox m_list;

        afl::base::SignalConnection conn_fleetChange;
        afl::base::SignalConnection conn_listScroll;
        bool m_updating;

        void onFleetChange(const game::ref::FleetMemberList& memList, game::Id_t memId);
        void onListScroll(game::proxy::FleetProxy& proxy);
    };

} }

#endif
