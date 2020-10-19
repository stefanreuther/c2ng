/**
  *  \file server/play/outmessagecommandhandler.hpp
  *  \brief Class server::play::OutMessageCommandHandler
  */
#ifndef C2NG_SERVER_PLAY_OUTMESSAGECOMMANDHANDLER_HPP
#define C2NG_SERVER_PLAY_OUTMESSAGECOMMANDHANDLER_HPP

#include "server/play/commandhandler.hpp"
#include "game/session.hpp"
#include "game/types.hpp"

namespace server { namespace play {

    /** Command Handler for "obj/outmsgX". */
    class OutMessageCommandHandler : public CommandHandler {
     public:
        /** Constructor.
            \param session Session
            \param id Message Id, see game::msg::Outbox::getMessageId() */
        OutMessageCommandHandler(game::Session& session, game::Id_t id);

        // CommandHandler:
        virtual void processCommand(const String_t& cmd, interpreter::Arguments& args, PackerList& objs);

     private:
        game::Session& m_session;
        game::Id_t m_id;
    };

} }

#endif
