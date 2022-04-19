/**
  *  \file u/t_game_interface_costsummarycontext.cpp
  *  \brief Test for game::interface::CostSummaryContext
  */

#include "game/interface/costsummarycontext.hpp"

#include "t_game_interface.hpp"
#include "game/spec/costsummary.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/error.hpp"
#include "afl/io/internalsink.hpp"
#include "interpreter/vmio/nullsavecontext.hpp"

/** Test null/empty cases. */
void
TestGameInterfaceCostSummaryContext::testEmpty()
{
    std::auto_ptr<game::interface::CostSummaryContext> p;

    // Create from null
    {
        afl::base::Ptr<game::spec::CostSummary> cs;
        p.reset(game::interface::CostSummaryContext::create(cs));
        TS_ASSERT(p.get() == 0);
    }

    // Create from empty
    {
        afl::base::Ptr<game::spec::CostSummary> cs = new game::spec::CostSummary();
        p.reset(game::interface::CostSummaryContext::create(cs));
        TS_ASSERT(p.get() == 0);
    }
}

/** Test normal behaviour. */
void
TestGameInterfaceCostSummaryContext::testNormal()
{
    afl::base::Ptr<game::spec::CostSummary> cs = new game::spec::CostSummary();
    cs->add(game::spec::CostSummary::Item(1, 4, "Quad", game::spec::Cost::fromString("10T 200$")));
    cs->add(game::spec::CostSummary::Item(1, 2, "Pair", game::spec::Cost::fromString("1T 2D 3M 4S 5$")));

    std::auto_ptr<game::interface::CostSummaryContext> p(game::interface::CostSummaryContext::create(cs));
    TS_ASSERT(p.get() != 0);

    TS_ASSERT_DIFFERS(p->toString(false), "");
    TS_ASSERT(p->getObject() == 0);

    std::auto_ptr<game::interface::CostSummaryContext> clone(p->clone());
    TS_ASSERT(clone.get() != 0);
    TS_ASSERT(clone.get() != p.get());

    {
        interpreter::TagNode tag;
        afl::io::InternalSink out;
        interpreter::vmio::NullSaveContext saveContext;
        TS_ASSERT_THROWS(p->store(tag, out, saveContext), interpreter::Error);
    }

    // Verify first instance
    interpreter::test::ContextVerifier verif(*p, "testNormal: first");
    verif.verifyTypes();
    verif.verifyInteger("COUNT",    4);
    verif.verifyString("NAME",      "Quad");
    verif.verifyInteger("T",       10);
    verif.verifyInteger("D",        0);
    verif.verifyInteger("M",        0);
    verif.verifyInteger("MONEY",  200);
    verif.verifyInteger("SUPPLIES", 0);
    verif.verifyInteger("CASH",   200);

    // Verify second instance
    TS_ASSERT(p->next());
    verif.verifyInteger("COUNT",    2);
    verif.verifyString("NAME",      "Pair");
    verif.verifyInteger("T",        1);
    verif.verifyInteger("D",        2);
    verif.verifyInteger("M",        3);
    verif.verifyInteger("MONEY",    5);
    verif.verifyInteger("SUPPLIES", 4);
    verif.verifyInteger("CASH",     9);

    // No third instance
    TS_ASSERT(!p->next());
}

