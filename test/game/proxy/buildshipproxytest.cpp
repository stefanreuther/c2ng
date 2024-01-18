/**
  *  \file test/game/proxy/buildshipproxytest.cpp
  *  \brief Test for game::proxy::BuildShipProxy
  */

#include "game/proxy/buildshipproxy.hpp"

#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/basedata.hpp"
#include "game/map/planet.hpp"
#include "game/map/planetdata.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/shiplist.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"

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
            // Set base storage with variable amounts derived from slot number
            bd.engineStorage.set(i, i&1);
            bd.hullStorage.set(i, i&2);
            bd.beamStorage.set(i, i&3);
            bd.launcherStorage.set(i, i&4);
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
        sd.hullType = game::test::OUTRIDER_HULL_ID;
        sd.engineType = 9;
        sd.beamType = 0;
        sd.numBeams = 0;
        sd.torpedoType = 0;
        sd.numLaunchers = 0;
        sd.crew = 10;
        sh->addCurrentShipData(sd, game::PlayerSet_t(PLAYER_NR));
        sh->internalCheck(game::PlayerSet_t(PLAYER_NR), 15);
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
AFL_TEST("game.proxy.BuildShipProxy:empty", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::BuildShipProxy testee(t.gameSender(), ind, 99);

    // Get current status -> returns unsuccessful, zero
    game::proxy::BuildShipProxy::Status st;
    testee.getStatus(ind, st);

    a.checkDifferent("01. status", st.status, game::actions::BuildShip::Success);
    a.check("02. totalCost", st.totalCost.isZero());
    a.check("03. partCost", st.partCost.isZero());
    a.check("04. available", st.available.isZero());
    a.check("05. remaining", st.remaining.isZero());
    a.check("06. missing", st.missing.isZero());
    a.checkEqual("07. partTech", st.partTech, 0);
    a.checkEqual("08. availableTech", st.availableTech, 0);
    a.checkEqual("09. order", st.order.getHullIndex(), 0);
    a.checkEqual("10. description", st.description.size(), 0U);
    a.checkEqual("11. numEngines", st.numEngines, 0);
    a.checkEqual("12. maxBeams", st.maxBeams, 0);
    a.checkEqual("13. maxLaunchers", st.maxLaunchers, 0);
    a.checkEqual("14. isNew", st.isNew, false);
    a.checkEqual("15. isUsePartsFromStorage", st.isUsePartsFromStorage, false);
    a.checkEqual("16. isUseTechUpgrade", st.isUseTechUpgrade, false);
    a.checkEqual("17. isChange", st.isChange, false);

    // Look for cloning ship
    game::Id_t id;
    String_t name;
    a.checkEqual("21. findShipCloningHere", testee.findShipCloningHere(ind, id, name), false);
}

/** Test normal behaviour.
    A: create BuildShipProxy on session with a planet. Exercise modification calls.
    E: verify result */
AFL_TEST("game.proxy.BuildShipProxy:normal", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    game::proxy::BuildShipProxy testee(t.gameSender(), ind, PLANET_ID);

    // Get current status
    game::proxy::BuildShipProxy::Status st;
    testee.getStatus(ind, st);
    a.checkEqual("01. status",                st.status, game::actions::BuildShip::Success);
    a.checkEqual("02. available",             st.available.get(Cost::Tritanium), 2000);
    a.checkEqual("03. available",             st.available.get(Cost::Duranium), 3000);
    a.checkEqual("04. available",             st.available.get(Cost::Molybdenum), 4000);
    a.checkEqual("05. available",             st.available.get(Cost::Supplies), 5000);
    a.checkEqual("06. available",             st.available.get(Cost::Money), 10000);
    a.check("07. missing",                    st.missing.isZero());
    a.checkEqual("08. getHullIndex",          st.order.getHullIndex(), game::test::ANNIHILATION_HULL_ID);
    a.checkEqual("09. description",           st.description.size(), 4U);
    a.checkEqual("10. numEngines",            st.numEngines, 6);
    a.checkEqual("11. maxBeams",              st.maxBeams, 10);
    a.checkEqual("12. maxLaunchers",          st.maxLaunchers, 10);
    a.checkEqual("13. isNew",                 st.isNew, true);
    a.checkEqual("14. isUsePartsFromStorage", st.isUsePartsFromStorage, false);
    a.checkEqual("15. isUseTechUpgrade",      st.isUseTechUpgrade, true);
    a.checkEqual("16. isChange",              st.isChange, false);

    // Look for cloning ship
    game::Id_t id;
    String_t name;
    a.checkEqual("21. findShipCloningHere", testee.findShipCloningHere(ind, id, name), false);

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

    a.checkEqual("31. status",          recv.getResult().status, game::actions::BuildShip::Success);
    a.checkEqual("32. getBeamType",     recv.getResult().order.getBeamType(), 4);
    a.checkEqual("33. getNumBeams",     recv.getResult().order.getNumBeams(), 3);
    a.checkEqual("34. getNumLaunchers", recv.getResult().order.getNumLaunchers(), 8);
    a.checkEqual("35. partTech",        recv.getResult().partTech, 1);
    a.checkEqual("36. Tritanium",       recv.getResult().partCost.get(Cost::Tritanium), 1);
    a.checkEqual("37. isChange",        recv.getResult().isChange, false);                    // Not a change: there is no pre-existing order

    // Verify details
    game::spec::CostSummary sum;
    testee.getCostSummary(ind, sum);
    a.checkEqual("41. getNumItems", sum.getNumItems(), 7U);
    a.checkEqual("42. name 0",      sum.get(0)->name, "Hull tech upgrade");
    a.checkEqual("43. name 1",      sum.get(1)->name, "ANNIHILATION CLASS BATTLESHIP");

    game::ShipQuery q = testee.getQuery(ind);
    a.checkEqual("51. getHullType", q.getHullType(), game::test::ANNIHILATION_HULL_ID);
    a.checkEqual("52. getOwner",    q.getOwner(), PLAYER_NR);

    String_t cmd = testee.toScriptCommand(ind, "Build");
    a.checkEqual("61. toScriptCommand", cmd, "Build 53, 9, 4, 3, 2, 8   % ANNIHILATION CLASS BATTLESHIP");

    // Commit; verify that order is executed
    testee.commit();

    t.sync();
    ind.processQueue();

    const game::map::Planet* p = t.session().getGame()->currentTurn().universe().planets().get(PLANET_ID);
    a.checkEqual("71. getHullIndex",    p->getBaseBuildOrder().getHullIndex(), HULL_INDEX);
    a.checkEqual("72. getNumLaunchers", p->getBaseBuildOrder().getNumLaunchers(), 8);
}

/** Test normal behaviour, setBuildOrder().
    A: create BuildShipProxy on session with a planet. Use setBuildOrder().
    E: verify result */
AFL_TEST("game.proxy.BuildShipProxy:setBuildOrder", a)
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
    o.setTorpedoType(10);
    o.setNumLaunchers(7);
    testee.setBuildOrder(o);

    t.sync();
    ind.processQueue();

    a.checkEqual("01. getBeamType",     recv.getResult().order.getBeamType(), 8);
    a.checkEqual("02. getNumBeams",     recv.getResult().order.getNumBeams(), 2);
    a.checkEqual("03. getNumLaunchers", recv.getResult().order.getNumLaunchers(), 7);
    a.checkEqual("04. getTorpedoType",  recv.getResult().order.getTorpedoType(), 10);
    a.checkEqual("05. getEngineType",   recv.getResult().order.getEngineType(), 9);
}

/** Test normal behaviour, pre-existing build order.
    A: create BuildShipProxy on session with a planet and a pre-existing build order. Exercise modification calls including cancel().
    E: verify result */
AFL_TEST("game.proxy.BuildShipProxy:preexisting", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    addBuildOrder(t);
    game::proxy::BuildShipProxy testee(t.gameSender(), ind, PLANET_ID);

    // Get current status, cost is zero
    game::proxy::BuildShipProxy::Status st;
    testee.getStatus(ind, st);
    a.checkEqual("01. status",                st.status, game::actions::BuildShip::Success);
    a.checkEqual("02. isNew",                 st.isNew, false);
    a.checkEqual("03. isUsePartsFromStorage", st.isUsePartsFromStorage, true);
    a.checkEqual("04. totalCost",             st.totalCost.isZero(), true);
    a.checkEqual("05. isChange",              st.isChange, false);
    a.checkEqual("06. isUsePartsFromStorage", st.isUsePartsFromStorage, true);
    a.checkEqual("07. isUseTechUpgrade",      st.isUseTechUpgrade, true);

    // Modification is reported
    testee.addParts(game::actions::BuildShip::BeamWeapon, 3);
    testee.setUsePartsFromStorage(false);
    testee.getStatus(ind, st);
    a.checkEqual("11. isChange",              st.isChange, true);
    a.checkEqual("12. isUsePartsFromStorage", st.isUsePartsFromStorage, false);

    // Cancel
    testee.cancel();
    t.sync();
    ind.processQueue();

    const game::map::Planet* p = t.session().getGame()->currentTurn().universe().planets().get(PLANET_ID);
    a.checkEqual("21. getHullIndex", p->getBaseBuildOrder().getHullIndex(), 0);
}

/** Test clone interface.
    A: create BuildShipProxy on session with a base and some ships.
    E: verify correct results of findShipCloningHere(), cancelAllCloneOrders(). */
AFL_TEST("game.proxy.BuildShipProxy:cloning", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    addShip(t, X,   Y, 100, "xyz", "one");
    addShip(t, X+1, Y, 150, "cln", "half");
    addShip(t, X,   Y, 200, "cln", "two");
    addShip(t, X,   Y, 300, "abc", "three");
    addShip(t, X,   Y, 400, "cln", "four");
    game::proxy::BuildShipProxy testee(t.gameSender(), ind, PLANET_ID);

    // Look for cloning ship; must return first applicable
    game::Id_t id;
    String_t name;
    a.checkEqual("01. findShipCloningHere", testee.findShipCloningHere(ind, id, name), true);
    a.checkEqual("02. id", id, 200);
    a.checkEqual("03. name", name, "two");

    // Clear clone orders; must cancel all 'cln' codes
    testee.cancelAllCloneOrders();
    t.sync();
    ind.processQueue();

    game::map::Universe& univ = t.session().getGame()->currentTurn().universe();
    a.checkEqual    ("11. getFriendlyCode", univ.ships().get(100)->getFriendlyCode().orElse(""), "xyz");
    a.checkEqual    ("12. getFriendlyCode", univ.ships().get(150)->getFriendlyCode().orElse(""), "cln");
    a.checkDifferent("13. getFriendlyCode", univ.ships().get(200)->getFriendlyCode().orElse(""), "cln");
    a.checkEqual    ("14. getFriendlyCode", univ.ships().get(300)->getFriendlyCode().orElse(""), "abc");
    a.checkDifferent("15. getFriendlyCode", univ.ships().get(400)->getFriendlyCode().orElse(""), "cln");
}

/** Test custom StarbaseAdaptor.
    A: create session. Create custom adaptor with custom findShipCloningHere() method.
    E: proxy findShipCloningHere() returns expected values */
AFL_TEST("game.proxy.BuildShipProxy:custom-adaptor", a)
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
                m_planet.addCurrentBaseData(bd, game::PlayerSet_t(PLAYER_NR));

                // - position
                m_planet.setPosition(game::map::Point(X, Y));

                // - internal metadata
                game::map::Configuration config;
                m_planet.internalCheck(config, game::PlayerSet_t(PLAYER_NR), 15, session.translator(), session.log());
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
    a.checkEqual("01. findShipCloningHere", testee.findShipCloningHere(ind, id, name), true);
    a.checkEqual("02. id", id, 444);
    a.checkEqual("03. name", name, "dolly");
}

/** Test normal behaviour.
    A: create BuildShipProxy on session with a planet. Exercise modification calls; disable setUseTechUpgrade()
    E: verify result */
AFL_TEST("game.proxy.BuildShipProxy:setUseTechUpgrade", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    game::proxy::BuildShipProxy testee(t.gameSender(), ind, PLANET_ID);

    // Listen for updates
    UpdateReceiver recv;
    testee.sig_change.add(&recv, &UpdateReceiver::onUpdate);

    // Modify (same sequence as testNormal())
    testee.selectPart(game::BeamTech, 1);
    testee.setNumParts(game::actions::BuildShip::BeamWeapon, 3);
    testee.setPart(game::BeamTech, 4);
    testee.addParts(game::actions::BuildShip::TorpedoWeapon, -2);
    testee.setUseTechUpgrade(false);

    t.sync();
    ind.processQueue();

    a.checkEqual("01. status", recv.getResult().status, game::actions::BuildShip::DisabledTech);

    // Verify details
    game::spec::CostSummary sum;
    testee.getCostSummary(ind, sum);
    a.checkEqual("11. getNumItems", sum.getNumItems(), 4U);
    a.checkEqual("12. name", sum.get(0)->name, "ANNIHILATION CLASS BATTLESHIP");
}
