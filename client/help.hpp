/**
  *  \file client/help.hpp
  */
#ifndef C2NG_CLIENT_HELP_HPP
#define C2NG_CLIENT_HELP_HPP

#include "util/helpindex.hpp"
#include "game/session.hpp"

namespace client {

    util::HelpIndex& getHelpIndex(game::Session& session);

}

#endif
