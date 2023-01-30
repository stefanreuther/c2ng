/**
  *  \file u/t_game_proxy_vcroverviewproxy.cpp
  *  \brief Test for game::proxy::VcrOverviewProxy
  */

#include "game/proxy/vcroverviewproxy.hpp"

#include "t_game_proxy.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "game/test/waitindicator.hpp"
#include "game/vcr/classic/database.hpp"
#include "game/vcr/classic/battle.hpp"

namespace {
    // A freighter (will be captured)
    game::vcr::Object makeFreighter(int id, int owner)
    {
        game::vcr::Object r;
        r.setMass(200);
        r.setShield(0);
        r.setDamage(0);
        r.setCrew(1);
        r.setId(id);
        r.setOwner(owner);
        r.setName(afl::string::Format("F%d", id));
        return r;
    }

    // A captor (will capture/destroy the other ship)
    game::vcr::Object makeCaptor(int id, int owner)
    {
        game::vcr::Object r;
        r.setMass(400);
        r.setShield(100);
        r.setDamage(0);
        r.setCrew(300);
        r.setId(id);
        r.setOwner(owner);
        r.setNumBeams(5);
        r.setBeamType(9);
        r.setName(afl::string::Format("C%d", id));
        return r;
    }

    class TestAdaptor : public game::proxy::VcrDatabaseAdaptor {
     public:
        TestAdaptor(game::Root& root, game::spec::ShipList& shipList, game::vcr::Database& battles)
            : m_root(root), m_shipList(shipList), m_battles(battles)
            { }
        virtual const game::Root& root() const
            { return m_root; }
        virtual const game::spec::ShipList& shipList() const
            { return m_shipList; }
        virtual const game::TeamSettings* getTeamSettings() const
            { return 0; }
        virtual game::vcr::Database& battles()
            { return m_battles; }
        virtual afl::string::Translator& translator()
            { return m_translator; }
        virtual afl::sys::LogListener& log()
            { return m_log; }
        virtual size_t getCurrentBattle() const
            { return 0; }
        virtual void setCurrentBattle(size_t /*n*/)
            { }
        virtual game::sim::Setup* getSimulationSetup() const
            { return 0; }
        virtual bool isGameObject(const game::vcr::Object& /*obj*/) const
            { return false; }
     private:
        game::Root& m_root;
        game::spec::ShipList& m_shipList;
        game::vcr::Database& m_battles;
        afl::string::NullTranslator m_translator;
        afl::sys::Log m_log;
    };
}

void
TestGameProxyVcrOverviewProxy::testEmpty()
{
    // Null sender
    util::RequestSender<game::proxy::VcrDatabaseAdaptor> nullSender;
    game::proxy::VcrOverviewProxy testee(nullSender);
    game::test::WaitIndicator ind;

    // Verify empty diagram
    {
        game::vcr::Overview::Diagram diag;
        testee.buildDiagram(ind, diag);
        TS_ASSERT_EQUALS(diag.units.size(), 0U);
        TS_ASSERT_EQUALS(diag.battles.size(), 0U);
    }

    // Verify empty scores
    {
        game::vcr::Overview::ScoreSummary sum;
        testee.buildScoreSummary(ind, sum);
        TS_ASSERT_EQUALS(sum.players.toInteger(), 0U);
    }
}

void
TestGameProxyVcrOverviewProxy::testBuildDiagram()
{
    // Environment
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion()));
    game::spec::ShipList shipList;
    game::test::initStandardBeams(shipList);
    game::test::initStandardTorpedoes(shipList);

    // Battles
    game::vcr::classic::Database db;
    db.addNewBattle(new game::vcr::classic::Battle(makeFreighter(110, 1), makeCaptor(120, 2), 1, 0, 0))
        ->setType(game::vcr::classic::Host, 0);

    // Adaptor in a (mock) thread
    TestAdaptor ad(*root, shipList, db);
    game::test::WaitIndicator ind;
    util::RequestReceiver<game::proxy::VcrDatabaseAdaptor> recv(ind, ad);

    // Proxy under test
    game::proxy::VcrOverviewProxy testee(recv.getSender());

    // Verify
    game::vcr::Overview::Diagram diag;
    testee.buildDiagram(ind, diag);
    TS_ASSERT_EQUALS(diag.units.size(), 2U);
    TS_ASSERT_EQUALS(diag.units[0].name, "F110 (ship #110)");
    TS_ASSERT_EQUALS(diag.units[1].name, "C120 (ship #120)");
    TS_ASSERT_EQUALS(diag.battles.size(), 1U);
}

void
TestGameProxyVcrOverviewProxy::testBuildScores()
{
    // Environment
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion()));
    game::spec::ShipList shipList;
    game::test::initStandardBeams(shipList);
    game::test::initStandardTorpedoes(shipList);

    // Battles
    game::vcr::classic::Database db;
    db.addNewBattle(new game::vcr::classic::Battle(makeFreighter(110, 1), makeCaptor(120, 2), 1, 0, 0))
        ->setType(game::vcr::classic::Host, 0);

    // Adaptor in a (mock) thread
    TestAdaptor ad(*root, shipList, db);
    game::test::WaitIndicator ind;
    util::RequestReceiver<game::proxy::VcrDatabaseAdaptor> recv(ind, ad);

    // Proxy under test
    game::proxy::VcrOverviewProxy testee(recv.getSender());

    // Verify
    game::vcr::Overview::ScoreSummary sum;
    testee.buildScoreSummary(ind, sum);
    TS_ASSERT_EQUALS(sum.players.toInteger(), (1U << 1) | (1U << 2));
}

