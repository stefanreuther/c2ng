/**
  *  \file client/help.cpp
  */

#include "client/help.hpp"
#include "game/extra.hpp"

namespace {
    class HelpExtra : public game::Extra {
     public:
        util::HelpIndex index;
    };

    game::ExtraIdentifier<game::Session, HelpExtra> HELP_ID;
}


util::HelpIndex&
client::getHelpIndex(game::Session& session)
{
    return session.extra().create(HELP_ID).index;
}

/*
 *  FIXME: functions to load help pages would also be here
 */
