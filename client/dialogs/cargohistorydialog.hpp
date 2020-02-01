/**
  *  \file client/dialogs/cargohistorydialog.hpp
  *  \brief Cargo History Display
  */
#ifndef C2NG_CLIENT_DIALOGS_CARGOHISTORYDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_CARGOHISTORYDIALOG_HPP

#include "afl/string/translator.hpp"
#include "game/map/shipinfo.hpp"
#include "ui/root.hpp"

namespace client { namespace dialogs {

    /** Display cargo history.
        Shows the content of the given ShipCargoInfos_t, including advice when it is empty.
        \param info   Information to display
        \param root   UI Root
        \param tx     Translator */
    void doCargoHistory(const game::map::ShipCargoInfos_t& info, ui::Root& root, afl::string::Translator& tx);

} }

#endif
