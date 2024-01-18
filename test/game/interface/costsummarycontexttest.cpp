/**
  *  \file test/game/interface/costsummarycontexttest.cpp
  *  \brief Test for game::interface::CostSummaryContext
  */

#include "game/interface/costsummarycontext.hpp"

#include "afl/test/testrunner.hpp"
#include "game/spec/costsummary.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/contextverifier.hpp"

/** Test null/empty cases. */
    // Create from null
AFL_TEST("game.interface.CostSummaryContext:create-from-null", a)
{
    std::auto_ptr<game::interface::CostSummaryContext> p;
    afl::base::Ptr<game::spec::CostSummary> cs;
    p.reset(game::interface::CostSummaryContext::create(cs));
    a.checkNull("create", p.get());
}

// Create from empty
AFL_TEST("game.interface.CostSummaryContext:create-from-empty", a)
{
    std::auto_ptr<game::interface::CostSummaryContext> p;
    afl::base::Ptr<game::spec::CostSummary> cs = new game::spec::CostSummary();
    p.reset(game::interface::CostSummaryContext::create(cs));
    a.checkNull("create", p.get());
}

/** Test normal behaviour. */
AFL_TEST("game.interface.CostSummaryContext:normal", a)
{
    afl::base::Ptr<game::spec::CostSummary> cs = new game::spec::CostSummary();
    cs->add(game::spec::CostSummary::Item(1, 4, "Quad", game::spec::Cost::fromString("10T 200$")));
    cs->add(game::spec::CostSummary::Item(1, 2, "Pair", game::spec::Cost::fromString("1T 2D 3M 4S 5$")));

    std::auto_ptr<game::interface::CostSummaryContext> p(game::interface::CostSummaryContext::create(cs));
    a.checkNonNull("01. create", p.get());

    a.checkDifferent("11. toString", p->toString(false), "");
    a.checkNull("12. getObject", p->getObject());

    // Verify first instance
    interpreter::test::ContextVerifier verif(*p, a);
    verif.verifyBasics();
    verif.verifyNotSerializable();

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
    a.check("21. next", p->next());
    verif.verifyInteger("COUNT",    2);
    verif.verifyString("NAME",      "Pair");
    verif.verifyInteger("T",        1);
    verif.verifyInteger("D",        2);
    verif.verifyInteger("M",        3);
    verif.verifyInteger("MONEY",    5);
    verif.verifyInteger("SUPPLIES", 4);
    verif.verifyInteger("CASH",     9);

    // No third instance
    a.check("31. next", !p->next());
}
