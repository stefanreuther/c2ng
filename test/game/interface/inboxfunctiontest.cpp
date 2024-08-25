/**
  *  \file test/game/interface/inboxfunctiontest.cpp
  *  \brief Test for game::interface::InboxFunction
  */

#include "game/interface/inboxfunction.hpp"

#include "afl/data/segment.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/msg/inbox.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

namespace {
    void prepare(afl::test::Assert a, game::Session& s)
    {
        // Add some messages
        game::msg::Inbox& in = s.getGame()->currentTurn().inbox();
        in.addMessage("(-a000)<<< First >>>\nThis is the first message.", 10);
        in.addMessage("(-a000)<<< Second >>>\nThis is the second message.", 10);
        in.addMessage("(-a000)<<< Third >>>\nThis is the third message.", 11);

        // Verify our assumptions
        a.checkEqual("prepare > getNumMessages",     in.getNumMessages(), 3U);
        a.checkEqual("prepare > getMessageText",     in.getMessageText(0, s.translator(), s.getRoot()->playerList()), "(-a000)<<< First >>>\nThis is the first message.");
        a.checkEqual("prepare > getMessageHeading",  in.getMessageHeading(0, s.translator(), s.getRoot()->playerList()), "(a) First");
        a.checkEqual("prepare > getMessageMetadata", in.getMessageMetadata(0, s.translator(), s.getRoot()->playerList()).turnNumber, 10);
    }
}

/** Test normal operation. */
AFL_TEST("game.interface.InboxFunction:basics", a)
{
    // Session
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setGame(new game::Game());
    prepare(a, session);

    // Testee
    game::interface::InboxFunction testee(session);
    interpreter::test::ValueVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyNotSerializable();
    a.checkEqual("01. getDimension 0", testee.getDimension(0), 1U);
    a.checkEqual("02. getDimension 1", testee.getDimension(1), 4U);

    // Invoke successfully
    {
        afl::data::Segment seg;
        seg.pushBackInteger(2);
        interpreter::Arguments args(seg, 0, 1);

        std::auto_ptr<interpreter::Context> p(testee.get(args));
        a.checkNonNull("11. get", p.get());
        interpreter::test::ContextVerifier(*p, a("12. get")).verifyString("FULLTEXT", "(-a000)<<< Second >>>\nThis is the second message.");
    }

    // Invoke with null
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        interpreter::test::verifyNewNull(a("21. null"), testee.get(args));
    }

    // Out-of-range
    {
        afl::data::Segment seg;
        seg.pushBackInteger(4);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("31. out-of-range"), testee.get(args), interpreter::Error);
    }
    {
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("32. out-of-range"), testee.get(args), interpreter::Error);
    }

    // Type error
    {
        afl::data::Segment seg;
        seg.pushBackString("X");
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("41. type error"), testee.get(args), interpreter::Error);
    }

    // Arity error
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        AFL_CHECK_THROWS(a("51. arity error"), testee.get(args), interpreter::Error);
    }

    // First
    {
        std::auto_ptr<interpreter::Context> p(testee.makeFirstContext());
        a.checkNonNull("61. makeFirstContext", p.get());
        interpreter::test::ContextVerifier(*p, a("62. makeFirstContext")).verifyString("FULLTEXT", "(-a000)<<< First >>>\nThis is the first message.");
    }

    // Assignment
    {
        afl::data::Segment seg;
        seg.pushBackInteger(2);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("71. set"), testee.set(args, 0), interpreter::Error);
    }
}

/** Test empty session. */
AFL_TEST("game.interface.InboxFunction:empty", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    afl::data::Segment seg;

    // Session empty
    {
        game::Session session(tx, fs);
        game::interface::InboxFunction testee(session);
        interpreter::test::ValueVerifier verif(testee, a("empty session"));
        a.checkEqual("01. getDimension 0", testee.getDimension(0), 1U);
        a.checkEqual("02. getDimension 1", testee.getDimension(1), 0U);

        // Invoke with null
        interpreter::Arguments args(seg, 0, 1);
        interpreter::test::verifyNewNull(a("empty session null"), testee.get(args));

        // First
        interpreter::test::verifyNewNull(a("empty session first"), testee.makeFirstContext());
    }

    // Session populated but no messages
    {
        game::Session session(tx, fs);
        session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
        session.setGame(new game::Game());

        game::interface::InboxFunction testee(session);
        interpreter::test::ValueVerifier verif(testee, a("empty inbox"));
        a.checkEqual("11. getDimension 0", testee.getDimension(0), 1U);
        a.checkEqual("12. getDimension 1", testee.getDimension(1), 1U);

        // Invoke with null
        interpreter::Arguments args(seg, 0, 1);
        interpreter::test::verifyNewNull(a("empty inbox null"), testee.get(args));

        // First
        interpreter::test::verifyNewNull(a("empty inbox first"), testee.makeFirstContext());
    }
}
