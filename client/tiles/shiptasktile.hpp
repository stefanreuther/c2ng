/**
  *  \file client/tiles/shiptasktile.hpp
  *  \brief Class client::tiles::ShipTaskTile
  */
#ifndef C2NG_CLIENT_TILES_SHIPTASKTILE_HPP
#define C2NG_CLIENT_TILES_SHIPTASKTILE_HPP

#include "afl/string/translator.hpp"
#include "client/tiles/taskmessagetile.hpp"
#include "game/proxy/taskeditorproxy.hpp"
#include "gfx/keyeventconsumer.hpp"
#include "ui/root.hpp"

namespace client { namespace tiles {

    /** Ship task tile.
        Displays commands to manipulate a ship task,
        ship's prediction, and notification message.

        To use,
        - create
        - connect TaskEditorProxy::sig_shipChange to setShipStatus
        - connect TaskEditorProxy::sig_messageChange to setMessageStatus */
    class ShipTaskTile : public TaskMessageTile {
     public:
        /** Constructor.
            @param root        UI root
            @param keyHandler  Key handler to process button events
            @param tx          Translator */
        ShipTaskTile(ui::Root& root, gfx::KeyEventConsumer& keyHandler, afl::string::Translator& tx);
        ~ShipTaskTile();

        /** Set ship prediction status.
            @param st Status */
        void setShipStatus(const game::proxy::TaskEditorProxy::ShipStatus& st);

     private:
        ui::rich::DocumentView m_statusView;
    };

} }

#endif
