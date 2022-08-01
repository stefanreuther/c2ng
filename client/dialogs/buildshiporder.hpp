/**
  *  \file client/dialogs/buildshiporder.hpp
  *  \brief Starbase Ship Build Order Editor
  */
#ifndef C2NG_CLIENT_DIALOGS_BUILDSHIPORDER_HPP
#define C2NG_CLIENT_DIALOGS_BUILDSHIPORDER_HPP

#include "afl/string/translator.hpp"
#include "game/proxy/starbaseadaptor.hpp"
#include "game/session.hpp"
#include "game/shipbuildorder.hpp"
#include "game/types.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Starbase ship build order editor.
        Provides a plain editor for a ShipBuildOrder object, with no "commit" logic.

        @param [in]       root           UI root
        @param [in,out]   order          ShipBuildOrder
        @param [in]       adaptorSender  StarbaseAdaptor sender to access underlying starbase
        @param [in]       gameSender     Game sender (help, ConfigurationProxy, etc.)
        @param [in]       planetId       Planet Id to use for BuildShipMain (if nonzero, dialog offers part building)
        @param [in]       tx             Translator */
    bool doEditShipBuildOrder(ui::Root& root,
                              game::ShipBuildOrder& order,
                              util::RequestSender<game::proxy::StarbaseAdaptor> adaptorSender,
                              util::RequestSender<game::Session> gameSender,
                              game::Id_t planetId,
                              afl::string::Translator& tx);

} }

#endif
