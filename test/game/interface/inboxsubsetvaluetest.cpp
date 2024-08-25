/**
  *  \file test/game/interface/inboxsubsetvaluetest.cpp
  *  \brief Test for game::interface::InboxSubsetValue
  */

#include "game/interface/inboxsubsetvalue.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/msg/inbox.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/contextverifier.hpp"

using game::interface::InboxSubsetValue;

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

    void prepare(TestHarness& h)
    {
        game::msg::Inbox& in = h.game->currentTurn().inbox();
        in.addMessage("(-a000)<<< First >>>\nThis is the first message.", 10);
        in.addMessage("(-a000)<<< Second >>>\nThis is the second message.", 10);
        in.addMessage("(-a000)<<< Third >>>\nThis is the third message.", 11);
        in.addMessage("(-a000)<<< Fourth >>>\nThis is the fourth message.", 11);
        in.addMessage("(-a000)<<< Fifth >>>\nThis is the fifth message.", 11);

        h.session.setRoot(h.root.asPtr());
        h.session.setGame(h.game.asPtr());
    }
}

/** Creating from an empty vector produces a null object. */
AFL_TEST("game.interface.InboxSubsetValue:empty", a)
{
    TestHarness h;
    std::vector<size_t> indexes;

    // Factory method
    {
        std::auto_ptr<InboxSubsetValue> value(InboxSubsetValue::create(indexes, h.session, h.game->currentTurn()));
        a.checkNull("01. factory method", value.get());
    }

    // Explicit creation
    {
        InboxSubsetValue value(indexes, h.session, h.game->currentTurn());
        a.checkNull("11. explicit", value.makeFirstContext());
    }
}

/** Test iteration over a InboxSubsetValue ("ForEach (unit).Messages"). */
AFL_TEST("game.interface.InboxSubsetValue:iteration", a)
{
    TestHarness h;
    prepare(h);

    std::vector<size_t> indexes;
    indexes.push_back(3);                 // "Fourth"
    indexes.push_back(0);                 // "First"

    std::auto_ptr<InboxSubsetValue> value(InboxSubsetValue::create(indexes, h.session, h.game->currentTurn()));
    a.checkNonNull("01. create", value.get());

    // Basic properties
    a.checkDifferent("11. toString", value->toString(false), "");
    a.checkEqual("12. getDimension 0", value->getDimension(0), 1U);
    a.checkEqual("13. getDimension 1", value->getDimension(1), 3U);
    interpreter::test::ValueVerifier vv(*value, a("values"));
    vv.verifyBasics();
    vv.verifyNotSerializable();

    // Access first and verify
    std::auto_ptr<interpreter::Context> ctx(value->makeFirstContext());
    a.checkNonNull("21. ctx", ctx.get());
    a.checkDifferent("22. toString", ctx->toString(false), "");

    interpreter::test::ContextVerifier v(*ctx, a("iteration"));
    v.verifyTypes();
    v.verifyBasics();
    v.verifyNotSerializable();
    a.checkNull("31. getObject", ctx->getObject());

    // Iterate
    v.verifyInteger("ID", 4);
    a.check("41. next", ctx->next());
    v.verifyInteger("ID", 1);
    a.check("42. next", !ctx->next());
}

/** Test indexed access ("(unit).Messages(x)"). */
AFL_TEST("game.interface.InboxSubsetValue:indexing", a)
{
    TestHarness h;
    prepare(h);

    std::vector<size_t> indexes;
    indexes.push_back(3);                 // "Fourth"
    indexes.push_back(0);                 // "First"

    std::auto_ptr<InboxSubsetValue> value(InboxSubsetValue::create(indexes, h.session, h.game->currentTurn()));
    a.checkNonNull("01. create", value.get());

    // Success case
    {
        afl::data::Segment seg;
        seg.pushBackInteger(2);
        interpreter::Arguments args(seg, 0, 1);

        std::auto_ptr<afl::data::Value> result(value->get(args));
        a.checkNonNull("11. get", result.get());

        interpreter::Context* ctx = dynamic_cast<interpreter::Context*>(result.get());
        a.checkNonNull("21. ctx", ctx);
        a.checkDifferent("22. toString", ctx->toString(false), "");

        interpreter::test::ContextVerifier v(*ctx, a);
        v.verifyInteger("ID", 1);
    }

    // Null index
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);

        std::auto_ptr<afl::data::Value> result(value->get(args));
        a.checkNull("31. null", result.get());
    }

    // Arity error
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        AFL_CHECK_THROWS(a("41. arity error"), value->get(args), interpreter::Error);
    }

    // Type error
    {
        afl::data::Segment seg;
        seg.pushBackString("x");
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("51. type error"), value->get(args), interpreter::Error);
    }

    // Cannot assign
    {
        afl::data::Segment seg;
        seg.pushBackInteger(2);
        interpreter::Arguments args(seg, 0, 1);

        AFL_CHECK_THROWS(a("61. set"), value->set(args, 0), interpreter::Error);
    }
}
