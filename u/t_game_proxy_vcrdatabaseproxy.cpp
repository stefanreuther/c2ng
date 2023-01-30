/**
  *  \file u/t_game_proxy_vcrdatabaseproxy.cpp
  *  \brief Test for game::proxy::VcrDatabaseProxy
  */

#include "game/proxy/vcrdatabaseproxy.hpp"

#include "t_game_proxy.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/sim/ship.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "game/test/waitindicator.hpp"
#include "game/vcr/classic/battle.hpp"
#include "game/vcr/classic/database.hpp"

namespace {
    struct Environment {
        afl::base::Ref<game::Root> root;
        game::spec::ShipList shipList;
        game::TeamSettings* pTeamSettings;
        game::vcr::classic::Database battles;
        afl::string::NullTranslator translator;
        afl::sys::Log log;
        size_t currentBattle;
        game::sim::Setup setup;

        Environment()
            : root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0)))),
              shipList(), pTeamSettings(0), battles(), translator(), currentBattle(0), setup()
            { }
    };

    class TestAdaptor : public game::proxy::VcrDatabaseAdaptor {
     public:
        TestAdaptor(Environment& env)
            : m_env(env)
            { }
        virtual const game::Root& root() const
            { return *m_env.root; }
        virtual const game::spec::ShipList& shipList() const
            { return m_env.shipList; }
        virtual const game::TeamSettings* getTeamSettings() const
            { return m_env.pTeamSettings; }
        virtual game::vcr::Database& battles()
            { return m_env.battles; }
        virtual afl::string::Translator& translator()
            { return m_env.translator; }
        virtual afl::sys::LogListener& log()
            { return m_env.log; }
        virtual size_t getCurrentBattle() const
            { return m_env.currentBattle; }
        virtual void setCurrentBattle(size_t n)
            { m_env.currentBattle = n; }
        virtual game::sim::Setup* getSimulationSetup() const
            { return &m_env.setup; }
        virtual bool isGameObject(const game::vcr::Object&) const
            { return false; }
     private:
        Environment& m_env;
    };

    class TestPictureNamer : public game::spec::info::PictureNamer {
     public:
        virtual String_t getHullPicture(const game::spec::Hull& h) const
            { return afl::string::Format("hull-%d", h.getId()); }
        virtual String_t getEnginePicture(const game::spec::Engine& /*e*/) const
            { return String_t(); }
        virtual String_t getBeamPicture(const game::spec::Beam& /*b*/) const
            { return String_t(); }
        virtual String_t getLauncherPicture(const game::spec::TorpedoLauncher& /*tl*/) const
            { return String_t(); }
        virtual String_t getAbilityPicture(const String_t& /*abilityName*/, game::spec::info::AbilityFlags_t /*flags*/) const
            { return String_t(); }
        virtual String_t getPlayerPicture(const game::Player& /*pl*/) const
            { return String_t(); }
        virtual String_t getFighterPicture(int /*raceNr*/, int /*playerNr*/) const
            { return String_t(); }
        virtual String_t getVcrObjectPicture(bool isPlanet, int pictureNumber) const
            { return afl::string::Format("obj-%d-%d", int(isPlanet), pictureNumber); }
    };

    game::vcr::Object makeLeftShip()
    {
        game::vcr::Object left;
        left.setMass(150);
        left.setCrew(2);
        left.setId(14);
        left.setOwner(2);
        left.setBeamType(0);
        left.setNumBeams(0);
        left.setNumBays(0);
        left.setTorpedoType(0);
        left.setNumLaunchers(0);
        left.setNumTorpedoes(0);
        left.setNumFighters(0);
        left.setShield(100);
        left.setPicture(84);
        left.setName("Liz");
        return left;
    }

    game::vcr::Object makeRightShip()
    {
        game::vcr::Object right;
        right.setMass(233);
        right.setCrew(240);
        right.setId(434);
        right.setOwner(3);
        right.setBeamType(5);
        right.setNumBeams(6);
        right.setNumBays(0);
        right.setTorpedoType(7);
        right.setNumLaunchers(4);
        right.setNumTorpedoes(0);
        right.setNumFighters(0);
        right.setShield(100);
        right.setPicture(777);
        right.setName("Bird");
        return right;
    }

    struct UpdateReceiver {
        UpdateReceiver()
            : m_index(999), m_data()
            { }

        void onUpdate(size_t index, const game::vcr::BattleInfo& d)
            { m_index = index; m_data = d; }

        void onSideUpdate(const game::proxy::VcrDatabaseProxy::SideInfo& d)
            { m_sideInfo = d; }

        void onHullUpdate(const game::proxy::VcrDatabaseProxy::HullInfo& d)
            { m_hullInfo = d; }

        size_t m_index;
        game::vcr::BattleInfo m_data;
        game::proxy::VcrDatabaseProxy::SideInfo m_sideInfo;
        game::proxy::VcrDatabaseProxy::HullInfo m_hullInfo;
    };
}


void
TestGameProxyVcrDatabaseProxy::testIt()
{
    // Make simple environment
    Environment env;
    game::test::initStandardBeams(env.shipList);
    game::test::initStandardTorpedoes(env.shipList);
    game::test::addAnnihilation(env.shipList);
    env.battles.addNewBattle(new game::vcr::classic::Battle(makeRightShip(), makeLeftShip(), 42, 0, 0))
        ->setType(game::vcr::classic::PHost4, 0);
    env.battles.addNewBattle(new game::vcr::classic::Battle(makeLeftShip(), makeRightShip(), 42, 0, 0))
        ->setType(game::vcr::classic::PHost4, 0);

    // Set up tasking
    // WaitIndicator's RequestDispatcher personality serves both sides
    game::test::WaitIndicator ind;
    TestAdaptor ad(env);
    util::RequestReceiver<game::proxy::VcrDatabaseAdaptor> recv(ind, ad);

    // Make proxy
    game::proxy::VcrDatabaseProxy proxy(recv.getSender(), ind, env.translator, std::auto_ptr<game::spec::info::PictureNamer>(new TestPictureNamer()));

    // getStatus
    game::proxy::VcrDatabaseProxy::Status st;
    proxy.getStatus(ind, st);
    TS_ASSERT_EQUALS(st.numBattles, 2U);
    TS_ASSERT_EQUALS(st.currentBattle, 0U);
    TS_ASSERT_EQUALS(st.kind, game::proxy::VcrDatabaseProxy::ClassicCombat);

    // setCurrentBattle
    UpdateReceiver u;
    proxy.sig_update.add(&u, &UpdateReceiver::onUpdate);
    proxy.sig_sideUpdate.add(&u, &UpdateReceiver::onSideUpdate);
    proxy.sig_hullUpdate.add(&u, &UpdateReceiver::onHullUpdate);
    proxy.setCurrentBattle(1);
    ind.processQueue();
    TS_ASSERT_EQUALS(u.m_index, 1U);
    TS_ASSERT_EQUALS(env.currentBattle, 1U);
    TS_ASSERT_EQUALS(u.m_data.heading, "Battle 2 of 2");
    TS_ASSERT_EQUALS(u.m_data.algorithmName, "PHost 4");
    TS_ASSERT_EQUALS(u.m_data.seed.orElse(-1), 42);
    TS_ASSERT_EQUALS(u.m_data.units.size(), 2U);
    TS_ASSERT_EQUALS(u.m_data.units[0].text[0], "Liz (Id #14, a Player 2 ANNIHILATION CLASS BATTLESHIP)");
    TS_ASSERT_EQUALS(u.m_data.units[1].text[0], "Bird (Id #434, a Player 3 starship)");
    TS_ASSERT_EQUALS(u.m_data.groups.size(), 2U);
    TS_ASSERT_EQUALS(u.m_data.groups[0].firstObject, 0U);
    TS_ASSERT_EQUALS(u.m_data.groups[0].numObjects, 1U);
    TS_ASSERT_EQUALS(u.m_data.groups[0].x, -29000);
    TS_ASSERT_EQUALS(u.m_data.groups[0].y, 0);
    TS_ASSERT_EQUALS(u.m_data.groups[0].owner, 2);
    TS_ASSERT_EQUALS(u.m_data.groups[0].speed, 75);
    TS_ASSERT_EQUALS(u.m_data.groups[1].firstObject, 1U);
    TS_ASSERT_EQUALS(u.m_data.groups[1].numObjects, 1U);
    TS_ASSERT_EQUALS(u.m_data.groups[1].x, 29000);
    TS_ASSERT_EQUALS(u.m_data.groups[1].y, 0);
    TS_ASSERT_EQUALS(u.m_data.groups[1].owner, 3);
    TS_ASSERT_EQUALS(u.m_data.groups[1].speed, 75);

    TS_ASSERT(u.m_sideInfo.name.empty());
    TS_ASSERT(!u.m_hullInfo.planetInfo.isValid());
    TS_ASSERT(!u.m_hullInfo.shipInfo.isValid());
    TS_ASSERT(!u.m_hullInfo.shipQuery.isValid());

    // setSide(false) -> sets m_sideInfo, but not m_hullInfo
    proxy.setSide(0, false);
    ind.processQueue();
    TS_ASSERT_EQUALS(u.m_sideInfo.name, "Liz");
    TS_ASSERT_EQUALS(u.m_sideInfo.subtitle, "Id #14, a Player 2 ANNIHILATION CLASS BATTLESHIP");
    TS_ASSERT_EQUALS(u.m_sideInfo.typeChoices.size(), 1U);

    int32_t id;
    String_t name;
    TS_ASSERT(u.m_sideInfo.typeChoices.get(0, id, name));
    TS_ASSERT_EQUALS(id, game::test::ANNIHILATION_HULL_ID);
    TS_ASSERT_EQUALS(name, "ANNIHILATION CLASS BATTLESHIP");

    TS_ASSERT(!u.m_hullInfo.planetInfo.isValid());
    TS_ASSERT(!u.m_hullInfo.shipInfo.isValid());
    TS_ASSERT(!u.m_hullInfo.shipQuery.isValid());

    // setHullType -> sets m_hullInfo
    proxy.setHullType(game::test::ANNIHILATION_HULL_ID);
    ind.processQueue();
    TS_ASSERT(!u.m_hullInfo.planetInfo.isValid());
    TS_ASSERT(u.m_hullInfo.shipInfo.isValid());
    TS_ASSERT(u.m_hullInfo.shipQuery.isValid());
    TS_ASSERT_EQUALS(u.m_hullInfo.shipInfo.get()->engine.second, "6 engines");
    TS_ASSERT_EQUALS(u.m_hullInfo.imageName, "hull-53");
    TS_ASSERT_EQUALS(u.m_hullInfo.shipQuery.get()->getHullType(), game::test::ANNIHILATION_HULL_ID);
    TS_ASSERT_EQUALS(u.m_hullInfo.shipQuery.get()->getOwner(), 2);

    // setSide(true) -> replaces both m_hullInfo and m_sideInfo
    proxy.setSide(1, true);
    ind.processQueue();
    TS_ASSERT_EQUALS(u.m_sideInfo.name, "Bird");
    TS_ASSERT(u.m_hullInfo.shipInfo.isValid());
    TS_ASSERT(!u.m_hullInfo.shipQuery.isValid());
    TS_ASSERT_EQUALS(u.m_hullInfo.shipInfo.get()->engine.second, "");
    TS_ASSERT_EQUALS(u.m_hullInfo.imageName, "obj-0-777");

    // Add to sim
    game::proxy::VcrDatabaseProxy::AddResult ar = proxy.addToSimulation(ind, 0, true);
    TS_ASSERT_EQUALS(ar, game::proxy::VcrDatabaseProxy::Success);
    TS_ASSERT_EQUALS(env.setup.getNumShips(), 1U);
    TS_ASSERT_EQUALS(env.setup.getShip(0)->getName(), "Bird");
}

/** Test getTeamSettings(), no team settings in game side. */
void
TestGameProxyVcrDatabaseProxy::testGetTeamSettings()
{
    // Environment
    Environment env;
    game::test::WaitIndicator ind;
    TestAdaptor ad(env);
    util::RequestReceiver<game::proxy::VcrDatabaseAdaptor> recv(ind, ad);
    game::proxy::VcrDatabaseProxy proxy(recv.getSender(), ind, env.translator, std::auto_ptr<game::spec::info::PictureNamer>(new TestPictureNamer()));

    // Room for result
    game::TeamSettings teams;
    teams.setViewpointPlayer(10);

    // Retrieve result
    proxy.getTeamSettings(ind, teams);

    // Check
    TS_ASSERT_EQUALS(teams.getViewpointPlayer(), 0);
}

/** Test getTeamSettings(), team settings present in game side. */
void
TestGameProxyVcrDatabaseProxy::testGetTeamSettings2()
{
    // Environment
    game::TeamSettings gameTeams;
    gameTeams.setViewpointPlayer(7);
    gameTeams.setPlayerTeam(3, 7);

    Environment env;
    env.pTeamSettings = &gameTeams;
    game::test::WaitIndicator ind;
    TestAdaptor ad(env);
    util::RequestReceiver<game::proxy::VcrDatabaseAdaptor> recv(ind, ad);
    game::proxy::VcrDatabaseProxy proxy(recv.getSender(), ind, env.translator, std::auto_ptr<game::spec::info::PictureNamer>(new TestPictureNamer()));

    // Room for result
    game::TeamSettings teams;
    teams.setViewpointPlayer(10);

    // Retrieve result
    proxy.getTeamSettings(ind, teams);

    // Check
    TS_ASSERT_EQUALS(teams.getViewpointPlayer(), 7);
    TS_ASSERT_EQUALS(teams.getPlayerTeam(3), 7);
}

/** Test getPlayerNames(). */
void
TestGameProxyVcrDatabaseProxy::testGetPlayerNames()
{
    // Environment
    Environment env;
    game::Player* p3 = env.root->playerList().create(3);
    game::Player* p9 = env.root->playerList().create(9);
    p3->setName(game::Player::AdjectiveName, "three");
    p9->setName(game::Player::LongName, "Nine");
    game::test::WaitIndicator ind;
    TestAdaptor ad(env);
    util::RequestReceiver<game::proxy::VcrDatabaseAdaptor> recv(ind, ad);
    game::proxy::VcrDatabaseProxy proxy(recv.getSender(), ind, env.translator, std::auto_ptr<game::spec::info::PictureNamer>(new TestPictureNamer()));

    // Retrieve result
    game::PlayerArray<String_t> adj  = proxy.getPlayerNames(ind, game::Player::AdjectiveName);
    game::PlayerArray<String_t> full = proxy.getPlayerNames(ind, game::Player::LongName);

    // Check
    TS_ASSERT_EQUALS(adj.get(3), "three");
    TS_ASSERT_EQUALS(adj.get(9), "Player 9");
    TS_ASSERT_EQUALS(full.get(3), "Player 3");
    TS_ASSERT_EQUALS(full.get(9), "Nine");
}

