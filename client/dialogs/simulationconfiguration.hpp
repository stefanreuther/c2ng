/**
  *  \file client/dialogs/simulationconfiguration.hpp
  *  \brief Simulation Configuration Editor
  */
#ifndef C2NG_CLIENT_DIALOGS_SIMULATIONCONFIGURATION_HPP
#define C2NG_CLIENT_DIALOGS_SIMULATIONCONFIGURATION_HPP

#include "afl/string/translator.hpp"
#include "game/session.hpp"
#include "game/sim/configuration.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Simulation Configuration Editor.
        Displays a dialog to edit simulation configuration.
        \param [in]     root        Root
        \param [in]     gameSender  Game sender
        \param [in,out] config      Configuration to edit
        \param [in]     tx          Translator
        \retval true User confirmed dialog
        \retval false User cancelled dialog (config may still be modified) */
    bool editSimulationConfiguration(ui::Root& root,
                                     util::RequestSender<game::Session> gameSender,
                                     game::sim::Configuration& config,
                                     afl::string::Translator& tx);

} }

#endif
