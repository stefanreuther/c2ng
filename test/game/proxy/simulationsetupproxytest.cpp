/**
  *  \file test/game/proxy/simulationsetupproxytest.cpp
  *  \brief Test for game::proxy::SimulationSetupProxy
  */

#include "game/proxy/simulationsetupproxy.hpp"

#include "afl/base/staticassert.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"
#include "game/proxy/simulationadaptorfromsession.hpp"
#include "game/sim/object.hpp"                // Flag definitions
#include "game/sim/ship.hpp"
#include "game/test/counter.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/shiplist.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"

using game::test::SessionThread;
using game::test::WaitIndicator;
using game::test::Counter;
using game::proxy::SimulationAdaptorFromSession;
using game::proxy::SimulationSetupProxy;

namespace {
    void prepare(SessionThread& thread)
    {
        // Shiplist
        afl::base::Ptr<game::spec::ShipList> list = new game::spec::ShipList();
        game::test::initStandardBeams(*list);
        game::test::initStandardTorpedoes(*list);
        game::test::addOutrider(*list);
        game::test::addGorbie(*list);
        game::test::addAnnihilation(*list);
        game::test::addNovaDrive(*list);
        game::test::addTranswarp(*list);
        thread.session().setShipList(list);

        // Root
        afl::base::Ptr<game::Root> root = game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4, 0, 0))).asPtr();
        game::Player* p1 = root->playerList().create(1);
        p1->setName(game::Player::ShortName, "The Federation");
        p1->setName(game::Player::AdjectiveName, "Federal");
        game::Player* p2 = root->playerList().create(2);
        p2->setName(game::Player::ShortName, "The Lizards");
        p2->setName(game::Player::AdjectiveName, "Lizard");
        thread.session().setRoot(root);
    }

    void prepareFriendlyCodes(SessionThread& thread)
    {
        afl::string::NullTranslator tx;
        game::spec::FriendlyCodeList& list = thread.session().getShipList()->friendlyCodes();
        list.addCode(game::spec::FriendlyCode("sc1", "s,ship code",   tx));
        list.addCode(game::spec::FriendlyCode("plc", "p,planet code", tx));
        list.addCode(game::spec::FriendlyCode("sc2", "s,ship code 2", tx));
    }

    void preparePlanetNames(SessionThread& thread)
    {
        // Since we're using a game::sim::Session derived from a game::Session,
        // game::sim::SessionExtra will have connected it with the game universe.
        // To see planet names, we need to add real planets.
        afl::base::Ptr<game::Game> g = new game::Game();
        game::map::Universe& univ = g->currentTurn().universe();
        univ.planets().create(1)->setName("One");
        univ.planets().create(5)->setName("Five");
        univ.planets().create(51)->setName("Fifty-One");
        thread.session().setGame(g);
    }

    void prepareUniverse(SessionThread& thread)
    {
        // Similar to preparePlanetNames; just satisfy Id range preconditions.
        afl::base::Ptr<game::Game> g = new game::Game();
        game::map::Universe& univ = g->currentTurn().universe();
        univ.planets().create(444);
        univ.ships().create(333);
        thread.session().setGame(g);
    }

    void preparePlayedShip(SessionThread& thread, int shipId)
    {
        afl::base::Ptr<game::Game> g = new game::Game();
        game::map::Universe& univ = g->currentTurn().universe();

        game::map::ShipData sd;
        sd.owner = 4;
        sd.hullType = game::test::OUTRIDER_HULL_ID;
        sd.x = 2000;
        sd.y = 2000;
        sd.engineType = 5;
        sd.beamType = 7;
        sd.numBeams = 1;
        sd.torpedoType = 0;
        sd.numLaunchers = 0;
        sd.ammo = 0;
        sd.friendlyCode = "abc";
        sd.name = "The Ship";

        game::map::Ship* sh = univ.ships().create(shipId);
        sh->addCurrentShipData(sd, game::PlayerSet_t(4));
        sh->internalCheck(game::PlayerSet_t(4), 10);
        sh->setPlayability(game::map::Object::Playable);

        thread.session().setGame(g);
    }

    void prepareAlliances(SessionThread& thread)
    {
        afl::base::Ptr<game::Game> g = new game::Game();
        g->setViewpointPlayer(3);
        g->teamSettings().setPlayerTeam(4, 3);
        thread.session().setGame(g);
    }

    void makeHullCloakable(SessionThread& thread, int hullId)
    {
        game::spec::ShipList& list = *thread.session().getShipList();
        list.hulls().get(hullId)->changeHullFunction(
            list.modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::Cloak),
            game::PlayerSet_t::allUpTo(20),
            game::PlayerSet_t(),
            true);
    }

    void assignHull(SessionThread& thread, int player, int slot, int hullId)
    {
        thread.session().getShipList()->hullAssignments().add(player, slot, hullId);
    }

    struct Observer {
     public:
        Observer()
            : m_slot(), m_info()
            { }
        void onObjectChange(SimulationSetupProxy::Slot_t slot, const SimulationSetupProxy::ObjectInfo& info)
            {
                m_slot = slot;
                m_info = info;
            }
        SimulationSetupProxy::Slot_t getSlot() const
            { return m_slot; }
        const SimulationSetupProxy::ObjectInfo& info() const
            { return m_info; }
     private:
        SimulationSetupProxy::Slot_t m_slot;
        SimulationSetupProxy::ObjectInfo m_info;
    };
}

/** Test behaviour on uninitialized session.
    A: create blank session. Create SimulationSetupProxy.
    E: reports empty list, no object (no crash). */
AFL_TEST("game.proxy.SimulationSetupProxy:uninitialized", a)
{
    SessionThread thread;
    WaitIndicator ind;
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Check list
    SimulationSetupProxy::ListItems_t list;
    t.getList(ind, list);
    a.checkEqual("01. size", list.size(), 0U);

    // Check object
    SimulationSetupProxy::ObjectInfo obj;
    bool ok = t.getObject(ind, 0, obj);
    a.checkEqual("11. getObject", ok, false);
}

/** Test behaviour on empty setup.
    A: create session with ship list. Create SimulationSetupProxy.
    E: reports empty list, no object (no crash). */
AFL_TEST("game.proxy.SimulationSetupProxy:empty", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Check list
    SimulationSetupProxy::ListItems_t list;
    t.getList(ind, list);
    a.checkEqual("01. size", list.size(), 0U);

    // Check object
    SimulationSetupProxy::ObjectInfo obj;
    bool ok = t.getObject(ind, 0, obj);
    a.checkEqual("11. getObject", ok, false);
}

/** Test addShip().
    A: create session. Call addShip().
    E: reports sig_listChange. Reports correct slot number. */
AFL_TEST("game.proxy.SimulationSetupProxy:addShip", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);
    thread.sync();

    Counter c;
    t.sig_listChange.add(&c, &Counter::increment);

    // Add ship on empty list -> slot 0
    SimulationSetupProxy::Slot_t s1 = t.addShip(ind, 0, 1);
    a.checkEqual("01. addShip", s1, 0U);
    a.check("02. signal", c.get() > 0);

    // Add ship by cloning slot 0 -> slot 1
    SimulationSetupProxy::Slot_t s2 = t.addShip(ind, 0, 1);
    a.checkEqual("11. addShip", s2, 1U);

    // Verify list
    SimulationSetupProxy::ListItems_t list;
    t.getList(ind, list);
    a.checkEqual("21. size", list.size(), 2U);
    a.checkEqual("22. id",       list[0].id, 1);
    a.checkEqual("23. isPlanet", list[0].isPlanet, false);
    a.checkEqual("24. name",     list[0].name, "Ship 1");
    a.checkEqual("25. info",     list[0].info, "Player 12 custom ship");
    a.checkEqual("26. id",       list[1].id, 2);
    a.checkEqual("27. isPlanet", list[1].isPlanet, false);
    a.checkEqual("28. name",     list[1].name, "Ship 2");
    a.checkEqual("29. info",     list[1].info, "Player 12 custom ship");

    // Verify content
    // Ship must have valid beams/engines
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("31. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("32. beam type", oi.beamType.first, 10);
    a.checkEqual("33. beam name", oi.beamType.second, "Heavy Phaser");
    a.checkEqual("34. engine type", oi.engineType.first, 9);
    a.checkEqual("35. engine name", oi.engineType.second, "Transwarp Drive");
}

/** Test addPlanet().
    A: create session. Call addPlanet().
    E: reports sig_listChange. Reports correct slot number (always last). */
AFL_TEST("game.proxy.SimulationSetupProxy:addPlanet", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    Counter c;
    t.sig_listChange.add(&c, &Counter::increment);

    // Add planet on empty list -> slot 0
    SimulationSetupProxy::Slot_t p = t.addPlanet(ind);
    a.checkEqual("01. addPlanet", p, 0U);
    a.check("02. counter", c.get() > 0);

    // Adding planet after ships will still report last Id
    t.addShip(ind, 0, 7);
    p = t.addPlanet(ind);
    a.checkEqual("11. addPlanet", p, 7U);

    // Verify list
    SimulationSetupProxy::ListItems_t list;
    t.getList(ind, list);
    a.checkEqual("21. size", list.size(), 8U);
    a.checkEqual("22. id",       list[7].id, 1);
    a.checkEqual("23. isPlanet", list[7].isPlanet, true);
    a.checkEqual("24. name",     list[7].name, "?");
    a.checkEqual("25. info",     list[7].info, "Player 12 planet");
}

/** Test swapShips().
    A: create session. Add some ships. Call swapShips().
    E: verify list content */
AFL_TEST("game.proxy.SimulationSetupProxy:swapShips", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add ships; this will produce sequence 1,2,3,4,5
    t.addShip(ind, 0, 5);

    // Swap some; this will produce sequence 1,2,4,3,5
    t.swapShips(2, 3);

    // Verify list
    SimulationSetupProxy::ListItems_t list;
    t.getList(ind, list);
    a.checkEqual("01. size", list.size(), 5U);
    a.checkEqual("02. id", list[0].id, 1);
    a.checkEqual("03. id", list[1].id, 2);
    a.checkEqual("04. id", list[2].id, 4);
    a.checkEqual("05. id", list[3].id, 3);
    a.checkEqual("06. id", list[4].id, 5);
}

/** Test removeObject().
    A: create session. Add some ships and a planet. Call removeObject().
    E: verify list content */
AFL_TEST("game.proxy.SimulationSetupProxy:removeObject", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add some units; this will produce sequence 1,2,3,4,5,p
    t.addShip(ind, 0, 5);
    t.addPlanet(ind);

    // Remove some units
    t.removeObject(2);
    t.removeObject(4);

    // Verify list
    SimulationSetupProxy::ListItems_t list;
    t.getList(ind, list);
    a.checkEqual("01. size", list.size(), 4U);
    a.checkEqual("02. id", list[0].id, 1);
    a.checkEqual("03. id", list[1].id, 2);
    a.checkEqual("04. id", list[2].id, 4);
    a.checkEqual("05. id", list[3].id, 5);
}

/** Test removeObject().
    A: create session. Add some ships and a planet. Call clear().
    E: verify list content */
AFL_TEST("game.proxy.SimulationSetupProxy:clear", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add some units; this will produce sequence 1,2,3,4,5,p
    t.addShip(ind, 0, 5);
    t.addPlanet(ind);

    // Clear
    t.clear();

    // Verify list
    SimulationSetupProxy::ListItems_t list;
    t.getList(ind, list);
    a.checkEqual("01. size", list.size(), 0U);
}

/** Test getObject().
    A: create session. Add ship and planet. Call getObject() for both.
    E: verify object content */
AFL_TEST("game.proxy.SimulationSetupProxy:getObject", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add units
    t.addShip(ind, 0, 1);
    t.addPlanet(ind);

    // Verify ship
    SimulationSetupProxy::ObjectInfo si;
    a.checkEqual("01. getObject", t.getObject(ind, 0, si), true);
    a.checkEqual("02. isPlanet",                si.isPlanet, false);
    a.checkEqual("03. id",                      si.id, 1);
    a.checkEqual("04. name",                    si.name, "Ship 1");
    a.checkEqual("05. friendlyCode",            si.friendlyCode, "?""?""?");
    a.checkEqual("06. damage",                  si.damage, 0);
    a.checkEqual("07. shield",                  si.shield, 100);
    a.checkEqual("08. owner",                   si.owner.first, 12);
    a.checkEqual("09. owner name",              si.owner.second, "Player 12");
    a.checkEqual("10. defaultFlakRating",       si.defaultFlakRating, 110);
    a.checkEqual("11. defaultFlakCompensation", si.defaultFlakCompensation, 30);

    // Verify planet
    SimulationSetupProxy::ObjectInfo pi;
    a.checkEqual("21. getObject", t.getObject(ind, 1, pi), true);
    a.checkEqual("22. isPlanet",                pi.isPlanet, true);
    a.checkEqual("23. id",                      pi.id, 1);
    a.checkEqual("24. name",                    pi.name, "?");
    a.checkEqual("25. friendlyCode",            pi.friendlyCode, "?""?""?");
    a.checkEqual("26. damage",                  pi.damage, 0);
    a.checkEqual("27. shield",                  pi.shield, 100);
    a.checkEqual("28. owner",                   pi.owner.first, 12);
    a.checkEqual("29. owner name",              pi.owner.second, "Player 12");
    a.checkEqual("30. defaultFlakRating",       pi.defaultFlakRating, 0);
    a.checkEqual("31. defaultFlakCompensation", pi.defaultFlakCompensation, 0);
}

/** Test getObject().
    A: create session. Add some ships and a planet. Call isDuplicateId() to verify Ids.
    E: correct results returned. */
AFL_TEST("game.proxy.SimulationSetupProxy:isDuplicateId", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add some units; this will produce sequence 1,2,3,4,5,p
    t.addShip(ind, 0, 5);
    t.addPlanet(ind);

    // Verify second ship: can have Ids 2 and 6+
    a.checkEqual("01", t.isDuplicateId(ind, 1, 1), true);
    a.checkEqual("02", t.isDuplicateId(ind, 1, 2), false);
    a.checkEqual("03", t.isDuplicateId(ind, 1, 3), true);
    a.checkEqual("04", t.isDuplicateId(ind, 1, 6), false);
    a.checkEqual("05", t.isDuplicateId(ind, 1, 66), false);

    // Verify planet: can have any Id
    a.checkEqual("11", t.isDuplicateId(ind, 5, 1), false);
    a.checkEqual("12", t.isDuplicateId(ind, 5, 2), false);
    a.checkEqual("13", t.isDuplicateId(ind, 5, 3), false);
    a.checkEqual("14", t.isDuplicateId(ind, 5, 6), false);
    a.checkEqual("15", t.isDuplicateId(ind, 5, 66), false);
}

/** Test getNumBaseTorpedoes().
    A: create session. Add a planet and configure some torpedoes. Call getNumBaseTorpedoes().
    E: correct results returned. */
AFL_TEST("game.proxy.SimulationSetupProxy:getNumBaseTorpedoes", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add base
    t.addPlanet(ind);
    t.setBaseBeamTech(0, 4);
    t.setBaseTorpedoTech(0, 3);

    // Set
    SimulationSetupProxy::Elements_t es;
    es.push_back(SimulationSetupProxy::Element_t(0, ""));
    es.push_back(SimulationSetupProxy::Element_t(0, ""));
    es.push_back(SimulationSetupProxy::Element_t(55, ""));
    es.push_back(SimulationSetupProxy::Element_t(0, ""));
    es.push_back(SimulationSetupProxy::Element_t(66, ""));
    t.setNumBaseTorpedoes(0, es);

    // Retrieve list
    SimulationSetupProxy::Elements_t result;
    t.getNumBaseTorpedoes(ind, 0, result);

    // Verify
    a.checkEqual("01. size", result.size(), 10U);
    a.checkEqual("02. num",  result[0].first, 0);
    a.checkEqual("03. name", result[0].second, "Mark 1 Photon");
    a.checkEqual("04. num",  result[2].first, 55);
    a.checkEqual("05. name", result[2].second, "Mark 2 Photon");
    a.checkEqual("06. num",  result[4].first, 66);
    a.checkEqual("07. name", result[4].second, "Mark 3 Photon");
}

/** Test setFlags().
    A: create session. Add a ship. Call setFlags() with various parameters.
    E: verify updated object. */
AFL_TEST("game.proxy.SimulationSetupProxy:setFlags", a)
{
    // Flags for testing that have no intrinsic logic
    const int32_t F1 = game::sim::Object::fl_RatingOverride;
    const int32_t F2 = game::sim::Object::fl_RandomFC;

    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add a ship
    t.addShip(ind, 0, 1);

    // Clear all flags
    SimulationSetupProxy::ObjectInfo oi;
    t.setFlags(0, 0, 0);
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. flags", oi.flags, 0);

    // Set flag
    t.setFlags(0, 0, F1);
    a.checkEqual("11. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("12. flags", oi.flags, F1);

    // Set another flag
    t.setFlags(0, ~F2, F2);
    a.checkEqual("21. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("22. flags", oi.flags, F1 + F2);

    // Toggle a flag
    t.setFlags(0, ~0, F1);
    a.checkEqual("31. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("32. flags", oi.flags, F2);
}

/** Test toggleDisabled().
    A: create session. Add a ship. Call toggleDisabled().
    E: verify updated object and list. */
AFL_TEST("game.proxy.SimulationSetupProxy:toggleDisabled", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add a ship
    t.addShip(ind, 0, 1);

    // Verify object and list
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. flags", oi.flags & game::sim::Object::fl_Deactivated, 0);

    SimulationSetupProxy::ListItems_t list;
    t.getList(ind, list);
    a.checkEqual("11. size", list.size(), 1U);
    a.checkEqual("12. disabled", list[0].disabled, false);

    // Toggle
    t.toggleDisabled(0);
    a.checkEqual("21. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("22. flags", oi.flags & game::sim::Object::fl_Deactivated, game::sim::Object::fl_Deactivated);

    list.clear();
    t.getList(ind, list);
    a.checkEqual("31. size", list.size(), 1U);
    a.checkEqual("32. disabled", list[0].disabled, true);
}

/** Test toggleCloak().
    A: create session. Add a ship. Call toggleCloak().
    E: verify updated object. Enabling cloak will cancel "Kill". */
AFL_TEST("game.proxy.SimulationSetupProxy:toggleCloak", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add a ship
    t.addShip(ind, 0, 1);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. aggressiveness", oi.aggressiveness.first, game::sim::Ship::agg_Kill);
    a.checkEqual("03. flags", oi.flags & game::sim::Object::fl_Cloaked, 0);

    // Toggle
    t.toggleCloak(0);

    // Verify
    a.checkEqual("11. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("12. aggressiveness", oi.aggressiveness.first, game::sim::Ship::agg_Passive);
    a.checkEqual("13. flags", oi.flags & game::sim::Object::fl_Cloaked, game::sim::Object::fl_Cloaked);
}

/** Test toggleRandomFriendlyCode().
    A: create session. Add a ship. Call toggleRandomFriendlyCode().
    E: verify updated object. */
AFL_TEST("game.proxy.SimulationSetupProxy:toggleRandomFriendlyCode", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add a ship
    t.addShip(ind, 0, 1);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. flags", oi.flags & game::sim::Object::fl_RandomFC, 0);

    // Toggle
    t.toggleRandomFriendlyCode(0);

    // Verify
    a.checkEqual("11. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("12. flags", oi.flags & game::sim::Object::fl_RandomFC, game::sim::Object::fl_RandomFC);
}

/** Test setAbilities().
    A: create session. Add a ship. Call setAbilities().
    E: verify updated object. */
AFL_TEST("game.proxy.SimulationSetupProxy:setAbilities", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add a ship
    t.addShip(ind, 0, 1);

    // Set some abilities
    {
        SimulationSetupProxy::AbilityChoices a;
        // - explicitly set 3xBeam to off
        a.available += game::sim::TripleBeamKillAbility;
        a.set       += game::sim::TripleBeamKillAbility;
        // - explicitly set Commander to on
        a.available += game::sim::CommanderAbility;
        a.set       += game::sim::CommanderAbility;
        a.active    += game::sim::CommanderAbility;
        // - do not modify 2xBeam (set/active is ignored if available is not set)
        a.set       += game::sim::DoubleBeamChargeAbility;
        t.setAbilities(0, a);
    }

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. flags", oi.flags, game::sim::Object::fl_TripleBeamKillSet + game::sim::Object::fl_CommanderSet + game::sim::Object::fl_Commander);

    // Clear Commander (reset to default) by mentioning it in available, but not as set/active.
    {
        SimulationSetupProxy::AbilityChoices a;
        a.available += game::sim::CommanderAbility;
        t.setAbilities(0, a);
    }

    // Verify
    a.checkEqual("11. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("12. flags", oi.flags, game::sim::Object::fl_TripleBeamKillSet);
}

/** Test setSequentialFriendlyCode().
    A: create session. Add some ships. Call setSequentialFriendlyCode().
    E: verify sequential friendly codes. */
AFL_TEST("game.proxy.SimulationSetupProxy:setSequentialFriendlyCode", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add ships, set FC on first
    t.addShip(ind, 0, 3);
    t.setFriendlyCode(0, "150");

    // Call setSequentialFriendlyCode()
    t.setSequentialFriendlyCode(1);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 1, oi), true);
    a.checkEqual("02. friendlyCode", oi.friendlyCode, "151");
}

/** Test setId().
    A: create session. Add a ship and planet. Call setId().
    E: verify updated object and list. */
AFL_TEST("game.proxy.SimulationSetupProxy:setId", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add a ship
    t.addShip(ind, 0, 1);
    t.addPlanet(ind);

    // Set
    t.setId(0, 77);
    t.setId(1, 33);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. id", oi.id, 77);
    a.checkEqual("03. name", oi.name, "Ship 77");

    a.checkEqual("11. getObject", t.getObject(ind, 1, oi), true);
    a.checkEqual("12. id", oi.id, 33);

    SimulationSetupProxy::ListItems_t list;
    t.getList(ind, list);
    a.checkEqual("21. size", list.size(), 2U);
    a.checkEqual("22. id", list[0].id, 77);
    a.checkEqual("23. id", list[1].id, 33);
}

/** Test setName().
    A: create session. Add a ship. Call setName().
    E: verify updated object and list. */
AFL_TEST("game.proxy.SimulationSetupProxy:setName", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add a ship
    t.addShip(ind, 0, 1);

    // Set
    t.setName(0, "Titanic");

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. name", oi.name, "Titanic");

    SimulationSetupProxy::ListItems_t list;
    t.getList(ind, list);
    a.checkEqual("11. size", list.size(), 1U);
    a.checkEqual("12. name", list[0].name, "Titanic");
}

/** Test setFriendlyCode().
    A: create session. Add a ship. Call setFriendlyCode().
    E: verify updated object. */
AFL_TEST("game.proxy.SimulationSetupProxy:setFriendlyCode", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add a ship
    t.addShip(ind, 0, 1);

    // Set
    t.setFriendlyCode(0, "ijk");

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. friendlyCode", oi.friendlyCode, "ijk");

    // Set code with '#'
    t.setFriendlyCode(0, "a#b");

    // Verify
    a.checkEqual("11. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("12. friendlyCode", oi.friendlyCode, "a#b");
    a.checkEqual("13. flags", oi.flags, game::sim::Object::fl_RandomFC | game::sim::Object::fl_RandomFC2);
}

/** Test setDamage().
    A: create session. Add a ship. Call setDamage().
    E: verify updated object. Shield automatically downgraded. */
AFL_TEST("game.proxy.SimulationSetupProxy:setDamage", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add a ship
    t.addShip(ind, 0, 1);

    // Set
    t.setDamage(0, 23);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. damage", oi.damage, 23);
    a.checkEqual("03. shield", oi.shield, 77);
}

/** Test setShield().
    A: create session. Add a ship. Call setShield().
    E: verify updated object. */
AFL_TEST("game.proxy.SimulationSetupProxy:setShield", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add a ship
    t.addShip(ind, 0, 1);

    // Set
    t.setShield(0, 95);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. shield", oi.shield, 95);
}

/** Test setOwner().
    A: create session. Add a ship. Call setOwner().
    E: verify updated object and list. */
AFL_TEST("game.proxy.SimulationSetupProxy:setOwner", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add a ship
    t.addShip(ind, 0, 1);

    // Set
    t.setOwner(0, 2);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. owner", oi.owner.first, 2);
    a.checkEqual("03. name", oi.owner.second, "The Lizards");

    SimulationSetupProxy::ListItems_t list;
    t.getList(ind, list);
    a.checkEqual("11. size", list.size(), 1U);
    a.checkEqual("12. info", list[0].info, "Lizard custom ship");

    // Set damage; change owner back. This will limit the damage.
    t.setDamage(0, 140);
    t.setOwner(0, 1);

    a.checkEqual("21. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("22. damage", oi.damage, 99);
}

/** Test setExperienceLevel().
    A: create session. Add a ship. Call setExperienceLevel().
    E: verify updated object. */
AFL_TEST("game.proxy.SimulationSetupProxy:setExperienceLevel", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add a ship
    t.addShip(ind, 0, 1);

    // Set
    t.setExperienceLevel(0, 4);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. level", oi.experienceLevel.first, 4);
    a.checkEqual("03. level name", oi.experienceLevel.second, "Ultra Elite");
}

/** Test setFlakRatingOverride().
    A: create session. Add a ship. Call setFlakRatingOverride().
    E: verify updated object. */
AFL_TEST("game.proxy.SimulationSetupProxy:setFlakRatingOverride", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add a ship
    t.addShip(ind, 0, 1);

    // Set
    t.setFlakRatingOverride(0, 222);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. flakRatingOverride", oi.flakRatingOverride, 222);
}

/** Test setFlakCompensationOverride().
    A: create session. Add a ship. Call setFlakCompensationOverride().
    E: verify updated object. */
AFL_TEST("game.proxy.SimulationSetupProxy:setFlakCompensationOverride", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add a ship
    t.addShip(ind, 0, 1);

    // Set
    t.setFlakCompensationOverride(0, 7777);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. flakCompensationOverride", oi.flakCompensationOverride, 7777);
}

/** Test setCrew().
    A: create session. Add a ship. Call setCrew().
    E: verify updated object. */
AFL_TEST("game.proxy.SimulationSetupProxy:setCrew", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add a ship
    t.addShip(ind, 0, 1);

    // Set
    t.setCrew(0, 4);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. crew", oi.crew, 4);
}

/** Test setHullType().
    A: create session. Add a ship. Call setHullType().
    E: verify updated object and list. */
AFL_TEST("game.proxy.SimulationSetupProxy:setHullType", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add a ship
    t.addShip(ind, 0, 1);

    // Set
    t.setHullType(0, game::test::OUTRIDER_HULL_ID, false);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. hull type", oi.hullType.first, game::test::OUTRIDER_HULL_ID);
    a.checkEqual("03. hull name", oi.hullType.second, "OUTRIDER CLASS SCOUT");
    a.checkEqual("04. numBeams", oi.numBeams, 1);
    a.checkEqual("05. numLaunchers", oi.numLaunchers, 0);
    a.checkEqual("06. numBays", oi.numBays, 0);
    a.checkEqual("07. hullPicture", oi.hullPicture, 9);

    SimulationSetupProxy::ListItems_t list;
    t.getList(ind, list);
    a.checkEqual("11. size", list.size(), 1U);
    a.checkEqual("12. info", list[0].info, "Player 12 OUTRIDER CLASS SCOUT");
}

/** Test setHullType(), after add interaction.
    A: create session. Add a ship. Call setHullType(afterAdd=true).
    E: verify updated object. */
AFL_TEST("game.proxy.SimulationSetupProxy:setHullType:after-add", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    // Only player 1 can build Outriders
    assignHull(thread, 1, 1, game::test::OUTRIDER_HULL_ID);
    // 3+4 can build Gorbies
    assignHull(thread, 3, 1, game::test::GORBIE_HULL_ID);
    assignHull(thread, 4, 1, game::test::GORBIE_HULL_ID);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add a ship
    t.addShip(ind, 0, 1);
    t.setFriendlyCode(0, "xxx");
    t.setAggressiveness(0, 1);

    // Exercise default-owner case
    t.setHullType(0, game::test::OUTRIDER_HULL_ID, true);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. hullType", oi.hullType.first, game::test::OUTRIDER_HULL_ID);
    a.checkEqual("03. owner", oi.owner.first, 1);
    a.checkEqual("04. friendlyCode", oi.friendlyCode, "?""?""?");
    a.checkEqual("05. aggressiveness", oi.aggressiveness.first, game::sim::Ship::agg_Kill);

    // Exercise cannot-build case
    t.setFriendlyCode(0, "222");
    t.setHullType(0, game::test::GORBIE_HULL_ID, true);

    // Verify
    a.checkEqual("11. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("12. hullType", oi.hullType.first, game::test::GORBIE_HULL_ID);
    a.checkEqual("13. owner", oi.owner.first, 1);
    a.checkEqual("14. friendlyCode", oi.friendlyCode, "?""?""?");
    a.checkEqual("15. aggressiveness", oi.aggressiveness.first, game::sim::Ship::agg_Kill);
}

/** Test setHullType(), after add interaction, cloak.
    A: create session. Add some ships and cloak them. Call setHullType(afterAdd=true).
    E: verify updated object. */
AFL_TEST("game.proxy.SimulationSetupProxy:setHullType:after-add:cloak", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    makeHullCloakable(thread, game::test::GORBIE_HULL_ID);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add ships
    t.addShip(ind, 0, 2);
    t.setFlags(0, 0, game::sim::Object::fl_Cloaked);
    t.setFlags(1, 0, game::sim::Object::fl_Cloaked);

    // Convert first to Gorbie, second to Outrider
    t.setHullType(0, game::test::GORBIE_HULL_ID, true);
    t.setHullType(1, game::test::OUTRIDER_HULL_ID, true);

    // First still cloaked, second one isn't
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. flags", oi.flags, game::sim::Object::fl_Cloaked);

    a.checkEqual("11. getObject", t.getObject(ind, 1, oi), true);
    a.checkEqual("12. flags", oi.flags, 0);

    // Convert first back to custom. Cloak remains.
    t.setHullType(0, game::test::GORBIE_HULL_ID, 0);
    a.checkEqual("21. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("22. flags", oi.flags, game::sim::Object::fl_Cloaked);
}

/** Test setHullType(), after add interaction, damage.
    A: create session. Add a ship owned by lizard with excess damage. Call setHullType(afterAdd=true) to change to fed-only ship.
    E: verify updated object. Ship must be owned by Fed, damage limited. */
AFL_TEST("game.proxy.SimulationSetupProxy:setHullType:after-add:damage", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    // Only player 1 can build Outriders
    assignHull(thread, 1, 1, game::test::OUTRIDER_HULL_ID);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add a ship
    t.addShip(ind, 0, 1);
    t.setOwner(0, 2);
    t.setDamage(0, 140);

    // Set type to Outrider.
    t.setHullType(0, game::test::OUTRIDER_HULL_ID, true);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. hullType", oi.hullType.first, game::test::OUTRIDER_HULL_ID);
    a.checkEqual("03. owner", oi.owner.first, 1);
    a.checkEqual("04. damage", oi.damage, 99);
}

/** Test setHullType(), after add interaction, self-agression avoidance.
    A: create session. Add a cloaked ship with PE Fed. Change type to cloakable fed-only ship.
    E: verify updated object. Ship must be owned by Fed, no longer cloaked, Kill mission. */
AFL_TEST("game.proxy.SimulationSetupProxy:setHullType:after-add:self-aggression", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    // Only player 1 can build Outriders, Outrider can cloak.
    assignHull(thread, 1, 1, game::test::OUTRIDER_HULL_ID);
    makeHullCloakable(thread, game::test::OUTRIDER_HULL_ID);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add a ship
    t.addShip(ind, 0, 1);
    t.setOwner(0, 7);
    t.setFlags(0, 0, game::sim::Object::fl_Cloaked);
    t.setAggressiveness(0, 1);

    // Set type to Outrider.
    t.setHullType(0, game::test::OUTRIDER_HULL_ID, true);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. hullType", oi.hullType.first, game::test::OUTRIDER_HULL_ID);
    a.checkEqual("03. owner", oi.owner.first, 1);
    a.checkEqual("04. aggressiveness", oi.aggressiveness.first, game::sim::Ship::agg_Kill);
}

/** Test setMass().
    A: create session. Add a ship. Call setMass().
    E: verify updated object. */
AFL_TEST("game.proxy.SimulationSetupProxy:setMass", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add a ship
    t.addShip(ind, 0, 1);

    // Set
    t.setMass(0, 333);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. mass", oi.mass, 333);
}

/** Test setBeamType(), setNumBeams().
    A: create session. Add a ship. Add beams.
    E: verify updated object. */
AFL_TEST("game.proxy.SimulationSetupProxy:setBeamType", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add a ship
    t.addShip(ind, 0, 1);

    // Set
    t.setBeamType(0, 4);
    t.setNumBeams(0, 7);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. numBeams", oi.numBeams, 7);
    a.checkEqual("03. beam type", oi.beamType.first, 4);
    a.checkEqual("04. beam name", oi.beamType.second, "Blaster");
}

/** Test setTorpedoType(), setNumLaunchers(), setAmmo().
    A: create session. Add a ship. Add torpedoes.
    E: verify updated object. */
AFL_TEST("game.proxy.SimulationSetupProxy:setTorpedoType", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add a ship
    t.addShip(ind, 0, 1);

    // Set
    t.setTorpedoType(0, 3);
    t.setNumLaunchers(0, 8);
    t.setAmmo(0, 111);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. numLaunchers", oi.numLaunchers, 8);
    a.checkEqual("03. torp type", oi.torpedoType.first, 3);
    a.checkEqual("04. torp name", oi.torpedoType.second, "Mark 2 Photon");
    a.checkEqual("05. ammo", oi.ammo, 111);
}

/** Test setNumBays(), setAmmo().
    A: create session. Add a ship. Add fighters.
    E: verify updated object. */
AFL_TEST("game.proxy.SimulationSetupProxy:setNumBays", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add a ship
    t.addShip(ind, 0, 1);

    // Set
    t.setNumBays(0, 6);
    t.setAmmo(0, 99);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. numBays", oi.numBays, 6);
    a.checkEqual("03. ammo", oi.ammo, 99);
}

/** Test setEngineType().
    A: create session. Add a ship. Call setEngineType().
    E: verify updated object. */
AFL_TEST("game.proxy.SimulationSetupProxy:setEngineType", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add a ship
    t.addShip(ind, 0, 1);

    // Set
    t.setEngineType(0, 5);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. engine type", oi.engineType.first, 5);
    a.checkEqual("03. engine name", oi.engineType.second, "Nova Drive 5");
}

/** Test setAggressiveness().
    A: create session. Add a ship. Call setAggressiveness().
    E: verify updated object. */
AFL_TEST("game.proxy.SimulationSetupProxy:setAggressiveness", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add a ship
    t.addShip(ind, 0, 1);

    // Set
    t.setAggressiveness(0, 1);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. aggressiveness", oi.aggressiveness.first, 1);
    a.checkEqual("03. aggressiveness name", oi.aggressiveness.second, "Primary Enemy The Federation");
}

/** Test setAggressiveness(), cloak/intercept interaction.
    A: create session. Add ship and cloak them/make them intercept. Call setAggressiveness().
    E: verify updated object. */
AFL_TEST("game.proxy.SimulationSetupProxy:setAggressiveness:interaction", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add ships
    t.addShip(ind, 0, 2);
    t.setFlags(0, 0, game::sim::Object::fl_Cloaked);
    t.setInterceptId(1, 44);

    // Set
    t.setAggressiveness(0, game::sim::Ship::agg_Kill);
    t.setAggressiveness(1, game::sim::Ship::agg_NoFuel);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. aggressiveness", oi.aggressiveness.first, game::sim::Ship::agg_Kill);
    a.checkEqual("03. flags", oi.flags, 0);

    a.checkEqual("11. getObject", t.getObject(ind, 1, oi), true);
    a.checkEqual("12. aggressiveness", oi.aggressiveness.first, game::sim::Ship::agg_NoFuel);
    a.checkEqual("13. interceptId", oi.interceptId.first, 0);
}

/** Test setInterceptId().
    A: create session. Add a ship. Call setInterceptId().
    E: verify updated object. */
AFL_TEST("game.proxy.SimulationSetupProxy:setInterceptId", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add a ship
    t.addShip(ind, 0, 2);
    t.setName(0, "One");
    t.setName(1, "Two");

    // Set
    t.setInterceptId(0, 99);
    t.setInterceptId(1, 1);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. intercept id", oi.interceptId.first, 99);
    a.checkEqual("03. intercept name", oi.interceptId.second, "Ship #99");

    a.checkEqual("11. getObject", t.getObject(ind, 1, oi), true);
    a.checkEqual("12. intercept id", oi.interceptId.first, 1);
    a.checkEqual("13. intercept name", oi.interceptId.second, "One (#1)");
}

/** Test setDefense().
    A: create session. Add a planet. Call setDefense().
    E: verify updated object. */
AFL_TEST("game.proxy.SimulationSetupProxy:setDefense", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add planet
    t.addPlanet(ind);

    // Set
    t.setDefense(0, 34);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. defense", oi.defense, 34);
}

/** Test setPopulation().
    A: create session. Add a planet. Call setPopulation().
    E: verify updated object. */
AFL_TEST("game.proxy.SimulationSetupProxy:setPopulation", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add planet
    t.addPlanet(ind);

    // Set
    t.setPopulation(0, 150);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. defense", oi.defense, 60);

    // Once more
    t.setPopulation(0, 20);
    a.checkEqual("11. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("12. defense", oi.defense, 20);
}

/** Test setBaseDefense().
    A: create session. Add a starbase. Call setBaseDefense().
    E: verify updated object. */
AFL_TEST("game.proxy.SimulationSetupProxy:setBaseDefense", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add planet
    t.addPlanet(ind);
    t.setBaseBeamTech(0, 4);

    // Set
    t.setBaseDefense(0, 150);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. baseDefense", oi.baseDefense, 150);
}

/** Test setBaseBeamTech().
    A: create session. Add a starbase. Call setBaseBeamTech().
    E: verify updated object. */
AFL_TEST("game.proxy.SimulationSetupProxy:setBaseBeamTech", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add planet
    t.addPlanet(ind);

    // Set
    t.setBaseBeamTech(0, 8);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. baseBeamTech", oi.baseBeamTech, 8);
}

/** Test setBaseTorpedoTech().
    A: create session. Add a starbase. Call setBaseTorpedoTech().
    E: verify updated object. */
AFL_TEST("game.proxy.SimulationSetupProxy:setBaseTorpedoTech", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add base
    t.addPlanet(ind);
    t.setBaseBeamTech(0, 4);

    // Set
    t.setBaseTorpedoTech(0, 5);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. baseTorpedoTech", oi.baseTorpedoTech, 5);
}

/** Test setNumBaseFighters().
    A: create session. Add a starbase. Call setNumBaseFighters().
    E: verify updated object. */
AFL_TEST("game.proxy.SimulationSetupProxy:setNumBaseFighters", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add base
    t.addPlanet(ind);
    t.setBaseBeamTech(0, 4);

    // Set
    t.setNumBaseFighters(0, 55);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. numBaseFighters", oi.numBaseFighters, 55);
}

/** Test setNumBaseTorpedoes().
    A: create session. Add a starbase. Call setNumBaseTorpedoes().
    E: verify updated object. */
AFL_TEST("game.proxy.SimulationSetupProxy:setNumBaseTorpedoes", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add base
    t.addPlanet(ind);
    t.setBaseBeamTech(0, 4);
    t.setBaseTorpedoTech(0, 3);

    // Set
    SimulationSetupProxy::Elements_t es;
    es.push_back(SimulationSetupProxy::Element_t(0, ""));
    es.push_back(SimulationSetupProxy::Element_t(0, ""));
    es.push_back(SimulationSetupProxy::Element_t(123, ""));
    t.setNumBaseTorpedoes(0, es);

    // Verify
    SimulationSetupProxy::ObjectInfo oi;
    a.checkEqual("01. getObject", t.getObject(ind, 0, oi), true);
    a.checkEqual("02. effBaseTorpedoes", oi.effBaseTorpedoes, 123);
}

/** Test getAbilityChoices().
    A: create session. Add ship. Call getAbilityChoices().
    E: verify returned value */
AFL_TEST("game.proxy.SimulationSetupProxy:getAbilityChoices:ship", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add ship
    t.addShip(ind, 0, 1);
    t.setFlags(0, 0, game::sim::Object::fl_Commander + game::sim::Object::fl_CommanderSet + game::sim::Object::fl_ElusiveSet);
    t.setOwner(0, 5);     /* implies TripleBeamKill */

    // Query
    SimulationSetupProxy::AbilityChoices ac;
    t.getAbilityChoices(ind, 0, ac);

    // Verify
    // - available
    a.check("01. PlanetImmunityAbility", ac.available.contains(game::sim::PlanetImmunityAbility));
    a.check("02. CommanderAbility",      ac.available.contains(game::sim::CommanderAbility));
    a.check("03. CloakedBaysAbility",    ac.available.contains(game::sim::CloakedBaysAbility));

    // - set
    a.checkEqual("11. set", ac.set, game::sim::Abilities_t() + game::sim::CommanderAbility + game::sim::ElusiveAbility);
    a.checkEqual("12. active", ac.active, game::sim::Abilities_t() + game::sim::CommanderAbility);
    a.checkEqual("13. implied", ac.implied, game::sim::Abilities_t() + game::sim::TripleBeamKillAbility);
}

/** Test getAbilityChoices() for planet.
    A: create session. Add planet. Call getAbilityChoices().
    E: verify that only planet-specific abilities are available */
AFL_TEST("game.proxy.SimulationSetupProxy:getAbilityChoices:planet", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add ship
    t.addPlanet(ind);

    // Query
    SimulationSetupProxy::AbilityChoices ac;
    t.getAbilityChoices(ind, 0, ac);

    // Verify that only planet-specific abilities are available
    a.check("01. TripleBeamKillAbility",      ac.available.contains(game::sim::TripleBeamKillAbility));
    a.check("02. DoubleBeamChargeAbility",    ac.available.contains(game::sim::DoubleBeamChargeAbility));
    a.check("03. DoubleTorpedoChargeAbility", ac.available.contains(game::sim::DoubleTorpedoChargeAbility));
    a.check("04. CloakedBaysAbility",        !ac.available.contains(game::sim::CloakedBaysAbility));
}

/** Test getFriendlyCodeChoices().
    A: create session. Define some friendly codes. Add ship and planet. Call getFriendlyCodeChoices() for both.
    E: verify returned values */
AFL_TEST("game.proxy.SimulationSetupProxy:getFriendlyCodeChoices", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    prepareFriendlyCodes(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add ship and planet
    t.addShip(ind, 0, 1);
    t.addPlanet(ind);

    // Query ship codes
    game::spec::FriendlyCodeList::Infos_t shipInfos;
    t.getFriendlyCodeChoices(ind, 0, shipInfos);
    a.checkEqual("01. size", shipInfos.size(), 2U);
    a.checkEqual("02. code", shipInfos[0].code, "sc1");
    a.checkEqual("03. code", shipInfos[1].code, "sc2");

    // Query planet codes
    game::spec::FriendlyCodeList::Infos_t planetInfos;
    t.getFriendlyCodeChoices(ind, 1, planetInfos);
    a.checkEqual("11. size", planetInfos.size(), 1U);
    a.checkEqual("12. code", planetInfos[0].code, "plc");
}

/** Test getOwnerChoices().
    A: create session. Call getOwnerChoices().
    E: verify returned value */
AFL_TEST("game.proxy.SimulationSetupProxy:getOwnerChoices", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Query
    SimulationSetupProxy::Elements_t list;
    t.getOwnerChoices(ind, list);

    // Verify
    a.checkEqual("01. size", list.size(), 2U);
    a.checkEqual("02. id",   list[0].first, 1);
    a.checkEqual("03. name", list[0].second, "The Federation");
    a.checkEqual("04. id",   list[1].first, 2);
    a.checkEqual("05. name", list[1].second, "The Lizards");
}

/** Test getOwnerChoices().
    A: create session. Enable experience. Call getExperienceLevelChoices().
    E: verify returned value */
AFL_TEST("game.proxy.SimulationSetupProxy:getExperienceLevelChoices", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    thread.session().getRoot()->hostConfiguration()[game::config::HostConfiguration::NumExperienceLevels].set(3);
    thread.session().getRoot()->hostConfiguration()[game::config::HostConfiguration::ExperienceLevelNames].set("Noob,Intern,Apprentice,Junior,Senior");
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Query
    SimulationSetupProxy::Elements_t list;
    t.getExperienceLevelChoices(ind, list);

    // Verify
    a.checkEqual("01. size", list.size(), 4U);
    a.checkEqual("02. id",   list[0].first, 0);
    a.checkEqual("03. name", list[0].second, "Noob");
    a.checkEqual("04. id",   list[1].first, 1);
    a.checkEqual("05. name", list[1].second, "Intern");
    a.checkEqual("06. id",   list[2].first, 2);
    a.checkEqual("07. name", list[2].second, "Apprentice");
    a.checkEqual("08. id",   list[3].first, 3);
    a.checkEqual("09. name", list[3].second, "Junior");
}

/** Test getHullTypeChoices().
    A: create session. Call getHullTypeChoices().
    E: verify returned value */
AFL_TEST("game.proxy.SimulationSetupProxy:getHullTypeChoices", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Query
    SimulationSetupProxy::Elements_t list;
    t.getHullTypeChoices(ind, list);

    // Verify
    static_assert(game::test::OUTRIDER_HULL_ID < game::test::ANNIHILATION_HULL_ID, "Outrider vs Anni");
    static_assert(game::test::ANNIHILATION_HULL_ID < game::test::GORBIE_HULL_ID, "Anni vs Gorbi");
    a.checkEqual("01. size", list.size(), 4U);
    a.checkEqual("02. id",   list[0].first, 0);
    a.checkEqual("03. name", list[0].second, "Custom Ship");
    a.checkEqual("04. id",   list[1].first, game::test::OUTRIDER_HULL_ID);
    a.checkEqual("05. name", list[1].second, "OUTRIDER CLASS SCOUT");
    a.checkEqual("06. id",   list[2].first, game::test::ANNIHILATION_HULL_ID);
    a.checkEqual("07. name", list[2].second, "ANNIHILATION CLASS BATTLESHIP");
    a.checkEqual("08. id",   list[3].first, game::test::GORBIE_HULL_ID);
    a.checkEqual("09. name", list[3].second, "GORBIE CLASS BATTLECARRIER");
}

/** Test getPrimaryChoices().
    A: create session. Add ships. Call getPrimaryChoices().
    E: verify returned value */
AFL_TEST("game.proxy.SimulationSetupProxy:getPrimaryChoices", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add 2 ships, one custom, one outrider
    t.addShip(ind, 0, 2);
    t.setHullType(1, game::test::OUTRIDER_HULL_ID, false);

    // Verify both
    SimulationSetupProxy::PrimaryChoices customChoice;
    t.getPrimaryChoices(ind, 0, customChoice);
    a.checkEqual("01. beamTypes", customChoice.beamTypes.size(), 11U);
    a.checkEqual("02. beamTypes", customChoice.beamTypes[0].first, 0);
    a.checkEqual("03. beamTypes", customChoice.beamTypes[0].second, "none");
    a.checkEqual("04. beamTypes", customChoice.beamTypes[10].first, 10);
    a.checkEqual("05. beamTypes", customChoice.beamTypes[10].second, "Heavy Phaser\t(tech 10, K35, D45)");
    a.checkEqual("06. numBeams", customChoice.numBeams.min(), 0);
    a.checkEqual("07. numBeams", customChoice.numBeams.max(), 20);

    SimulationSetupProxy::PrimaryChoices outriderChoice;
    t.getPrimaryChoices(ind, 1, outriderChoice);
    a.check("11. beamTypes", outriderChoice.beamTypes == customChoice.beamTypes);
    a.checkEqual("12. numBeams", outriderChoice.numBeams.min(), 0);
    a.checkEqual("13. numBeams", outriderChoice.numBeams.max(), 1);
}

/** Test getSecondaryChoices().
    A: create session. Add ships. Call getSecondaryChoices().
    E: verify returned value */
AFL_TEST("game.proxy.SimulationSetupProxy:getSecondaryChoices", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add 4 ships: custom, outrider, anni, gorbie
    t.addShip(ind, 0, 4);
    t.setHullType(1, game::test::OUTRIDER_HULL_ID, false);
    t.setHullType(2, game::test::ANNIHILATION_HULL_ID, false);
    t.setHullType(3, game::test::GORBIE_HULL_ID, false);

    // Verify all
    SimulationSetupProxy::SecondaryChoices customChoice;
    t.getSecondaryChoices(ind, 0, customChoice);
    a.checkEqual("01. torpedoTypes", customChoice.torpedoTypes.size(), 11U);
    a.checkEqual("02. torpedoTypes", customChoice.torpedoTypes[0].first, 0);
    a.checkEqual("03. torpedoTypes", customChoice.torpedoTypes[0].second, "none");
    a.checkEqual("04. torpedoTypes", customChoice.torpedoTypes[10].first, 10);
    a.checkEqual("05. torpedoTypes", customChoice.torpedoTypes[10].second, "Mark 8 Photon\t(tech 10, K70, D110)");
    a.checkEqual("06. numLaunchers", customChoice.numLaunchers.min(), 0);
    a.checkEqual("07. numLaunchers", customChoice.numLaunchers.max(), 20);
    a.checkEqual("08. numBays", customChoice.numBays.min(), 0);
    a.checkEqual("09. numBays", customChoice.numBays.max(), 20);
    a.checkEqual("10. ammo", customChoice.ammo.min(), 0);
    a.checkEqual("11. ammo", customChoice.ammo.max(), 10000);

    SimulationSetupProxy::SecondaryChoices outriderChoice;
    t.getSecondaryChoices(ind, 1, outriderChoice);
    a.check("21. torpedoTypes", outriderChoice.torpedoTypes == customChoice.torpedoTypes);
    a.checkEqual("22. numLaunchers", outriderChoice.numLaunchers.min(), 0);
    a.checkEqual("23. numLaunchers", outriderChoice.numLaunchers.max(), 0);
    a.checkEqual("24. numBays", outriderChoice.numBays.min(), 0);
    a.checkEqual("25. numBays", outriderChoice.numBays.max(), 0);

    SimulationSetupProxy::SecondaryChoices anniChoice;
    t.getSecondaryChoices(ind, 2, anniChoice);
    a.check("31. torpedoTypes", anniChoice.torpedoTypes == customChoice.torpedoTypes);
    a.checkEqual("32. numLaunchers", anniChoice.numLaunchers.min(), 0);
    a.checkEqual("33. numLaunchers", anniChoice.numLaunchers.max(), 10);
    a.checkEqual("34. numBays", anniChoice.numBays.min(), 0);
    a.checkEqual("35. numBays", anniChoice.numBays.max(), 0);
    a.checkEqual("36. ammo", anniChoice.ammo.min(), 0);
    a.checkEqual("37. ammo", anniChoice.ammo.max(), 320);

    SimulationSetupProxy::SecondaryChoices gorbieChoice;
    t.getSecondaryChoices(ind, 3, gorbieChoice);
    a.check("41. torpedoTypes", gorbieChoice.torpedoTypes == customChoice.torpedoTypes);
    a.checkEqual("42. numLaunchers", gorbieChoice.numLaunchers.min(), 0);
    a.checkEqual("43. numLaunchers", gorbieChoice.numLaunchers.max(), 0);
    a.checkEqual("44. numBays", gorbieChoice.numBays.min(), 10);
    a.checkEqual("45. numBays", gorbieChoice.numBays.max(), 10);
    a.checkEqual("46. ammo", gorbieChoice.ammo.min(), 0);
    a.checkEqual("47. ammo", gorbieChoice.ammo.max(), 250);
}

/** Test getSecondaryChoices().
    A: create session. Call getEngineTypeChoices().
    E: verify returned value */
AFL_TEST("game.proxy.SimulationSetupProxy:getEngineTypeChoices", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Query
    SimulationSetupProxy::Elements_t list;
    t.getEngineTypeChoices(ind, list);

    // Verify
    a.checkEqual("01. size", list.size(), 2U);
    a.checkEqual("02. id",   list[0].first, 5);
    a.checkEqual("03. name", list[0].second, "Nova Drive 5");
    a.checkEqual("04. id",   list[1].first, 9);
    a.checkEqual("05. name", list[1].second, "Transwarp Drive");
}

/** Test getAggressivenessChoices().
    A: create session. Call getAggressivenessChoices().
    E: verify returned value */
AFL_TEST("game.proxy.SimulationSetupProxy:getAggressivenessChoices", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Query
    SimulationSetupProxy::Elements_t list;
    t.getAggressivenessChoices(ind, list);

    // Verify
    a.checkEqual("01. size", list.size(), 5U);

    // Convert to map for order-independence
    std::map<int32_t, String_t> map(list.begin(), list.end());
    a.checkEqual("11. Kill",    map[game::sim::Ship::agg_Kill], "Kill Mission");
    a.checkEqual("12. Passive", map[game::sim::Ship::agg_Passive], "Passive Ship");
    a.checkEqual("13. NoFuel",  map[game::sim::Ship::agg_NoFuel], "No Fuel");
    a.checkEqual("14. p1",      map[1], "Primary Enemy The Federation");
    a.checkEqual("15. p2",      map[2], "Primary Enemy The Lizards");
}

/** Test getBaseBeamLevelChoices().
    A: create session. Call getBaseBeamLevelChoices().
    E: verify returned value */
AFL_TEST("game.proxy.SimulationSetupProxy:getBaseBeamLevelChoices", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Query
    SimulationSetupProxy::Elements_t list;
    t.getBaseBeamLevelChoices(ind, list);

    // Verify
    a.checkEqual("01. size", list.size(), 11U);
    a.checkEqual("02. id",   list[0].first, 0);
    a.checkEqual("03. name", list[0].second, "No base");
    a.checkEqual("04. id",   list[1].first, 1);
    a.checkEqual("05. name", list[1].second, "Laser");
    a.checkEqual("06. id",   list[10].first, 10);
    a.checkEqual("07. name", list[10].second, "Heavy Phaser");
}

/** Test getBaseTorpedoLevelChoices().
    A: create session. Call getBaseTorpedoLevelChoices().
    E: verify returned value */
AFL_TEST("game.proxy.SimulationSetupProxy:getBaseTorpedoLevelChoices", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Query
    SimulationSetupProxy::Elements_t list;
    t.getBaseTorpedoLevelChoices(ind, list);

    // Verify
    a.checkEqual("01. size", list.size(), 10U);
    a.checkEqual("02. id",   list[0].first, 1);
    a.checkEqual("03. name", list[0].second, "Mark 1 Photon");
    a.checkEqual("04. id",   list[9].first, 10);
    a.checkEqual("05. name", list[9].second, "Mark 8 Photon");
}

/** Test getPlanetNameChoices().
    A: create session. Call getPlanetNameChoices().
    E: verify returned value */
AFL_TEST("game.proxy.SimulationSetupProxy:getPlanetNameChoices", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    preparePlanetNames(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Query
    SimulationSetupProxy::Elements_t list;
    t.getPlanetNameChoices(ind, list);

    // Verify
    a.checkEqual("01. size", list.size(), 3U);
    a.checkEqual("02. id",   list[0].first, 1);
    a.checkEqual("03. name", list[0].second, "One");
    a.checkEqual("04. id",   list[1].first, 5);
    a.checkEqual("05. name", list[1].second, "Five");
    a.checkEqual("06. id",   list[2].first, 51);
    a.checkEqual("07. name", list[2].second, "Fifty-One");
}

/** Test getPopulationChoices().
    A: create session. Add planet. Call getPopulationChoices().
    E: verify returned value */
AFL_TEST("game.proxy.SimulationSetupProxy:getPopulationChoices", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    preparePlanetNames(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Create planet
    t.addPlanet(ind);
    t.setDefense(0, 30);

    // Query
    SimulationSetupProxy::PopulationChoices choices;
    t.getPopulationChoices(ind, 0, choices);

    // Verify
    a.checkEqual("01. population", choices.population, 30);
    a.checkEqual("02. sampleDefense", choices.sampleDefense, 30);
    a.checkEqual("03. samplePopulation", choices.samplePopulation, 30);
    a.checkEqual("04. range", choices.range.min(), 0);

    // Again
    t.setDefense(0, 70);
    t.getPopulationChoices(ind, 0, choices);
    a.checkEqual("11. population", choices.population, 450);
    a.checkEqual("12. sampleDefense", choices.sampleDefense, 70);
    a.checkEqual("13. samplePopulation", choices.samplePopulation, 450);

    // Yet again
    t.setDefense(0, 5);
    t.getPopulationChoices(ind, 0, choices);
    a.checkEqual("21. population", choices.population, 5);
    a.checkEqual("22. sampleDefense", choices.sampleDefense, 60);
    a.checkEqual("23. samplePopulation", choices.samplePopulation, 150);
}

/** Test getIdRange().
    A: create session. Add planets and ships to game. Add planets and ships to session. Call getIdRange().
    E: verify returned value */
AFL_TEST("game.proxy.SimulationSetupProxy:getIdRange", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    prepareUniverse(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add ship and planet.
    t.addShip(ind, 0, 1);
    t.addPlanet(ind);

    // Verify
    SimulationSetupProxy::Range_t shipRange = t.getIdRange(ind, 0);
    a.checkEqual("01. min", shipRange.min(), 1);
    a.checkEqual("02. max", shipRange.max(), 333);

    SimulationSetupProxy::Range_t planetRange = t.getIdRange(ind, 1);
    a.checkEqual("11. min", planetRange.min(), 1);
    a.checkEqual("12. max", planetRange.max(), 444);
}

/** Test getDamageRange().
    A: create session. Add ships. Call getDamageRange().
    E: verify returned value */
AFL_TEST("game.proxy.SimulationSetupProxy:getDamageRange", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add ships, one Lizard
    t.addShip(ind, 0, 2);
    t.setOwner(0, 2);

    // Verify
    SimulationSetupProxy::Range_t lizardRange = t.getDamageRange(ind, 0);
    a.checkEqual("01. min", lizardRange.min(), 0);
    a.checkEqual("02. max", lizardRange.max(), 150);

    SimulationSetupProxy::Range_t normRange = t.getDamageRange(ind, 1);
    a.checkEqual("11. min", normRange.min(), 0);
    a.checkEqual("12. max", normRange.max(), 99);
}

/** Test getShieldRange().
    A: create session. Add ships. Call getShieldRange().
    E: verify returned value */
AFL_TEST("game.proxy.SimulationSetupProxy:getShieldRange", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add ships, one damaged
    t.addShip(ind, 0, 2);
    t.setDamage(0, 20);

    // Verify
    SimulationSetupProxy::Range_t r1 = t.getShieldRange(ind, 0);
    a.checkEqual("01. min", r1.min(), 0);
    a.checkEqual("02. max", r1.max(), 80);

    SimulationSetupProxy::Range_t r2 = t.getShieldRange(ind, 1);
    a.checkEqual("11. min", r2.min(), 0);
    a.checkEqual("12. max", r2.max(), 100);
}

/** Test getCrewRange().
    A: create session. Add ships. Call getCrewRange().
    E: verify returned value */
AFL_TEST("game.proxy.SimulationSetupProxy:getCrewRange", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add ships, one Gorbie
    t.addShip(ind, 0, 2);
    t.setHullType(0, game::test::GORBIE_HULL_ID, false);

    // Verify
    SimulationSetupProxy::Range_t r1 = t.getCrewRange(ind, 0);
    a.checkEqual("01. min", r1.min(), 1);
    a.checkEqual("02. max", r1.max(), 2287);

    SimulationSetupProxy::Range_t r2 = t.getCrewRange(ind, 1);
    a.checkEqual("11. min", r2.min(), 1);
    a.checkEqual("12. max", r2.max(), 10000);
}

/** Test getInterceptIdRange().
    A: create session. Add planets and ships to game. Add ship to session. Call getInterceptIdRange().
    E: verify returned value */
AFL_TEST("game.proxy.SimulationSetupProxy:getInterceptIdRange", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    prepareUniverse(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add ship
    t.addShip(ind, 0, 1);

    // Verify
    SimulationSetupProxy::Range_t r1 = t.getInterceptIdRange(ind, 0);
    a.checkEqual("01. min", r1.min(), 0);
    a.checkEqual("02. max", r1.max(), 333);
}

/** Test getBaseDefenseRange().
    A: create session. Add base. Call getInterceptIdRange().
    E: verify returned value for different owners */
AFL_TEST("game.proxy.SimulationSetupProxy:getBaseDefenseRange", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    thread.session().getRoot()->hostConfiguration()[game::config::HostConfiguration::MaximumDefenseOnBase].set("10,15,20,30");
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add base
    t.addPlanet(ind);
    t.setBaseBeamTech(0, 10);
    t.setOwner(0, 1);

    // Verify
    SimulationSetupProxy::Range_t r1 = t.getBaseDefenseRange(ind, 0);
    a.checkEqual("01. min", r1.min(), 0);
    a.checkEqual("02. max", r1.max(), 10);

    // Change owner, verify again
    t.setOwner(0, 3);
    SimulationSetupProxy::Range_t r2 = t.getBaseDefenseRange(ind, 0);
    a.checkEqual("11. min", r2.min(), 0);
    a.checkEqual("12. max", r2.max(), 20);
}

/** Test getNumBaseFightersRange().
    A: create session. Add base. Call getNumBaseFightersRange().
    E: verify returned value for different owners */
AFL_TEST("game.proxy.SimulationSetupProxy:getNumBaseFightersRange", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    thread.session().getRoot()->hostConfiguration()[game::config::HostConfiguration::MaximumFightersOnBase].set("32,16,8,4,2");
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add base
    t.addPlanet(ind);
    t.setBaseBeamTech(0, 10);
    t.setOwner(0, 1);

    // Verify
    SimulationSetupProxy::Range_t r1 = t.getNumBaseFightersRange(ind, 0);
    a.checkEqual("01. min", r1.min(), 0);
    a.checkEqual("02. max", r1.max(), 32);

    // Change owner, verify again
    t.setOwner(0, 3);
    SimulationSetupProxy::Range_t r2 = t.getNumBaseFightersRange(ind, 0);
    a.checkEqual("11. min", r2.min(), 0);
    a.checkEqual("12. max", r2.max(), 8);
}

/** Test setSlot().
    A: create session with ships. Call setSlot(). Modify units and list.
    E: verify correct callbacks. */
AFL_TEST("game.proxy.SimulationSetupProxy:setSlot", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add some ships: 1,2,3,4,5
    t.addShip(ind, 0, 5);

    // Observe one ship
    Observer obs;
    t.sig_objectChange.add(&obs, &Observer::onObjectChange);
    t.setSlot(2);
    thread.sync();
    ind.processQueue();

    // Verify initial callback
    a.checkEqual("01. getSlot", obs.getSlot(), 2U);
    a.checkEqual("02. id", obs.info().id, 3);

    // Modify ship and check callback
    t.setId(2, 66);
    thread.sync();
    ind.processQueue();
    a.checkEqual("11. getSlot", obs.getSlot(), 2U);
    a.checkEqual("12. id", obs.info().id, 66);

    // Delete one ship: 1,3,4,5
    t.removeObject(1);

    // Modify ship at its new position and observe callback at new position
    t.setId(1, 77);
    thread.sync();
    ind.processQueue();
    a.checkEqual("21. getSlot", obs.getSlot(), 1U);
    a.checkEqual("22. id", obs.info().id, 77);

    // Swap: 1,5,4,3
    t.swapShips(1, 3);

    // Modify ship at its new position and observe callback at new position
    t.setId(3, 88);
    thread.sync();
    ind.processQueue();
    a.checkEqual("31. getSlot", obs.getSlot(), 3U);
    a.checkEqual("32. id", obs.info().id, 88);
}

/** Test setConfiguration(), getConfiguration(). */
AFL_TEST("game.proxy.SimulationSetupProxy:configuration", a)
{
    using game::sim::Configuration;

    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Set configuration
    {
        Configuration config;
        config.setEngineShieldBonus(55);
        t.setConfiguration(config, Configuration::Areas_t(Configuration::MainArea));
    }

    // Retrieve configuration
    {
        Configuration config;
        t.getConfiguration(ind, config);
        a.checkEqual("01. getEngineShieldBonus", config.getEngineShieldBonus(), 55);
    }
}

/** Test sortShips().
    A: create session with ships. Sort by Id.
    E: verify correct callbacks even in presence of setSlot(). Verify correct order. */
AFL_TEST("game.proxy.SimulationSetupProxy:sort", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add ships
    t.addShip(ind, 0, 4);
    t.setId(0, 10);
    t.setId(1, 30);
    t.setId(2, 40);
    t.setId(3, 20);

    // Observe one unit
    Observer o;
    t.sig_objectChange.add(&o, &Observer::onObjectChange);
    t.setSlot(3);
    thread.sync();
    ind.processQueue();
    a.checkEqual("01. getSlot", o.getSlot(), 3U);

    // Sort
    t.sortShips(SimulationSetupProxy::SortById);

    // Verify: retrieve list
    SimulationSetupProxy::ListItems_t list;
    t.getList(ind, list);
    a.checkEqual("11. size", list.size(), 4U);
    a.checkEqual("12. id", list[0].id, 10);
    a.checkEqual("13. id", list[1].id, 20);
    a.checkEqual("14. id", list[2].id, 30);
    a.checkEqual("15. id", list[3].id, 40);

    // Verify: modify object at slot 1 (which was previously at slot 3)
    t.setId(1, 22);
    thread.sync();
    ind.processQueue();
    a.checkEqual("21. getSlot", o.getSlot(), 1U);
    a.checkEqual("22. id", o.info().id, 22);
}

/** Test sortShips(), sort by battle order.
    A: create session with ships. Sort by battle order with different configuration.
    E: verify correct order. */
AFL_TEST("game.proxy.SimulationSetupProxy:sort:battle-order", a)
{
    using game::sim::Configuration;

    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add ships
    t.addShip(ind, 0, 3);
    t.setId(0, 1);
    t.setId(1, 2);
    t.setId(2, 3);
    t.setFriendlyCode(0, "123");
    t.setFriendlyCode(1, "abc");
    t.setFriendlyCode(2, "-12");
    t.setAggressiveness(0, 0);
    t.setAggressiveness(1, 0);
    t.setAggressiveness(2, 0);

    // Configure PHost
    {
        Configuration config;
        game::config::HostConfiguration hostConfiguration;
        config.setMode(Configuration::VcrPHost4, 0, hostConfiguration);
        t.setConfiguration(config, Configuration::Areas_t(Configuration::MainArea));
    }

    // Sort
    t.sortShips(SimulationSetupProxy::SortByBattleOrder);

    // Verify: retrieve list
    {
        SimulationSetupProxy::ListItems_t list;
        t.getList(ind, list);
        a.checkEqual("01. size", list.size(), 3U);
        a.checkEqual("02. id", list[0].id, 3);
        a.checkEqual("03. id", list[1].id, 1);
        a.checkEqual("04. id", list[2].id, 2);
    }

    // Configure Host
    {
        Configuration config;
        game::config::HostConfiguration hostConfiguration;
        config.setMode(Configuration::VcrHost, 0, hostConfiguration);
        t.setConfiguration(config, Configuration::Areas_t(Configuration::MainArea));
    }

    // Sort
    t.sortShips(SimulationSetupProxy::SortByBattleOrder);

    // Verify: retrieve list
    {
        SimulationSetupProxy::ListItems_t list;
        t.getList(ind, list);
        a.checkEqual("11. size", list.size(), 3U);
        a.checkEqual("12. id", list[0].id, 1);
        a.checkEqual("13. id", list[1].id, 2);
        a.checkEqual("14. id", list[2].id, 3);
    }
}

/** Test copyFromGame(), copyToGame().
    A: create session with ships. Create matching game ship. Call copyFromGame, copyToGame.
    E: verify data is being transferred */
AFL_TEST("game.proxy.SimulationSetupProxy:copy", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    preparePlayedShip(thread, 77);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Add ship to sim
    t.addShip(ind, 0, 1);
    t.setOwner(0, 4);
    t.setId(0, 77);
    t.setHullType(0, game::test::OUTRIDER_HULL_ID, false);

    // Retrieve data
    SimulationSetupProxy::ObjectInfo oi;
    t.getObject(ind, 0, oi);
    a.checkEqual("01. id", oi.id, 77);
    a.checkEqual("02. relation", oi.relation, game::sim::GameInterface::Playable);
    a.checkEqual("03. position", oi.position.orElse(game::map::Point()), game::map::Point(2000, 2000));

    // Copy from game
    game::sim::Setup::Status st = t.copyFromGame(ind, 0, 1);
    a.checkEqual("11. succeeded", st.succeeded, 1U);
    a.checkEqual("12. failed", st.failed, 0U);

    t.getObject(ind, 0, oi);
    a.checkEqual("21. name", oi.name, "The Ship");

    // Modify and copy back
    t.setName(0, "Modified");
    st = t.copyToGame(ind, 0, 1);
    a.checkEqual("31. succeeded", st.succeeded, 1U);
    a.checkEqual("32. failed", st.failed, 0U);

    a.checkEqual("41. getName", thread.session().getGame()->currentTurn().universe().ships().get(77)->getName(), "Modified");
}

/** Test load(), success case.
    A: create .ccb file. Call load().
    E: verify file correctly loaded */
AFL_TEST("game.proxy.SimulationSetupProxy:load", a)
{
    // File
    const char* FILE_NAME = "testload.tmp";
    static const uint8_t FILE_CONTENT[] = {
        0x43, 0x43, 0x62, 0x73, 0x69, 0x6d, 0x32, 0x1a, 0x02, 0x00, 0x59, 0x6e, 0x50, 0x76, 0x5a, 0x6b,
        0x72, 0x63, 0x65, 0x6e, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x39, 0x00,
        0x9d, 0x02, 0x1c, 0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x07, 0x00,
        0x00, 0x00, 0x05, 0x00, 0x09, 0x00, 0x17, 0x00, 0x64, 0x00, 0x3f, 0x3f, 0x3f, 0xff, 0xff, 0xa5,
        0x01, 0x00, 0x00, 0x54, 0x72, 0x65, 0x61, 0x6e, 0x74, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0xde, 0x00, 0x8a, 0x01, 0x04, 0x00, 0x00,
        0x00, 0x0a, 0x00, 0x02, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x16, 0x00, 0x04, 0x00, 0x09, 0x00, 0x26,
        0x00, 0x64, 0x00, 0x6d, 0x6b, 0x74, 0x00, 0x00, 0x5a, 0x00, 0x00, 0x00
    };

    // Set up file system
    afl::io::InternalFileSystem fs;
    afl::base::Ref<afl::io::Directory> currentDir = fs.openDirectory(fs.getWorkingDirectoryName());
    currentDir->openFile(FILE_NAME, afl::io::FileSystem::Create)
        ->fullWrite(FILE_CONTENT);

    // Test environment
    SessionThread thread(fs);
    WaitIndicator ind;
    prepare(thread);
    preparePlayedShip(thread, 77);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Load
    String_t error;
    bool result = t.load(ind, fs.makePathName(fs.getWorkingDirectoryName(), FILE_NAME), error);

    // Verify
    a.checkEqual("01. result", result, true);

    SimulationSetupProxy::ListItems_t list;
    t.getList(ind, list);
    a.checkEqual("11. size", list.size(), 2U);
    a.checkEqual("12. id", list[0].id, 28);
    a.checkEqual("13. id", list[1].id, 394);

    // Remove file
    fs.openDirectory(fs.getWorkingDirectoryName())
        ->eraseNT(FILE_NAME);
}

/** Test load(), failure case.
    A: Call load() with invalid file name.
    E: verify error return */
AFL_TEST("game.proxy.SimulationSetupProxy:load:error", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Load
    String_t error;
    bool result = t.load(ind, "/this/file/hopefully/does/not/exist", error);

    // Verify
    a.checkEqual("01. result", result, false);
    a.checkDifferent("02. error", error, "");
}

/** Test player relations functions.
    A: set up universe with an alliance (team). Call player-relation functions.
    E: alliances are properly reported and copied to configuration. */
AFL_TEST("game.proxy.SimulationSetupProxy:relations", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    prepareAlliances(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Get status
    // - initially, no alliances
    game::sim::Configuration config;
    t.getConfiguration(ind, config);
    a.check("01. allianceSettings", !config.allianceSettings().get(3, 4));

    // - get status
    game::proxy::SimulationSetupProxy::PlayerRelations rel;
    t.getPlayerRelations(ind, rel);
    a.check("11. alliances", rel.alliances.get(3, 4));
    a.check("12. usePlayerRelations", rel.usePlayerRelations);

    // - use alliances
    t.usePlayerRelations();

    // - get configuration status
    t.getConfiguration(ind, config);
    a.check("21. allianceSettings", config.allianceSettings().get(3, 4));
}

/** Test player relations functions, with auto-sync disabled.
    A: set up universe with an alliance (team). Disable auto-sync. Call player-relation functions.
    E: alliances are properly reported, but not copied to configuration. */
AFL_TEST("game.proxy.SimulationSetupProxy:relations:disabled", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    prepareAlliances(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    // Disable
    t.setUsePlayerRelations(false);

    // Get status
    // - initially, no alliances
    game::sim::Configuration config;
    t.getConfiguration(ind, config);
    a.check("01. allianceSettings", !config.allianceSettings().get(3, 4));

    // - get status
    game::proxy::SimulationSetupProxy::PlayerRelations rel;
    t.getPlayerRelations(ind, rel);
    a.check("11. alliances", rel.alliances.get(3, 4));
    a.check("12. usePlayerRelations", !rel.usePlayerRelations);

    // - use alliances
    t.usePlayerRelations();

    // - get configuration status
    t.getConfiguration(ind, config);
    a.check("21. allianceSettings", !config.allianceSettings().get(3, 4));
}

/** Test isMatchingShipList(), empty.
    A: set up empty simulation.
    E: isMatchingShipList() reports true. */
AFL_TEST("game.proxy.SimulationSetupProxy:isMatchingShipList:empty", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    prepareAlliances(thread);
    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);

    a.checkEqual("01. isMatchingShipList", t.isMatchingShipList(ind), true);
}

/** Test isMatchingShipList(), match.
    A: set up a simulation with a matching ship (Outrider with no torpedoes).
    E: isMatchingShipList() reports true. */
AFL_TEST("game.proxy.SimulationSetupProxy:isMatchingShipList:match", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    prepareAlliances(thread);

    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);
    t.addShip(ind, 0, 1);
    t.setHullType(0, game::test::OUTRIDER_HULL_ID, false);
    t.setTorpedoType(0, 0);
    t.setNumLaunchers(0, 0);

    a.checkEqual("01. isMatchingShipList", t.isMatchingShipList(ind), true);
}

/** Test isMatchingShipList(), mismatch.
    A: set up a simulation with not-matching ship (Outrider with torpedoes).
    E: isMatchingShipList() reports true. */
AFL_TEST("game.proxy.SimulationSetupProxy:isMatchingShipList:mismatch", a)
{
    SessionThread thread;
    WaitIndicator ind;
    prepare(thread);
    prepareAlliances(thread);

    SimulationSetupProxy t(thread.gameSender().makeTemporary(new SimulationAdaptorFromSession()), ind);
    t.addShip(ind, 0, 1);
    t.setHullType(0, game::test::OUTRIDER_HULL_ID, false);
    t.setTorpedoType(0, 5);
    t.setNumLaunchers(0, 3);

    a.checkEqual("01. isMatchingShipList", t.isMatchingShipList(ind), false);
}
