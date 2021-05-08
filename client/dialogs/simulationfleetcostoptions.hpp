/**
  *  \file client/dialogs/simulationfleetcostoptions.hpp
  *  \brief Simulation Fleet Cost Options
  */
#ifndef C2NG_CLIENT_DIALOGS_SIMULATIONFLEETCOSTOPTIONS_HPP
#define C2NG_CLIENT_DIALOGS_SIMULATIONFLEETCOSTOPTIONS_HPP

#include "afl/string/translator.hpp"
#include "game/session.hpp"
#include "game/sim/fleetcost.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Edit Simulation Fleet Cost Options.
        \param [in]     root        Root
        \param [in]     gameSender  Game sender
        \param [in,out] options     Options to edit
        \param [in,out] byTeam      If non-null, the "by team" option; null to prevent it from being edited
        \param [in]     tx          Translator
        \retval true Dialog confirmed
        \retval false Dialog canceled (options, byTeam may have been modified anyway) */
    bool editSimulationFleetCostOptions(ui::Root& root,
                                        util::RequestSender<game::Session> gameSender,
                                        game::sim::FleetCostOptions& options,
                                        bool* byTeam,
                                        afl::string::Translator& tx);

} }

#endif
