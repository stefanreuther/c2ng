/**
  *  \file test/server/play/hullpackertest.cpp
  *  \brief Test for server::play::HullPacker
  */

#include "server/play/hullpacker.hpp"

#include "afl/data/access.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"

namespace {
    /* Disable all host config options that would assign automatic hull functions. */
    void disableAutomaticHullFunctions(game::config::HostConfiguration& c)
    {
        using game::config::HostConfiguration;

        // To be able to disable automatic Tow ability
        c[HostConfiguration::AllowOneEngineTowing].set(0);

        // Disable Boarding
        c[HostConfiguration::AllowPrivateerTowCapture].set(0);
        c[HostConfiguration::AllowCrystalTowCapture].set(0);

        // Disable AntiCloakImmunity
        c[HostConfiguration::AntiCloakImmunity].set(0);

        // Disable PlanetImmunity
        c[HostConfiguration::PlanetsAttackKlingons].set(1);
        c[HostConfiguration::PlanetsAttackRebels].set(1);

        // Disable FullWeaponry
        c[HostConfiguration::AllowFedCombatBonus].set(0);
    }
}

AFL_TEST("server.play.HullPacker", a)
{
    const int HULL_NR = 12;

    // Environment
    afl::base::Ref<game::Root> r(game::test::makeRoot(game::HostVersion()));
    afl::base::Ref<game::spec::ShipList> sl(*new game::spec::ShipList());
    disableAutomaticHullFunctions(r->hostConfiguration());

    // Define a hull
    game::spec::Hull* h = sl->hulls().create(HULL_NR);
    h->setName("BEETLE");
    h->setTechLevel(2);
    h->setMaxBeams(3);
    h->setNumEngines(1);
    h->setMaxCargo(120);
    h->changeHullFunction(1, game::PlayerSet_t(4), game::PlayerSet_t(), true);
    h->changeHullFunction(9, game::PlayerSet_t(2), game::PlayerSet_t(), false);

    // Verify constructor
    server::play::HullPacker testee(*sl, *r, HULL_NR);
    a.checkEqual("01. getName", testee.getName(), "hull12");

    // Verify buildValue
    std::auto_ptr<server::Value_t> result(testee.buildValue());
    afl::data::Access ap(result.get());
    a.checkEqual("11", ap("NAME").toString(), "BEETLE");
    a.checkEqual("12", ap("BEAM.MAX").toInteger(), 3);
    a.checkEqual("13", ap("ENGINE.COUNT").toInteger(), 1);
    a.checkEqual("14", ap("CARGO.MAX").toInteger(), 120);

    // Note that the order in which these two are output is not currently contractual
    a.checkEqual("21", ap("FUNC").getArraySize(), 2U);
    a.checkEqual("22", ap("FUNC")[0]("ID").toInteger(), 9);
    a.checkEqual("23", ap("FUNC")[0]("PLAYERS").toInteger(), 1<<2);
    a.checkEqual("24", ap("FUNC")[0]("KIND").toInteger(), 0 /* AssignedToHull */);
    a.checkEqual("25", ap("FUNC")[1]("ID").toInteger(), 1);
    a.checkEqual("26", ap("FUNC")[1]("PLAYERS").toInteger(), 1<<4);
    a.checkEqual("27", ap("FUNC")[1]("KIND").toInteger(), 1 /* AssignedToShip */);
}
