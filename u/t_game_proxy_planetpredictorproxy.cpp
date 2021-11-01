/**
  *  \file u/t_game_proxy_planetpredictorproxy.cpp
  *  \brief Test for game::proxy::PlanetPredictorProxy
  */

#include "game/proxy/planetpredictorproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/game.hpp"
#include "game/hostversion.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"

using afl::base::Ptr;
using game::Game;
using game::HostVersion;
using game::map::Planet;
using game::map::Universe;
using game::proxy::PlanetPredictorProxy;
using game::spec::ShipList;

namespace {
    const int LOC_X = 1;
    const int LOC_Y = 2;

    Planet& addPlanet(Universe& univ, int id, int owner)
    {
        Planet& p = *univ.planets().create(id);
        p.setPosition(game::map::Point(LOC_X, LOC_Y));

        game::map::PlanetData pd;
        pd.owner = owner;
        pd.minedNeutronium = 1000;
        pd.minedTritanium = 1000;
        pd.minedDuranium = 1000;
        pd.minedMolybdenum = 1000;
        pd.money = 1000;
        pd.supplies = 1000;

        // Same setup as in TaxationAction test
        pd.colonistClans = 1000;
        pd.nativeRace = game::ReptilianNatives;
        pd.nativeGovernment = 5;
        pd.nativeClans = 20000;
        pd.colonistHappiness = 100;
        pd.nativeHappiness = 100;
        pd.temperature = 50;
        pd.colonistTax = 1;
        pd.nativeTax = 2;

        p.addCurrentPlanetData(pd, game::PlayerSet_t(owner));

        afl::string::NullTranslator tx;
        afl::sys::Log log;
        p.internalCheck(game::map::Configuration(), tx, log);
        p.setPlayability(game::map::Object::Playable);

        return p;
    }

    const int PLANET_ID = 42;
    const int OWNER = 3;

    Planet& setup(game::test::SessionThread& h)
    {
        // Root
        h.session().setRoot(new game::test::Root(HostVersion(HostVersion::PHost, MKVERSION(3,4,0))));

        // Game
        Ptr<Game> g = new Game();
        Universe& u = g->currentTurn().universe();
        Planet& p = addPlanet(u, PLANET_ID, OWNER);
        h.session().setGame(g);

        // Shiplist
        Ptr<ShipList> sl = new ShipList();
        h.session().setShipList(sl);

        return p;
    }

    class UpdateReceiver {
     public:
        UpdateReceiver()
            : m_status(),
              m_ok(false)
            { }
        void onUpdate(const PlanetPredictorProxy::Status& st)
            {
                m_status = st;
                m_ok = true;
            }
        bool hasUpdate() const
            { return m_ok; }
        const PlanetPredictorProxy::Status& status() const
            { return m_status; }
     private:
        PlanetPredictorProxy::Status m_status;
        bool m_ok;
    };
}

/** Test empty universe.
    A: create PlanetPredictorProxy on empty universe.
    E: proxy must report all values unavailable */
void
TestGameProxyPlanetPredictorProxy::testEmpty()
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    PlanetPredictorProxy testee(ind, h.gameSender(), 77);

    PlanetPredictorProxy::Status status;
    testee.getStatus(ind, status);
    TS_ASSERT_EQUALS(status.colonistClans.size(), 0U);
    TS_ASSERT_EQUALS(status.nativeClans.size(), 0U);
    TS_ASSERT_EQUALS(status.experienceLevel.size(), 0U);
    TS_ASSERT_EQUALS(status.experiencePoints.size(), 0U);
    TS_ASSERT_EQUALS(status.effectorLabel, "");

    game::map::PlanetEffectors eff = testee.getEffectors(ind);
    TS_ASSERT_EQUALS(eff.getNumTerraformers(), 0);
}

/** Test normal situation.
    A: create PlanetPredictorProxy on universe containing a planet.
    E: proxy must report correct values */
void
TestGameProxyPlanetPredictorProxy::testNormal()
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    Planet& p = setup(h);

    // Add some experience
    game::UnitScoreDefinitionList::Definition def;
    def.name = "Exp";
    def.id = game::ScoreId_ExpPoints;
    def.limit = 9999;
    p.unitScores().set(h.session().getGame()->planetScores().add(def), 700, 1);
    h.session().getRoot()->hostConfiguration()[game::config::HostConfiguration::NumExperienceLevels].set(4);
    h.session().getRoot()->hostConfiguration()[game::config::HostConfiguration::EPPlanetAging].set(40);

    // Testee
    PlanetPredictorProxy testee(ind, h.gameSender(), PLANET_ID);
    testee.setNumTurns(4);

    PlanetPredictorProxy::Status status;
    testee.getStatus(ind, status);

    TS_ASSERT_EQUALS(status.colonistClans.size(), 5U);
    TS_ASSERT_EQUALS(status.colonistClans[0], 1000);
    TS_ASSERT_EQUALS(status.colonistClans[1], 1042);
    TS_ASSERT_EQUALS(status.colonistClans[2], 1085);
    TS_ASSERT_EQUALS(status.colonistClans[3], 1130);
    TS_ASSERT_EQUALS(status.colonistClans[4], 1177);

    TS_ASSERT_EQUALS(status.nativeClans.size(), 5U);
    TS_ASSERT_EQUALS(status.nativeClans[0], 20000);
    TS_ASSERT_EQUALS(status.nativeClans[1], 20571);
    TS_ASSERT_EQUALS(status.nativeClans[2], 21158);
    TS_ASSERT_EQUALS(status.nativeClans[3], 21762);
    TS_ASSERT_EQUALS(status.nativeClans[4], 22383);

    TS_ASSERT_EQUALS(status.experiencePoints.size(), 5U);
    TS_ASSERT_EQUALS(status.experiencePoints[0], 700);
    TS_ASSERT_EQUALS(status.experiencePoints[1], 740);
    TS_ASSERT_EQUALS(status.experiencePoints[2], 780);
    TS_ASSERT_EQUALS(status.experiencePoints[3], 820);
    TS_ASSERT_EQUALS(status.experiencePoints[4], 860);

    TS_ASSERT_EQUALS(status.experienceLevel.size(), 5U);
    TS_ASSERT_EQUALS(status.experienceLevel[0], 0);
    TS_ASSERT_EQUALS(status.experienceLevel[1], 0);
    TS_ASSERT_EQUALS(status.experienceLevel[2], 1);
    TS_ASSERT_EQUALS(status.experienceLevel[3], 1);
    TS_ASSERT_EQUALS(status.experienceLevel[4], 1);

    TS_ASSERT_EQUALS(status.effectorLabel, "No ship effects considered");
}

/** Test update handling.
    A: create PlanetPredictorProxy on universe containing a planet. Register a sig_update handler. Modify taxes.
    E: sig_update handler must eventually report final values */
void
TestGameProxyPlanetPredictorProxy::testUpdate()
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    /*Planet& p =*/ setup(h);

    // Testee
    PlanetPredictorProxy testee(ind, h.gameSender(), PLANET_ID);

    // Signal
    UpdateReceiver up;
    testee.sig_update.add(&up, &UpdateReceiver::onUpdate);

    // Configure PlanetPredictorProxy
    testee.setNumTurns(2);
    testee.setTax(game::actions::TaxationAction::Colonists, 3);
    testee.setTax(game::actions::TaxationAction::Natives, 7);
    testee.setNumBuildings(game::FactoryBuilding, 20);
    testee.setNumBuildings(game::MineBuilding, 30);

    // Do it
    h.sync();
    ind.processQueue();

    // Verify: update must have arrived
    TS_ASSERT(up.hasUpdate());

    // Verify: update must match explicit query
    PlanetPredictorProxy::Status status;
    testee.getStatus(ind, status);
    TS_ASSERT_EQUALS(status.colonistClans, up.status().colonistClans);
    TS_ASSERT_EQUALS(status.nativeClans, up.status().nativeClans);

    TS_ASSERT_EQUALS(status.colonistClans.size(), 3U);
    TS_ASSERT_EQUALS(status.colonistClans[0], 1000);
    TS_ASSERT_EQUALS(status.colonistClans[1], 1031);
    TS_ASSERT_EQUALS(status.colonistClans[2], 1063);

    TS_ASSERT_EQUALS(status.nativeClans.size(), 3U);
    TS_ASSERT_EQUALS(status.nativeClans[0], 20000);
    TS_ASSERT_EQUALS(status.nativeClans[1], 20333);
    TS_ASSERT_EQUALS(status.nativeClans[2], 20671);
}

/** Test PlanetEffector handling.
    A: create PlanetPredictorProxy on universe containing a planet and a HeatsTo100 ship. Verify reported values.
    E: correct initial PlanetEffector reported. Values update when PlanetEffector changed. */
void
TestGameProxyPlanetPredictorProxy::testEffectors()
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    /*Planet& p =*/ setup(h);

    // Add some ships
    const int HULL_ID = 72;
    game::spec::Hull* hull = h.session().getShipList()->hulls().create(HULL_ID);
    hull->setNumEngines(1);
    hull->setMass(100);

    const int NUM_SHIPS = 5;
    for (int i = 1; i <= NUM_SHIPS; ++i) {
        game::map::Ship* ship = h.session().getGame()->currentTurn().universe().ships().create(i);

        game::map::ShipData d;
        d.x = LOC_X;
        d.y = LOC_Y;
        d.owner = 1;
        d.hullType = HULL_ID;
        ship->addCurrentShipData(d, game::PlayerSet_t(1));
        ship->addShipSpecialFunction(h.session().getShipList()->modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::HeatsTo100));
        ship->internalCheck();
        ship->setPlayability(game::map::Object::Playable);
    }

    // Testee
    PlanetPredictorProxy testee(ind, h.gameSender(), PLANET_ID);
    testee.setNumTurns(4);

    // Verify effectors
    game::map::PlanetEffectors eff = testee.getEffectors(ind);
    TS_ASSERT_EQUALS(eff.getNumTerraformers(), 5);
    TS_ASSERT_EQUALS(eff.get(eff.HeatsTo100), 5);

    // Verify status
    {
        PlanetPredictorProxy::Status status;
        testee.getStatus(ind, status);

        TS_ASSERT_EQUALS(status.colonistClans.size(), 5U);
        TS_ASSERT_EQUALS(status.colonistClans[0], 1000);
        TS_ASSERT_EQUALS(status.colonistClans[1], 1041);
        TS_ASSERT_EQUALS(status.colonistClans[2], 1082);
        TS_ASSERT_EQUALS(status.colonistClans[3], 1122);
        TS_ASSERT_EQUALS(status.colonistClans[4], 1160);
    }

    // More terraformers; verify again
    eff.add(eff.HeatsTo100, 13);
    testee.setEffectors(eff);
    {
        PlanetPredictorProxy::Status status;
        testee.getStatus(ind, status);

        TS_ASSERT_EQUALS(status.colonistClans.size(), 5U);
        TS_ASSERT_EQUALS(status.colonistClans[0], 1000);
        TS_ASSERT_EQUALS(status.colonistClans[1], 1035);
        TS_ASSERT_EQUALS(status.colonistClans[2],  931);
        TS_ASSERT_EQUALS(status.colonistClans[3],  837);
        TS_ASSERT_EQUALS(status.colonistClans[4],  753);
    }
}

