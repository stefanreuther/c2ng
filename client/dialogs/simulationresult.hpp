/**
  *  \file client/dialogs/simulationresult.hpp
  *  \brief Simulation Result Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_SIMULATIONRESULT_HPP
#define C2NG_CLIENT_DIALOGS_SIMULATIONRESULT_HPP

#include "afl/string/translator.hpp"
#include "game/proxy/simulationrunproxy.hpp"
#include "game/proxy/simulationsetupproxy.hpp"
#include "game/reference.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    struct SimulationResultStatus {
        enum Status {
            Nothing,
            ScrollToSlot,
            GoToReference
        };
        Status status;
        size_t slot;
        game::Reference reference;

        SimulationResultStatus()
            : status(Nothing),
              slot(),
              reference()
            { }
    };

    SimulationResultStatus doBattleSimulationResults(game::proxy::SimulationSetupProxy& setupProxy,
                                                     game::proxy::SimulationRunProxy& runProxy,
                                                     ui::Root& root,
                                                     afl::string::Translator& tx,
                                                     util::RequestSender<game::Session> gameSender);

} }

#endif
