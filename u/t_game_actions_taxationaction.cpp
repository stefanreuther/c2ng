/**
  *  \file u/t_game_actions_taxationaction.cpp
  *  \brief Test for game::actions::TaxationAction
  */

#include "game/actions/taxationaction.hpp"

#include "t_game_actions.hpp"
#include "game/map/planet.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/hostversion.hpp"
#include "game/exception.hpp"
#include "game/test/simpleturn.hpp"

using game::Element;
using game::HostVersion;
using game::actions::TaxationAction;
using game::config::HostConfiguration;
using game::map::Planet;

namespace {
    Planet& makePlanet(game::test::SimpleTurn& t)
    {
        Planet& pl = t.addPlanet(17, 3, Planet::Playable);
        pl.setCargo(Element::Colonists, 1000);
        pl.setNativeRace(game::ReptilianNatives);
        pl.setNativeGovernment(5);
        pl.setNatives(20000);
        pl.setColonistHappiness(100);
        pl.setNativeHappiness(100);
        pl.setTemperature(50);
        pl.setColonistTax(1);
        pl.setNativeTax(2);
        return pl;
    }
}


/** Test empty planet (base case).
    Taxes will report 0, not be changeable, and committing a change will fail. */
void
TestGameActionsTaxationAction::testEmpty()
{
    Planet pl(42);
    HostConfiguration config;
    TaxationAction testee(pl, config, HostVersion());

    // Check initial state
    // - colonists always available, natives not because there are none
    //   (we don't special-case colonists here)
    TS_ASSERT_EQUALS(testee.isAvailable(TaxationAction::Colonists), true);
    TS_ASSERT_EQUALS(testee.isAvailable(TaxationAction::Natives), false);

    // - nothing is modifyable
    TS_ASSERT_EQUALS(testee.isModifyable(TaxationAction::Colonists), false);
    TS_ASSERT_EQUALS(testee.isModifyable(TaxationAction::Natives), false);

    // - valid because nothing modified yet
    TS_ASSERT_EQUALS(testee.isValid(), true);

    // - all values zero
    TS_ASSERT_EQUALS(testee.getTax(TaxationAction::Colonists), 0);
    TS_ASSERT_EQUALS(testee.getTax(TaxationAction::Natives), 0);
    TS_ASSERT_EQUALS(testee.getDue(TaxationAction::Colonists), 0);
    TS_ASSERT_EQUALS(testee.getDue(TaxationAction::Natives), 0);
    TS_ASSERT_EQUALS(testee.getMinTax(TaxationAction::Colonists), 0);
    TS_ASSERT_EQUALS(testee.getMinTax(TaxationAction::Natives), 0);
    TS_ASSERT_EQUALS(testee.getMaxTax(TaxationAction::Colonists), 0);
    TS_ASSERT_EQUALS(testee.getMaxTax(TaxationAction::Natives), 0);
    TS_ASSERT_EQUALS(testee.getBovinoidSupplyContribution(), 0);

    // - commit is a no-op
    TS_ASSERT_THROWS_NOTHING(testee.commit());

    // Modify
    // - after modification, it cannot be commited
    TS_ASSERT_THROWS_NOTHING(testee.setTax(TaxationAction::Colonists, 1));
    TS_ASSERT_EQUALS(testee.isValid(), false);
    TS_ASSERT_THROWS(testee.commit(), game::Exception);
}

/** Test normal case (PHost formulas). */
void
TestGameActionsTaxationAction::testNormal()
{
    // Configure
    game::test::SimpleTurn t;
    Planet& pl = makePlanet(t);

    // Testee
    HostConfiguration config;
    TaxationAction testee(pl, config, HostVersion(HostVersion::PHost, MKVERSION(3,4,0)));

    // Check initial state
    // - everything available and modifyable, valid
    TS_ASSERT_EQUALS(testee.isAvailable(TaxationAction::Colonists), true);
    TS_ASSERT_EQUALS(testee.isAvailable(TaxationAction::Natives), true);
    TS_ASSERT_EQUALS(testee.isModifyable(TaxationAction::Colonists), true);
    TS_ASSERT_EQUALS(testee.isModifyable(TaxationAction::Natives), true);
    TS_ASSERT_EQUALS(testee.isValid(), true);

    // - initial values
    TS_ASSERT_EQUALS(testee.getTax(TaxationAction::Colonists), 1);
    TS_ASSERT_EQUALS(testee.getTax(TaxationAction::Natives), 2);
    TS_ASSERT_EQUALS(testee.getDue(TaxationAction::Colonists), 1);
    TS_ASSERT_EQUALS(testee.getDue(TaxationAction::Natives), 40);
    TS_ASSERT_EQUALS(testee.getDueLimited(TaxationAction::Colonists), 1);
    TS_ASSERT_EQUALS(testee.getDueLimited(TaxationAction::Natives), 40);
    TS_ASSERT_EQUALS(testee.getMinTax(TaxationAction::Colonists), 0);
    TS_ASSERT_EQUALS(testee.getMinTax(TaxationAction::Natives), 0);
    TS_ASSERT_EQUALS(testee.getMaxTax(TaxationAction::Colonists), 100);
    TS_ASSERT_EQUALS(testee.getMaxTax(TaxationAction::Natives), 100);
    TS_ASSERT_EQUALS(testee.getHappinessChange(TaxationAction::Colonists), 8);
    TS_ASSERT_EQUALS(testee.getHappinessChange(TaxationAction::Natives), 4);
    TS_ASSERT_EQUALS(testee.getBovinoidSupplyContribution(), 0);

    // Change
    testee.setTax(TaxationAction::Colonists, 2);
    TS_ASSERT_EQUALS(testee.getTax(TaxationAction::Colonists), 2);
    TS_ASSERT_EQUALS(testee.getDueLimited(TaxationAction::Colonists), 2);
    TS_ASSERT_EQUALS(testee.getHappinessChange(TaxationAction::Colonists), 8);
    TS_ASSERT_EQUALS(pl.getColonistTax().orElse(-1), 1);

    // Commit
    TS_ASSERT_THROWS_NOTHING(testee.commit());
    TS_ASSERT_EQUALS(pl.getColonistTax().orElse(-1), 2);
}

/** Test normal case (THost formulas). */
void
TestGameActionsTaxationAction::testNormalTim()
{
    // Configure
    game::test::SimpleTurn t;
    Planet& pl = makePlanet(t);

    // Testee
    HostConfiguration config;
    TaxationAction testee(pl, config, HostVersion(HostVersion::Host, MKVERSION(3,22,0)));

    // Check initial state
    // - initial values
    TS_ASSERT_EQUALS(testee.getDue(TaxationAction::Colonists), 1);
    TS_ASSERT_EQUALS(testee.getDue(TaxationAction::Natives), 40);
    TS_ASSERT_EQUALS(testee.getDueLimited(TaxationAction::Colonists), 1);
    TS_ASSERT_EQUALS(testee.getDueLimited(TaxationAction::Natives), 40);
    TS_ASSERT_EQUALS(testee.getHappinessChange(TaxationAction::Colonists), 8);
    TS_ASSERT_EQUALS(testee.getHappinessChange(TaxationAction::Natives), 4);

    // Change
    testee.setTax(TaxationAction::Colonists, 2);
    TS_ASSERT_EQUALS(testee.getTax(TaxationAction::Colonists), 2);
    TS_ASSERT_EQUALS(testee.getDueLimited(TaxationAction::Colonists), 2);
    TS_ASSERT_EQUALS(testee.getHappinessChange(TaxationAction::Colonists), 8);
    TS_ASSERT_EQUALS(pl.getColonistTax().orElse(-1), 1);

    // Commit
    TS_ASSERT_THROWS_NOTHING(testee.commit());
    TS_ASSERT_EQUALS(pl.getColonistTax().orElse(-1), 2);
}

/** Test income limit (MaxPlanetaryIncome). */
void
TestGameActionsTaxationAction::testIncomeLimit()
{
    // Configure
    game::test::SimpleTurn t;
    Planet& pl = makePlanet(t);

    // Testee
    HostConfiguration config;
    config[HostConfiguration::MaxPlanetaryIncome].set(25);
    TaxationAction testee(pl, config, HostVersion(HostVersion::PHost, MKVERSION(3,4,0)));

    // Check initial state
    TS_ASSERT_EQUALS(testee.getTax(TaxationAction::Colonists), 1);
    TS_ASSERT_EQUALS(testee.getTax(TaxationAction::Natives), 2);
    TS_ASSERT_EQUALS(testee.getDue(TaxationAction::Colonists), 1);
    TS_ASSERT_EQUALS(testee.getDue(TaxationAction::Natives), 40);
    TS_ASSERT_EQUALS(testee.getDueLimited(TaxationAction::Colonists), 1);
    TS_ASSERT_EQUALS(testee.getDueLimited(TaxationAction::Natives), 24);   // <- MaxPlanetaryIncome limit

    // Change colonist tax
    testee.setTax(TaxationAction::Colonists, 4);
    TS_ASSERT_EQUALS(testee.getDue(TaxationAction::Colonists), 4);
    TS_ASSERT_EQUALS(testee.getDue(TaxationAction::Natives), 40);
    TS_ASSERT_EQUALS(testee.getDueLimited(TaxationAction::Colonists), 4);
    TS_ASSERT_EQUALS(testee.getDueLimited(TaxationAction::Natives), 21);   // <- MaxPlanetaryIncome limit

    // Change native tax
    testee.setTax(TaxationAction::Natives, 3);
    TS_ASSERT_EQUALS(testee.getDue(TaxationAction::Colonists), 4);
    TS_ASSERT_EQUALS(testee.getDue(TaxationAction::Natives), 60);
    TS_ASSERT_EQUALS(testee.getDueLimited(TaxationAction::Colonists), 4);
    TS_ASSERT_EQUALS(testee.getDueLimited(TaxationAction::Natives), 21);   // <- MaxPlanetaryIncome limit

    // Change configuration
    config[HostConfiguration::MaxPlanetaryIncome].set(1000);
    TS_ASSERT_EQUALS(testee.getDueLimited(TaxationAction::Colonists), 4);
    TS_ASSERT_EQUALS(testee.getDueLimited(TaxationAction::Natives), 60);  // <- no more MaxPlanetaryIncome limit
}

