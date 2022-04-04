/**
  *  \file client/dialogs/shipcostcalculator.hpp
  *  \brief Starship cost calculator
  */
#ifndef C2NG_CLIENT_DIALOGS_SHIPCOSTCALCULATOR_HPP
#define C2NG_CLIENT_DIALOGS_SHIPCOSTCALCULATOR_HPP

#include "afl/string/translator.hpp"
#include "game/proxy/starbaseadaptor.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Starship cost calculator.

        @param [in] root           UI root
        @param [in] adaptorSender  StarbaseAdaptor sender to access underlying starbase
        @param [in] gameSender     Game sender (help, ConfigurationProxy, etc.)
        @param [in] useStorage     true to allow "use parts from storage" option
        @param [in] tx             Translator */
    void doShipCostCalculator(ui::Root& root,
                              util::RequestSender<game::proxy::StarbaseAdaptor> adaptorSender,
                              util::RequestSender<game::Session> gameSender,
                              bool useStorage,
                              afl::string::Translator& tx);

} }

#endif
