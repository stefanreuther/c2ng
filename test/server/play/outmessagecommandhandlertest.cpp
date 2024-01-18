/**
  *  \file test/server/play/outmessagecommandhandlertest.cpp
  *  \brief Test for server::play::OutMessageCommandHandler
  */

#include "server/play/outmessagecommandhandler.hpp"

#include "afl/base/ref.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/msg/outbox.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"
#include "server/play/packerlist.hpp"
#include <stdexcept>

/** Test success cases. */
AFL_TEST("server.play.OutMessageCommandHandler:basics", a)
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
    a.checkEqual("01. getNumMessages", outbox.getNumMessages(), 2U);

    // Delete one message
    {
        server::play::OutMessageCommandHandler t1(session, i);
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        server::play::PackerList objs;
        t1.processCommand("delete", args, objs);

        a.checkEqual("11. getNumMessages", outbox.getNumMessages(), 1U);
        a.checkEqual("12. getMessageId", outbox.getMessageId(0), j);
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

        a.checkEqual("21. getNumMessages", outbox.getNumMessages(), 1U);
        a.checkEqual("22. getMessageId", outbox.getMessageId(0), j);
        a.checkEqual("23. getMessageRawText", outbox.getMessageRawText(0), "qq");
        a.checkEqual("24. getMessageReceivers", outbox.getMessageReceivers(0), game::PlayerSet_t(9));
    }
}

/** Test error cases. */
AFL_TEST("server.play.OutMessageCommandHandler:errors", a)
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
        AFL_CHECK_THROWS(a("01. bad command"), t.processCommand("frobnicate", args, objs), std::exception);
    }

    // Valid command to invalid address (will throw "404")
    {
        server::play::OutMessageCommandHandler t(session, i+1);

        afl::data::Segment seg;
        seg.pushBackString("qq");
        interpreter::Arguments args(seg, 0, 1);
        server::play::PackerList objs;
        AFL_CHECK_THROWS(a("11. bad target"), t.processCommand("settext", args, objs), std::exception);
    }

    // Type error (will throw interpreter::Error)
    {
        server::play::OutMessageCommandHandler t(session, i);

        afl::data::Segment seg;
        seg.pushBackString("qq");
        interpreter::Arguments args(seg, 0, 1);
        server::play::PackerList objs;
        AFL_CHECK_THROWS(a("21. type error"), t.processCommand("setreceivers", args, objs), std::exception);
    }
}
