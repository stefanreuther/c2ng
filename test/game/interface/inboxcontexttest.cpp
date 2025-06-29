/**
  *  \file test/game/interface/inboxcontexttest.cpp
  *  \brief Test for game::interface::InboxContext
  */

#include "game/interface/inboxcontext.hpp"

#include "afl/data/access.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/msg/inbox.hpp"
#include "game/root.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/error.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/process.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/world.hpp"
#include "util/io.hpp"

namespace {
    struct TestHarness {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        afl::base::Ref<game::Root> root;
        afl::base::Ref<game::Game> game;
        game::Session session;

        TestHarness()
            : tx(), fs(), root(game::test::makeRoot(game::HostVersion())), game(*new game::Game()),
              session(tx, fs)
            { }
    };

    void prepare(afl::test::Assert a, TestHarness& h)
    {
        // Add some messages
        game::msg::Inbox& in = h.game->currentTurn().inbox();
        in.addMessage("(-a000)<<< First >>>\nThis is the first message.", 10);
        in.addMessage("(-a000)<<< Second >>>\nThis is the second message.", 10);
        in.addMessage("(-a000)<<< Third >>>\nThis is the third message.", 11);

        // Verify our assumptions
        a.checkEqual("prepare > getNumMessages",     in.getNumMessages(), 3U);
        a.checkEqual("prepare > getMessageText",     in.getMessageText(0, h.tx, h.root->playerList()), "(-a000)<<< First >>>\nThis is the first message.");
        a.checkEqual("prepare > getMessageHeading",  in.getMessageHeading(0, h.tx, h.root->playerList()), "(a) First");
        a.checkEqual("prepare > getMessageMetadata", in.getMessageMetadata(0, h.tx, h.root->playerList()).turnNumber, 10);

        h.session.setGame(h.game.asPtr());
        h.session.setRoot(h.root.asPtr());
    }
}

/** Test common property access. */
AFL_TEST("game.interface.InboxContext:properties", a)
{
    TestHarness h;
    prepare(a, h);

    game::interface::InboxContext testee(2, h.session, h.game->currentTurn());

    // Values (lookup, get)
    interpreter::test::ContextVerifier v(testee, a);
    v.verifyTypes();
    v.verifyBasics();
    v.verifyNotSerializable();
    v.verifyInteger("ID", 3);                 // 1-based
    v.verifyString("GROUP", "(a) Third");
    v.verifyInteger("LINES", 2);
    v.verifyBoolean("KILLED", false);
    v.verifyString("FULLTEXT", "(-a000)<<< Third >>>\nThis is the third message.");

    // Extras
    std::auto_ptr<interpreter::Context> c(testee.clone());
    a.checkNonNull("01. clone", c.get());
    interpreter::test::ContextVerifier(*c, a("02. clone")).verifyInteger("ID", 3);

    a.checkNull("11. getObject", testee.getObject());
    a.checkDifferent("12. toString", testee.toString(false), "");
}

/** Test the WRITE method. */
AFL_TEST("game.interface.InboxContext:write", a)
{
    const int FD = 17;

    TestHarness h;
    prepare(a, h);

    // Fetch 'WRITE' property
    game::interface::InboxContext testee(2, h.session, h.game->currentTurn());
    std::auto_ptr<afl::data::Value> write(interpreter::test::ContextVerifier(testee, a).getValue("WRITE"));
    a.checkNonNull("01. write", write.get());

    // Verify that it is callable
    interpreter::CallableValue* cv = dynamic_cast<interpreter::CallableValue*>(write.get());
    a.checkNonNull("11. CallableValue", cv);
    a.check("12. isProcedureCall", cv->isProcedureCall());
    a.checkEqual("13. getDimension", cv->getDimension(0), 0U);
    AFL_CHECK_THROWS(a("14. makeFirstContext"), cv->makeFirstContext(), interpreter::Error);
    a.checkDifferent("15. toString", cv->toString(false), "");

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
    AFL_CHECK_SUCCEEDS(a("21. call"), cv->call(proc, args, false));

    // Close file to flush
    world.fileTable().closeFile(FD);

    // Verify file content
    String_t content = util::normalizeLinefeeds(s->getContent());
    a.checkEqual("31. content", content,
                 "=== Turn 11 ===\n"
                 "--- Message 3 ---\n"
                 "(-a000)<<< Third >>>\n"
                 "This is the third message.\n");
}

/** Test the TEXT property. */
AFL_TEST("game.interface.InboxContext:text", a)
{
    TestHarness h;
    prepare(a, h);

    // Fetch 'TEXT' property
    game::interface::InboxContext testee(2, h.session, h.game->currentTurn());
    std::auto_ptr<afl::data::Value> text(interpreter::test::ContextVerifier(testee, a).getValue("TEXT"));
    a.checkNonNull("01. text", text.get());

    // Verify that it is indexable
    interpreter::IndexableValue* iv = dynamic_cast<interpreter::IndexableValue*>(text.get());
    a.checkNonNull("11. IndexableValue", iv);
    a.check("12. isProcedureCall", !iv->isProcedureCall());
    a.checkEqual("13. getDimension 0", iv->getDimension(0), 1U);
    a.checkEqual("14. getDimension 1", iv->getDimension(1), 3U);    /* 2 lines */
    AFL_CHECK_THROWS(a("15. makeFirstContext"), iv->makeFirstContext(), interpreter::Error);
    a.checkDifferent("16. toString", iv->toString(false), "");

    // Fetch a line, success case
    {
        afl::data::Segment args;
        args.pushBackInteger(1);
        interpreter::Arguments ap(args, 0, 1);
        std::auto_ptr<afl::data::Value> result(iv->get(ap));
        a.checkNonNull("21. get", result.get());
        a.checkEqual("22. value", afl::data::Access(result.get()).toString(), "(-a000)<<< Third >>>");
    }
    {
        afl::data::Segment args;
        args.pushBackInteger(2);
        interpreter::Arguments ap(args, 0, 1);
        std::auto_ptr<afl::data::Value> result(iv->get(ap));
        a.checkNonNull("26. get", result.get());
        a.checkEqual("27. value", afl::data::Access(result.get()).toString(), "This is the third message.");
    }

    // Wrong-number-of-arguments case
    {
        afl::data::Segment args;
        interpreter::Arguments ap(args, 0, 0);
        AFL_CHECK_THROWS(a("31. arity error"), iv->get(ap), interpreter::Error);
    }

    // Null case
    {
        afl::data::Segment args;
        interpreter::Arguments ap(args, 0, 1);
        std::auto_ptr<afl::data::Value> result(iv->get(ap));
        a.checkNull("41. null", result.get());
    }

    // Range error
    {
        afl::data::Segment args;
        args.pushBackInteger(0);
        interpreter::Arguments ap(args, 0, 1);
        AFL_CHECK_THROWS(a("51. range error"), iv->get(ap), interpreter::Error);
    }
    {
        afl::data::Segment args;
        args.pushBackInteger(3);
        interpreter::Arguments ap(args, 0, 1);
        AFL_CHECK_THROWS(a("52. range error"), iv->get(ap), interpreter::Error);
    }
}

/** Test iteration. */
AFL_TEST("game.interface.InboxContext:iteration", a)
{
    TestHarness h;
    prepare(a, h);

    game::interface::InboxContext testee(0, h.session, h.game->currentTurn());
    interpreter::test::ContextVerifier v(testee, a);
    v.verifyInteger("ID", 1);

    // Next
    a.check("01. next", testee.next());
    v.verifyInteger("ID", 2);

    // Next
    a.check("11. next", testee.next());
    v.verifyInteger("ID", 3);

    // No more messages; remain at #3
    a.check("21. next", !testee.next());
    v.verifyInteger("ID", 3);
}
