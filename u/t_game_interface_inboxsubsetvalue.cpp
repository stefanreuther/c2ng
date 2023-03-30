/**
  *  \file u/t_game_interface_inboxsubsetvalue.cpp
  *  \brief Test for game::interface::InboxSubsetValue
  */

#include "game/interface/inboxsubsetvalue.hpp"

#include "t_game_interface.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/msg/inbox.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/contextverifier.hpp"

using game::interface::InboxSubsetValue;

namespace {
    struct TestHarness {
        afl::string::NullTranslator tx;
        afl::base::Ref<game::Root> root;
        afl::base::Ref<game::Game> game;

        TestHarness()
            : tx(), root(game::test::makeRoot(game::HostVersion())), game(*new game::Game())
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
    }
}

/** Creating from an empty vector produces a null object. */
void
TestGameInterfaceInboxSubsetValue::testEmpty()
{
    TestHarness h;
    std::vector<size_t> indexes;

    std::auto_ptr<InboxSubsetValue> value(InboxSubsetValue::create(indexes, h.tx, h.root, h.game));
    TS_ASSERT(value.get() == 0);
}

/** Test iteration over a InboxSubsetValue ("ForEach (unit).Messages"). */
void
TestGameInterfaceInboxSubsetValue::testIteration()
{
    TestHarness h;
    prepare(h);

    std::vector<size_t> indexes;
    indexes.push_back(3);                 // "Fourth"
    indexes.push_back(0);                 // "First"

    std::auto_ptr<InboxSubsetValue> value(InboxSubsetValue::create(indexes, h.tx, h.root, h.game));
    TS_ASSERT(value.get() != 0);

    // Basic properties
    TS_ASSERT_DIFFERS(value->toString(false), "");
    TS_ASSERT_EQUALS(value->getDimension(0), 1);
    TS_ASSERT_EQUALS(value->getDimension(1), 3);

    // Access first and iterate
    std::auto_ptr<interpreter::Context> ctx(value->makeFirstContext());
    TS_ASSERT(ctx.get() != 0);
    TS_ASSERT_DIFFERS(ctx->toString(false), "");

    interpreter::test::ContextVerifier v(*ctx, "testIteration");
    v.verifyTypes();
    v.verifyBasics();
    v.verifyInteger("ID", 4);

    TS_ASSERT(ctx->next());
    v.verifyInteger("ID", 1);

    TS_ASSERT(!ctx->next());
}

/** Test indexed access ("(unit).Messages(x)"). */
void
TestGameInterfaceInboxSubsetValue::testIndexing()
{
    TestHarness h;
    prepare(h);

    std::vector<size_t> indexes;
    indexes.push_back(3);                 // "Fourth"
    indexes.push_back(0);                 // "First"

    std::auto_ptr<InboxSubsetValue> value(InboxSubsetValue::create(indexes, h.tx, h.root, h.game));
    TS_ASSERT(value.get() != 0);

    // Success case
    {
        afl::data::Segment seg;
        seg.pushBackInteger(2);
        interpreter::Arguments args(seg, 0, 1);

        std::auto_ptr<afl::data::Value> result(value->get(args));
        TS_ASSERT(result.get() != 0);

        interpreter::Context* ctx = dynamic_cast<interpreter::Context*>(result.get());
        TS_ASSERT(ctx != 0);
        TS_ASSERT_DIFFERS(ctx->toString(false), "");

        interpreter::test::ContextVerifier v(*ctx, "testIndexing");
        v.verifyInteger("ID", 1);
    }

    // Null index
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);

        std::auto_ptr<afl::data::Value> result(value->get(args));
        TS_ASSERT(result.get() == 0);
    }

    // Arity error
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(value->get(args), interpreter::Error);
    }

    // Type error
    {
        afl::data::Segment seg;
        seg.pushBackString("x");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(value->get(args), interpreter::Error);
    }
}

