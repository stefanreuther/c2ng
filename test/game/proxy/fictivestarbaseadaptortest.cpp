/**
  *  \file test/game/proxy/fictivestarbaseadaptortest.cpp
  *  \brief Test for game::proxy::FictiveStarbaseAdaptor
  */

#include "game/proxy/fictivestarbaseadaptor.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"

using afl::string::NullTranslator;
using afl::io::NullFileSystem;

/** Test operation on empty session: object can correctly be constructed. */
AFL_TEST("game.proxy.FictiveStarbaseAdaptor:empty", a)
{
    // Environment
    NullTranslator tx;
    NullFileSystem fs;
    game::Session session(tx, fs);

    // Verify
    game::proxy::FictiveStarbaseAdaptor testee(session, 99);
    a.checkEqual("01. getName",   testee.planet().getName(tx), "Magrathea");
    a.checkEqual("02. getId",     testee.planet().getId(), 99);
    a.checkEqual("03. hasBase",   testee.planet().hasBase(), true);
    a.checkEqual("04. Tritanium", testee.planet().getCargo(game::Element::Tritanium).orElse(-1), 1000);
    a.checkEqual("05. HullTech",  testee.planet().getBaseTechLevel(game::HullTech).orElse(-1), 1);
}

/** Test operation on nonempty session, Id zero: object can correctly be constructed. */
AFL_TEST("game.proxy.FictiveStarbaseAdaptor:zero", a)
{
    // Environment
    NullTranslator tx;
    NullFileSystem fs;
    game::Session session(tx, fs);

    // Create game (provides viewpoint player)
    afl::base::Ptr<game::Game> g = new game::Game();
    g->setViewpointPlayer(3);
    session.setGame(g);

    // Verify
    game::proxy::FictiveStarbaseAdaptor testee(session, 0);
    a.checkEqual("01. getName", testee.planet().getName(tx), "Magrathea");
    a.checkEqual("02. getId",   testee.planet().getId(), 42);      /* invented Id */
    a.checkEqual("03. hasBase", testee.planet().hasBase(), true);

    int owner = 0;
    a.checkEqual("11. getOwner", testee.planet().getOwner().get(owner), true);
    a.checkEqual("12. owner", owner, 3);
}

/** Test operation on nonempty session, partially populated planet. */
AFL_TEST("game.proxy.FictiveStarbaseAdaptor:mixed", a)
{
    // Environment
    const int PLANET_ID = 7;
    NullTranslator tx;
    NullFileSystem fs;
    game::Session session(tx, fs);

    // Create game
    afl::base::Ptr<game::Game> g = new game::Game();
    g->setViewpointPlayer(3);
    session.setGame(g);

    // Create planet with some known properties
    game::map::Planet* pl = g->currentTurn().universe().planets().create(PLANET_ID);
    pl->setName("Saturn");
    pl->setNativeRace(game::HumanoidNatives);
    pl->setOwner(4);
    pl->setNumBuildings(game::FactoryBuilding, 3);

    // Verify
    game::proxy::FictiveStarbaseAdaptor testee(session, PLANET_ID);
    a.checkEqual("01. getName",         testee.planet().getName(tx), "Saturn");
    a.checkEqual("02. getId",           testee.planet().getId(), PLANET_ID);
    a.checkEqual("03. HullTech",        testee.planet().getBaseTechLevel(game::HullTech).orElse(0), 10);    /* from natives */
    a.checkEqual("04. BeamTech",        testee.planet().getBaseTechLevel(game::BeamTech).orElse(0), 1);
    a.checkEqual("05. getNatives",      testee.planet().getNatives().orElse(0), 100);
    a.checkEqual("06. Colonists",       testee.planet().getCargo(game::Element::Colonists).orElse(0), 100);
    a.checkEqual("07. hasBase",         testee.planet().hasBase(), true);
    a.checkEqual("08. MineBuilding",    testee.planet().getNumBuildings(game::MineBuilding).orElse(0), 10); /* default */
    a.checkEqual("09. FactoryBuilding", testee.planet().getNumBuildings(game::FactoryBuilding).orElse(0), 3); /* as configured */

    int owner = 0;
    a.checkEqual("11. getOwner", testee.planet().getOwner().get(owner), true);
    a.checkEqual("12. owner", owner, 4);
}

/** Test extra methods, for coverage. */
AFL_TEST("game.proxy.FictiveStarbaseAdaptor:extra", a)
{
    // Environment
    NullTranslator tx;
    NullFileSystem fs;
    game::Session session(tx, fs);

    // Verify
    game::proxy::FictiveStarbaseAdaptor testee(session, 99);
    a.checkEqual("01. session", &testee.session(), &session);
    AFL_CHECK_SUCCEEDS(a("02. cancelAllCloneOrders"), testee.cancelAllCloneOrders());
    AFL_CHECK_SUCCEEDS(a("03. notifyListeners"), testee.notifyListeners());

    String_t name;
    game::Id_t id;
    a.checkEqual("11. findShipCloningHere", testee.findShipCloningHere(id, name), false);
}

/** Test creation using factory method. */
AFL_TEST("game.proxy.FictiveStarbaseAdaptor:factory", a)
{
    // Environment
    NullTranslator tx;
    NullFileSystem fs;
    game::Session session(tx, fs);

    // Verify (same as testEmpty)
    game::proxy::FictiveStarbaseAdaptorFromSession factory(0);
    std::auto_ptr<game::proxy::StarbaseAdaptor> ad(factory.call(session));
    a.checkNonNull("01. result", ad.get());
    a.checkEqual("02. getName", ad->planet().getName(tx), "Magrathea");
}
