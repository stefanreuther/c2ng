/**
  *  \file server/play/planetcommandhandler.hpp
  */
#ifndef C2NG_SERVER_PLAY_PLANETCOMMANDHANDLER_HPP
#define C2NG_SERVER_PLAY_PLANETCOMMANDHANDLER_HPP

#include "server/play/commandhandler.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    class PlanetCommandHandler : public CommandHandler {
     public:
        PlanetCommandHandler(game::Session& session, game::Id_t id);
        virtual void processCommand(const String_t& cmd, interpreter::Arguments& args, PackerList& objs);

     private:
        game::Session& m_session;
        game::Id_t m_id;
    };

} }

#endif
