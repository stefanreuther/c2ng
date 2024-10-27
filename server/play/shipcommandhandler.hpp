/**
  *  \file server/play/shipcommandhandler.hpp
  *  \brief Class server::play::ShipCommandHandler
  */
#ifndef C2NG_SERVER_PLAY_SHIPCOMMANDHANDLER_HPP
#define C2NG_SERVER_PLAY_SHIPCOMMANDHANDLER_HPP

#include "server/play/commandhandler.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    /** Implementation of CommandHandler for 'obj/shipX'. */
    class ShipCommandHandler : public CommandHandler {
     public:
        /** Constructor.
            @param session  Session (must have ShipList, Root, Game)
            @param id       Ship Id
            @see ShipPacker */
        ShipCommandHandler(game::Session& session, game::Id_t id);

        // CommandHandler:
        virtual void processCommand(const String_t& cmd, interpreter::Arguments& args, PackerList& objs);

     private:
        game::Session& m_session;
        const game::Id_t m_id;
    };

} }

#endif
