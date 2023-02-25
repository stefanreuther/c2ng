/**
  *  \file u/t_game_actions_cloneship.cpp
  *  \brief Test for game::actions::CloneShip
  */

#include "game/actions/cloneship.hpp"

#include "t_game_actions.hpp"
#include "game/test/simpleturn.hpp"
#include "game/test/root.hpp"
#include "game/map/planetstorage.hpp"
#include "game/test/shiplist.hpp"
#include "game/exception.hpp"
#include "game/test/interpreterinterface.hpp"
#include "afl/string/nulltranslator.hpp"

using game::map::Planet;
using game::map::PlanetStorage;
using game::map::Ship;
using game::map::Object;

namespace {
    const int PLANET_OWNER = 3;
    const int PLANET_ID = 200;
    const int BEAM_TYPE = 4;

    Planet& init(game::test::SimpleTurn& t)
    {
        // Define ship list
        game::test::initStandardBeams(t.shipList());
        game::test::initStandardTorpedoes(t.shipList());
        game::test::addOutrider(t.shipList());
        game::test::addNovaDrive(t.shipList());

        // Create a planet with minimum content
        Planet& pl = t.addBase(PLANET_ID, PLANET_OWNER, Object::Playable);
        pl.setBaseTechLevel(game::HullTech, 1);
        pl.setBaseTechLevel(game::BeamTech, 1);
        pl.setBaseTechLevel(game::EngineTech, 1);
        pl.setBaseTechLevel(game::TorpedoTech, 1);

        // Preset hull number for convenience
        t.setHull(game::test::OUTRIDER_HULL_ID);
        return pl;
    }

    Ship& addOutrider(game::test::SimpleTurn& t)
    {
        Ship& sh = t.addShip(100, PLANET_OWNER, Object::Playable);
        sh.setEngineType(game::test::NOVA_ENGINE_ID);
        sh.setNumBeams(1);
        sh.setBeamType(BEAM_TYPE);
        return sh;
    }
}

/** Test normal ("happy") case: ship being cloned.
    A: prepare ship and planet
    E: correct result reported, commits correctly */
void
TestGameActionsCloneShip::testNormal()
{
    // Environment
    game::test::SimpleTurn t;
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    game::UnitScoreDefinitionList shipScores;

    // Units
    Planet& pl = init(t);
    Ship& sh = addOutrider(t);

    // Action
    game::actions::CloneShip testee(pl, sh, t.universe(), shipScores, t.shipList(), *root);
    TS_ASSERT_EQUALS(&testee.ship(), &sh);
    TS_ASSERT_EQUALS(&testee.planet(), &pl);

    // Tech upgrade cost: 1000$ for hull tech, 300$ for beam tech
    TS_ASSERT_EQUALS(testee.getTechUpgradeAction().getCost().toCargoSpecString(), "1300$");

    // Outrider:     40T 20D  5M 50$
    // Nova drive:    3T  3D  7M 25$
    // Blaster        1T 12D  1M 10$
    // Total         44T 35D 13M 85$ (-> 170$ due to cloning, +1300 for tech)
    TS_ASSERT_EQUALS(testee.getCloneAction().getCost().toCargoSpecString(), "44T 35D 13M 1470$");

    // Build order
    TS_ASSERT_EQUALS(testee.getBuildOrder().getHullIndex(), game::test::OUTRIDER_HULL_ID);
    TS_ASSERT_EQUALS(testee.getBuildOrder().getEngineType(), game::test::NOVA_ENGINE_ID);
    TS_ASSERT_EQUALS(testee.getBuildOrder().getBeamType(), BEAM_TYPE);
    TS_ASSERT_EQUALS(testee.getBuildOrder().getTorpedoType(), 0);

    // Status
    TS_ASSERT_EQUALS(testee.getOrderStatus(), game::actions::CloneShip::CanClone);
    TS_ASSERT_EQUALS(testee.getPaymentStatus(), game::actions::CloneShip::CanPay);
    TS_ASSERT_EQUALS(testee.isCloneOnce(), false);

    // Commit
    game::map::Configuration mapConfig;
    util::RandomNumberGenerator rng(1);
    TS_ASSERT_THROWS_NOTHING(testee.commit(mapConfig, rng));
    TS_ASSERT_EQUALS(sh.getFriendlyCode().orElse(""), "cln");
    TS_ASSERT_EQUALS(pl.getBaseTechLevel(game::EngineTech).orElse(-1), 5);
    TS_ASSERT_EQUALS(pl.getBaseTechLevel(game::BeamTech).orElse(-1), 3);
    TS_ASSERT_EQUALS(pl.getCargo(game::Element::Money).orElse(-1), 170);
    TS_ASSERT_EQUALS(pl.getCargo(game::Element::Supplies).orElse(-1), 530);
}

/** Test normal case, but can only pay tech.
    A: prepare ship and planet with less money
    E: correct result reported, commits correctly */
void
TestGameActionsCloneShip::testNormalPayTech()
{
    // Environment
    game::test::SimpleTurn t;
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    game::UnitScoreDefinitionList shipScores;

    // Units
    Planet& pl = init(t);
    Ship& sh = addOutrider(t);
    pl.setCargo(game::Element::Supplies, 0);
    pl.setCargo(game::Element::Money, 1300);

    // Action
    game::actions::CloneShip testee(pl, sh, t.universe(), shipScores, t.shipList(), *root);
    TS_ASSERT_EQUALS(testee.getOrderStatus(), game::actions::CloneShip::CanClone);
    TS_ASSERT_EQUALS(testee.getPaymentStatus(), game::actions::CloneShip::CannotPayComponents);

    // Commit
    game::map::Configuration mapConfig;
    util::RandomNumberGenerator rng(1);
    TS_ASSERT_THROWS_NOTHING(testee.commit(mapConfig, rng));
    TS_ASSERT_EQUALS(sh.getFriendlyCode().orElse(""), "cln");
    TS_ASSERT_EQUALS(pl.getBaseTechLevel(game::EngineTech).orElse(-1), 5);
    TS_ASSERT_EQUALS(pl.getBaseTechLevel(game::BeamTech).orElse(-1), 3);
}

/** Test normal case, but cannot even pay tech.
    A: prepare ship and planet with very little money
    E: correct result reported, commits fails */
void
TestGameActionsCloneShip::testNormalPayNone()
{
    // Environment
    game::test::SimpleTurn t;
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    game::UnitScoreDefinitionList shipScores;

    // Units
    Planet& pl = init(t);
    Ship& sh = addOutrider(t);
    pl.setCargo(game::Element::Supplies, 0);
    pl.setCargo(game::Element::Money, 100);
    sh.setFriendlyCode(String_t("xyz"));

    // Action
    game::actions::CloneShip testee(pl, sh, t.universe(), shipScores, t.shipList(), *root);
    TS_ASSERT_EQUALS(testee.getOrderStatus(), game::actions::CloneShip::CanClone);
    TS_ASSERT_EQUALS(testee.getPaymentStatus(), game::actions::CloneShip::CannotPayTech);

    // Commit
    game::map::Configuration mapConfig;
    util::RandomNumberGenerator rng(1);
    TS_ASSERT_THROWS(testee.commit(mapConfig, rng), game::Exception);
    TS_ASSERT_EQUALS(sh.getFriendlyCode().orElse(""), "xyz");
}

/** Test CanBuild case.
    A: prepare ship and planet, player can build the ship
    E: correct result reported: CanBuild */
void
TestGameActionsCloneShip::testCanBuild()
{
    // Environment
    game::test::SimpleTurn t;
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    game::UnitScoreDefinitionList shipScores;

    // Units
    Planet& pl = init(t);
    Ship& sh = addOutrider(t);
    t.shipList().hullAssignments().add(PLANET_OWNER, 1, game::test::OUTRIDER_HULL_ID);

    // Action
    game::actions::CloneShip testee(pl, sh, t.universe(), shipScores, t.shipList(), *root);
    TS_ASSERT_EQUALS(testee.getOrderStatus(), game::actions::CloneShip::CanBuild);
}

/** Test cloning as Tholian, Host case: result is PlayerCannotClone.
    A: prepare ship and planet, both owned by Tholians, check with Host
    E: correct result reported: PlayerCannotClone */
void
TestGameActionsCloneShip::testTholianHost()
{
    // Environment
    game::test::SimpleTurn t;
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion(game::HostVersion::Host, MKVERSION(3,22,4)));
    game::UnitScoreDefinitionList shipScores;

    // Units
    Planet& pl = init(t);
    Ship& sh = addOutrider(t);
    pl.setOwner(7);
    sh.setOwner(7);

    // Action
    game::actions::CloneShip testee(pl, sh, t.universe(), shipScores, t.shipList(), *root);
    TS_ASSERT_EQUALS(testee.getOrderStatus(), game::actions::CloneShip::PlayerCannotClone);
}

/** Test cloning as Tholian, PHost case: can clone, but expensive.
    A: prepare ship and planet, both owned by Tholians, check with PHost
    E: correct result reported */
void
TestGameActionsCloneShip::testTholianPHost()
{
    // Environment
    game::test::SimpleTurn t;
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(3,2,4)));
    game::UnitScoreDefinitionList shipScores;

    // Units
    Planet& pl = init(t);
    Ship& sh = addOutrider(t);
    pl.setOwner(7);
    sh.setOwner(7);

    // Action
    game::actions::CloneShip testee(pl, sh, t.universe(), shipScores, t.shipList(), *root);
    TS_ASSERT_EQUALS(testee.getOrderStatus(), game::actions::CloneShip::CanClone);

    // Cost is 1300$ for tech + 85*327.68=27851 for the ship
    TS_ASSERT_EQUALS(testee.getCloneAction().getCost().toCargoSpecString(), "44T 35D 13M 29151$");
}

/** Test tech limit.
    A: prepare ship and planet, ship has high-tech engine.
    E: correct result reported: TechLimitExceeded */
void
TestGameActionsCloneShip::testTechLimit()
{
    // Environment
    game::test::SimpleTurn t;
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    game::UnitScoreDefinitionList shipScores;
    game::test::addTranswarp(t.shipList());

    // Units
    Planet& pl = init(t);
    Ship& sh = addOutrider(t);
    sh.setEngineType(game::test::TRANSWARP_ENGINE_ID);

    // Action
    game::actions::CloneShip testee(pl, sh, t.universe(), shipScores, t.shipList(), *root);
    TS_ASSERT_EQUALS(testee.getOrderStatus(), game::actions::CloneShip::TechLimitExceeded);
}

/** Test RemoteOwnerCanBuild case.
    A: prepare ship and planet. Ship is remotely-controlled, remote owner can build it.
    E: correct result reported: RemoteOwnerCanBuild */
void
TestGameActionsCloneShip::testRemoteOwner()
{
    const int REMOTE_OWNER = 9;

    // Environment
    game::test::SimpleTurn t;
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(3,2,4)));
    game::UnitScoreDefinitionList shipScores;
    game::test::addTranswarp(t.shipList());

    // Units
    Planet& pl = init(t);
    Ship& sh = addOutrider(t);
    t.shipList().hullAssignments().add(REMOTE_OWNER, 1, game::test::OUTRIDER_HULL_ID);

    game::parser::MessageInformation info(game::parser::MessageInformation::Ship, sh.getId(), 10);
    info.addValue(game::parser::mi_ShipRemoteFlag, REMOTE_OWNER);
    sh.addMessageInformation(info, game::PlayerSet_t());

    // Action
    game::actions::CloneShip testee(pl, sh, t.universe(), shipScores, t.shipList(), *root);
    TS_ASSERT_EQUALS(testee.getOrderStatus(), game::actions::CloneShip::RemoteOwnerCanBuild);
}

/** Test ShipIsUnclonable case.
    A: prepare ship and planet. Ship has "Unclonable" function.
    E: correct result reported: ShipIsUnclonable */
void
TestGameActionsCloneShip::testUnclonable()
{
    // Environment
    game::test::SimpleTurn t;
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    game::UnitScoreDefinitionList shipScores;
    game::test::addTranswarp(t.shipList());

    // Units
    Planet& pl = init(t);
    Ship& sh = addOutrider(t);
    sh.addShipSpecialFunction(t.shipList().modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::Unclonable));

    // Action
    game::actions::CloneShip testee(pl, sh, t.universe(), shipScores, t.shipList(), *root);
    TS_ASSERT_EQUALS(testee.getOrderStatus(), game::actions::CloneShip::ShipIsUnclonable);
}


/** Test friendly-code validation: good case.
    A: prepare ship and planet. Friendly code "cln" requires registration, player is registered.
    E: correct result reported: CanClone */
void
TestGameActionsCloneShip::testFriendlyCodeGood()
{
    // Environment
    game::test::SimpleTurn t;
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion(), game::RegistrationKey::Registered, 10);
    game::UnitScoreDefinitionList shipScores;
    afl::string::NullTranslator tx;
    t.shipList().friendlyCodes().addCode(game::spec::FriendlyCode("cln", "sr,foo", tx));

    // Units
    Planet& pl = init(t);
    Ship& sh = addOutrider(t);
    TS_ASSERT(t.shipList().friendlyCodes().isAcceptedFriendlyCode("cln", game::spec::FriendlyCode::Filter::fromShip(sh, shipScores, t.shipList(), root->hostConfiguration()), root->registrationKey(),
                                                                  game::spec::FriendlyCodeList::DefaultAvailable));

    // Action
    game::actions::CloneShip testee(pl, sh, t.universe(), shipScores, t.shipList(), *root);
    TS_ASSERT_EQUALS(testee.getOrderStatus(), game::actions::CloneShip::CanClone);
}

/** Test friendly-code validation: bad case.
    A: prepare ship and planet. Friendly code "cln" requires registration, player is not registered.
    E: correct result reported: PlayerCannotClone */
void
TestGameActionsCloneShip::testFriendlyCodeBad()
{
    // Environment
    game::test::SimpleTurn t;
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion(), game::RegistrationKey::Unregistered, 6);
    game::UnitScoreDefinitionList shipScores;
    afl::string::NullTranslator tx;
    t.shipList().friendlyCodes().addCode(game::spec::FriendlyCode("cln", "sr,foo", tx));

    // Units
    Planet& pl = init(t);
    Ship& sh = addOutrider(t);

    // Action
    game::actions::CloneShip testee(pl, sh, t.universe(), shipScores, t.shipList(), *root);
    TS_ASSERT_EQUALS(testee.getOrderStatus(), game::actions::CloneShip::PlayerCannotClone);
}

/** Test friendly-code validation: open friendly code.
    A: prepare ship and planet. Friendly code "cln" does not require registration
    E: correct result reported: CanClone */
void
TestGameActionsCloneShip::testFriendlyCodeOpen()
{
    // Environment
    game::test::SimpleTurn t;
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion(), game::RegistrationKey::Unregistered, 6);
    game::UnitScoreDefinitionList shipScores;
    afl::string::NullTranslator tx;
    t.shipList().friendlyCodes().addCode(game::spec::FriendlyCode("cln", "s,foo", tx));

    // Units
    Planet& pl = init(t);
    Ship& sh = addOutrider(t);

    // Action
    game::actions::CloneShip testee(pl, sh, t.universe(), shipScores, t.shipList(), *root);
    TS_ASSERT_EQUALS(testee.getOrderStatus(), game::actions::CloneShip::CanClone);
}

/** Test conflict check: no conflict.
    A: prepare ship and planet
    E: correct result reported */
void
TestGameActionsCloneShip::testNoConflict()
{
    // Environment
    game::test::SimpleTurn t;
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion(), game::RegistrationKey::Unregistered, 6);
    game::UnitScoreDefinitionList shipScores;

    // Units
    Planet& pl = init(t);
    Ship& sh = addOutrider(t);

    // Action
    game::actions::CloneShip testee(pl, sh, t.universe(), shipScores, t.shipList(), *root);

    afl::string::NullTranslator tx;
    game::test::InterpreterInterface iface;
    game::actions::CloneShip::ConflictStatus st = testee.findConflict(0, tx, iface);
    TS_ASSERT_EQUALS(st, game::actions::CloneShip::NoConflict);
}

/** Test conflict check: conflicting build.
    A: prepare ship and planet, planet is building a ship
    E: correct result reported: IsBuilding, with hull Id and name */
void
TestGameActionsCloneShip::testBuildConflict()
{
    // Environment
    game::test::SimpleTurn t;
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion(), game::RegistrationKey::Unregistered, 6);
    game::UnitScoreDefinitionList shipScores;

    // Units
    Planet& pl = init(t);
    Ship& sh = addOutrider(t);

    // Build order
    game::test::addGorbie(t.shipList());
    t.shipList().hullAssignments().add(PLANET_OWNER, 7, game::test::GORBIE_HULL_ID);
    game::ShipBuildOrder o;
    o.setHullIndex(7);
    pl.setBaseStorage(game::HullTech, 7, 1);
    pl.setBaseBuildOrder(o);

    // Action
    game::actions::CloneShip testee(pl, sh, t.universe(), shipScores, t.shipList(), *root);

    afl::string::NullTranslator tx;
    game::test::InterpreterInterface iface;
    game::actions::CloneShip::Conflict conf;
    game::actions::CloneShip::ConflictStatus st = testee.findConflict(&conf, tx, iface);
    TS_ASSERT_EQUALS(st, game::actions::CloneShip::IsBuilding);
    TS_ASSERT_EQUALS(conf.id, game::test::GORBIE_HULL_ID);
    TS_ASSERT_EQUALS(conf.name, "GORBIE CLASS BATTLECARRIER");
}

/** Test conflict check: conflicting clone.
    A: prepare ship and planet, other ships are cloning
    E: correct result reported: IsCloning, with ship Id and name */
void
TestGameActionsCloneShip::testCloneConflict()
{
    // Environment
    game::test::SimpleTurn t;
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion(), game::RegistrationKey::Unregistered, 6);
    game::UnitScoreDefinitionList shipScores;

    // Units
    Planet& pl = init(t);
    Ship& sh = addOutrider(t);

    // Conflicting clones
    // - conflict
    Ship& c1 = t.addShip(300, PLANET_OWNER, Object::Playable);
    c1.setFriendlyCode(String_t("cln"));
    c1.setName(String_t("one"));

    // - conflict
    Ship& c2 = t.addShip(301, PLANET_OWNER, Object::Playable);
    c2.setFriendlyCode(String_t("cln"));
    c2.setName(String_t("two"));

    // - not a conflict: not cloning
    Ship& c3 = t.addShip(302, PLANET_OWNER, Object::Playable);
    c3.setFriendlyCode(String_t("abc"));
    c3.setName(String_t("three"));

    // - not a conflict: not played
    Ship& c4 = t.addShip(303, PLANET_OWNER+1, Object::NotPlayable);
    c4.setFriendlyCode(String_t("cln"));
    c4.setName(String_t("four"));

    // - not a conflict: wrong position
    t.setPosition(game::map::Point(99, 77));
    Ship& c5 = t.addShip(304, PLANET_OWNER, Object::Playable);
    c5.setFriendlyCode(String_t("cln"));
    c5.setName(String_t("five"));

    // Action
    game::actions::CloneShip testee(pl, sh, t.universe(), shipScores, t.shipList(), *root);

    afl::string::NullTranslator tx;
    game::test::InterpreterInterface iface;
    game::actions::CloneShip::Conflict conf;
    game::actions::CloneShip::ConflictStatus st = testee.findConflict(&conf, tx, iface);
    TS_ASSERT_EQUALS(st, game::actions::CloneShip::IsCloning);
    TS_ASSERT_EQUALS(conf.id, 300);
    TS_ASSERT_EQUALS(conf.name, "Ship #300: one");

    // Commit
    game::map::Configuration mapConfig;
    util::RandomNumberGenerator rng(1);
    TS_ASSERT_THROWS_NOTHING(testee.commit(mapConfig, rng));
    TS_ASSERT_EQUALS(sh.getFriendlyCode().orElse(""), "cln");

    // Verify conflicting ships
    TS_ASSERT_DIFFERS(c1.getFriendlyCode().orElse(""), "cln");
    TS_ASSERT_DIFFERS(c2.getFriendlyCode().orElse(""), "cln");
    TS_ASSERT_EQUALS(c3.getFriendlyCode().orElse(""), "abc");
    TS_ASSERT_EQUALS(c4.getFriendlyCode().orElse(""), "cln");
    TS_ASSERT_EQUALS(c5.getFriendlyCode().orElse(""), "cln");
}

/** Test conflict check: non-conflicting clone.
    A: prepare ship and planet, ship is already cloning
    E: correct result reported: NoConflict, ship itself does not count as a conflict */
void
TestGameActionsCloneShip::testCloneConflictSelf()
{
    // Environment
    game::test::SimpleTurn t;
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion(), game::RegistrationKey::Unregistered, 6);
    game::UnitScoreDefinitionList shipScores;

    // Units
    Planet& pl = init(t);
    Ship& sh = addOutrider(t);
    sh.setFriendlyCode(String_t("cln"));

    // Action
    game::actions::CloneShip testee(pl, sh, t.universe(), shipScores, t.shipList(), *root);

    afl::string::NullTranslator tx;
    game::test::InterpreterInterface iface;
    game::actions::CloneShip::ConflictStatus st = testee.findConflict(0, tx, iface);
    TS_ASSERT_EQUALS(st, game::actions::CloneShip::NoConflict);
}

/** Test conflict check: non-conflicting clone.
    A: prepare ship and planet, ship is already cloning and there is a conflicting other ship
    E: correct result reported: IsCloning, with ship Id and name of other ship */
void
TestGameActionsCloneShip::testCloneConflictMore()
{
    // Environment
    game::test::SimpleTurn t;
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion(), game::RegistrationKey::Unregistered, 6);
    game::UnitScoreDefinitionList shipScores;

    // Units
    Planet& pl = init(t);
    Ship& sh = addOutrider(t);
    sh.setFriendlyCode(String_t("cln"));

    // A conflicting ship
    Ship& c1 = t.addShip(300, PLANET_OWNER, Object::Playable);
    c1.setFriendlyCode(String_t("cln"));
    c1.setName(String_t("one"));

    // Action
    game::actions::CloneShip testee(pl, sh, t.universe(), shipScores, t.shipList(), *root);

    afl::string::NullTranslator tx;
    game::test::InterpreterInterface iface;
    game::actions::CloneShip::Conflict conf;
    game::actions::CloneShip::ConflictStatus st = testee.findConflict(&conf, tx, iface);
    TS_ASSERT_EQUALS(st, game::actions::CloneShip::IsCloning);
    TS_ASSERT_EQUALS(conf.id, 300);
    TS_ASSERT_EQUALS(conf.name, "Ship #300: one");
}

/** Test CloneOnce case.
    A: prepare ship and planet. Ship has "CloneOnce" function.
    E: correct result reported: CanClone, but isCloneOnce() */
void
TestGameActionsCloneShip::testCloneOnce()
{
    // Environment
    game::test::SimpleTurn t;
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion());
    game::UnitScoreDefinitionList shipScores;
    game::test::addTranswarp(t.shipList());

    // Units
    Planet& pl = init(t);
    Ship& sh = addOutrider(t);
    sh.addShipSpecialFunction(t.shipList().modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::CloneOnce));

    // Action
    game::actions::CloneShip testee(pl, sh, t.universe(), shipScores, t.shipList(), *root);
    TS_ASSERT_EQUALS(testee.getOrderStatus(), game::actions::CloneShip::CanClone);
    TS_ASSERT_EQUALS(testee.isCloneOnce(), true);
}

/** Test commit() for ship in fleet.
    A: prepare ship and planet, ship is leader of a fleet
    E: commits correctly, ship will leave the fleet */
void
TestGameActionsCloneShip::testFleet()
{
    // Environment
    game::test::SimpleTurn t;
    afl::base::Ref<game::Root> root = game::test::makeRoot(game::HostVersion(), game::RegistrationKey::Unregistered, 6);
    game::UnitScoreDefinitionList shipScores;

    // Units
    Planet& pl = init(t);
    Ship& sh = addOutrider(t);
    sh.setFleetNumber(sh.getId());

    // Other fleet members
    Ship& c1 = t.addShip(300, PLANET_OWNER, Object::Playable);
    c1.setFleetNumber(sh.getId());

    Ship& c2 = t.addShip(301, PLANET_OWNER, Object::Playable);
    c2.setFleetNumber(sh.getId());

    // Action
    game::actions::CloneShip testee(pl, sh, t.universe(), shipScores, t.shipList(), *root);
    game::map::Configuration mapConfig;
    util::RandomNumberGenerator rng(1);
    TS_ASSERT_THROWS_NOTHING(testee.commit(mapConfig, rng));
    TS_ASSERT_EQUALS(sh.getFriendlyCode().orElse(""), "cln");

    // Verify fleet membership
    TS_ASSERT_EQUALS(sh.getFleetNumber(), 0);
    TS_ASSERT_EQUALS(c1.getFleetNumber(), c1.getId());
    TS_ASSERT_EQUALS(c2.getFleetNumber(), c1.getId());
}
