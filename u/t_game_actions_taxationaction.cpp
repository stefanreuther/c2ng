/**
  *  \file u/t_game_actions_taxationaction.cpp
  *  \brief Test for game::actions::TaxationAction
  */

#include "game/actions/taxationaction.hpp"

#include "t_game_actions.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/exception.hpp"
#include "game/hostversion.hpp"
#include "game/map/planet.hpp"
#include "game/test/simpleturn.hpp"
#include "util/numberformatter.hpp"

using afl::string::NullTranslator;
using game::Element;
using game::HostVersion;
using game::actions::TaxationAction;
using game::config::HostConfiguration;
using game::map::Planet;
using util::NumberFormatter;

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

/** Test changeRevenue().
    A: prepare planet. Call changeRevenue().
    E: tax rate and revenue must change */
void
TestGameActionsTaxationAction::testChangeRevenue()
{
    // Configure
    game::test::SimpleTurn t;
    Planet& pl = makePlanet(t);
    pl.setCargo(Element::Colonists, 100);

    // Testee
    HostConfiguration config;
    TaxationAction testee(pl, config, HostVersion(HostVersion::PHost, MKVERSION(3,4,0)));

    // - initial values
    TS_ASSERT_EQUALS(testee.getTax(TaxationAction::Colonists), 1);
    TS_ASSERT_EQUALS(testee.getDueLimited(TaxationAction::Colonists), 0);

    // Change up
    testee.changeRevenue(TaxationAction::Colonists, TaxationAction::Up);
    TS_ASSERT_EQUALS(testee.getTax(TaxationAction::Colonists), 5);
    TS_ASSERT_EQUALS(testee.getDueLimited(TaxationAction::Colonists), 1);

    // Change down
    testee.changeRevenue(TaxationAction::Colonists, TaxationAction::Down);
    TS_ASSERT_EQUALS(testee.getTax(TaxationAction::Colonists), 4);
    TS_ASSERT_EQUALS(testee.getDueLimited(TaxationAction::Colonists), 0);
}

/** Test changeTax(), revert().
    A: prepare planet. Call changeTax(), revert().
    E: tax rate must change accordingly */
void
TestGameActionsTaxationAction::testModifyRevert()
{
    // Configure
    game::test::SimpleTurn t;
    Planet& pl = makePlanet(t);

    // Testee
    HostConfiguration config;
    TaxationAction testee(pl, config, HostVersion(HostVersion::PHost, MKVERSION(3,4,0)));

    // Check initial state
    TS_ASSERT_EQUALS(testee.getTax(TaxationAction::Colonists), 1);
    TS_ASSERT_EQUALS(testee.getTax(TaxationAction::Natives), 2);

    // Modify
    testee.changeTax(TaxationAction::Colonists, 10);
    testee.changeTax(TaxationAction::Natives, -1);
    TS_ASSERT_EQUALS(testee.getTax(TaxationAction::Colonists), 11);
    TS_ASSERT_EQUALS(testee.getTax(TaxationAction::Natives), 1);

    // Revert
    testee.revert(TaxationAction::Areas_t(TaxationAction::Natives));
    TS_ASSERT_EQUALS(testee.getTax(TaxationAction::Colonists), 11);
    TS_ASSERT_EQUALS(testee.getTax(TaxationAction::Natives), 2);

    // Revert more
    testee.revert(TaxationAction::Areas_t(TaxationAction::Colonists));
    TS_ASSERT_EQUALS(testee.getTax(TaxationAction::Colonists), 1);
    TS_ASSERT_EQUALS(testee.getTax(TaxationAction::Natives), 2);
}

/** Test setSafeTax().
    A: prepare planet. Call setSafeTax().
    E: tax rate must be set for a change of 0 */
void
TestGameActionsTaxationAction::testSafeTax()
{
    // Configure
    game::test::SimpleTurn t;
    Planet& pl = makePlanet(t);

    // Testee
    HostConfiguration config;
    TaxationAction testee(pl, config, HostVersion(HostVersion::PHost, MKVERSION(3,4,0)));

    // Check initial state
    TS_ASSERT_EQUALS(testee.getTax(TaxationAction::Colonists), 1);
    TS_ASSERT_EQUALS(testee.getTax(TaxationAction::Natives), 2);

    // Colonists
    testee.setSafeTax(TaxationAction::Areas_t(TaxationAction::Colonists));
    TS_ASSERT_EQUALS(testee.getTax(TaxationAction::Colonists), 13);
    TS_ASSERT_EQUALS(testee.getHappinessChange(TaxationAction::Colonists), 0);

    // Natives
    testee.setSafeTax(TaxationAction::Areas_t(TaxationAction::Natives));
    TS_ASSERT_EQUALS(testee.getTax(TaxationAction::Natives), 8);
    TS_ASSERT_EQUALS(testee.getHappinessChange(TaxationAction::Natives), 0);
}

/** Test setNumBuildings().
    A: prepare planet. Call setNumBuildings().
    E: happiness must change according to number of buildings */
void
TestGameActionsTaxationAction::testSetNumBuildings()
{
    // Configure
    game::test::SimpleTurn t;
    Planet& pl = makePlanet(t);

    // Testee
    HostConfiguration config;
    TaxationAction testee(pl, config, HostVersion(HostVersion::PHost, MKVERSION(3,4,0)));

    // Check initial state
    TS_ASSERT_EQUALS(testee.getHappinessChange(TaxationAction::Colonists), 8);
    TS_ASSERT_EQUALS(testee.getHappinessChange(TaxationAction::Natives), 4);

    // Change number of buildings
    testee.setNumBuildings(300);
    TS_ASSERT_EQUALS(testee.getHappinessChange(TaxationAction::Colonists), 7);
    TS_ASSERT_EQUALS(testee.getHappinessChange(TaxationAction::Natives), 2);
}

/** Test describe().
    A: prepare planet. Call describe().
    E: verify returned text (regression). */
void
TestGameActionsTaxationAction::testDescribe()
{
    // Environment
    NullTranslator tx;
    NumberFormatter fmt(false, false);
    HostVersion host(HostVersion::PHost, MKVERSION(4,0,0));

    // Normal
    {
        game::test::SimpleTurn t;
        Planet& pl = makePlanet(t);
        TaxationAction testee(pl, t.config(), host);
        TS_ASSERT_EQUALS(testee.describe(TaxationAction::Colonists, tx, fmt), "Colonists pay 1 mc.\nNew happiness: happy (108)");
        TS_ASSERT_EQUALS(testee.describe(TaxationAction::Natives, tx, fmt), "They need to pay 40 mc.\nYou can collect all the money.\nNew happiness: happy (104)");
    }

    // Bovis
    {
        game::test::SimpleTurn t;
        Planet& pl = makePlanet(t);
        pl.setNativeRace(game::BovinoidNatives);
        TaxationAction testee(pl, t.config(), host);
        TS_ASSERT_EQUALS(testee.describe(TaxationAction::Natives, tx, fmt), "They need to pay 40 mc and 200 kt supplies.\nYou can collect all the money and supplies.\nNew happiness: happy (104)");
    }

    // Bovis - supply limit
    {
        game::test::SimpleTurn t;
        Planet& pl = makePlanet(t);
        pl.setNativeRace(game::BovinoidNatives);
        pl.setCargo(Element::Colonists, 50);
        TaxationAction testee(pl, t.config(), host);
        TS_ASSERT_EQUALS(testee.describe(TaxationAction::Natives, tx, fmt), "They need to pay 40 mc and 200 kt supplies.\nYou can collect all the money, but only 50 kt supplies.\nNew happiness: happy (104)");
    }

    // Income limit - colonists
    {
        game::test::SimpleTurn t;
        Planet& pl = makePlanet(t);
        pl.setColonistTax(30);
        t.config()[HostConfiguration::MaxPlanetaryIncome].set(20);
        TaxationAction testee(pl, t.config(), host);
        TS_ASSERT_EQUALS(testee.describe(TaxationAction::Colonists, tx, fmt), "Colonists pay 20 of 30 mc.\nNew happiness: calm (86)");
    }

    // Income limit - natives
    {
        game::test::SimpleTurn t;
        Planet& pl = makePlanet(t);
        t.config()[HostConfiguration::MaxPlanetaryIncome].set(20);
        TaxationAction testee(pl, t.config(), host);
        TS_ASSERT_EQUALS(testee.describe(TaxationAction::Colonists, tx, fmt), "Colonists pay 1 mc.\nNew happiness: happy (108)");
        TS_ASSERT_EQUALS(testee.describe(TaxationAction::Natives, tx, fmt), "They need to pay 40 mc.\nYou can only collect 19 mc.\nNew happiness: happy (104)");
    }

    // Income limit - bovis
    {
        game::test::SimpleTurn t;
        Planet& pl = makePlanet(t);
        t.config()[HostConfiguration::MaxPlanetaryIncome].set(20);
        pl.setNativeRace(game::BovinoidNatives);
        pl.setCargo(Element::Colonists, 50);
        pl.setColonistTax(0);
        TaxationAction testee(pl, t.config(), host);
        TS_ASSERT_EQUALS(testee.describe(TaxationAction::Natives, tx, fmt), "They need to pay 40 mc and 200 kt supplies.\nYou can collect only 20 mc and 50 kt supplies.\nNew happiness: happy (104)");
    }

    // Income limit - bovis
    {
        game::test::SimpleTurn t;
        Planet& pl = makePlanet(t);
        t.config()[HostConfiguration::MaxPlanetaryIncome].set(20);
        pl.setNativeRace(game::BovinoidNatives);
        pl.setColonistTax(0);
        TaxationAction testee(pl, t.config(), host);
        TS_ASSERT_EQUALS(testee.describe(TaxationAction::Natives, tx, fmt), "They need to pay 40 mc and 200 kt supplies.\nYou can collect only 20 mc, but all supplies.\nNew happiness: happy (104)");
    }

    // Riots
    {
        game::test::SimpleTurn t;
        Planet& pl = makePlanet(t);
        pl.setColonistHappiness(20);
        pl.setNativeHappiness(20);
        TaxationAction testee(pl, t.config(), host);
        TS_ASSERT_EQUALS(testee.describe(TaxationAction::Colonists, tx, fmt), "Riots \xE2\x80\x94 Colonists do not pay 1 mc.\nNew happiness: rioting (28)");
        TS_ASSERT_EQUALS(testee.describe(TaxationAction::Natives, tx, fmt), "They need to pay 40 mc.\nRiots \xE2\x80\x94 Natives do not pay taxes.\nNew happiness: rioting (24)");
    }

    // Amorphous
    {
        game::test::SimpleTurn t;
        Planet& pl = makePlanet(t);
        pl.setNativeRace(game::AmorphousNatives);
        TaxationAction testee(pl, t.config(), host);
        TS_ASSERT_EQUALS(testee.describe(TaxationAction::Natives, tx, fmt), "They need to pay 40 mc.\nAmorphous worms don't pay taxes. They eat 5 colonist clans.\nNew happiness: happy (104)");
    }
}
