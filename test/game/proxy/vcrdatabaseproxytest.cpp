/**
  *  \file test/game/proxy/vcrdatabaseproxytest.cpp
  *  \brief Test for game::proxy::VcrDatabaseProxy
  */

#include "game/proxy/vcrdatabaseproxy.hpp"

#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
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


AFL_TEST("game.proxy.VcrDatabaseProxy:basics", a)
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
    a.checkEqual("01. numBattles", st.numBattles, 2U);
    a.checkEqual("02. currentBattle", st.currentBattle, 0U);
    a.checkEqual("03. kind", st.kind, game::proxy::VcrDatabaseProxy::ClassicCombat);

    // setCurrentBattle
    UpdateReceiver u;
    proxy.sig_update.add(&u, &UpdateReceiver::onUpdate);
    proxy.sig_sideUpdate.add(&u, &UpdateReceiver::onSideUpdate);
    proxy.sig_hullUpdate.add(&u, &UpdateReceiver::onHullUpdate);
    proxy.setCurrentBattle(1);
    ind.processQueue();
    a.checkEqual("11. m_index", u.m_index, 1U);
    a.checkEqual("12. currentBattle", env.currentBattle, 1U);
    a.checkEqual("13. heading",       u.m_data.heading, "Battle 2 of 2");
    a.checkEqual("14. algorithmName", u.m_data.algorithmName, "PHost 4");
    a.checkEqual("15. seed",          u.m_data.seed.orElse(-1), 42);
    a.checkEqual("16. units",         u.m_data.units.size(), 2U);
    a.checkEqual("17. units",         u.m_data.units[0].text[0], "Liz (Id #14, a Player 2 ANNIHILATION CLASS BATTLESHIP)");
    a.checkEqual("18. units",         u.m_data.units[1].text[0], "Bird (Id #434, a Player 3 starship)");
    a.checkEqual("19. groups",        u.m_data.groups.size(), 2U);
    a.checkEqual("20. firstObject",   u.m_data.groups[0].firstObject, 0U);
    a.checkEqual("21. numObjects",    u.m_data.groups[0].numObjects, 1U);
    a.checkEqual("22. x",             u.m_data.groups[0].x, -29000);
    a.checkEqual("23. y",             u.m_data.groups[0].y, 0);
    a.checkEqual("24. owner",         u.m_data.groups[0].owner, 2);
    a.checkEqual("25. speed",         u.m_data.groups[0].speed, 75);
    a.checkEqual("26. firstObject",   u.m_data.groups[1].firstObject, 1U);
    a.checkEqual("27. numObjects",    u.m_data.groups[1].numObjects, 1U);
    a.checkEqual("28. x",             u.m_data.groups[1].x, 29000);
    a.checkEqual("29. y",             u.m_data.groups[1].y, 0);
    a.checkEqual("30. owner",         u.m_data.groups[1].owner, 3);
    a.checkEqual("31. speed",         u.m_data.groups[1].speed, 75);

    a.check("41. name",        u.m_sideInfo.name.empty());
    a.check("42. planetInfo", !u.m_hullInfo.planetInfo.isValid());
    a.check("43. shipInfo",   !u.m_hullInfo.shipInfo.isValid());
    a.check("44. shipQuery",  !u.m_hullInfo.shipQuery.isValid());

    // setSide(false) -> sets m_sideInfo, but not m_hullInfo
    proxy.setSide(0, false);
    ind.processQueue();
    a.checkEqual("51. name",        u.m_sideInfo.name, "Liz");
    a.checkEqual("52. subtitle",    u.m_sideInfo.subtitle, "Id #14, a Player 2 ANNIHILATION CLASS BATTLESHIP");
    a.checkEqual("53. typeChoices", u.m_sideInfo.typeChoices.size(), 1U);

    int32_t id;
    String_t name;
    a.check("61. typeChoices", u.m_sideInfo.typeChoices.get(0, id, name));
    a.checkEqual("62. id", id, game::test::ANNIHILATION_HULL_ID);
    a.checkEqual("63. name", name, "ANNIHILATION CLASS BATTLESHIP");

    a.check("71. planetInfo", !u.m_hullInfo.planetInfo.isValid());
    a.check("72. shipInfo",   !u.m_hullInfo.shipInfo.isValid());
    a.check("73. shipQuery",  !u.m_hullInfo.shipQuery.isValid());

    // setHullType -> sets m_hullInfo
    proxy.setHullType(game::test::ANNIHILATION_HULL_ID);
    ind.processQueue();
    a.check("81. planetInfo",      !u.m_hullInfo.planetInfo.isValid());
    a.check("82. shipInfo",         u.m_hullInfo.shipInfo.isValid());
    a.check("83. shipQuery",        u.m_hullInfo.shipQuery.isValid());
    a.checkEqual("84. engine",      u.m_hullInfo.shipInfo.get()->engine.second, "6 engines");
    a.checkEqual("85. imageName",   u.m_hullInfo.imageName, "hull-53");
    a.checkEqual("86. getHullType", u.m_hullInfo.shipQuery.get()->getHullType(), game::test::ANNIHILATION_HULL_ID);
    a.checkEqual("87. getOwner",    u.m_hullInfo.shipQuery.get()->getOwner(), 2);

    // setSide(true) -> replaces both m_hullInfo and m_sideInfo
    proxy.setSide(1, true);
    ind.processQueue();
    a.checkEqual("91. name",      u.m_sideInfo.name, "Bird");
    a.check("92. shipInfo",       u.m_hullInfo.shipInfo.isValid());
    a.check("93. shipQuery",     !u.m_hullInfo.shipQuery.isValid());
    a.checkEqual("94. engine",    u.m_hullInfo.shipInfo.get()->engine.second, "");
    a.checkEqual("95. imageName", u.m_hullInfo.imageName, "obj-0-777");

    // Add to sim
    game::proxy::VcrDatabaseProxy::AddResult ar = proxy.addToSimulation(ind, 0, true);
    a.checkEqual("101. addToSimulation", ar, game::proxy::VcrDatabaseProxy::Success);
    a.checkEqual("102. getNumShips", env.setup.getNumShips(), 1U);
    a.checkEqual("103. getShip", env.setup.getShip(0)->getName(), "Bird");
}

/** Test getTeamSettings(), no team settings in game side. */
AFL_TEST("game.proxy.VcrDatabaseProxy:getTeamSettings:empty", a)
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
    a.checkEqual("01. getViewpointPlayer", teams.getViewpointPlayer(), 0);
}

/** Test getTeamSettings(), team settings present in game side. */
AFL_TEST("game.proxy.VcrDatabaseProxy:getTeamSettings", a)
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
    a.checkEqual("01. getViewpointPlayer", teams.getViewpointPlayer(), 7);
    a.checkEqual("02. getPlayerTeam", teams.getPlayerTeam(3), 7);
}

/** Test getPlayerNames(). */
AFL_TEST("game.proxy.VcrDatabaseProxy:getPlayerNames", a)
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
    a.checkEqual("01. adj", adj.get(3), "three");
    a.checkEqual("02. adj", adj.get(9), "Player 9");
    a.checkEqual("03. full", full.get(3), "Player 3");
    a.checkEqual("04. full", full.get(9), "Nine");
}
