/**
  *  \file server/play/maincommandhandler.hpp
  */
#ifndef C2NG_SERVER_PLAY_MAINCOMMANDHANDLER_HPP
#define C2NG_SERVER_PLAY_MAINCOMMANDHANDLER_HPP

#include "server/play/commandhandler.hpp"
#include "game/session.hpp"

namespace server { namespace play {

    class MainCommandHandler : public CommandHandler {
     public:
        MainCommandHandler(game::Session& session);
        virtual void processCommand(const String_t& cmd, interpreter::Arguments& args, PackerList& objs);

     private:
        game::Session& m_session;
    };

} }

#endif
