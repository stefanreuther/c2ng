/**
  *  \file test/game/interface/simclassresultcontexttest.cpp
  *  \brief Test for game::interface::SimClassResultContext
  */

#include "game/interface/simclassresultcontext.hpp"

#include "afl/test/testrunner.hpp"
#include "game/sim/setup.hpp"
#include "game/sim/ship.hpp"
#include "interpreter/test/contextverifier.hpp"

using afl::base::Ptr;
using game::interface::SimClassResultContext;
using game::sim::ResultList;
using game::sim::Setup;
using interpreter::Context;
using interpreter::test::ContextVerifier;

namespace {
    void addShip(Setup& setup, int owner, int damage, int fighters)
    {
        game::sim::Ship& sh = setup.addShip();
        sh.setOwner(owner);
        sh.setDamage(damage);
        sh.setNumBays(3);
        sh.setAmmo(fighters);
    }
}

AFL_TEST("game.interface.SimClassResultContext:basics", a)
{
    Ptr<SimClassResultContext::Infos_t> infos = new SimClassResultContext::Infos_t();

    ResultList::ClassInfo ci;
    ci.label = "10x (50%)";
    ci.weight = 150;
    ci.ownedUnits.set(4, 2);
    ci.ownedUnits.set(9, 1);
    infos->push_back(ci);

    game::interface::SimClassResultContext testee(infos, 300, 0);
    a.checkNull("getObject", testee.getObject());

    // Verify basics
    ContextVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyTypes();
    verif.verifyNotSerializable();

    // Failure to look up
    Context::PropertyIndex_t idx;
    a.checkNull("lookup failure", testee.lookup("NARF", idx));

    // Properties
    verif.verifyInteger("COUNT", 150);
    verif.verifyString("LABEL", "10x (50%)");
    verif.verifyInteger("PLAYER1", 0);
    verif.verifyInteger("PLAYER2", 0);
    verif.verifyInteger("PLAYER3", 0);
    verif.verifyInteger("PLAYER4", 2);
    verif.verifyInteger("PLAYER5", 0);
    verif.verifyInteger("PLAYER6", 0);
    verif.verifyInteger("PLAYER7", 0);
    verif.verifyInteger("PLAYER8", 0);
    verif.verifyInteger("PLAYER9", 1);
    verif.verifyInteger("PLAYER10", 0);
    verif.verifyInteger("PLAYER11", 0);
    verif.verifyInteger("PLAYER12", 0);
    verif.verifyFloat("RATIO", 50, 0.01);
}

/** Test iteration. */
AFL_TEST("game.interface.SimClassResultContext:iteration", a)
{
    Ptr<SimClassResultContext::Infos_t> infos = new SimClassResultContext::Infos_t();

    {
        ResultList::ClassInfo ci;
        ci.weight = 42;
        infos->push_back(ci);
    }
    {
        ResultList::ClassInfo ci;
        ci.weight = 69;
        infos->push_back(ci);
    }

    game::interface::SimClassResultContext testee(infos, 300, 0);

    // Verify
    ContextVerifier(testee, a("first")).verifyInteger("COUNT", 42);
    a.check("first next", testee.next());

    ContextVerifier(testee, a("second")).verifyInteger("COUNT", 69);
    a.check("second next", !testee.next());
}

/** Test out-of-range access. */
AFL_TEST("game.interface.SimClassResultContext:out-of-range", a)
{
    Ptr<SimClassResultContext::Infos_t> infos = new SimClassResultContext::Infos_t();
    game::interface::SimClassResultContext testee(infos, 300, 0);
    a.checkNull("getObject", testee.getObject());

    // Verify basics
    ContextVerifier verif(testee, a);
    verif.verifyNull("COUNT");
    verif.verifyNull("LABEL");
    verif.verifyNull("PLAYER1");
    verif.verifyNull("PLAYER2");
    verif.verifyNull("PLAYER3");
    verif.verifyNull("PLAYER4");
    verif.verifyNull("PLAYER5");
    verif.verifyNull("PLAYER6");
    verif.verifyNull("PLAYER7");
    verif.verifyNull("PLAYER8");
    verif.verifyNull("PLAYER9");
    verif.verifyNull("PLAYER10");
    verif.verifyNull("PLAYER11");
    verif.verifyNull("PLAYER12");
    verif.verifyNull("RATIO");
}

/** Test creation from result. */
AFL_TEST("game.interface.SimClassResultContext:create", a)
{
    game::vcr::Statistic stat[2];

    game::sim::ResultList list;
    Setup before; addShip(before, 2,   0, 10); addShip(before, 3,   0, 20);
    Setup after1; addShip(after1, 0, 100,  3); addShip(after1, 3,  50,  9);
    list.addResult(before, after1, stat, game::sim::Result(game::sim::Configuration(), 0));

    Setup after2; addShip(after2, 2, 90,   8); addShip(after2, 0, 100,  2);
    list.addResult(before, after2, stat, game::sim::Result(game::sim::Configuration(), 0));

    a.checkEqual("must have two classes", list.getNumClassResults(), 2U);

    util::NumberFormatter fmt(false, false);
    std::auto_ptr<SimClassResultContext> testee(SimClassResultContext::create(list, fmt));
    a.checkNonNull("must have context", testee.get());

    // Verify
    {
        ContextVerifier verif(*testee, a);
        verif.verifyInteger("COUNT", 1);
        verif.verifyInteger("PLAYER2", 0);
        verif.verifyInteger("PLAYER3", 1);
    }
    a.check("can advance to next", testee->next());
    {
        ContextVerifier verif(*testee, a);
        verif.verifyInteger("COUNT", 1);
        verif.verifyInteger("PLAYER2", 1);
        verif.verifyInteger("PLAYER3", 0);
    }
    a.check("end reached", !testee->next());
}

/** Test creation from empty result. */
AFL_TEST("game.interface.SimClassResultContext:create:empty", a)
{
    game::sim::ResultList list;
    util::NumberFormatter fmt(false, false);
    std::auto_ptr<SimClassResultContext> testee(SimClassResultContext::create(list, fmt));
    a.checkNull("must have null context", testee.get());
}
