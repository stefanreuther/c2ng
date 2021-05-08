/**
  *  \file client/dialogs/simulationabilities.hpp
  *  \brief Simulation Unit Abilities Editor
  */
#ifndef C2NG_CLIENT_DIALOGS_SIMULATIONABILITIES_HPP
#define C2NG_CLIENT_DIALOGS_SIMULATIONABILITIES_HPP

#include "afl/string/translator.hpp"
#include "game/proxy/simulationsetupproxy.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Simulation Unit Abilities Editor.
        Displays a dialog to modify unit abilities (game::sim::Ability).
        \param [in]     root        Root
        \param [in]     gameSender  Game sender (required for help)
        \param [in,out] choices     Choices. Defines available abilities and their current setting; will be updated according to user actions.
        \param [in]     tx          Translator
        \retval true User confirmed dialog
        \retval false User cancelled dialog (choices may still be modified) */
    bool editSimulationAbilities(ui::Root& root,
                                 util::RequestSender<game::Session> gameSender,
                                 game::proxy::SimulationSetupProxy::AbilityChoices& choices,
                                 afl::string::Translator& tx);

} }

#endif
