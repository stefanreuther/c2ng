/**
  *  \file u/t_game_proxy_buildpartsproxy.cpp
  *  \brief Test for game::proxy::BuildPartsProxy
  */

#include "game/proxy/buildpartsproxy.hpp"

#include "t_game_proxy.hpp"
#include "afl/base/staticassert.hpp"
#include "game/game.hpp"
#include "game/map/basedata.hpp"
#include "game/map/planet.hpp"
#include "game/map/planetdata.hpp"
#include "game/root.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/shiplist.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"

using afl::base::Ptr;
using game::HostVersion;
using game::actions::BuildParts;
using game::map::Planet;
using game::proxy::BuildPartsProxy;
using game::spec::Cost;

namespace {
    /* Constants */
    const int PLANET_ID = 363;
    const int PLAYER_NR = 7;
    const int HULL_SLOT = 3;

    /* Hull slot must differ from hull Id to detect mismatches */
    static_assert(HULL_SLOT != game::test::GORBIE_HULL_ID, "HULL_SLOT");

    /* Prepare default environment */
    void prepare(game::test::SessionThread& t)
    {
        // ShipList
        Ptr<game::spec::ShipList> shipList(new game::spec::ShipList());
        game::test::initPListBeams(*shipList);
        game::test::addGorbie(*shipList);
        shipList->hullAssignments().add(PLAYER_NR, HULL_SLOT, game::test::GORBIE_HULL_ID);
        t.session().setShipList(shipList);

        // Root
        Ptr<game::Root> r(new game::test::Root(HostVersion(HostVersion::PHost, MKVERSION(4,0,0))));
        t.session().setRoot(r);

        // Game
        Ptr<game::Game> g(new game::Game());
        Planet* p = g->currentTurn().universe().planets().create(PLANET_ID);
        game::map::PlanetData pd;
        pd.owner = PLAYER_NR;
        pd.colonistClans = 100;
        pd.money = 2000;
        pd.supplies = 1000;
        pd.minedTritanium = 2000;
        pd.minedDuranium = 2000;
        pd.minedMolybdenum = 2000;
        p->addCurrentPlanetData(pd, game::PlayerSet_t(PLAYER_NR));

        game::map::BaseData bd;
        bd.owner = PLAYER_NR;
        for (size_t i = 0; i < game::NUM_TECH_AREAS; ++i) {
            bd.techLevels[i] = 10;
        }
        for (int i = 1; i <= 20; ++i) {
            bd.hullStorage.set(i, 0);
            bd.beamStorage.set(i, 0);
            bd.engineStorage.set(i, 0);
            bd.launcherStorage.set(i, 0);
        }
        p->addCurrentBaseData(bd, game::PlayerSet_t(PLAYER_NR));
        p->setPosition(game::map::Point(1000, 1000));
        p->setName("P");
        g->currentTurn().universe().postprocess(game::PlayerSet_t(PLAYER_NR), game::PlayerSet_t(PLAYER_NR), game::map::Object::Playable,
                                                r->hostVersion(), r->hostConfiguration(), 12, *shipList, t.session().translator(), t.session().log());

        t.session().setGame(g);
    }

    /* Receive updates from a proxy */
    class UpdateReceiver {
     public:
        const BuildPartsProxy::Status& getStatus() const
            { return m_status; }

        void onChange(const BuildPartsProxy::Status& status)
            { m_status = status; }
     private:
        BuildPartsProxy::Status m_status;
    };
}

/** Test behaviour on empty session.
    A: create empty session. Create BuildPartsProxy.
    E: status must report failure */
void
TestGameProxyBuildPartsProxy::testEmpty()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    BuildPartsProxy testee(t.gameSender(), ind, 99);

    // Get current status -> returns unsuccessful, zero
    BuildPartsProxy::Status st;
    testee.getStatus(ind, st);
    TS_ASSERT_DIFFERS(st.status, BuildParts::Success);
    TS_ASSERT_EQUALS(st.numParts, 0);
}

/** Test normal behaviour.
    A: create populated session. Create BuildParts. Select and build some parts.
    E: correct status reported, action correctly committed */
void
TestGameProxyBuildPartsProxy::testNormal()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    BuildPartsProxy testee(t.gameSender(), ind, PLANET_ID);

    // Get current status -> success, nothing selected
    BuildPartsProxy::Status st;
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.status, BuildParts::Success);
    TS_ASSERT_EQUALS(st.numParts, 0);
    TS_ASSERT(st.cost.isZero());

    // Select Gorbie and build one
    testee.selectPart(game::HullTech, game::test::GORBIE_HULL_ID);
    testee.add(1);
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.status, BuildParts::Success);
    TS_ASSERT_EQUALS(st.numParts, 1);
    TS_ASSERT_EQUALS(st.cost.get(Cost::Money),      790);
    TS_ASSERT_EQUALS(st.cost.get(Cost::Tritanium),  471);
    TS_ASSERT_EQUALS(st.cost.get(Cost::Duranium),   142);
    TS_ASSERT_EQUALS(st.cost.get(Cost::Molybdenum), 442);
    TS_ASSERT_EQUALS(st.name, "GORBIE CLASS BATTLECARRIER");

    // Select Kill-O-Zap and build 3
    testee.selectPart(game::BeamTech, 2);
    testee.add(3);
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.status, BuildParts::Success);
    TS_ASSERT_EQUALS(st.numParts, 3);
    TS_ASSERT_EQUALS(st.cost.get(Cost::Money),      790 + 15);
    TS_ASSERT_EQUALS(st.cost.get(Cost::Tritanium),  471 +  3);
    TS_ASSERT_EQUALS(st.cost.get(Cost::Duranium),   142 +  6);
    TS_ASSERT_EQUALS(st.cost.get(Cost::Molybdenum), 442 +  0);
    TS_ASSERT_EQUALS(st.name, "Kill-O-Zap");

    // Commit
    testee.commit();
    t.sync();
    ind.processQueue();

    // Verify
    const Planet& p = *t.session().getGame()->currentTurn().universe().planets().get(PLANET_ID);
    TS_ASSERT_EQUALS(p.getBaseStorage(game::HullTech, HULL_SLOT).orElse(-1), 1);
    TS_ASSERT_EQUALS(p.getBaseStorage(game::BeamTech, 2).orElse(-1), 3);
    TS_ASSERT_EQUALS(p.getCargo(game::Element::Money).orElse(-1), 2000 - 790 - 15);
}

/** Test signalisation of changes.
    A: create populated session. Create BuildParts. Register listener. Select and build a part.
    E: correct status reported through listener */
void
TestGameProxyBuildPartsProxy::testSignal()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    BuildPartsProxy testee(t.gameSender(), ind, PLANET_ID);

    UpdateReceiver recv;
    testee.sig_change.add(&recv, &UpdateReceiver::onChange);

    // Select Gorbie and build one
    testee.selectPart(game::HullTech, game::test::GORBIE_HULL_ID);
    testee.add(1);

    // Wait for update
    t.sync();
    ind.processQueue();
    const BuildPartsProxy::Status& st = recv.getStatus();
    TS_ASSERT_EQUALS(st.status, BuildParts::Success);
    TS_ASSERT_EQUALS(st.numParts, 1);
    TS_ASSERT_EQUALS(st.cost.get(Cost::Money),      790);
    TS_ASSERT_EQUALS(st.cost.get(Cost::Tritanium),  471);
    TS_ASSERT_EQUALS(st.cost.get(Cost::Duranium),   142);
    TS_ASSERT_EQUALS(st.cost.get(Cost::Molybdenum), 442);
}

/** Test error behaviour.
    A: create populated session including a hull we cannot build. Create BuildParts. Select and build that hull.
    E: no change to action */
void
TestGameProxyBuildPartsProxy::testError()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    game::test::addOutrider(*t.session().getShipList());
    BuildPartsProxy testee(t.gameSender(), ind, PLANET_ID);

    // Build a hull we cannot build
    testee.selectPart(game::HullTech, game::test::OUTRIDER_HULL_ID);
    testee.add(1);

    // Verify: no change. The request is ignored.
    BuildPartsProxy::Status st;
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.status, BuildParts::Success);
    TS_ASSERT_EQUALS(st.numParts, 0);
    TS_ASSERT(st.cost.isZero());
}

/** Test error reporting.
    A: create populated session. Create BuildParts. Select and build more parts than there are resources.
    E: status reported as failure */
void
TestGameProxyBuildPartsProxy::testErrorResources()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    BuildPartsProxy testee(t.gameSender(), ind, PLANET_ID);

    // Select Gorbie and build some
    BuildPartsProxy::Status st;
    testee.selectPart(game::HullTech, game::test::GORBIE_HULL_ID);
    testee.add(10);
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.status, BuildParts::MissingResources);
    TS_ASSERT_EQUALS(st.numParts, 10);
    TS_ASSERT_EQUALS(st.cost.get(Cost::Money),      7900);
    TS_ASSERT_EQUALS(st.cost.get(Cost::Tritanium),  4710);
    TS_ASSERT_EQUALS(st.cost.get(Cost::Duranium),   1420);
    TS_ASSERT_EQUALS(st.cost.get(Cost::Molybdenum), 4420);
}

