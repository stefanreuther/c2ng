/**
  *  \file test/game/proxy/techupgradeproxytest.cpp
  *  \brief Test for game::proxy::TechUpgradeProxy
  */

#include "game/proxy/techupgradeproxy.hpp"

#include "afl/test/testrunner.hpp"
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
        afl::base::Ptr<game::Root> r(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0)), game::RegistrationKey::Unknown, MAX_TECH).asPtr());
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
        for (size_t i = 0; i < game::NUM_TECH_AREAS; ++i) {
            bd.techLevels[i] = 3;
        }
        p->addCurrentBaseData(bd, game::PlayerSet_t(PLAYER_NR));
        p->setPosition(game::map::Point(1000, 1000));
        p->setName("P");
        t.session().setGame(g);
        t.session().postprocessTurn(g->currentTurn(), game::PlayerSet_t(PLAYER_NR), game::PlayerSet_t(PLAYER_NR), game::map::Object::Playable);
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


AFL_TEST("game.proxy.TechUpgradeProxy:empty", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::TechUpgradeProxy testee(t.gameSender(), ind, 99);

    // Get current status -> returns unsuccessful, zero
    game::proxy::TechUpgradeProxy::Status st;
    testee.getStatus(ind, st);
    a.checkDifferent("01. status", st.status, game::actions::TechUpgrade::Success);
    a.checkEqual("02. max", st.max[0], 0);
}

AFL_TEST("game.proxy.TechUpgradeProxy:normal", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    game::proxy::TechUpgradeProxy testee(t.gameSender(), ind, PLANET_ID);

    // Get current status -> returns successful
    game::proxy::TechUpgradeProxy::Status st;
    testee.getStatus(ind, st);
    a.checkEqual("01. status", st.status, game::actions::TechUpgrade::Success);
    a.checkEqual("02. max", st.max[0], MAX_TECH);
    a.checkEqual("03. min", st.min[0], 3);

    // Perform some upgrades
    game::proxy::TechUpgradeProxy::Order o = {{4,4,4,4}};
    testee.setAll(o);
    testee.setTechLevel(game::HullTech, 5);

    // Verify status
    testee.getStatus(ind, st);
    a.checkEqual("11. status",      st.status, game::actions::TechUpgrade::Success);
    a.checkEqual("12. HullTech",    st.current[game::HullTech], 5);
    a.checkEqual("13. EngineTech",  st.current[game::EngineTech], 4);
    a.checkEqual("14. BeamTech",    st.current[game::BeamTech], 4);
    a.checkEqual("15. TorpedoTech", st.current[game::TorpedoTech], 4);
    a.checkEqual("16. Money",       st.cost.get(game::spec::Cost::Money), 1600);

    // Commit
    testee.commit();
    t.sync();
    ind.processQueue();

    // Verify
    game::map::Planet& p = *t.session().getGame()->currentTurn().universe().planets().get(PLANET_ID);
    a.checkEqual("21. HullTech",    p.getBaseTechLevel(game::HullTech).orElse(-1),    5);
    a.checkEqual("22. EngineTech",  p.getBaseTechLevel(game::EngineTech).orElse(-1),  4);
    a.checkEqual("23. BeamTech",    p.getBaseTechLevel(game::BeamTech).orElse(-1),    4);
    a.checkEqual("24. TorpedoTech", p.getBaseTechLevel(game::TorpedoTech).orElse(-1), 4);
}

AFL_TEST("game.proxy.TechUpgradeProxy:signal", a)
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
    a.checkDifferent("01. status", recv.getStatus().status, game::actions::TechUpgrade::Success);

    // Modify and wait for update
    testee.setTechLevel(game::BeamTech, 6);
    t.sync();
    ind.processQueue();

    // Verify update content
    a.checkEqual("11. BeamTech", recv.getStatus().current[game::BeamTech], 6);
    a.checkEqual("12. status",   recv.getStatus().status, game::actions::TechUpgrade::Success);
}

AFL_TEST("game.proxy.TechUpgradeProxy:upgrade", a)
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
    a.checkEqual("01. status", st.status, game::actions::TechUpgrade::Success);
    a.checkEqual("02. HullTech", st.current[game::HullTech], 4);
    a.checkEqual("03. BeamTech", st.current[game::BeamTech], 3);       // unchanged
}

AFL_TEST("game.proxy.TechUpgradeProxy:setReservedAmount", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    prepare(t);
    game::proxy::TechUpgradeProxy testee(t.gameSender(), ind, PLANET_ID);

    // Get current status -> returns successful
    game::proxy::TechUpgradeProxy::Status st;
    testee.getStatus(ind, st);
    a.checkEqual("01. status", st.status, game::actions::TechUpgrade::Success);
    a.checkEqual("02. max", st.max[0], MAX_TECH);
    a.checkEqual("03. min", st.min[0], 3);

    // Tech levels are at 3, and we have 3000$ in total.
    // Upgrading to tech 6 costs 1200$.
    testee.setReservedAmount(game::spec::Cost::fromString("$1000"));
    testee.setTechLevel(game::HullTech, 6);
    testee.getStatus(ind, st);
    a.checkEqual("11. status",      st.status, game::actions::TechUpgrade::Success);
    a.checkEqual("12. HullTech",    st.current[game::HullTech], 6);
    a.checkEqual("13. EngineTech",  st.current[game::EngineTech], 3);
    a.checkEqual("14. BeamTech",    st.current[game::BeamTech], 3);
    a.checkEqual("15. TorpedoTech", st.current[game::TorpedoTech], 3);
    a.checkEqual("16. Money",       st.cost.get(game::spec::Cost::Money), 1200);
    a.checkEqual("17. Money",       st.remaining.get(game::spec::Cost::Money), 0);
    a.checkEqual("18. Supplies",    st.remaining.get(game::spec::Cost::Supplies), 800);

    // Upgrade another one, this will fail
    testee.setTechLevel(game::BeamTech, 6);
    testee.getStatus(ind, st);
    a.checkEqual("21. status",      st.status, game::actions::TechUpgrade::MissingResources);
    a.checkEqual("22. HullTech",    st.current[game::HullTech], 6);
    a.checkEqual("23. EngineTech",  st.current[game::EngineTech], 3);
    a.checkEqual("24. BeamTech",    st.current[game::BeamTech], 6);
    a.checkEqual("25. TorpedoTech", st.current[game::TorpedoTech], 3);
    a.checkEqual("26. Money",       st.cost.get(game::spec::Cost::Money), 2400);
    a.checkEqual("27. Money",       st.remaining.get(game::spec::Cost::Money), 0);
    a.checkEqual("28. Supplies",    st.remaining.get(game::spec::Cost::Supplies), -400);

    // Undo reservation; action ok now
    testee.setReservedAmount(game::spec::Cost());
    testee.getStatus(ind, st);
    a.checkEqual("31. status",      st.status, game::actions::TechUpgrade::Success);
    a.checkEqual("32. HullTech",    st.current[game::HullTech], 6);
    a.checkEqual("33. EngineTech",  st.current[game::EngineTech], 3);
    a.checkEqual("34. BeamTech",    st.current[game::BeamTech], 6);
    a.checkEqual("35. TorpedoTech", st.current[game::TorpedoTech], 3);
    a.checkEqual("36. Money",       st.cost.get(game::spec::Cost::Money), 2400);
    a.checkEqual("37. Money",       st.remaining.get(game::spec::Cost::Money), 0);
    a.checkEqual("38. successful",  st.remaining.get(game::spec::Cost::Supplies), 600);
}
