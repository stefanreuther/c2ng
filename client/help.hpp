/**
  *  \file client/help.hpp
  */
#ifndef C2NG_CLIENT_HELP_HPP
#define C2NG_CLIENT_HELP_HPP

#include "util/helpindex.hpp"
#include "game/session.hpp"
#include "afl/io/xml/node.hpp"

namespace client {

    util::HelpIndex& getHelpIndex(game::Session& session);

    void loadHelpPage(game::Session& session,
                      afl::io::xml::Nodes_t& result,
                      String_t pageName);

}

#endif
