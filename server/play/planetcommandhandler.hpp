/**
  *  \file server/play/planetcommandhandler.hpp
  *  \brief Class server::play::PlanetCommandHandler
  */
#ifndef C2NG_SERVER_PLAY_PLANETCOMMANDHANDLER_HPP
#define C2NG_SERVER_PLAY_PLANETCOMMANDHANDLER_HPP

#include "server/play/commandhandler.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    /** Implementation of CommandHandler for 'obj/planetX'. */
    class PlanetCommandHandler : public CommandHandler {
     public:
        /** Constructor.
            @param session  Session (must have ShipList, Root, Game)
            @param planetNr Planet Id
            @see PlanetPacker */
        PlanetCommandHandler(game::Session& session, game::Id_t id);

        // CommandHandler:
        virtual void processCommand(const String_t& cmd, interpreter::Arguments& args, PackerList& objs);

     private:
        game::Session& m_session;
        game::Id_t m_id;
    };

} }

#endif
