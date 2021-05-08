/**
  *  \file client/dialogs/ionstormforecast.hpp
  *  \brief Ion Storm Forecast Dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_IONSTORMFORECAST_HPP
#define C2NG_CLIENT_DIALOGS_IONSTORMFORECAST_HPP

#include "game/proxy/ionstormproxy.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Show ion storm forecast.
        \param root        UI root
        \param gameSender  Game sender (for map rendering, help)
        \param tx          Translator
        \param info        Information to display */
    void doIonStormForecastDialog(ui::Root& root, util::RequestSender<game::Session> gameSender, afl::string::Translator& tx, const game::proxy::IonStormProxy::IonStormInfo& info);

} }

#endif
