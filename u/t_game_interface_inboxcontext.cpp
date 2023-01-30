/**
  *  \file u/t_game_interface_inboxcontext.cpp
  *  \brief Test for game::interface::InboxContext
  */

#include "game/interface/inboxcontext.hpp"

#include "t_game_interface.hpp"
#include "afl/data/access.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/msg/inbox.hpp"
#include "game/root.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/error.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/process.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/world.hpp"

namespace {
    struct TestHarness {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        afl::base::Ref<game::Root> root;
        afl::base::Ref<game::Game> game;

        TestHarness()
            : tx(), fs(), root(game::test::makeRoot(game::HostVersion())), game(*new game::Game())
            { }
    };

    void prepare(TestHarness& h)
    {
        // Add some messages
        game::msg::Inbox& in = h.game->currentTurn().inbox();
        in.addMessage("(-a000)<<< First >>>\nThis is the first message.", 10);
        in.addMessage("(-a000)<<< Second >>>\nThis is the second message.", 10);
        in.addMessage("(-a000)<<< Third >>>\nThis is the third message.", 11);

        // Verify our assumptions
        TS_ASSERT_EQUALS(in.getNumMessages(), 3U);
        TS_ASSERT_EQUALS(in.getMessageText(0, h.tx, h.root->playerList()), "(-a000)<<< First >>>\nThis is the first message.");
        TS_ASSERT_EQUALS(in.getMessageHeading(0, h.tx, h.root->playerList()), "(a) First");
        TS_ASSERT_EQUALS(in.getMessageMetadata(0, h.tx, h.root->playerList()).turnNumber, 10);
    }
}

/** Test common property access. */
void
TestGameInterfaceInboxContext::testProperties()
{
    TestHarness h;
    prepare(h);

    game::interface::InboxContext testee(2, h.tx, h.root, h.game);

    // Values (lookup, get)
    interpreter::test::ContextVerifier v(testee, "testProperties");
    v.verifyTypes();
    v.verifyInteger("ID", 3);                 // 1-based
    v.verifyString("GROUP", "(a) Third");
    v.verifyInteger("LINES", 2);
    v.verifyBoolean("KILLED", false);
    v.verifyString("FULLTEXT", "(-a000)<<< Third >>>\nThis is the third message.");

    // Extras
    std::auto_ptr<interpreter::Context> c(testee.clone());
    TS_ASSERT(c.get() != 0);
    interpreter::test::ContextVerifier(*c, "testProperties#2").verifyInteger("ID", 3);

    TS_ASSERT(testee.getObject() == 0);
    TS_ASSERT_DIFFERS(testee.toString(false), "");
}

/** Test the WRITE method. */
void
TestGameInterfaceInboxContext::testWrite()
{
    const int FD = 17;

    TestHarness h;
    prepare(h);

    // Fetch 'WRITE' property
    game::interface::InboxContext testee(2, h.tx, h.root, h.game);
    std::auto_ptr<afl::data::Value> write(interpreter::test::ContextVerifier(testee, "testWrite").getValue("WRITE"));
    TS_ASSERT(write.get() != 0);

    // Verify that it is callable
    interpreter::CallableValue* cv = dynamic_cast<interpreter::CallableValue*>(write.get());
    TS_ASSERT(cv != 0);
    TS_ASSERT(cv->isProcedureCall());
    TS_ASSERT_EQUALS(cv->getDimension(0), 0);
    TS_ASSERT_THROWS(cv->makeFirstContext(), interpreter::Error);
    TS_ASSERT_DIFFERS(cv->toString(false), "");

    // Set up a world to call it
    afl::sys::Log log;
    interpreter::World world(log, h.tx, h.fs);
    interpreter::Process proc(world, "tester", 777);

    // Open a pseudo file
    afl::base::Ref<afl::io::InternalStream> s = *new afl::io::InternalStream();
    world.fileTable().setMaxFiles(100);
    world.fileTable().openFile(FD, s);

    // Call the WRITE method
    afl::data::Segment args;
    args.pushBackInteger(FD);
    TS_ASSERT_THROWS_NOTHING(cv->call(proc, args, false));

    // Close file to flush
    world.fileTable().closeFile(FD);

    // Verify file content
    String_t content = afl::string::fromBytes(s->getContent());
    String_t::size_type n;
    while ((n = content.find('\r')) != String_t::npos) {
        content.erase(n, 1);
    }

    TS_ASSERT_EQUALS(content,
                     "=== Turn 11 ===\n"
                     "--- Message 3 ---\n"
                     "(-a000)<<< Third >>>\n"
                     "This is the third message.\n");
}

/** Test the TEXT property. */
void
TestGameInterfaceInboxContext::testText()
{
    TestHarness h;
    prepare(h);

    // Fetch 'TEXT' property
    game::interface::InboxContext testee(2, h.tx, h.root, h.game);
    std::auto_ptr<afl::data::Value> write(interpreter::test::ContextVerifier(testee, "testText").getValue("TEXT"));
    TS_ASSERT(write.get() != 0);

    // Verify that it is indexable
    interpreter::IndexableValue* iv = dynamic_cast<interpreter::IndexableValue*>(write.get());
    TS_ASSERT(iv != 0);
    TS_ASSERT(!iv->isProcedureCall());
    TS_ASSERT_EQUALS(iv->getDimension(0), 1);
    TS_ASSERT_EQUALS(iv->getDimension(1), 3);    /* 2 lines */
    TS_ASSERT_THROWS(iv->makeFirstContext(), interpreter::Error);
    TS_ASSERT_DIFFERS(iv->toString(false), "");

    // Fetch a line, success case
    {
        afl::data::Segment args;
        args.pushBackInteger(1);
        interpreter::Arguments a(args, 0, 1);
        std::auto_ptr<afl::data::Value> result(iv->get(a));
        TS_ASSERT(result.get() != 0);
        TS_ASSERT_EQUALS(afl::data::Access(result.get()).toString(), "(-a000)<<< Third >>>");

    }

    // Wrong-number-of-arguments case
    {
        afl::data::Segment args;
        interpreter::Arguments a(args, 0, 0);
        TS_ASSERT_THROWS(iv->get(a), interpreter::Error);
    }

    // Null case
    {
        afl::data::Segment args;
        interpreter::Arguments a(args, 0, 1);
        std::auto_ptr<afl::data::Value> result(iv->get(a));
        TS_ASSERT(result.get() == 0);
    }
}

/** Test iteration. */
void
TestGameInterfaceInboxContext::testIteration()
{
    TestHarness h;
    prepare(h);

    game::interface::InboxContext testee(0, h.tx, h.root, h.game);
    interpreter::test::ContextVerifier v(testee, "testProperties");
    v.verifyInteger("ID", 1);

    // Next
    TS_ASSERT(testee.next());
    v.verifyInteger("ID", 2);

    // Next
    TS_ASSERT(testee.next());
    v.verifyInteger("ID", 3);

    // No more messages; remain at #3
    TS_ASSERT(!testee.next());
    v.verifyInteger("ID", 3);
}

