/**
  *  \file test/game/actions/taxationactiontest.cpp
  *  \brief Test for game::actions::TaxationAction
  */

#include "game/actions/taxationaction.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
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
AFL_TEST("game.actions.TaxationAction:empty", a)
{
    Planet pl(42);
    HostConfiguration config;
    TaxationAction testee(pl, config, HostVersion());

    // Check initial state
    // - colonists always available, natives not because there are none
    //   (we don't special-case colonists here)
    a.checkEqual("01. isAvailable", testee.isAvailable(TaxationAction::Colonists), true);
    a.checkEqual("02. isAvailable", testee.isAvailable(TaxationAction::Natives), false);

    // - nothing is modifyable
    a.checkEqual("11. isModifyable", testee.isModifyable(TaxationAction::Colonists), false);
    a.checkEqual("12. isModifyable", testee.isModifyable(TaxationAction::Natives), false);

    // - valid because nothing modified yet
    a.checkEqual("21. isValid", testee.isValid(), true);

    // - all values zero
    a.checkEqual("31. getTax", testee.getTax(TaxationAction::Colonists), 0);
    a.checkEqual("32. getTax", testee.getTax(TaxationAction::Natives), 0);
    a.checkEqual("33. getDue", testee.getDue(TaxationAction::Colonists), 0);
    a.checkEqual("34. getDue", testee.getDue(TaxationAction::Natives), 0);
    a.checkEqual("35. getMinTax", testee.getMinTax(TaxationAction::Colonists), 0);
    a.checkEqual("36. getMinTax", testee.getMinTax(TaxationAction::Natives), 0);
    a.checkEqual("37. getMaxTax", testee.getMaxTax(TaxationAction::Colonists), 0);
    a.checkEqual("38. getMaxTax", testee.getMaxTax(TaxationAction::Natives), 0);
    a.checkEqual("39. getBovinoidSupplyContribution", testee.getBovinoidSupplyContribution(), 0);

    // - commit is a no-op
    AFL_CHECK_SUCCEEDS(a("41. commit"), testee.commit());

    // Modify
    // - after modification, it cannot be commited
    AFL_CHECK_SUCCEEDS(a("51. setTax"), testee.setTax(TaxationAction::Colonists, 1));
    a.checkEqual("52. isValid", testee.isValid(), false);
    AFL_CHECK_THROWS(a("53. commit"), testee.commit(), game::Exception);
}

/** Test normal case (PHost formulas). */
AFL_TEST("game.actions.TaxationAction:normal:phost", a)
{
    // Configure
    game::test::SimpleTurn t;
    Planet& pl = makePlanet(t);

    // Testee
    HostConfiguration config;
    TaxationAction testee(pl, config, HostVersion(HostVersion::PHost, MKVERSION(3,4,0)));

    // Check initial state
    // - everything available and modifyable, valid
    a.checkEqual("01. isAvailable", testee.isAvailable(TaxationAction::Colonists), true);
    a.checkEqual("02. isAvailable", testee.isAvailable(TaxationAction::Natives), true);
    a.checkEqual("03. isModifyable", testee.isModifyable(TaxationAction::Colonists), true);
    a.checkEqual("04. isModifyable", testee.isModifyable(TaxationAction::Natives), true);
    a.checkEqual("05. isValid", testee.isValid(), true);

    // - initial values
    a.checkEqual("11. getTax", testee.getTax(TaxationAction::Colonists), 1);
    a.checkEqual("12. getTax", testee.getTax(TaxationAction::Natives), 2);
    a.checkEqual("13. getDue", testee.getDue(TaxationAction::Colonists), 1);
    a.checkEqual("14. getDue", testee.getDue(TaxationAction::Natives), 40);
    a.checkEqual("15. getDue", testee.getDueLimited(TaxationAction::Colonists), 1);
    a.checkEqual("16. getDue", testee.getDueLimited(TaxationAction::Natives), 40);
    a.checkEqual("17. getMinTax", testee.getMinTax(TaxationAction::Colonists), 0);
    a.checkEqual("18. getMinTax", testee.getMinTax(TaxationAction::Natives), 0);
    a.checkEqual("19. getMaxTax", testee.getMaxTax(TaxationAction::Colonists), 100);
    a.checkEqual("20. getMaxTax", testee.getMaxTax(TaxationAction::Natives), 100);
    a.checkEqual("21. getHappinessChange", testee.getHappinessChange(TaxationAction::Colonists), 8);
    a.checkEqual("22. getHappinessChange", testee.getHappinessChange(TaxationAction::Natives), 4);
    a.checkEqual("23. getBovinoidSupplyContribution", testee.getBovinoidSupplyContribution(), 0);

    // Change
    testee.setTax(TaxationAction::Colonists, 2);
    a.checkEqual("31. getTax", testee.getTax(TaxationAction::Colonists), 2);
    a.checkEqual("32. getDue", testee.getDueLimited(TaxationAction::Colonists), 2);
    a.checkEqual("33. getHappinessChange", testee.getHappinessChange(TaxationAction::Colonists), 8);
    a.checkEqual("34. getColonistTax", pl.getColonistTax().orElse(-1), 1);

    // Commit
    AFL_CHECK_SUCCEEDS(a("41. commit"), testee.commit());
    a.checkEqual("42. getColonistTax", pl.getColonistTax().orElse(-1), 2);
}

/** Test normal case (THost formulas). */
AFL_TEST("game.actions.TaxationAction:normal:host", a)
{
    // Configure
    game::test::SimpleTurn t;
    Planet& pl = makePlanet(t);

    // Testee
    HostConfiguration config;
    TaxationAction testee(pl, config, HostVersion(HostVersion::Host, MKVERSION(3,22,0)));

    // Check initial state
    // - initial values
    a.checkEqual("01. getDue", testee.getDue(TaxationAction::Colonists), 1);
    a.checkEqual("02. getDue", testee.getDue(TaxationAction::Natives), 40);
    a.checkEqual("03. getDue", testee.getDueLimited(TaxationAction::Colonists), 1);
    a.checkEqual("04. getDue", testee.getDueLimited(TaxationAction::Natives), 40);
    a.checkEqual("05. getHappinessChange", testee.getHappinessChange(TaxationAction::Colonists), 8);
    a.checkEqual("06. getHappinessChange", testee.getHappinessChange(TaxationAction::Natives), 4);

    // Change
    testee.setTax(TaxationAction::Colonists, 2);
    a.checkEqual("11. getTax", testee.getTax(TaxationAction::Colonists), 2);
    a.checkEqual("12. getDue", testee.getDueLimited(TaxationAction::Colonists), 2);
    a.checkEqual("13. getHappinessChange", testee.getHappinessChange(TaxationAction::Colonists), 8);
    a.checkEqual("14. getColonistTax", pl.getColonistTax().orElse(-1), 1);

    // Commit
    AFL_CHECK_SUCCEEDS(a("21. commit"), testee.commit());
    a.checkEqual("22. getColonistTax", pl.getColonistTax().orElse(-1), 2);
}

/** Test income limit (MaxPlanetaryIncome). */
AFL_TEST("game.actions.TaxationAction:MaxPlanetaryIncome", a)
{
    // Configure
    game::test::SimpleTurn t;
    Planet& pl = makePlanet(t);

    // Testee
    HostConfiguration config;
    config[HostConfiguration::MaxPlanetaryIncome].set(25);
    TaxationAction testee(pl, config, HostVersion(HostVersion::PHost, MKVERSION(3,4,0)));

    // Check initial state
    a.checkEqual("01. getTax", testee.getTax(TaxationAction::Colonists), 1);
    a.checkEqual("02. getTax", testee.getTax(TaxationAction::Natives), 2);
    a.checkEqual("03. getDue", testee.getDue(TaxationAction::Colonists), 1);
    a.checkEqual("04. getDue", testee.getDue(TaxationAction::Natives), 40);
    a.checkEqual("05. getDue", testee.getDueLimited(TaxationAction::Colonists), 1);
    a.checkEqual("06. getDue", testee.getDueLimited(TaxationAction::Natives), 24);   // <- MaxPlanetaryIncome limit

    // Change colonist tax
    testee.setTax(TaxationAction::Colonists, 4);
    a.checkEqual("11. getDue", testee.getDue(TaxationAction::Colonists), 4);
    a.checkEqual("12. getDue", testee.getDue(TaxationAction::Natives), 40);
    a.checkEqual("13. getDue", testee.getDueLimited(TaxationAction::Colonists), 4);
    a.checkEqual("14. getDue", testee.getDueLimited(TaxationAction::Natives), 21);   // <- MaxPlanetaryIncome limit

    // Change native tax
    testee.setTax(TaxationAction::Natives, 3);
    a.checkEqual("21. getDue", testee.getDue(TaxationAction::Colonists), 4);
    a.checkEqual("22. getDue", testee.getDue(TaxationAction::Natives), 60);
    a.checkEqual("23. getDue", testee.getDueLimited(TaxationAction::Colonists), 4);
    a.checkEqual("24. getDue", testee.getDueLimited(TaxationAction::Natives), 21);   // <- MaxPlanetaryIncome limit

    // Change configuration
    config[HostConfiguration::MaxPlanetaryIncome].set(1000);
    a.checkEqual("31. getDue", testee.getDueLimited(TaxationAction::Colonists), 4);
    a.checkEqual("32. getDue", testee.getDueLimited(TaxationAction::Natives), 60);  // <- no more MaxPlanetaryIncome limit
}

/** Test changeRevenue().
    A: prepare planet. Call changeRevenue().
    E: tax rate and revenue must change */
AFL_TEST("game.actions.TaxationAction:changeRevenue", a)
{
    // Configure
    game::test::SimpleTurn t;
    Planet& pl = makePlanet(t);
    pl.setCargo(Element::Colonists, 100);

    // Testee
    HostConfiguration config;
    TaxationAction testee(pl, config, HostVersion(HostVersion::PHost, MKVERSION(3,4,0)));

    // - initial values
    a.checkEqual("01. getTax", testee.getTax(TaxationAction::Colonists), 1);
    a.checkEqual("02. getDue", testee.getDueLimited(TaxationAction::Colonists), 0);

    // Change up
    testee.changeRevenue(TaxationAction::Colonists, TaxationAction::Up);
    a.checkEqual("11. getTax", testee.getTax(TaxationAction::Colonists), 5);
    a.checkEqual("12. getDue", testee.getDueLimited(TaxationAction::Colonists), 1);

    // Change down
    testee.changeRevenue(TaxationAction::Colonists, TaxationAction::Down);
    a.checkEqual("21. getTax", testee.getTax(TaxationAction::Colonists), 4);
    a.checkEqual("22. getDue", testee.getDueLimited(TaxationAction::Colonists), 0);
}

/** Test changeTax(), revert().
    A: prepare planet. Call changeTax(), revert().
    E: tax rate must change accordingly */
AFL_TEST("game.actions.TaxationAction:revert", a)
{
    // Configure
    game::test::SimpleTurn t;
    Planet& pl = makePlanet(t);

    // Testee
    HostConfiguration config;
    TaxationAction testee(pl, config, HostVersion(HostVersion::PHost, MKVERSION(3,4,0)));

    // Check initial state
    a.checkEqual("01. getTax", testee.getTax(TaxationAction::Colonists), 1);
    a.checkEqual("02. getTax", testee.getTax(TaxationAction::Natives), 2);

    // Modify
    testee.changeTax(TaxationAction::Colonists, 10);
    testee.changeTax(TaxationAction::Natives, -1);
    a.checkEqual("11. getTax", testee.getTax(TaxationAction::Colonists), 11);
    a.checkEqual("12. getTax", testee.getTax(TaxationAction::Natives), 1);

    // Revert
    testee.revert(TaxationAction::Areas_t(TaxationAction::Natives));
    a.checkEqual("21. getTax", testee.getTax(TaxationAction::Colonists), 11);
    a.checkEqual("22. getTax", testee.getTax(TaxationAction::Natives), 2);

    // Revert more
    testee.revert(TaxationAction::Areas_t(TaxationAction::Colonists));
    a.checkEqual("31. getTax", testee.getTax(TaxationAction::Colonists), 1);
    a.checkEqual("32. getTax", testee.getTax(TaxationAction::Natives), 2);
}

/** Test setSafeTax().
    A: prepare planet. Call setSafeTax().
    E: tax rate must be set for a change of 0 */
AFL_TEST("game.actions.TaxationAction:setSafeTax", a)
{
    // Configure
    game::test::SimpleTurn t;
    Planet& pl = makePlanet(t);

    // Testee
    HostConfiguration config;
    TaxationAction testee(pl, config, HostVersion(HostVersion::PHost, MKVERSION(3,4,0)));

    // Check initial state
    a.checkEqual("01. getTax", testee.getTax(TaxationAction::Colonists), 1);
    a.checkEqual("02. getTax", testee.getTax(TaxationAction::Natives), 2);

    // Colonists
    testee.setSafeTax(TaxationAction::Areas_t(TaxationAction::Colonists));
    a.checkEqual("11. getTax", testee.getTax(TaxationAction::Colonists), 13);
    a.checkEqual("12. getHappinessChange", testee.getHappinessChange(TaxationAction::Colonists), 0);

    // Natives
    testee.setSafeTax(TaxationAction::Areas_t(TaxationAction::Natives));
    a.checkEqual("21. getTax", testee.getTax(TaxationAction::Natives), 8);
    a.checkEqual("22. getHappinessChange", testee.getHappinessChange(TaxationAction::Natives), 0);
}

/** Test setNumBuildings().
    A: prepare planet. Call setNumBuildings().
    E: happiness must change according to number of buildings */
AFL_TEST("game.actions.TaxationAction:setNumBuildings", a)
{
    // Configure
    game::test::SimpleTurn t;
    Planet& pl = makePlanet(t);

    // Testee
    HostConfiguration config;
    TaxationAction testee(pl, config, HostVersion(HostVersion::PHost, MKVERSION(3,4,0)));

    // Check initial state
    a.checkEqual("01. getHappinessChange", testee.getHappinessChange(TaxationAction::Colonists), 8);
    a.checkEqual("02. getHappinessChange", testee.getHappinessChange(TaxationAction::Natives), 4);

    // Change number of buildings
    testee.setNumBuildings(300);
    a.checkEqual("11. getHappinessChange", testee.getHappinessChange(TaxationAction::Colonists), 7);
    a.checkEqual("12. getHappinessChange", testee.getHappinessChange(TaxationAction::Natives), 2);
}

/** Test describe().
    A: prepare planet. Call describe().
    E: verify returned text (regression). */
AFL_TEST("game.actions.TaxationAction:describe", a)
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
        a.checkEqual("01. describe", testee.describe(TaxationAction::Colonists, tx, fmt), "Colonists pay 1 mc.\nNew happiness: happy (108)");
        a.checkEqual("02. describe", testee.describe(TaxationAction::Natives, tx, fmt), "They need to pay 40 mc.\nYou can collect all the money.\nNew happiness: happy (104)");
    }

    // Bovis
    {
        game::test::SimpleTurn t;
        Planet& pl = makePlanet(t);
        pl.setNativeRace(game::BovinoidNatives);
        TaxationAction testee(pl, t.config(), host);
        a.checkEqual("11. describe", testee.describe(TaxationAction::Natives, tx, fmt), "They need to pay 40 mc and 200 kt supplies.\nYou can collect all the money and supplies.\nNew happiness: happy (104)");
    }

    // Bovis - supply limit
    {
        game::test::SimpleTurn t;
        Planet& pl = makePlanet(t);
        pl.setNativeRace(game::BovinoidNatives);
        pl.setCargo(Element::Colonists, 50);
        TaxationAction testee(pl, t.config(), host);
        a.checkEqual("21. describe", testee.describe(TaxationAction::Natives, tx, fmt), "They need to pay 40 mc and 200 kt supplies.\nYou can collect all the money, but only 50 kt supplies.\nNew happiness: happy (104)");
    }

    // Income limit - colonists
    {
        game::test::SimpleTurn t;
        Planet& pl = makePlanet(t);
        pl.setColonistTax(30);
        t.config()[HostConfiguration::MaxPlanetaryIncome].set(20);
        TaxationAction testee(pl, t.config(), host);
        a.checkEqual("31. describe", testee.describe(TaxationAction::Colonists, tx, fmt), "Colonists pay 20 of 30 mc.\nNew happiness: calm (86)");
    }

    // Income limit - natives
    {
        game::test::SimpleTurn t;
        Planet& pl = makePlanet(t);
        t.config()[HostConfiguration::MaxPlanetaryIncome].set(20);
        TaxationAction testee(pl, t.config(), host);
        a.checkEqual("41. describe", testee.describe(TaxationAction::Colonists, tx, fmt), "Colonists pay 1 mc.\nNew happiness: happy (108)");
        a.checkEqual("42. describe", testee.describe(TaxationAction::Natives, tx, fmt), "They need to pay 40 mc.\nYou can only collect 19 mc.\nNew happiness: happy (104)");
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
        a.checkEqual("51. describe", testee.describe(TaxationAction::Natives, tx, fmt), "They need to pay 40 mc and 200 kt supplies.\nYou can collect only 20 mc and 50 kt supplies.\nNew happiness: happy (104)");
    }

    // Income limit - bovis
    {
        game::test::SimpleTurn t;
        Planet& pl = makePlanet(t);
        t.config()[HostConfiguration::MaxPlanetaryIncome].set(20);
        pl.setNativeRace(game::BovinoidNatives);
        pl.setColonistTax(0);
        TaxationAction testee(pl, t.config(), host);
        a.checkEqual("61. describe", testee.describe(TaxationAction::Natives, tx, fmt), "They need to pay 40 mc and 200 kt supplies.\nYou can collect only 20 mc, but all supplies.\nNew happiness: happy (104)");
    }

    // Riots
    {
        game::test::SimpleTurn t;
        Planet& pl = makePlanet(t);
        pl.setColonistHappiness(20);
        pl.setNativeHappiness(20);
        TaxationAction testee(pl, t.config(), host);
        a.checkEqual("71. describe", testee.describe(TaxationAction::Colonists, tx, fmt), "Riots \xE2\x80\x94 Colonists do not pay 1 mc.\nNew happiness: rioting (28)");
        a.checkEqual("72. describe", testee.describe(TaxationAction::Natives, tx, fmt), "They need to pay 40 mc.\nRiots \xE2\x80\x94 Natives do not pay taxes.\nNew happiness: rioting (24)");
    }

    // Amorphous
    {
        game::test::SimpleTurn t;
        Planet& pl = makePlanet(t);
        pl.setNativeRace(game::AmorphousNatives);
        TaxationAction testee(pl, t.config(), host);
        a.checkEqual("81. describe", testee.describe(TaxationAction::Natives, tx, fmt), "They need to pay 40 mc.\nAmorphous worms don't pay taxes. They eat 5 colonist clans.\nNew happiness: happy (104)");
    }
}

/** Test describe() and setEffectors().
    A: prepare planet. Call setEffectors().
    E: verify text returned by describe(). */
AFL_TEST("game.actions.TaxationAction:setEffectors:Hiss", a)
{
    // Environment
    afl::string::NullTranslator tx;
    NumberFormatter fmt(false, false);

    // Configure
    game::test::SimpleTurn t;
    Planet& pl = makePlanet(t);
    pl.setColonistHappiness(91);

    // Testee
    HostConfiguration config;
    config[HostConfiguration::HissEffectRate].set(5);
    TaxationAction testee(pl, config, HostVersion(HostVersion::PHost, MKVERSION(3,4,0)));

    // Check initial state
    a.checkEqual("01. getHappinessChange", testee.getHappinessChange(TaxationAction::Colonists), 8);
    a.checkEqual("02. getHappinessChange", testee.getHappinessChange(TaxationAction::Natives), 4);
    a.checkEqual("03. getBovinoidSupplyContribution", testee.getBovinoidSupplyContribution(), 0);
    a.checkEqual("04. describe", testee.describe(TaxationAction::Colonists, tx, fmt), "Colonists pay 1 mc.\nNew happiness: happy (99)");

    // Change
    game::map::PlanetEffectors eff;
    eff.set(game::map::PlanetEffectors::Hiss, 3);
    testee.setEffectors(eff);

    // Verify
    a.checkEqual("11. describe", testee.describe(TaxationAction::Colonists, tx, fmt), "Colonists pay 1 mc.\nNew happiness: happy (108)");
}
