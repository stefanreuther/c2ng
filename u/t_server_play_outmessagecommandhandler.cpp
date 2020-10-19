/**
  *  \file u/t_server_play_outmessagecommandhandler.cpp
  *  \brief Test for server::play::OutMessageCommandHandler
  */

#include "server/play/outmessagecommandhandler.hpp"

#include <stdexcept>
#include "t_server_play.hpp"
#include "afl/base/ref.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/msg/outbox.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "server/play/packerlist.hpp"

/** Test success cases. */
void
TestServerPlayOutMessageCommandHandler::testIt()
{
    // Environment
    afl::base::Ref<game::Game> game(*new game::Game());
    game::msg::Outbox& outbox = game->currentTurn().outbox();

    // Create some messages
    game::Id_t i = outbox.addMessage(1, "a", game::PlayerSet_t(7));
    game::Id_t j = outbox.addMessage(3, "b", game::PlayerSet_t(2));

    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(game.asPtr());

    // Preconditions
    TS_ASSERT_EQUALS(outbox.getNumMessages(), 2U);

    // Delete one message
    {
        server::play::OutMessageCommandHandler t1(session, i);
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        server::play::PackerList objs;
        t1.processCommand("delete", args, objs);

        TS_ASSERT_EQUALS(outbox.getNumMessages(), 1U);
        TS_ASSERT_EQUALS(outbox.getMessageId(0), j);
    }

    // Modify a message
    {
        server::play::OutMessageCommandHandler t2(session, j);

        afl::data::Segment seg21;
        seg21.pushBackString("qq");
        interpreter::Arguments args21(seg21, 0, 1);
        server::play::PackerList objs;
        t2.processCommand("settext", args21, objs);

        afl::data::Segment seg22;
        seg22.pushBackInteger(9);
        interpreter::Arguments args22(seg22, 0, 1);
        t2.processCommand("setreceivers", args22, objs);

        TS_ASSERT_EQUALS(outbox.getNumMessages(), 1U);
        TS_ASSERT_EQUALS(outbox.getMessageId(0), j);
        TS_ASSERT_EQUALS(outbox.getMessageRawText(0), "qq");
        TS_ASSERT_EQUALS(outbox.getMessageReceivers(0), game::PlayerSet_t(9));
    }
}

/** Test error cases. */
void
TestServerPlayOutMessageCommandHandler::testError()
{
    // Environment
    afl::base::Ref<game::Game> game(*new game::Game());
    game::msg::Outbox& outbox = game->currentTurn().outbox();
    game::Id_t i = outbox.addMessage(1, "a", game::PlayerSet_t(7));

    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(game.asPtr());

    // Invalid command to valid address (will throw "400")
    {
        server::play::OutMessageCommandHandler t(session, i);
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        server::play::PackerList objs;
        TS_ASSERT_THROWS(t.processCommand("frobnicate", args, objs), std::exception);
    }

    // Valid command to invalid address (will throw "404")
    {
        server::play::OutMessageCommandHandler t(session, i+1);

        afl::data::Segment seg;
        seg.pushBackString("qq");
        interpreter::Arguments args(seg, 0, 1);
        server::play::PackerList objs;
        TS_ASSERT_THROWS(t.processCommand("settext", args, objs), std::exception);
    }

    // Type error (will throw interpreter::Error)
    {
        server::play::OutMessageCommandHandler t(session, i);

        afl::data::Segment seg;
        seg.pushBackString("qq");
        interpreter::Arguments args(seg, 0, 1);
        server::play::PackerList objs;
        TS_ASSERT_THROWS(t.processCommand("setreceivers", args, objs), std::exception);
    }
}

