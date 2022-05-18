/**
  *  \file u/t_game_proxy_techupgradeproxy.cpp
  *  \brief Test for game::proxy::TechUpgradeProxy
  */

#include "game/proxy/techupgradeproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/game.hpp"
#include "game/map/basedata.hpp"
#include "game/map/planet.hpp"
#include "game/map/planetdata.hpp"
#include "game/root.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"

namespace {
    const int PLANET_ID = 363;
    const int PLAYER_NR = 7;
    const int MAX_TECH = 6;

    void prepare(game::test::SessionThread& t)
    {
        // ShipList: needs to exist but can be empty
        afl::base::Ptr<game::spec::ShipList> shipList(new game::spec::ShipList());
        t.session().setShipList(shipList);

        // Root
        afl::base::Ptr<game::Root> r(new game::test::Root(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0)), game::RegistrationKey::Unknown, MAX_TECH));
        t.session().setRoot(r);

        // Game
        afl::base::Ptr<game::Game> g(new game::Game());
        game::map::Planet* p = g->currentTurn().universe().planets().create(PLANET_ID);
        game::map::PlanetData pd;
        pd.owner = PLAYER_NR;
        pd.colonistClans = 100;
        pd.money = 2000;
        pd.supplies = 1000;
        p->addCurrentPlanetData(pd, game::PlayerSet_t(PLAYER_NR));

        game::map::BaseData bd;
        bd.owner = PLAYER_NR;
        for (size_t i = 0; i < game::NUM_TECH_AREAS; ++i) {
            bd.techLevels[i] = 3;
        }
        p->addCurrentBaseData(bd, game::PlayerSet_t(PLAYER_NR));
        p->setPosition(game::map::Point(1000, 1000));
        p->setName("P");
        g->currentTurn().universe().postprocess(game::PlayerSet_t(PLAYER_NR), game::PlayerSet_t(PLAYER_NR), game::map::Object::Playable,
                                                g->mapConfiguration(),
                                                r->hostVersion(), r->hostConfiguration(), 12, *shipList, t.session().translator(), t.session().log());

        t.session().setGame(g);
    }

    /* Receive updates from a proxy */
    class UpdateReceiver {
     public:
        const game::proxy::TechUpgradeProxy::Status& getStatus() const
            { return m_status; }

        void onChange(const game::proxy::TechUpgradeProxy::Status& status)
            { m_status = status; }
     private:
        game::proxy::TechUpgradeProxy::Status m_status;
    };
}


void
TestGameProxyTechUpgradeProxy::testEmpty()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::TechUpgradeProxy testee(t.gameSender(), ind, 99);

    // Get current status -> returns unsuccessful, zero
    game::proxy::TechUpgradeProxy::Status st;
    testee.getStatus(ind, st);
    TS_ASSERT_DIFFERS(st.status, game::actions::TechUpgrade::Success);
    TS_ASSERT_EQUALS(st.max[0], 0);
}

void
TestGameProxyTechUpgradeProxy::testNormal()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    game::proxy::TechUpgradeProxy testee(t.gameSender(), ind, PLANET_ID);

    // Get current status -> returns successful
    game::proxy::TechUpgradeProxy::Status st;
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.status, game::actions::TechUpgrade::Success);
    TS_ASSERT_EQUALS(st.max[0], MAX_TECH);
    TS_ASSERT_EQUALS(st.min[0], 3);

    // Perform some upgrades
    game::proxy::TechUpgradeProxy::Order o = {{4,4,4,4}};
    testee.setAll(o);
    testee.setTechLevel(game::HullTech, 5);

    // Verify status
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.status, game::actions::TechUpgrade::Success);
    TS_ASSERT_EQUALS(st.current[game::HullTech], 5);
    TS_ASSERT_EQUALS(st.current[game::EngineTech], 4);
    TS_ASSERT_EQUALS(st.current[game::BeamTech], 4);
    TS_ASSERT_EQUALS(st.current[game::TorpedoTech], 4);
    TS_ASSERT_EQUALS(st.cost.get(game::spec::Cost::Money), 1600);

    // Commit
    testee.commit();
    t.sync();
    ind.processQueue();

    // Verify
    game::map::Planet& p = *t.session().getGame()->currentTurn().universe().planets().get(PLANET_ID);
    TS_ASSERT_EQUALS(p.getBaseTechLevel(game::HullTech).orElse(-1),    5);
    TS_ASSERT_EQUALS(p.getBaseTechLevel(game::EngineTech).orElse(-1),  4);
    TS_ASSERT_EQUALS(p.getBaseTechLevel(game::BeamTech).orElse(-1),    4);
    TS_ASSERT_EQUALS(p.getBaseTechLevel(game::TorpedoTech).orElse(-1), 4);
}

void
TestGameProxyTechUpgradeProxy::testSignal()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    game::proxy::TechUpgradeProxy testee(t.gameSender(), ind, PLANET_ID);

    // Flush signals
    t.sync();
    ind.processQueue();

    // Connect signal; verify that default state is NOT success
    UpdateReceiver recv;
    testee.sig_change.add(&recv, &UpdateReceiver::onChange);
    TS_ASSERT_DIFFERS(recv.getStatus().status, game::actions::TechUpgrade::Success);

    // Modify and wait for update
    testee.setTechLevel(game::BeamTech, 6);
    t.sync();
    ind.processQueue();

    // Verify update content
    TS_ASSERT_EQUALS(recv.getStatus().current[game::BeamTech], 6);
    TS_ASSERT_EQUALS(recv.getStatus().status, game::actions::TechUpgrade::Success);
}

void
TestGameProxyTechUpgradeProxy::testUpgrade()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    game::proxy::TechUpgradeProxy testee(t.gameSender(), ind, PLANET_ID);

    // Perform changes
    testee.upgradeTechLevel(game::HullTech, 4);
    testee.upgradeTechLevel(game::BeamTech, 2);

    // Verify status
    game::proxy::TechUpgradeProxy::Status st;
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.status, game::actions::TechUpgrade::Success);
    TS_ASSERT_EQUALS(st.current[game::HullTech], 4);
    TS_ASSERT_EQUALS(st.current[game::BeamTech], 3);       // unchanged
}

void
TestGameProxyTechUpgradeProxy::testReserve()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    game::proxy::TechUpgradeProxy testee(t.gameSender(), ind, PLANET_ID);

    // Get current status -> returns successful
    game::proxy::TechUpgradeProxy::Status st;
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.status, game::actions::TechUpgrade::Success);
    TS_ASSERT_EQUALS(st.max[0], MAX_TECH);
    TS_ASSERT_EQUALS(st.min[0], 3);

    // Tech levels are at 3, and we have 3000$ in total.
    // Upgrading to tech 6 costs 1200$.
    testee.setReservedAmount(game::spec::Cost::fromString("$1000"));
    testee.setTechLevel(game::HullTech, 6);
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.status, game::actions::TechUpgrade::Success);
    TS_ASSERT_EQUALS(st.current[game::HullTech], 6);
    TS_ASSERT_EQUALS(st.current[game::EngineTech], 3);
    TS_ASSERT_EQUALS(st.current[game::BeamTech], 3);
    TS_ASSERT_EQUALS(st.current[game::TorpedoTech], 3);
    TS_ASSERT_EQUALS(st.cost.get(game::spec::Cost::Money), 1200);
    TS_ASSERT_EQUALS(st.remaining.get(game::spec::Cost::Money), 0);
    TS_ASSERT_EQUALS(st.remaining.get(game::spec::Cost::Supplies), 800);

    // Upgrade another one, this will fail
    testee.setTechLevel(game::BeamTech, 6);
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.status, game::actions::TechUpgrade::MissingResources);
    TS_ASSERT_EQUALS(st.current[game::HullTech], 6);
    TS_ASSERT_EQUALS(st.current[game::EngineTech], 3);
    TS_ASSERT_EQUALS(st.current[game::BeamTech], 6);
    TS_ASSERT_EQUALS(st.current[game::TorpedoTech], 3);
    TS_ASSERT_EQUALS(st.cost.get(game::spec::Cost::Money), 2400);
    TS_ASSERT_EQUALS(st.remaining.get(game::spec::Cost::Money), 0);
    TS_ASSERT_EQUALS(st.remaining.get(game::spec::Cost::Supplies), -400);

    // Undo reservation; action ok now
    testee.setReservedAmount(game::spec::Cost());
    testee.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.status, game::actions::TechUpgrade::Success);
    TS_ASSERT_EQUALS(st.current[game::HullTech], 6);
    TS_ASSERT_EQUALS(st.current[game::EngineTech], 3);
    TS_ASSERT_EQUALS(st.current[game::BeamTech], 6);
    TS_ASSERT_EQUALS(st.current[game::TorpedoTech], 3);
    TS_ASSERT_EQUALS(st.cost.get(game::spec::Cost::Money), 2400);
    TS_ASSERT_EQUALS(st.remaining.get(game::spec::Cost::Money), 0);
    TS_ASSERT_EQUALS(st.remaining.get(game::spec::Cost::Supplies), 600);
}

