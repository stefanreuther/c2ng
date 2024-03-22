/**
  *  \file client/tiles/basetasktile.hpp
  *  \brief Class client::tiles::BaseTaskTile
  */
#ifndef C2NG_CLIENT_TILES_BASETASKTILE_HPP
#define C2NG_CLIENT_TILES_BASETASKTILE_HPP

#include "afl/string/translator.hpp"
#include "client/tiles/taskmessagetile.hpp"
#include "game/proxy/taskeditorproxy.hpp"
#include "gfx/keyeventconsumer.hpp"
#include "ui/root.hpp"
#include "ui/widgets/button.hpp"

namespace client { namespace tiles {

    /** Base task tile.
        Displays commands to manipulate a starbase task,
        base's current build order, and notification message.

        To use,
        - create
        - connect TaskEditorProxy::sig_baseChange to setBaseStatus
        - connect TaskEditorProxy::sig_messageChange to setMessageStatus */
    class BaseTaskTile : public TaskMessageTile {
     public:
        enum Personality {
            PlanetPersonality,
            BasePersonality
        };

        /** Constructor.
            @param pers        Personality
            @param root        UI root
            @param keyHandler  Key handler to process button events
            @param tx          Translator */
        BaseTaskTile(Personality pers, ui::Root& root, gfx::KeyEventConsumer& keyHandler, afl::string::Translator& tx);
        ~BaseTaskTile();

        /** Set base prediction status.
            @param st Status */
        void setBaseStatus(const game::proxy::TaskEditorProxy::BaseStatus& st);

     private:
        Personality m_personality;
        ui::rich::DocumentView m_statusView;
        ui::widgets::Button m_editButton;
    };

} }


#endif
