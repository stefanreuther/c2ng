/**
  *  \file client/dialogs/historyship.hpp
  *  \brief History Ship Selection
  */
#ifndef C2NG_CLIENT_DIALOGS_HISTORYSHIP_HPP
#define C2NG_CLIENT_DIALOGS_HISTORYSHIP_HPP

#include "afl/string/translator.hpp"
#include "game/ref/historyshipselection.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Choose a history ship from a listbox.

        \param sel        Initial selection. Sort order will be taken from user preferences.
        \param modes      Available filter modes
        \param root       UI root
        \param tx         Translator
        \param gameSender Game sender */
    int chooseHistoryShip(game::ref::HistoryShipSelection sel,
                          game::ref::HistoryShipSelection::Modes_t modes,
                          ui::Root& root,
                          afl::string::Translator& tx,
                          util::RequestSender<game::Session> gameSender);

} }

#endif
