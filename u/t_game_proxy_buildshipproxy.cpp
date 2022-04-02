/**
  *  \file u/t_game_proxy_buildshipproxy.cpp
  *  \brief Test for game::proxy::BuildShipProxy
  */

#include "game/proxy/buildshipproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/test/waitindicator.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/shiplist.hpp"
#include "game/game.hpp"
#include "game/turn.hpp"
#include "game/map/planet.hpp"
#include "game/map/planetdata.hpp"
#include "game/map/basedata.hpp"
#include "game/test/root.hpp"

using game::spec::Cost;

namespace {
    const int PLAYER_NR = 4;
    const int PLANET_ID = 77;
    const int HULL_INDEX = 3;
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
        game::test::addOutrider(*shipList);
        shipList->hullAssignments().add(PLAYER_NR, HULL_INDEX, game::test::ANNIHILATION_HULL_ID);
        t.session().setShipList(shipList);

        // Create root
        afl::base::Ptr<game::Root> r = new game::test::Root(game::HostVersion(game::HostVersion::PHost, MKVERSION(3,0,0)), game::RegistrationKey::Unregistered, 10);
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
            // Set base storage with variable amounts derived from slot number
            bd.engineStorage.set(i, i&1);
            bd.hullStorage.set(i, i&2);
            bd.beamStorage.set(i, i&3);
            bd.launcherStorage.set(i, i&4);
        }
        for (int i = 0; i < 4; ++i) {
            bd.techLevels[i] = 3;
        }
        bd.owner = PLAYER_NR;
        p->addCurrentBaseData(bd, game::PlayerSet_t(PLAYER_NR));
        p->setPosition(game::map::Point(X, Y));
        p->setName("P");
        g->currentTurn().universe().postprocess(game::PlayerSet_t(PLAYER_NR), game::PlayerSet_t(PLAYER_NR), game::map::Object::Playable,
                                                r->hostVersion(), r->hostConfiguration(), 12, *shipList, t.session().translator(), t.session().log());
        t.session().setGame(g);
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
        sd.hullType = game::test::OUTRIDER_HULL_ID;
        sd.engineType = 9;
        sd.beamType = 0;
        sd.numBeams = 0;
        sd.launcherType = 0;
        sd.numLaunchers = 0;
        sd.crew = 10;
        sh->addCurrentShipData(sd, game::PlayerSet_t(PLAYER_NR));
        sh->internalCheck();
        sh->setPlayability(game::map::Object::ReadOnly);
    }

    /* Add build order to planet in session */
    void addBuildOrder(game::test::SessionThread& t)
    {
        game::map::Planet* p = t.session().getGame()->currentTurn().universe().planets().get(PLANET_ID);
        p->setBaseStorage(game::HullTech, HULL_INDEX, 1);
        p->setBaseStorage(game::EngineTech, 9, 10);

        game::ShipBuildOrder order;
        order.setHullIndex(HULL_INDEX);
        order.setEngineType(9);
        p->setBaseBuildOrder(order);
    }

    /* Receive updates from a proxy */
    class UpdateReceiver {
     public:
        const game::proxy::BuildShipProxy::Status& getResult() const
            { return m_result; }

        void onUpdate(const game::proxy::BuildShipProxy::Status& status)
            { m_result = status; }
     private:
        game::proxy::BuildShipProxy::Status m_result;
    };
}

/** Test behaviour on empty session.
    A: create BuildShipProxy on empty session
    E: requests must produce empty results */
void
TestGameProxyBuildShipProxy::testEmpty()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::BuildShipProxy testee(t.gameSender(), ind, 99);

    // Get current status -> returns unsuccessful, zero
    game::proxy::BuildShipProxy::Status st;
    testee.getStatus(ind, st);

    TS_ASSERT_DIFFERS(st.status, game::actions::BuildShip::Success);
    TS_ASSERT(st.totalCost.isZero());
    TS_ASSERT(st.partCost.isZero());
    TS_ASSERT(st.available.isZero());
    TS_ASSERT(st.remaining.isZero());
    TS_ASSERT(st.missing.isZero());
    TS_ASSERT_EQUALS(st.partTech, 0);
    TS_ASSERT_EQUALS(st.availableTech, 0);
    TS_ASSERT_EQUALS(st.order.getHullIndex(), 0);
    TS_ASSERT_EQUALS(st.description.size(), 0U);
    TS_ASSERT_EQUALS(st.numEngines, 0);
    TS_ASSERT_EQUALS(st.maxBeams, 0);
    TS_ASSERT_EQUALS(st.maxLaunchers, 0);
    TS_ASSERT_EQUALS(st.isNew, false);
    TS_ASSERT_EQUALS(st.isUsePartsFromStorage, false);
    TS_ASSERT_EQUALS(st.isChange, false);

    // Look for cloning ship
    game::Id_t id;
    String_t name;
    TS_ASSERT_EQUALS(testee.findShipCloningHere(ind, id, name), false);
}

/** Test normal behaviour.
    A: create BuildShipProxy on session with a planet. Exercise modification calls.
    E: verify result */
void
TestGameProxyBuildShipProxy::testNormal()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    game::proxy::BuildShipProxy testee(t.gameSender(), ind, PLANET_ID);

    // Get current status
    game::proxy::BuildShipProxy::Status st;
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.status, game::actions::BuildShip::Success);
    TS_ASSERT_EQUALS(st.available.get(Cost::Tritanium), 2000);
    TS_ASSERT_EQUALS(st.available.get(Cost::Duranium), 3000);
    TS_ASSERT_EQUALS(st.available.get(Cost::Molybdenum), 4000);
    TS_ASSERT_EQUALS(st.available.get(Cost::Supplies), 5000);
    TS_ASSERT_EQUALS(st.available.get(Cost::Money), 10000);
    TS_ASSERT(st.missing.isZero());
    TS_ASSERT_EQUALS(st.order.getHullIndex(), game::test::ANNIHILATION_HULL_ID);
    TS_ASSERT_EQUALS(st.description.size(), 4U);
    TS_ASSERT_EQUALS(st.numEngines, 6);
    TS_ASSERT_EQUALS(st.maxBeams, 10);
    TS_ASSERT_EQUALS(st.maxLaunchers, 10);
    TS_ASSERT_EQUALS(st.isNew, true);
    TS_ASSERT_EQUALS(st.isUsePartsFromStorage, false);
    TS_ASSERT_EQUALS(st.isChange, false);

    // Look for cloning ship
    game::Id_t id;
    String_t name;
    TS_ASSERT_EQUALS(testee.findShipCloningHere(ind, id, name), false);

    // Listen for updates
    UpdateReceiver recv;
    testee.sig_change.add(&recv, &UpdateReceiver::onUpdate);

    // Modify
    testee.selectPart(game::BeamTech, 1);
    testee.setNumParts(game::actions::BuildShip::BeamWeapon, 3);
    testee.setPart(game::BeamTech, 4);
    testee.addParts(game::actions::BuildShip::TorpedoWeapon, -2);

    t.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(recv.getResult().status, game::actions::BuildShip::Success);
    TS_ASSERT_EQUALS(recv.getResult().order.getBeamType(), 4);
    TS_ASSERT_EQUALS(recv.getResult().order.getNumBeams(), 3);
    TS_ASSERT_EQUALS(recv.getResult().order.getNumLaunchers(), 8);
    TS_ASSERT_EQUALS(recv.getResult().partTech, 1);
    TS_ASSERT_EQUALS(recv.getResult().partCost.get(Cost::Tritanium), 1);
    TS_ASSERT_EQUALS(recv.getResult().isChange, false);                    // Not a change: there is no pre-existing order

    // Verify details
    game::spec::CostSummary sum;
    testee.getCostSummary(ind, sum);
    TS_ASSERT_EQUALS(sum.getNumItems(), 7U);
    TS_ASSERT_EQUALS(sum.get(0)->name, "Hull tech upgrade");
    TS_ASSERT_EQUALS(sum.get(1)->name, "ANNIHILATION CLASS BATTLESHIP");

    game::ShipQuery q = testee.getQuery(ind);
    TS_ASSERT_EQUALS(q.getHullType(), game::test::ANNIHILATION_HULL_ID);
    TS_ASSERT_EQUALS(q.getOwner(), PLAYER_NR);

    // Commit; verify that order is executed
    testee.commit();

    t.sync();
    ind.processQueue();

    const game::map::Planet* p = t.session().getGame()->currentTurn().universe().planets().get(PLANET_ID);
    TS_ASSERT_EQUALS(p->getBaseBuildOrder().getHullIndex(), HULL_INDEX);
    TS_ASSERT_EQUALS(p->getBaseBuildOrder().getNumLaunchers(), 8);
}

/** Test normal behaviour, setBuildOrder().
    A: create BuildShipProxy on session with a planet. Use setBuildOrder().
    E: verify result */
void
TestGameProxyBuildShipProxy::testSetBuildOrder()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    game::proxy::BuildShipProxy testee(t.gameSender(), ind, PLANET_ID);

    // Listen for updates
    UpdateReceiver recv;
    testee.sig_change.add(&recv, &UpdateReceiver::onUpdate);

    // Modify
    game::ShipBuildOrder o;
    o.setHullIndex(game::test::ANNIHILATION_HULL_ID);
    o.setEngineType(9);
    o.setBeamType(8);
    o.setNumBeams(2);
    o.setLauncherType(10);
    o.setNumLaunchers(7);
    testee.setBuildOrder(o);

    t.sync();
    ind.processQueue();

    TS_ASSERT_EQUALS(recv.getResult().order.getBeamType(), 8);
    TS_ASSERT_EQUALS(recv.getResult().order.getNumBeams(), 2);
    TS_ASSERT_EQUALS(recv.getResult().order.getNumLaunchers(), 7);
    TS_ASSERT_EQUALS(recv.getResult().order.getLauncherType(), 10);
    TS_ASSERT_EQUALS(recv.getResult().order.getEngineType(), 9);
}

/** Test normal behaviour, pre-existing build order.
    A: create BuildShipProxy on session with a planet and a pre-existing build order. Exercise modification calls including cancel().
    E: verify result */
void
TestGameProxyBuildShipProxy::testPreexisting()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    addBuildOrder(t);
    game::proxy::BuildShipProxy testee(t.gameSender(), ind, PLANET_ID);

    // Get current status, cost is zero
    game::proxy::BuildShipProxy::Status st;
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.status, game::actions::BuildShip::Success);
    TS_ASSERT_EQUALS(st.isNew, false);
    TS_ASSERT_EQUALS(st.isUsePartsFromStorage, true);
    TS_ASSERT_EQUALS(st.totalCost.isZero(), true);
    TS_ASSERT_EQUALS(st.isChange, false);
    TS_ASSERT_EQUALS(st.isUsePartsFromStorage, true);

    // Modification is reported
    testee.addParts(game::actions::BuildShip::BeamWeapon, 3);
    testee.setUsePartsFromStorage(false);
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.isChange, true);
    TS_ASSERT_EQUALS(st.isUsePartsFromStorage, false);

    // Cancel
    testee.cancel();
    t.sync();
    ind.processQueue();

    const game::map::Planet* p = t.session().getGame()->currentTurn().universe().planets().get(PLANET_ID);
    TS_ASSERT_EQUALS(p->getBaseBuildOrder().getHullIndex(), 0);
}

/** Test clone interface.
    A: create BuildShipProxy on session with a base and some ships.
    E: verify correct results of findShipCloningHere(), cancelAllCloneOrders(). */
void
TestGameProxyBuildShipProxy::testClone()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    addShip(t, X, Y, 100, "xyz", "one");
    addShip(t, X+1, Y, 150, "cln", "half");
    addShip(t, X, Y, 200, "cln", "two");
    addShip(t, X, Y, 300, "abc", "three");
    addShip(t, X, Y, 400, "cln", "four");
    game::proxy::BuildShipProxy testee(t.gameSender(), ind, PLANET_ID);

    // Look for cloning ship; must return first applicable
    game::Id_t id;
    String_t name;
    TS_ASSERT_EQUALS(testee.findShipCloningHere(ind, id, name), true);
    TS_ASSERT_EQUALS(id, 200);
    TS_ASSERT_EQUALS(name, "two");

    // Clear clone orders; must cancel all 'cln' codes
    testee.cancelAllCloneOrders();
    t.sync();
    ind.processQueue();

    game::map::Universe& univ = t.session().getGame()->currentTurn().universe();
    TS_ASSERT_EQUALS(univ.ships().get(100)->getFriendlyCode().orElse(""), "xyz");
    TS_ASSERT_EQUALS(univ.ships().get(150)->getFriendlyCode().orElse(""), "cln");
    TS_ASSERT_DIFFERS(univ.ships().get(200)->getFriendlyCode().orElse(""), "cln");
    TS_ASSERT_EQUALS(univ.ships().get(300)->getFriendlyCode().orElse(""), "abc");
    TS_ASSERT_DIFFERS(univ.ships().get(400)->getFriendlyCode().orElse(""), "cln");
}

/** Test custom StarbaseAdaptor.
    A: create session. Create custom adaptor with custom findShipCloningHere() method.
    E: proxy findShipCloningHere() returns expected values */
void
TestGameProxyBuildShipProxy::testCustom()
{
    // Adaptor implementation for testing
    class Adaptor : public game::proxy::StarbaseAdaptor {
     public:
        Adaptor(game::Session& session)
            : m_session(session), m_planet(111)
            {
                // Prepare planet with bare minimum
                // - planet
                game::map::PlanetData pd;
                pd.owner = PLAYER_NR;
                m_planet.addCurrentPlanetData(pd, game::PlayerSet_t(PLAYER_NR));

                // - base
                game::map::BaseData bd;
                bd.owner = PLAYER_NR;
                m_planet.addCurrentBaseData(bd, game::PlayerSet_t(PLAYER_NR));

                // - position
                m_planet.setPosition(game::map::Point(X, Y));

                // - internal metadata
                game::map::Configuration config;
                m_planet.internalCheck(config, session.translator(), session.log());
                m_planet.setPlayability(game::map::Object::Playable);
            }
        virtual game::map::Planet& planet()
            { return m_planet; }
        virtual game::Session& session()
            { return m_session; }
        virtual bool findShipCloningHere(game::Id_t& id, String_t& name)
            {
                id = 444;
                name = "dolly";
                return true;
            }
        virtual void cancelAllCloneOrders()
            { }
        virtual void notifyListeners()
            { }
     private:
        game::Session& m_session;
        game::map::Planet m_planet;
    };

    // Adaptor-adaptor
    class Maker : public afl::base::Closure<game::proxy::StarbaseAdaptor*(game::Session&)> {
     public:
        virtual Adaptor* call(game::Session& session)
            { return new Adaptor(session); }
    };

    // Setup
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    game::proxy::BuildShipProxy testee(t.gameSender().makeTemporary(new Maker()), ind);

    // Look for cloning ship; must return predefined value
    game::Id_t id;
    String_t name;
    TS_ASSERT_EQUALS(testee.findShipCloningHere(ind, id, name), true);
    TS_ASSERT_EQUALS(id, 444);
    TS_ASSERT_EQUALS(name, "dolly");
}

