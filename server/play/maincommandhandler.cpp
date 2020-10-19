/**
  *  \file server/play/maincommandhandler.cpp
  */

#include "server/play/maincommandhandler.hpp"
#include "server/errors.hpp"
#include "game/interface/globalcommands.hpp"
#include "server/play/mainpacker.hpp"
#include "server/play/packerlist.hpp"
#include "server/play/outmessageindexpacker.hpp"

server::play::MainCommandHandler::MainCommandHandler(game::Session& session)
    : CommandHandler(),
      m_session(session)
{ }

void
server::play::MainCommandHandler::processCommand(const String_t& cmd, interpreter::Arguments& args, PackerList& objs)
{
    // Temporary process
    interpreter::Process process(m_session.world(), "MainCommandHandler", 0);

    // Commands
    if (cmd == "sendmessage") {
        // Sending a message invalidates main because that contains the number of messages
        game::interface::IFSendMessage(process, m_session, args);
        objs.addNew(new MainPacker(m_session));
        objs.addNew(new OutMessageIndexPacker(m_session));
    } else {
        throw std::runtime_error(UNKNOWN_COMMAND);
    }
}
