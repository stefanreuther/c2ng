/**
  *  \file client/dialogs/missionselection.hpp
  *  \brief Mission Selection Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_MISSIONSELECTION_HPP
#define C2NG_CLIENT_DIALOGS_MISSIONSELECTION_HPP

#include "afl/base/optional.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "game/session.hpp"
#include "game/spec/missionlist.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Choose a mission.

        @param choices        Missions
        @param currentValue   Current mission (initial selection)
        @param title          Title of dialog
        @param helpId         Help page Id (can be empty)
        @param root           UI root
        @param tx             Translator
        @param gameSender     Game sender (for help)

        @return chosen mission; empty if canceled */
    afl::base::Optional<int> chooseMission(game::spec::MissionList::Grouped& choices,
                                           int currentValue,
                                           String_t title,
                                           String_t helpId,
                                           ui::Root& root,
                                           afl::string::Translator& tx,
                                           util::RequestSender<game::Session> gameSender);

} }

#endif
