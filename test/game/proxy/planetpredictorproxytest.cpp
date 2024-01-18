/**
  *  \file test/game/proxy/planetpredictorproxytest.cpp
  *  \brief Test for game::proxy::PlanetPredictorProxy
  */

#include "game/proxy/planetpredictorproxy.hpp"

#include "afl/test/testrunner.hpp"
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
        p.internalCheck(game::map::Configuration(), game::PlayerSet_t(owner), 15, tx, log);
        p.setPlayability(game::map::Object::Playable);

        return p;
    }

    const int PLANET_ID = 42;
    const int OWNER = 3;

    Planet& setup(game::test::SessionThread& h)
    {
        // Root
        h.session().setRoot(game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,4,0))).asPtr());

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
AFL_TEST("game.proxy.PlanetPredictorProxy:empty", a)
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    PlanetPredictorProxy testee(ind, h.gameSender(), 77);

    PlanetPredictorProxy::Status status;
    testee.getStatus(ind, status);
    a.checkEqual("01. colonistClans",    status.colonistClans.size(), 0U);
    a.checkEqual("02. nativeClans",      status.nativeClans.size(), 0U);
    a.checkEqual("03. experienceLevel",  status.experienceLevel.size(), 0U);
    a.checkEqual("04. experiencePoints", status.experiencePoints.size(), 0U);
    a.checkEqual("05. effectorLabel",    status.effectorLabel, "");

    game::map::PlanetEffectors eff = testee.getEffectors(ind);
    a.checkEqual("11. getNumTerraformers", eff.getNumTerraformers(), 0);
}

/** Test normal situation.
    A: create PlanetPredictorProxy on universe containing a planet.
    E: proxy must report correct values */
AFL_TEST("game.proxy.PlanetPredictorProxy:normal", a)
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

    a.checkEqual("01. colonistClans", status.colonistClans.size(), 5U);
    a.checkEqual("02. colonistClans", status.colonistClans[0], 1000);
    a.checkEqual("03. colonistClans", status.colonistClans[1], 1042);
    a.checkEqual("04. colonistClans", status.colonistClans[2], 1085);
    a.checkEqual("05. colonistClans", status.colonistClans[3], 1130);
    a.checkEqual("06. colonistClans", status.colonistClans[4], 1177);

    a.checkEqual("11. nativeClans", status.nativeClans.size(), 5U);
    a.checkEqual("12. nativeClans", status.nativeClans[0], 20000);
    a.checkEqual("13. nativeClans", status.nativeClans[1], 20571);
    a.checkEqual("14. nativeClans", status.nativeClans[2], 21158);
    a.checkEqual("15. nativeClans", status.nativeClans[3], 21762);
    a.checkEqual("16. nativeClans", status.nativeClans[4], 22383);

    a.checkEqual("21. experiencePoints", status.experiencePoints.size(), 5U);
    a.checkEqual("22. experiencePoints", status.experiencePoints[0], 700);
    a.checkEqual("23. experiencePoints", status.experiencePoints[1], 740);
    a.checkEqual("24. experiencePoints", status.experiencePoints[2], 780);
    a.checkEqual("25. experiencePoints", status.experiencePoints[3], 820);
    a.checkEqual("26. experiencePoints", status.experiencePoints[4], 860);

    a.checkEqual("31. experienceLevel", status.experienceLevel.size(), 5U);
    a.checkEqual("32. experienceLevel", status.experienceLevel[0], 0);
    a.checkEqual("33. experienceLevel", status.experienceLevel[1], 0);
    a.checkEqual("34. experienceLevel", status.experienceLevel[2], 1);
    a.checkEqual("35. experienceLevel", status.experienceLevel[3], 1);
    a.checkEqual("36. experienceLevel", status.experienceLevel[4], 1);

    a.checkEqual("41. effectorLabel", status.effectorLabel, "No ship effects considered");
}

/** Test update handling.
    A: create PlanetPredictorProxy on universe containing a planet. Register a sig_update handler. Modify taxes.
    E: sig_update handler must eventually report final values */
AFL_TEST("game.proxy.PlanetPredictorProxy:update", a)
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
    a.check("01. hasUpdate", up.hasUpdate());

    // Verify: update must match explicit query
    PlanetPredictorProxy::Status status;
    testee.getStatus(ind, status);
    a.check("11. colonistClans", status.colonistClans == up.status().colonistClans);
    a.check("12. nativeClans",   status.nativeClans   == up.status().nativeClans);

    a.checkEqual("21. colonistClans", status.colonistClans.size(), 3U);
    a.checkEqual("22. colonistClans", status.colonistClans[0], 1000);
    a.checkEqual("23. colonistClans", status.colonistClans[1], 1031);
    a.checkEqual("24. colonistClans", status.colonistClans[2], 1063);

    a.checkEqual("31. nativeClans", status.nativeClans.size(), 3U);
    a.checkEqual("32. nativeClans", status.nativeClans[0], 20000);
    a.checkEqual("33. nativeClans", status.nativeClans[1], 20333);
    a.checkEqual("34. nativeClans", status.nativeClans[2], 20671);
}

/** Test PlanetEffector handling.
    A: create PlanetPredictorProxy on universe containing a planet and a HeatsTo100 ship. Verify reported values.
    E: correct initial PlanetEffector reported. Values update when PlanetEffector changed. */
AFL_TEST("game.proxy.PlanetPredictorProxy:effectors", a)
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
        ship->internalCheck(game::PlayerSet_t(2), 15);
        ship->setPlayability(game::map::Object::Playable);
    }

    // Testee
    PlanetPredictorProxy testee(ind, h.gameSender(), PLANET_ID);
    testee.setNumTurns(4);

    // Verify effectors
    game::map::PlanetEffectors eff = testee.getEffectors(ind);
    a.checkEqual("01. getNumTerraformers", eff.getNumTerraformers(), 5);
    a.checkEqual("02. HeatsTo100", eff.get(eff.HeatsTo100), 5);

    // Verify status
    {
        PlanetPredictorProxy::Status status;
        testee.getStatus(ind, status);

        a.checkEqual("11. colonistClans", status.colonistClans.size(), 5U);
        a.checkEqual("12. colonistClans", status.colonistClans[0], 1000);
        a.checkEqual("13. colonistClans", status.colonistClans[1], 1041);
        a.checkEqual("14. colonistClans", status.colonistClans[2], 1082);
        a.checkEqual("15. colonistClans", status.colonistClans[3], 1122);
        a.checkEqual("16. colonistClans", status.colonistClans[4], 1160);
    }

    // More terraformers; verify again
    eff.add(eff.HeatsTo100, 13);
    testee.setEffectors(eff);
    {
        PlanetPredictorProxy::Status status;
        testee.getStatus(ind, status);

        a.checkEqual("21. colonistClans", status.colonistClans.size(), 5U);
        a.checkEqual("22. colonistClans", status.colonistClans[0], 1000);
        a.checkEqual("23. colonistClans", status.colonistClans[1], 1035);
        a.checkEqual("24. colonistClans", status.colonistClans[2],  931);
        a.checkEqual("25. colonistClans", status.colonistClans[3],  837);
        a.checkEqual("26. colonistClans", status.colonistClans[4],  753);
    }
}
