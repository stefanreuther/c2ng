/**
  *  \file test/game/proxy/cloneshipproxytest.cpp
  *  \brief Test for game::proxy::CloneShipProxy
  */

#include "game/proxy/cloneshipproxy.hpp"

#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/planetdata.hpp"
#include "game/map/ship.hpp"
#include "game/map/shipdata.hpp"
#include "game/map/universe.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/shiplist.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"

using game::PlayerSet_t;
using game::HostVersion;

namespace {
    const int PLAYER_NR = 3;
    const int PLANET_ID = 200;
    const int SHIP_ID = 300;

    void prepare(game::test::SessionThread& t)
    {
        // ShipList: needs to exist but can be empty
        afl::base::Ptr<game::spec::ShipList> shipList(new game::spec::ShipList());
        game::test::initStandardBeams(*shipList);
        game::test::initStandardTorpedoes(*shipList);
        game::test::addOutrider(*shipList);
        game::test::addNovaDrive(*shipList);
        t.session().setShipList(shipList);

        // Root
        afl::base::Ptr<game::Root> r(game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(4,0,0)), game::RegistrationKey::Registered, 10).asPtr());
        t.session().setRoot(r);

        // Game
        afl::base::Ptr<game::Game> g(new game::Game());

        // - planet
        game::map::Planet* p = g->currentTurn().universe().planets().create(PLANET_ID);
        game::map::PlanetData pd;
        pd.owner = PLAYER_NR;
        pd.colonistClans = 100;
        pd.money = 1313;             // required is 1300
        pd.supplies = 0;
        pd.minedTritanium = 100;
        pd.minedDuranium = 200;
        pd.minedMolybdenum = 300;
        p->addCurrentPlanetData(pd, PlayerSet_t(PLAYER_NR));

        game::map::BaseData bd;
        for (size_t i = 0; i < game::NUM_TECH_AREAS; ++i) {
            bd.techLevels[i] = 1;
        }
        p->addCurrentBaseData(bd, PlayerSet_t(PLAYER_NR));
        p->setPosition(game::map::Point(1000, 1000));
        p->setName("P");

        // - ship
        game::map::Ship* sh = g->currentTurn().universe().ships().create(SHIP_ID);
        game::map::ShipData sd;
        sd.owner = PLAYER_NR;
        sd.hullType = game::test::OUTRIDER_HULL_ID;
        sd.engineType = game::test::NOVA_ENGINE_ID;
        sd.numBeams = 1;
        sd.beamType = 4;
        sd.x = 1000;
        sd.y = 1000;
        sh->addCurrentShipData(sd, PlayerSet_t(PLAYER_NR));

        // - finalize
        t.session().setGame(g);
        t.session().postprocessTurn(g->currentTurn(), PlayerSet_t(PLAYER_NR), PlayerSet_t(PLAYER_NR), game::map::Object::Playable);
    }
}

/** Test behaviour on empty session.
    A: create an empty session. Create CloneShipProxy.
    E: Proxy reports not-valid status. */
AFL_TEST("game.proxy.CloneShipProxy:empty", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::CloneShipProxy testee(t.gameSender(), 33);

    // Get current status -> returns unsuccessful, zero
    game::proxy::CloneShipProxy::Status st;
    testee.getStatus(ind, st);
    a.checkEqual("01. valid", st.valid, false);
    a.checkEqual("02. planetId", st.planetId, 0);
}

/** Test normal behaviour.
    A: create a session with ship and planet. Create CloneShipProxy (same setup as in TestGameActionsCloneShip::testNormal).
    E: Proxy reports valid status. commit() succeeds. */
AFL_TEST("game.proxy.CloneShipProxy:normal", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    game::proxy::CloneShipProxy testee(t.gameSender(), SHIP_ID);

    // Get current status -> returns successful
    game::proxy::CloneShipProxy::Status st;
    testee.getStatus(ind, st);
    a.checkEqual("01. valid",          st.valid, true);
    a.checkEqual("02. planetId",       st.planetId, PLANET_ID);
    a.checkEqual("03. getHullIndex",   st.buildOrder.getHullIndex(), game::test::OUTRIDER_HULL_ID);
    a.checkEqual("04. orderStatus",    st.orderStatus, game::actions::CloneShip::CanClone);
    a.checkEqual("05. paymentStatus",  st.paymentStatus, game::actions::CloneShip::CannotPayComponents);
    a.checkEqual("06. cost",           st.cost.toCargoSpecString(), "44T 35D 13M 1470$");
    a.checkEqual("07. available",      st.available.toCargoSpecString(), "100T 200D 300M 1313$");
    a.checkEqual("08. remaining",      st.remaining.toCargoSpecString(), "56T 165D 287M -157S");
    a.checkEqual("09. missing",        st.missing.toCargoSpecString(), "157S");
    a.checkEqual("10. techCost",       st.techCost.toCargoSpecString(), "1300$");
    a.checkEqual("11. conflictStatus", st.conflictStatus, game::actions::CloneShip::NoConflict);

    // Commit
    testee.commit();
    t.sync();
    ind.processQueue();

    // Verify
    a.checkEqual("21. getFriendlyCode", t.session().getGame()->currentTurn().universe().ships().get(SHIP_ID)->getFriendlyCode().orElse(""), "cln");
    a.checkEqual("22. getBaseTechLevel", t.session().getGame()->currentTurn().universe().planets().get(PLANET_ID)->getBaseTechLevel(game::EngineTech).orElse(0), 5);
}
