/**
  *  \file server/play/outmessagecommandhandler.cpp
  *  \brief Class server::play::OutMessageCommandHandler
  */

#include "server/play/outmessagecommandhandler.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/interface/globalcommands.hpp"        // checkPlayerSetArg
#include "game/msg/outbox.hpp"
#include "game/turn.hpp"
#include "server/errors.hpp"
#include "server/play/mainpacker.hpp"
#include "server/play/outmessageindexpacker.hpp"
#include "server/play/outmessagepacker.hpp"
#include "server/play/packerlist.hpp"

server::play::OutMessageCommandHandler::OutMessageCommandHandler(game::Session& session, game::Id_t id)
    : CommandHandler(), m_session(session), m_id(id)
{ }

void
server::play::OutMessageCommandHandler::processCommand(const String_t& cmd, interpreter::Arguments& args, PackerList& objs)
{
    game::msg::Outbox& outbox = game::actions::mustHaveGame(m_session).currentTurn().outbox();
    size_t index = -1U;

    if (cmd == "settext") {
        args.checkArgumentCount(1);
        if (!outbox.findMessageById(m_id).get(index)) {
            throw std::runtime_error(ITEM_NOT_FOUND);
        }

        String_t text;
        if (interpreter::checkStringArg(text, args.getNext())) {
            outbox.setMessageText(index, text);
            objs.addNew(new OutMessagePacker(m_session, m_id));
        }
    } else if (cmd == "setreceivers") {
        args.checkArgumentCount(1);
        if (!outbox.findMessageById(m_id).get(index)) {
            throw std::runtime_error(ITEM_NOT_FOUND);
        }

        game::PlayerSet_t receivers;
        if (game::interface::checkPlayerSetArg(receivers, args.getNext())) {
            outbox.setMessageReceivers(index, receivers);
            objs.addNew(new OutMessagePacker(m_session, m_id));
        }
    } else if (cmd == "delete") {
        args.checkArgumentCount(0);
        if (outbox.findMessageById(m_id).get(index)) {
            outbox.deleteMessage(index);
            objs.addNew(new OutMessageIndexPacker(m_session));
            objs.addNew(new MainPacker(m_session));
        }
    } else {
        throw std::runtime_error(UNKNOWN_COMMAND);
    }
}
