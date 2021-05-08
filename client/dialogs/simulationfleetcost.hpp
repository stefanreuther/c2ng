/**
  *  \file client/dialogs/simulationfleetcost.hpp
  *  \brief Simulation Fleet Cost Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_SIMULATIONFLEETCOST_HPP
#define C2NG_CLIENT_DIALOGS_SIMULATIONFLEETCOST_HPP

#include "afl/string/translator.hpp"
#include "game/proxy/simulationsetupproxy.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Display Simulation Fleet Cost.
        This is a purely informative dialog.
        \param [in] root        Root
        \param [in] gameSender  Game sender
        \param [in] setupProxy  SimulationSetupProxy instance to observe
        \param [in] tx          Translator
        \see game::proxy::FleetCostProxy */
    void showSimulationFleetCost(ui::Root& root,
                                 util::RequestSender<game::Session> gameSender,
                                 game::proxy::SimulationSetupProxy& setupProxy,
                                 afl::string::Translator& tx);

} }

#endif
