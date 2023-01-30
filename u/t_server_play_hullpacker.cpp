/**
  *  \file u/t_server_play_hullpacker.cpp
  *  \brief Test for server::play::HullPacker
  */

#include "server/play/hullpacker.hpp"

#include "t_server_play.hpp"
#include "game/test/root.hpp"
#include "game/session.hpp"
#include "afl/data/access.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/io/nullfilesystem.hpp"

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

void
TestServerPlayHullPacker::testIt()
{
    const int HULL_NR = 12;

    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setShipList(new game::spec::ShipList());
    disableAutomaticHullFunctions(session.getRoot()->hostConfiguration());

    // Define a hull
    game::spec::Hull* h = session.getShipList()->hulls().create(HULL_NR);
    h->setName("BEETLE");
    h->setTechLevel(2);
    h->setMaxBeams(3);
    h->setNumEngines(1);
    h->setMaxCargo(120);
    h->changeHullFunction(1, game::PlayerSet_t(4), game::PlayerSet_t(), true);
    h->changeHullFunction(9, game::PlayerSet_t(2), game::PlayerSet_t(), false);

    // Verify constructor
    server::play::HullPacker testee(session, HULL_NR);
    TS_ASSERT_EQUALS(testee.getName(), "hull12");

    // Verify buildValue
    std::auto_ptr<server::Value_t> result(testee.buildValue());
    afl::data::Access a(result.get());
    TS_ASSERT_EQUALS(a("NAME").toString(), "BEETLE");
    TS_ASSERT_EQUALS(a("BEAM.MAX").toInteger(), 3);
    TS_ASSERT_EQUALS(a("ENGINE.COUNT").toInteger(), 1);
    TS_ASSERT_EQUALS(a("CARGO.MAX").toInteger(), 120);

    // Note that the order in which these two are output is not currently contractual
    TS_ASSERT_EQUALS(a("FUNC").getArraySize(), 2U);
    TS_ASSERT_EQUALS(a("FUNC")[0]("ID").toInteger(), 9);
    TS_ASSERT_EQUALS(a("FUNC")[0]("PLAYERS").toInteger(), 1<<2);
    TS_ASSERT_EQUALS(a("FUNC")[0]("KIND").toInteger(), 0 /* AssignedToHull */);
    TS_ASSERT_EQUALS(a("FUNC")[1]("ID").toInteger(), 1);
    TS_ASSERT_EQUALS(a("FUNC")[1]("PLAYERS").toInteger(), 1<<4);
    TS_ASSERT_EQUALS(a("FUNC")[1]("KIND").toInteger(), 1 /* AssignedToShip */);
}

