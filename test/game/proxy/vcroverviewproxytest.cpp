/**
  *  \file test/game/proxy/vcroverviewproxytest.cpp
  *  \brief Test for game::proxy::VcrOverviewProxy
  */

#include "game/proxy/vcroverviewproxy.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "game/test/waitindicator.hpp"
#include "game/vcr/classic/battle.hpp"
#include "game/vcr/classic/database.hpp"

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
        TestAdaptor(afl::base::Ref<game::Root> root, afl::base::Ref<game::spec::ShipList> shipList, afl::base::Ref<game::vcr::Database> battles)
            : m_root(root), m_shipList(shipList), m_battles(battles)
            { }
        virtual afl::base::Ref<const game::Root> getRoot() const
            { return m_root; }
        virtual afl::base::Ref<const game::spec::ShipList> getShipList() const
            { return m_shipList; }
        virtual const game::TeamSettings* getTeamSettings() const
            { return 0; }
        virtual afl::base::Ref<game::vcr::Database> getBattles()
            { return m_battles; }
        virtual afl::string::Translator& translator()
            { return m_translator; }
        virtual afl::sys::LogListener& log()
            { return m_log; }
        virtual afl::io::FileSystem& fileSystem()
            { return m_fileSystem; }
        virtual size_t getCurrentBattle() const
            { return 0; }
        virtual void setCurrentBattle(size_t /*n*/)
            { }
        virtual game::sim::Setup* getSimulationSetup() const
            { return 0; }
        virtual bool isGameObject(const game::vcr::Object& /*obj*/) const
            { return false; }
     private:
        afl::base::Ref<game::Root> m_root;
        afl::base::Ref<game::spec::ShipList> m_shipList;
        afl::base::Ref<game::vcr::Database> m_battles;
        afl::string::NullTranslator m_translator;
        afl::sys::Log m_log;
        afl::io::NullFileSystem m_fileSystem;
    };
}

AFL_TEST("game.proxy.VcrOverviewProxy:empty", a)
{
    // Null sender
    util::RequestSender<game::proxy::VcrDatabaseAdaptor> nullSender;
    game::proxy::VcrOverviewProxy testee(nullSender);
    game::test::WaitIndicator ind;

    // Verify empty diagram
    {
        game::vcr::Overview::Diagram diag;
        testee.buildDiagram(ind, diag);
        a.checkEqual("01. units", diag.units.size(), 0U);
        a.checkEqual("02. battles", diag.battles.size(), 0U);
    }

    // Verify empty scores
    {
        game::vcr::Overview::ScoreSummary sum;
        testee.buildScoreSummary(ind, sum);
        a.checkEqual("11. players", sum.players.toInteger(), 0U);
    }
}

AFL_TEST("game.proxy.VcrOverviewProxy:buildDiagram", a)
{
    // Environment
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion()));
    afl::base::Ref<game::spec::ShipList> shipList(*new game::spec::ShipList());
    game::test::initStandardBeams(*shipList);
    game::test::initStandardTorpedoes(*shipList);

    // Battles
    afl::base::Ref<game::vcr::classic::Database> db = *new game::vcr::classic::Database();
    db->addNewBattle(new game::vcr::classic::Battle(makeFreighter(110, 1), makeCaptor(120, 2), 1, 0, 0))
        ->setType(game::vcr::classic::Host, 0);

    // Adaptor in a (mock) thread
    TestAdaptor ad(root, shipList, db);
    game::test::WaitIndicator ind;
    util::RequestReceiver<game::proxy::VcrDatabaseAdaptor> recv(ind, ad);

    // Proxy under test
    game::proxy::VcrOverviewProxy testee(recv.getSender());

    // Verify
    game::vcr::Overview::Diagram diag;
    testee.buildDiagram(ind, diag);
    a.checkEqual("01. units", diag.units.size(), 2U);
    a.checkEqual("02. name", diag.units[0].name, "F110 (ship #110)");
    a.checkEqual("03. name", diag.units[1].name, "C120 (ship #120)");
    a.checkEqual("04. battles", diag.battles.size(), 1U);
}

AFL_TEST("game.proxy.VcrOverviewProxy:buildScoreSummary", a)
{
    // Environment
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion()));
    afl::base::Ref<game::spec::ShipList> shipList(*new game::spec::ShipList());
    game::test::initStandardBeams(*shipList);
    game::test::initStandardTorpedoes(*shipList);

    // Battles
    afl::base::Ref<game::vcr::classic::Database> db = *new game::vcr::classic::Database();
    db->addNewBattle(new game::vcr::classic::Battle(makeFreighter(110, 1), makeCaptor(120, 2), 1, 0, 0))
        ->setType(game::vcr::classic::Host, 0);

    // Adaptor in a (mock) thread
    TestAdaptor ad(root, shipList, db);
    game::test::WaitIndicator ind;
    util::RequestReceiver<game::proxy::VcrDatabaseAdaptor> recv(ind, ad);

    // Proxy under test
    game::proxy::VcrOverviewProxy testee(recv.getSender());

    // Verify
    game::vcr::Overview::ScoreSummary sum;
    testee.buildScoreSummary(ind, sum);
    a.checkEqual("01. players", sum.players.toInteger(), (1U << 1) | (1U << 2));
}
