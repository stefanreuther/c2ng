/**
  *  \file u/t_game_proxy_buildammoproxy.cpp
  *  \brief Test for game::proxy::BuildAmmoProxy
  */

#include "game/proxy/buildammoproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "game/game.hpp"
#include "game/map/universe.hpp"
#include "game/map/ship.hpp"
#include "game/map/planet.hpp"
#include "game/turn.hpp"

// using game::spec::Cost;

namespace {
    const int PLAYER_NR = 4;
    const int PLANET_ID = 77;
    const int X = 1000;
    const int Y = 2000;

    /* Prepare session with
       - root
       - specification
       - one planet */
    void prepare(game::test::SessionThread& t)
    {
        // Create ship list
        afl::base::Ptr<game::spec::ShipList> shipList = new game::spec::ShipList();
        game::test::initPListBeams(*shipList);
        game::test::initPListTorpedoes(*shipList);
        game::test::addTranswarp(*shipList);
        game::test::addAnnihilation(*shipList);
        t.session().setShipList(shipList);

        // Create root
        afl::base::Ptr<game::Root> r = game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(3,0,0)), game::RegistrationKey::Unregistered, 10).asPtr();
        t.session().setRoot(r);

        // Create game with universe
        afl::base::Ptr<game::Game> g = new game::Game();
        game::map::Planet* p = g->currentTurn().universe().planets().create(PLANET_ID);
        game::map::PlanetData pd;
        pd.owner = PLAYER_NR;
        pd.colonistClans = 100;
        pd.money = 10000;
        pd.supplies = 5000;
        pd.minedTritanium = 2000;
        pd.minedDuranium = 3000;
        pd.minedMolybdenum = 4000;
        p->addCurrentPlanetData(pd, game::PlayerSet_t(PLAYER_NR));

        game::map::BaseData bd;
        for (int i = 1; i < 10; ++i) {
            // Set base storage
            bd.torpedoStorage.set(i, 0);
        }
        for (int i = 0; i < 4; ++i) {
            bd.techLevels[i] = 3;
        }
        p->addCurrentBaseData(bd, game::PlayerSet_t(PLAYER_NR));
        p->setPosition(game::map::Point(X, Y));
        p->setName("P");
        t.session().setGame(g);
        t.session().postprocessTurn(g->currentTurn(), game::PlayerSet_t(PLAYER_NR), game::PlayerSet_t(PLAYER_NR), game::map::Object::Playable);
    }

    /* Add ship to given session */
    void addShip(game::test::SessionThread& t, int x, int y, game::Id_t id, String_t friendlyCode, String_t name)
    {
        game::map::Ship* sh = t.session().getGame()->currentTurn().universe().ships().create(id);
        game::map::ShipData sd;
        sd.owner = PLAYER_NR;
        sd.friendlyCode = friendlyCode;
        sd.name = name;
        sd.x = x;
        sd.y = y;
        sd.hullType = game::test::ANNIHILATION_HULL_ID;
        sd.engineType = 9;
        sd.beamType = 0;
        sd.numBeams = 0;
        sd.torpedoType = 2;
        sd.numLaunchers = 1;
        sd.crew = 10;
        sd.ammo = 20;
        sh->addCurrentShipData(sd, game::PlayerSet_t(PLAYER_NR));
        sh->internalCheck(game::PlayerSet_t(PLAYER_NR), 15);
        sh->setPlayability(game::map::Object::Playable);
    }

    /* Receive updates from a proxy */
    class UpdateReceiver {
     public:
        const game::proxy::BuildAmmoProxy::Status& getResult() const
            { return m_result; }

        void onUpdate(const game::proxy::BuildAmmoProxy::Status& status)
            { m_result = status; }
     private:
        game::proxy::BuildAmmoProxy::Status m_result;
    };
}

/** Test behaviour on empty session. */
void
TestGameProxyBuildAmmoProxy::testEmpty()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::BuildAmmoProxy testee(t.gameSender(), ind, 99);

    game::proxy::BuildAmmoProxy::Status st;
    testee.getStatus(ind, st);

    TS_ASSERT_EQUALS(st.parts.size(), 0U);
    TS_ASSERT_EQUALS(st.cost.isZero(), true);
    TS_ASSERT_EQUALS(st.available.isZero(), true);
    TS_ASSERT_EQUALS(st.remaining.isZero(), true);
    TS_ASSERT_EQUALS(st.missing.isZero(), true);
}

/** Test behaviour for planet/planet build. */
void
TestGameProxyBuildAmmoProxy::testPlanet()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    game::proxy::BuildAmmoProxy testee(t.gameSender(), ind, PLANET_ID);
    testee.setPlanet();

    game::proxy::BuildAmmoProxy::Status st;
    testee.getStatus(ind, st);

    TS_ASSERT_EQUALS(st.parts.size(), 11U);
    TS_ASSERT_EQUALS(st.parts[1].type, game::Element::fromTorpedoType(2));
    TS_ASSERT_EQUALS(st.parts[1].page, game::spec::info::TorpedoPage);
    TS_ASSERT_EQUALS(st.parts[1].id, 2);
    TS_ASSERT_EQUALS(st.parts[1].name, "Fusion Bomb");
    TS_ASSERT_EQUALS(st.parts[1].techStatus, game::AvailableTech);
    TS_ASSERT_EQUALS(st.parts[1].isAccessible, true);
    TS_ASSERT_EQUALS(st.parts[1].techLevel, 3);
    TS_ASSERT_EQUALS(st.parts[1].amount, 0);
    TS_ASSERT_EQUALS(st.parts[1].maxAmount, 10000);
    TS_ASSERT_EQUALS(st.parts[1].cost.toCargoSpecString(), "1TDM 8$");

    TS_ASSERT_EQUALS(st.parts[9].type, game::Element::fromTorpedoType(10));
    TS_ASSERT_EQUALS(st.parts[9].page, game::spec::info::TorpedoPage);
    TS_ASSERT_EQUALS(st.parts[9].id, 10);
    TS_ASSERT_EQUALS(st.parts[9].name, "Selphyr-Fataro-Dev.");
    TS_ASSERT_EQUALS(st.parts[9].techStatus, game::BuyableTech);
    TS_ASSERT_EQUALS(st.parts[9].isAccessible, true);
    TS_ASSERT_EQUALS(st.parts[9].techLevel, 10);
    TS_ASSERT_EQUALS(st.parts[9].amount, 0);
    TS_ASSERT_EQUALS(st.parts[9].maxAmount, 10000);
    TS_ASSERT_EQUALS(st.parts[9].cost.toCargoSpecString(), "1TDM 80$");

    TS_ASSERT_EQUALS(st.parts[10].type, game::Element::Fighters);
    TS_ASSERT_EQUALS(st.parts[10].page, game::spec::info::FighterPage);
    TS_ASSERT_EQUALS(st.parts[10].id, PLAYER_NR);
    TS_ASSERT(st.parts[10].name.find("ighter") != String_t::npos); // matches if race name is included or not
    TS_ASSERT_EQUALS(st.parts[10].techStatus, game::AvailableTech);
    TS_ASSERT_EQUALS(st.parts[10].isAccessible, true);
    TS_ASSERT_EQUALS(st.parts[10].techLevel, 1);
    TS_ASSERT_EQUALS(st.parts[10].amount, 0);
    TS_ASSERT_EQUALS(st.parts[10].maxAmount, 60);
    TS_ASSERT_EQUALS(st.parts[10].cost.toCargoSpecString(), "3T 2M 100$");

    TS_ASSERT_EQUALS(st.cost.isZero(), true);
    TS_ASSERT_EQUALS(st.available.toCargoSpecString(), "2000T 3000D 4000M 5000S 10000$");
    TS_ASSERT_EQUALS(st.remaining.toCargoSpecString(), "2000T 3000D 4000M 5000S 10000$");
    TS_ASSERT_EQUALS(st.missing.isZero(), true);
    TS_ASSERT_EQUALS(st.targetName, "P");
    TS_ASSERT_EQUALS(st.availableTech, 3);

    // Buy something and wait for update
    UpdateReceiver recv;
    testee.sig_update.add(&recv, &UpdateReceiver::onUpdate);
    testee.addLimitCash(game::Element::fromTorpedoType(2), 3);
    t.sync();
    ind.processQueue();

    const game::proxy::BuildAmmoProxy::Status& rst = recv.getResult();
    TS_ASSERT_EQUALS(rst.parts.size(), 11U);
    TS_ASSERT_EQUALS(rst.parts[1].amount, 3);
    TS_ASSERT_EQUALS(rst.parts[1].maxAmount, 10000);
    TS_ASSERT_EQUALS(rst.cost.toCargoSpecString(), "3TDM 24$");
    TS_ASSERT_EQUALS(rst.remaining.toCargoSpecString(), "1997T 2997D 3997M 5000S 9976$");
    TS_ASSERT_EQUALS(rst.targetName, "P");

    // Commit
    testee.commit();
    t.sync();
    ind.processQueue();

    // Verify
    TS_ASSERT_EQUALS(t.session().getGame()->currentTurn().universe().planets().get(PLANET_ID)->getCargo(game::Element::fromTorpedoType(2)).orElse(0), 3);
}

/** Test behaviour for planet/ship build. */
void
TestGameProxyBuildAmmoProxy::testShip()
{
    const int SHIP_ID = 456;
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    addShip(t, X, Y, SHIP_ID, "hi", "USS Nerf");
    game::proxy::BuildAmmoProxy testee(t.gameSender(), ind, PLANET_ID);
    testee.setShip(SHIP_ID);

    game::proxy::BuildAmmoProxy::Status st;
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.parts.size(), 11U);
    TS_ASSERT_EQUALS(st.parts[0].isAccessible, false);
    TS_ASSERT_EQUALS(st.parts[1].isAccessible, true);
    TS_ASSERT_EQUALS(st.parts[2].isAccessible, false);
    TS_ASSERT_EQUALS(st.parts[10].isAccessible, false);
    TS_ASSERT_EQUALS(st.targetName, "USS Nerf");
    TS_ASSERT_EQUALS(st.availableTech, 3);

    // Buy something and wait for update
    UpdateReceiver recv;
    testee.sig_update.add(&recv, &UpdateReceiver::onUpdate);
    testee.addLimitCash(game::Element::fromTorpedoType(2), 3);
    t.sync();
    ind.processQueue();

    const game::proxy::BuildAmmoProxy::Status& rst = recv.getResult();
    TS_ASSERT_EQUALS(rst.parts.size(), 11U);
    TS_ASSERT_EQUALS(rst.parts[1].amount, 23);
    TS_ASSERT_EQUALS(rst.parts[1].maxAmount, 320);
    TS_ASSERT_EQUALS(rst.cost.toCargoSpecString(), "3TDM 24$");
    TS_ASSERT_EQUALS(rst.remaining.toCargoSpecString(), "1997T 2997D 3997M 5000S 9976$");
    TS_ASSERT_EQUALS(rst.targetName, "USS Nerf");

    // Commit
    testee.commit();
    t.sync();
    ind.processQueue();

    // Verify
    TS_ASSERT_EQUALS(t.session().getGame()->currentTurn().universe().ships().get(SHIP_ID)->getCargo(game::Element::fromTorpedoType(2)).orElse(0), 23);
}

/** Test behaviour for planet/ship build, wrong ship. */
void
TestGameProxyBuildAmmoProxy::testFarShip()
{
    const int SHIP_ID = 456;
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    addShip(t, X+10, Y, SHIP_ID, "hi", "USS Nerf");      // note differing position
    game::proxy::BuildAmmoProxy testee(t.gameSender(), ind, PLANET_ID);
    testee.setShip(SHIP_ID);

    game::proxy::BuildAmmoProxy::Status st;
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.parts.size(), 11U);
    TS_ASSERT_EQUALS(st.parts[0].isAccessible, false);
    TS_ASSERT_EQUALS(st.parts[1].isAccessible, false);
    TS_ASSERT_EQUALS(st.parts[2].isAccessible, false);
    TS_ASSERT_EQUALS(st.parts[10].isAccessible, false);
}

