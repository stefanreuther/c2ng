/**
  *  \file client/dialogs/simulationbasetorpedoes.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_SIMULATIONBASETORPEDOES_HPP
#define C2NG_CLIENT_DIALOGS_SIMULATIONBASETORPEDOES_HPP

#include "ui/root.hpp"
#include "util/requestsender.hpp"
#include "game/session.hpp"
#include "game/proxy/simulationsetupproxy.hpp"
#include "afl/string/translator.hpp"

namespace client { namespace dialogs {

    bool editSimulationBaseTorpedoes(ui::Root& root,
                                     util::RequestSender<game::Session> gameSender,
                                     size_t initialFocus,
                                     game::proxy::SimulationSetupProxy::Elements_t& list,
                                     afl::string::Translator& tx);

} }

#endif
