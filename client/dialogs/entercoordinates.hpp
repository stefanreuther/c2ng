/**
  *  \file client/dialogs/entercoordinates.hpp
  *  \brief Coordinate input dialog
  */
#ifndef C2NG_CLIENT_DIALOGS_ENTERCOORDINATES_HPP
#define C2NG_CLIENT_DIALOGS_ENTERCOORDINATES_HPP

#include "afl/string/translator.hpp"
#include "game/map/configuration.hpp"
#include "game/map/point.hpp"
#include "game/session.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Show "Go To X/Y" dialog.
        This dialog asks the user for a coordinate.
        \param [out] out        Returns the coordinate if the user entered one
        \param [in]  config     Map configuration
        \param [in]  root       UI root
        \param [in]  gameSender Game sender (for help)
        \param [in]  tx         Translator
        \retval true  user entered a coordinate
        \retval false user cancelled the dialog */
    bool doEnterCoordinatesDialog(game::map::Point& result,
                                  const game::map::Configuration& config,
                                  ui::Root& root,
                                  util::RequestSender<game::Session> gameSender,
                                  afl::string::Translator& tx);

} }

#endif
