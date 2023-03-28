/**
  *  \file u/t_game_interface_inboxfunction.cpp
  *  \brief Test for game::interface::InboxFunction
  */

#include "game/interface/inboxfunction.hpp"

#include "t_game_interface.hpp"
#include "afl/data/segment.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/msg/inbox.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

namespace {
    void prepare(game::Session& s)
    {
        // Add some messages
        game::msg::Inbox& in = s.getGame()->currentTurn().inbox();
        in.addMessage("(-a000)<<< First >>>\nThis is the first message.", 10);
        in.addMessage("(-a000)<<< Second >>>\nThis is the second message.", 10);
        in.addMessage("(-a000)<<< Third >>>\nThis is the third message.", 11);

        // Verify our assumptions
        TS_ASSERT_EQUALS(in.getNumMessages(), 3U);
        TS_ASSERT_EQUALS(in.getMessageText(0, s.translator(), s.getRoot()->playerList()), "(-a000)<<< First >>>\nThis is the first message.");
        TS_ASSERT_EQUALS(in.getMessageHeading(0, s.translator(), s.getRoot()->playerList()), "(a) First");
        TS_ASSERT_EQUALS(in.getMessageMetadata(0, s.translator(), s.getRoot()->playerList()).turnNumber, 10);
    }
}

/** Test normal operation. */
void
TestGameInterfaceInboxFunction::testIt()
{
    // Session
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setGame(new game::Game());
    prepare(session);

    // Testee
    game::interface::InboxFunction testee(session);
    interpreter::test::ValueVerifier verif(testee, "testIt");
    verif.verifyBasics();
    verif.verifyNotSerializable();
    TS_ASSERT_EQUALS(testee.getDimension(0), 1);
    TS_ASSERT_EQUALS(testee.getDimension(1), 4);

    // Invoke successfully
    {
        afl::data::Segment seg;
        seg.pushBackInteger(2);
        interpreter::Arguments args(seg, 0, 1);

        std::auto_ptr<interpreter::Context> p(testee.get(args));
        TS_ASSERT(p.get() != 0);
        interpreter::test::ContextVerifier(*p, "(1)").verifyString("FULLTEXT", "(-a000)<<< Second >>>\nThis is the second message.");
    }

    // Invoke with null
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        interpreter::test::verifyNewNull("(null)", testee.get(args));
    }

    // Out-of-range
    {
        afl::data::Segment seg;
        seg.pushBackInteger(4);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(testee.get(args), interpreter::Error);
    }

    // Type error
    {
        afl::data::Segment seg;
        seg.pushBackString("X");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(testee.get(args), interpreter::Error);
    }

    // Arity error
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(testee.get(args), interpreter::Error);
    }

    // First
    {
        std::auto_ptr<interpreter::Context> p(testee.makeFirstContext());
        TS_ASSERT(p.get() != 0);
        interpreter::test::ContextVerifier(*p, "first").verifyString("FULLTEXT", "(-a000)<<< First >>>\nThis is the first message.");
    }

    // Assignment
    {
        afl::data::Segment seg;
        seg.pushBackInteger(2);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(testee.set(args, 0), interpreter::Error);
    }
}

/** Test empty session. */
void
TestGameInterfaceInboxFunction::testEmpty()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    afl::data::Segment seg;

    // Session empty
    {
        game::Session session(tx, fs);
        game::interface::InboxFunction testee(session);
        interpreter::test::ValueVerifier verif(testee, "empty session");
        TS_ASSERT_EQUALS(testee.getDimension(0), 1);
        TS_ASSERT_EQUALS(testee.getDimension(1), 0);

        // Invoke with null
        interpreter::Arguments args(seg, 0, 1);
        interpreter::test::verifyNewNull("empty session null", testee.get(args));

        // First
        interpreter::test::verifyNewNull("empty session first", testee.makeFirstContext());
    }

    // Session populated but no messages
    {
        game::Session session(tx, fs);
        session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
        session.setGame(new game::Game());

        game::interface::InboxFunction testee(session);
        interpreter::test::ValueVerifier verif(testee, "empty inbox");
        TS_ASSERT_EQUALS(testee.getDimension(0), 1);
        TS_ASSERT_EQUALS(testee.getDimension(1), 1);

        // Invoke with null
        interpreter::Arguments args(seg, 0, 1);
        interpreter::test::verifyNewNull("empty inbox null", testee.get(args));

        // First
        interpreter::test::verifyNewNull("empty inbox first", testee.makeFirstContext());
    }
}

