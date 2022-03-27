/**
  *  \file client/tiles/planettasktile.hpp
  *  \brief Class client::tiles::PlanetTaskTile
  */
#ifndef C2NG_CLIENT_TILES_PLANETTASKTILE_HPP
#define C2NG_CLIENT_TILES_PLANETTASKTILE_HPP

#include "afl/string/translator.hpp"
#include "client/tiles/taskmessagetile.hpp"
#include "gfx/keyeventconsumer.hpp"
#include "ui/root.hpp"

namespace client { namespace tiles {

    /** Planet task tile.
        Displays commands to manipulate a planet task and notification message.
        (For now, no prediction.)

        To use,
        - create
        - connect TaskEditorProxy::sig_messageChange to setMessageStatus */
    class PlanetTaskTile : public TaskMessageTile {
     public:
        /** Constructor.
            @param root        UI root
            @param keyHandler  Key handler to process button events
            @param tx          Translator */
        PlanetTaskTile(ui::Root& root, gfx::KeyEventConsumer& keyHandler, afl::string::Translator& tx);
        ~PlanetTaskTile();
    };

} }

#endif
