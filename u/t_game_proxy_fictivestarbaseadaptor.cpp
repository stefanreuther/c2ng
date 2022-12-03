/**
  *  \file u/t_game_proxy_fictivestarbaseadaptor.cpp
  *  \brief Test for game::proxy::FictiveStarbaseAdaptor
  */

#include "game/proxy/fictivestarbaseadaptor.hpp"

#include "t_game_proxy.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/turn.hpp"

using afl::string::NullTranslator;
using afl::io::NullFileSystem;

/** Test operation on empty session: object can correctly be constructed. */
void
TestGameProxyFictiveStarbaseAdaptor::testEmpty()
{
    // Environment
    NullTranslator tx;
    NullFileSystem fs;
    game::Session session(tx, fs);

    // Verify
    game::proxy::FictiveStarbaseAdaptor testee(session, 99);
    TS_ASSERT_EQUALS(testee.planet().getName(tx), "Magrathea");
    TS_ASSERT_EQUALS(testee.planet().getId(), 99);
    TS_ASSERT_EQUALS(testee.planet().hasBase(), true);
    TS_ASSERT_EQUALS(testee.planet().getCargo(game::Element::Tritanium).orElse(-1), 1000);
    TS_ASSERT_EQUALS(testee.planet().getBaseTechLevel(game::HullTech).orElse(-1), 1);
}

/** Test operation on nonempty session, Id zero: object can correctly be constructed. */
void
TestGameProxyFictiveStarbaseAdaptor::testZero()
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
    TS_ASSERT_EQUALS(testee.planet().getName(tx), "Magrathea");
    TS_ASSERT_EQUALS(testee.planet().getId(), 42);      /* invented Id */
    TS_ASSERT_EQUALS(testee.planet().hasBase(), true);

    int owner = 0;
    TS_ASSERT_EQUALS(testee.planet().getOwner().get(owner), true);
    TS_ASSERT_EQUALS(owner, 3);
}

/** Test operation on nonempty session, partially populated planet. */
void
TestGameProxyFictiveStarbaseAdaptor::testMixed()
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
    TS_ASSERT_EQUALS(testee.planet().getName(tx), "Saturn");
    TS_ASSERT_EQUALS(testee.planet().getId(), PLANET_ID);
    TS_ASSERT_EQUALS(testee.planet().getBaseTechLevel(game::HullTech).orElse(0), 10);    /* from natives */
    TS_ASSERT_EQUALS(testee.planet().getBaseTechLevel(game::BeamTech).orElse(0), 1);
    TS_ASSERT_EQUALS(testee.planet().getNatives().orElse(0), 100);
    TS_ASSERT_EQUALS(testee.planet().getCargo(game::Element::Colonists).orElse(0), 100);
    TS_ASSERT_EQUALS(testee.planet().hasBase(), true);
    TS_ASSERT_EQUALS(testee.planet().getNumBuildings(game::MineBuilding).orElse(0), 10); /* default */
    TS_ASSERT_EQUALS(testee.planet().getNumBuildings(game::FactoryBuilding).orElse(0), 3); /* as configured */

    int owner = 0;
    TS_ASSERT_EQUALS(testee.planet().getOwner().get(owner), true);
    TS_ASSERT_EQUALS(owner, 4);
}

/** Test extra methods, for coverage. */
void
TestGameProxyFictiveStarbaseAdaptor::testExtra()
{
    // Environment
    NullTranslator tx;
    NullFileSystem fs;
    game::Session session(tx, fs);

    // Verify
    game::proxy::FictiveStarbaseAdaptor testee(session, 99);
    TS_ASSERT_EQUALS(&testee.session(), &session);
    TS_ASSERT_THROWS_NOTHING(testee.cancelAllCloneOrders());
    TS_ASSERT_THROWS_NOTHING(testee.notifyListeners());

    String_t name;
    game::Id_t id;
    TS_ASSERT_EQUALS(testee.findShipCloningHere(id, name), false);

}

/** Test creation using factory method. */
void
TestGameProxyFictiveStarbaseAdaptor::testFactory()
{
    // Environment
    NullTranslator tx;
    NullFileSystem fs;
    game::Session session(tx, fs);

    // Verify (same as testEmpty)
    game::proxy::FictiveStarbaseAdaptorFromSession factory(0);
    std::auto_ptr<game::proxy::StarbaseAdaptor> ad(factory.call(session));
    TS_ASSERT(ad.get() != 0);
    TS_ASSERT_EQUALS(ad->planet().getName(tx), "Magrathea");
}

