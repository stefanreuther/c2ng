/**
  *  \file client/dialogs/simulationalliances.hpp
  *  \brief Alliance Editor for Battle Simulator
  */
#ifndef C2NG_CLIENT_DIALOGS_SIMULATIONALLIANCES_HPP
#define C2NG_CLIENT_DIALOGS_SIMULATIONALLIANCES_HPP

#include "afl/string/translator.hpp"
#include "game/proxy/simulationsetupproxy.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "ui/widget.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Edit alliances for battle simulator.
        @param proxy       SimulationSetupProxy instance
        @param gameSender  Access to game
        @param root        UI root
        @param tx          Translator */
    void editAlliances(game::proxy::SimulationSetupProxy& proxy,
                       util::RequestSender<game::Session> gameSender,
                       ui::Root& root,
                       afl::string::Translator& tx);

} }

#endif
