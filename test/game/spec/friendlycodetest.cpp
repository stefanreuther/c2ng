/**
  *  \file test/game/spec/friendlycodetest.cpp
  *  \brief Test for game::spec::FriendlyCode
  */

#include "game/spec/friendlycode.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/configuration.hpp"
#include "game/map/minefield.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/playerlist.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/registrationkey.hpp"
#include <stdexcept>

using game::spec::FriendlyCode;

/** Test friendly code constructors. */
AFL_TEST("game.spec.FriendlyCode:basics", a)
{
    // ex GameFcodeTestSuite::testFCode
    afl::string::NullTranslator tx;
    FriendlyCode mkt("mkt", "sc,make torps", tx);
    FriendlyCode lfm("lfm", "sc+9ab,make fighters", tx);
    FriendlyCode att("ATT", "p,attack", tx);

    game::PlayerList list;

    a.checkEqual("01. getCode", mkt.getCode(), "mkt");
    a.check("02. getRaces", mkt.getRaces().contains(1));
    a.check("03. getRaces", mkt.getRaces().contains(2));
    a.check("04. getRaces", mkt.getRaces().contains(10));
    a.checkEqual("05. getDescription", mkt.getDescription(list, tx), "make torps");

    a.check("11. getRaces", !lfm.getRaces().contains(1));
    a.check("12. getRaces", !lfm.getRaces().contains(8));
    a.check("13. getRaces", lfm.getRaces().contains(9));
    a.check("14. getRaces", lfm.getRaces().contains(10));
    a.check("15. getRaces", lfm.getRaces().contains(11));
}

/** Test constructor failures. */
AFL_TEST("game.spec.FriendlyCode:construction-failure", a)
{
    afl::string::NullTranslator tx;

    // Player character out of range
    AFL_CHECK_THROWS(a("01. bad player"), FriendlyCode("xy0", "+0,hi", tx), std::exception);
    AFL_CHECK_THROWS(a("02. bad player"), FriendlyCode("xyz", "+z,hi", tx), std::exception);

    // Missing description
    AFL_CHECK_THROWS(a("11. missing description"), FriendlyCode("xyz", "", tx), std::exception);
    AFL_CHECK_THROWS(a("12. missing description"), FriendlyCode("xyz", "p", tx), std::exception);
}

/** Test initial state getters. */
AFL_TEST("game.spec.FriendlyCode:defaults", a)
{
    afl::string::NullTranslator tx;
    game::PlayerList list;
    FriendlyCode testee;
    a.checkEqual("01. getCode", testee.getCode(), "");
    a.checkEqual("02. getDescription", testee.getDescription(list, tx), "");
    a.check("03. getFlags", testee.getFlags().empty());
    a.check("04. getRaces", testee.getRaces().empty());
}

/** Test getDescription(). */
AFL_TEST("game.spec.FriendlyCode:getDescription", a)
{
    afl::string::NullTranslator tx;

    // Player list
    game::PlayerList list;
    game::Player* pl = list.create(3);
    pl->setName(pl->LongName, "Long");
    pl->setName(pl->AdjectiveName, "Adj");
    pl->setName(pl->ShortName, "Short");
    pl->setName(pl->OriginalLongName, "OrigLong");
    pl->setName(pl->OriginalAdjectiveName, "OrigAdj");
    pl->setName(pl->OriginalShortName, "OrigShort");

    // Verify descriptions
    a.checkEqual("01", FriendlyCode("xyz",",[%3]", tx).getDescription(list, tx), "[Short]");
    a.checkEqual("02", FriendlyCode("xyz",",[%-3]", tx).getDescription(list, tx), "[Adj]");
    a.checkEqual("03", FriendlyCode("xyz",",[%2]", tx).getDescription(list, tx), "[2]");
    a.checkEqual("04", FriendlyCode("xyz",",[%-2]", tx).getDescription(list, tx), "[2]");
}

/** Test worksOn(). */
AFL_TEST("game.spec.FriendlyCode:worksOn", a)
{
    // Environment
    afl::base::Ref<game::config::HostConfiguration> rconfig = game::config::HostConfiguration::create();
    game::config::HostConfiguration& config = *rconfig;
    config.setDefaultValues();

    afl::sys::Log log;
    afl::string::NullTranslator tx;

    // Some fcodes
    FriendlyCode planetFC("pfc", "p,xxx", tx);
    FriendlyCode baseFC("bfc", "b,xxx", tx);
    FriendlyCode shipFC("sfc", "s,xxx", tx);
    FriendlyCode fedFC("ffc", "p+1,xxx", tx);
    FriendlyCode prefixFC("p", "X,xxx", tx);

    // Fed planet
    {
        game::map::Planet p(9);
        p.setOwner(1);
        p.setPlayability(p.ReadOnly);
        a.check("01", planetFC.worksOn(FriendlyCode::Filter::fromPlanet(p, config)));
        a.check("02", !baseFC.worksOn(FriendlyCode::Filter::fromPlanet(p, config)));
        a.check("03", !shipFC.worksOn(FriendlyCode::Filter::fromPlanet(p, config)));
        a.check("04", fedFC.worksOn(FriendlyCode::Filter::fromPlanet(p, config)));
        a.check("05", !prefixFC.worksOn(FriendlyCode::Filter::fromPlanet(p, config)));

        const game::map::Object& obj = p;
        const game::spec::ShipList shipList;
        const game::UnitScoreDefinitionList scoreDefinitions;
        a.check("11", planetFC.worksOn(FriendlyCode::Filter::fromObject(obj, scoreDefinitions, shipList, config)));
        a.check("12", !baseFC.worksOn(FriendlyCode::Filter::fromObject(obj, scoreDefinitions, shipList, config)));
        a.check("13", !shipFC.worksOn(FriendlyCode::Filter::fromObject(obj, scoreDefinitions, shipList, config)));
        a.check("14", fedFC.worksOn(FriendlyCode::Filter::fromObject(obj, scoreDefinitions, shipList, config)));
        a.check("15", !prefixFC.worksOn(FriendlyCode::Filter::fromObject(obj, scoreDefinitions, shipList, config)));
    }

    // Lizard planet
    {
        game::map::Planet p(9);
        p.setOwner(2);
        p.setPlayability(p.ReadOnly);
        a.check("21", planetFC.worksOn(FriendlyCode::Filter::fromPlanet(p, config)));
        a.check("22", !baseFC.worksOn(FriendlyCode::Filter::fromPlanet(p, config)));
        a.check("23", !shipFC.worksOn(FriendlyCode::Filter::fromPlanet(p, config)));
        a.check("24", !fedFC.worksOn(FriendlyCode::Filter::fromPlanet(p, config)));
        a.check("25", !prefixFC.worksOn(FriendlyCode::Filter::fromPlanet(p, config)));
    }

    // Unknown planet
    {
        game::map::Planet p(9);
        a.check("31", !planetFC.worksOn(FriendlyCode::Filter::fromPlanet(p, config)));
        a.check("32", !baseFC.worksOn(FriendlyCode::Filter::fromPlanet(p, config)));
        a.check("33", !shipFC.worksOn(FriendlyCode::Filter::fromPlanet(p, config)));
        a.check("34", !fedFC.worksOn(FriendlyCode::Filter::fromPlanet(p, config)));
        a.check("35", !prefixFC.worksOn(FriendlyCode::Filter::fromPlanet(p, config)));
    }

    // Unknown, played planet [cannot happen]
    {
        game::map::Planet p(9);
        p.setPlayability(p.ReadOnly);
        a.check("41", !planetFC.worksOn(FriendlyCode::Filter::fromPlanet(p, config)));
        a.check("42", !baseFC.worksOn(FriendlyCode::Filter::fromPlanet(p, config)));
        a.check("43", !shipFC.worksOn(FriendlyCode::Filter::fromPlanet(p, config)));
        a.check("44", !fedFC.worksOn(FriendlyCode::Filter::fromPlanet(p, config)));
        a.check("45", !prefixFC.worksOn(FriendlyCode::Filter::fromPlanet(p, config)));
    }

    // Lizard base
    {
        game::map::Planet p(9);
        p.setOwner(2);
        p.setPosition(game::map::Point(2000, 2000));
        p.addPlanetSource(game::PlayerSet_t(2));
        p.addBaseSource(game::PlayerSet_t(2));
        p.setPlayability(p.ReadOnly);
        p.internalCheck(game::map::Configuration(), game::PlayerSet_t(2), 15, tx, log);
        a.check("51", planetFC.worksOn(FriendlyCode::Filter::fromPlanet(p, config)));
        a.check("52", baseFC.worksOn(FriendlyCode::Filter::fromPlanet(p, config)));
        a.check("53", !shipFC.worksOn(FriendlyCode::Filter::fromPlanet(p, config)));
        a.check("54", !fedFC.worksOn(FriendlyCode::Filter::fromPlanet(p, config)));
        a.check("55", !prefixFC.worksOn(FriendlyCode::Filter::fromPlanet(p, config)));
    }

    // Minefield
    {
        game::map::Minefield m(90);
        const game::spec::ShipList shipList;
        const game::UnitScoreDefinitionList scoreDefinitions;
        m.addReport(game::map::Point(2000, 2000), 2, m.IsMine, m.RadiusKnown, 100, 5, m.MinefieldLaid);
        m.setPlayability(m.ReadOnly);
        a.check("61", !planetFC.worksOn(FriendlyCode::Filter::fromObject(m, scoreDefinitions, shipList, config)));
        a.check("62", !baseFC.worksOn(FriendlyCode::Filter::fromObject(m, scoreDefinitions, shipList, config)));
        a.check("63", !shipFC.worksOn(FriendlyCode::Filter::fromObject(m, scoreDefinitions, shipList, config)));
        a.check("64", !fedFC.worksOn(FriendlyCode::Filter::fromObject(m, scoreDefinitions, shipList, config)));
        a.check("65", !prefixFC.worksOn(FriendlyCode::Filter::fromObject(m, scoreDefinitions, shipList, config)));
    }
}

/** Test worksOn(), for ships. */
AFL_TEST("game.spec.FriendlyCode:worksOn:ship", a)
{
    // Environment
    game::UnitScoreDefinitionList scoreDefinitions;
    game::spec::ShipList shipList;
    afl::base::Ref<game::config::HostConfiguration> rconfig = game::config::HostConfiguration::create();
    game::config::HostConfiguration& config = *rconfig;
    config.setDefaultValues();

    const int HULL_NR = 12;
    shipList.hulls().create(HULL_NR);

    afl::sys::Log log;
    afl::string::NullTranslator tx;

    // Some fcodes
    FriendlyCode planetFC("pfc", "p,xxx", tx);
    FriendlyCode shipFC("sfc", "s,xxx", tx);
    FriendlyCode fedFC("ffc", "s+1,xxx", tx);
    FriendlyCode capFC("cfc", "sc,xxx", tx);
    FriendlyCode alchemyFC("afc", "sa,xxx", tx);
    FriendlyCode prefixFC("p", "X,xxx", tx);

    // Fed ship
    {
        game::map::Ship sh(9);
        sh.setOwner(1);
        sh.setHull(HULL_NR);
        sh.setPlayability(sh.ReadOnly);
        a.check("01", !planetFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
        a.check("02", shipFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
        a.check("03", fedFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
        a.check("04", !capFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
        a.check("05", !alchemyFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
        a.check("06", !prefixFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));

        const game::map::Object& obj = sh;
        a.check("11", !planetFC.worksOn(FriendlyCode::Filter::fromObject(obj, scoreDefinitions, shipList, config)));
        a.check("12", shipFC.worksOn(FriendlyCode::Filter::fromObject(obj, scoreDefinitions, shipList, config)));
        a.check("13", fedFC.worksOn(FriendlyCode::Filter::fromObject(obj, scoreDefinitions, shipList, config)));
        a.check("14", !capFC.worksOn(FriendlyCode::Filter::fromObject(obj, scoreDefinitions, shipList, config)));
        a.check("15", !alchemyFC.worksOn(FriendlyCode::Filter::fromObject(obj, scoreDefinitions, shipList, config)));
        a.check("16", !prefixFC.worksOn(FriendlyCode::Filter::fromObject(obj, scoreDefinitions, shipList, config)));
    }

    // Lizard warship
    {
        game::map::Ship sh(9);
        sh.setOwner(2);
        sh.setHull(HULL_NR);
        sh.setPlayability(sh.ReadOnly);
        sh.setNumBays(1);
        a.check("21", shipFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
        a.check("22", !fedFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
        a.check("23", capFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
        a.check("24", !alchemyFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
        a.check("25", !prefixFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
    }

    // Alchemy ship
    {
        game::map::Ship sh(9);
        sh.setOwner(2);
        sh.setHull(HULL_NR);
        sh.setPlayability(sh.ReadOnly);
        sh.setNumBeams(1);
        sh.setBeamType(10);
        sh.addShipSpecialFunction(game::spec::BasicHullFunction::NeutronicRefinery);
        a.check("31", shipFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
        a.check("32", !fedFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
        a.check("33", capFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
        a.check("34", alchemyFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
        a.check("35", !prefixFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
    }

    // Remote-controlled ship
    {
        game::map::Ship sh(9);
        sh.setOwner(1);
        sh.setHull(HULL_NR);

        game::parser::MessageInformation info(game::parser::MessageInformation::Ship, 9, 100);
        info.addValue(game::parser::mi_ShipRemoteFlag, 4);
        sh.addMessageInformation(info, game::PlayerSet_t(1));

        sh.setPlayability(sh.ReadOnly);
        a.check("41", !fedFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
        a.check("42", !prefixFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
    }

    // Not-played ship
    {
        game::map::Ship sh(9);
        sh.setOwner(2);
        sh.setHull(HULL_NR);
        sh.setPlayability(sh.NotPlayable);
        sh.setNumBays(1);
        a.check("51", !shipFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
        a.check("52", !fedFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
        a.check("53", !capFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
        a.check("54", !alchemyFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
        a.check("55", !prefixFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
    }

    // Ownerless ship [cannot happen normally]
    {
        game::map::Ship sh(9);
        sh.setHull(HULL_NR);
        sh.setPlayability(sh.ReadOnly);
        sh.setNumBays(1);
        a.check("61", !shipFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
        a.check("62", !fedFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
        a.check("63", !capFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
        a.check("64", !alchemyFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
        a.check("65", !prefixFC.worksOn(FriendlyCode::Filter::fromShip(sh, scoreDefinitions, shipList, config)));
    }
}

/** Test isPermitted(). */
AFL_TEST("game.spec.FriendlyCode:isPermitted", a)
{
    afl::string::NullTranslator tx;
    FriendlyCode unregFC("ufc", "s,xxx", tx);
    FriendlyCode regFC("rfc", "sr,xxx", tx);

    game::test::RegistrationKey unregKey(game::RegistrationKey::Unregistered, 6);
    game::test::RegistrationKey regKey(game::RegistrationKey::Registered, 10);

    a.check("01", unregFC.isPermitted(unregKey));
    a.check("02", unregFC.isPermitted(regKey));
    a.check("03", !regFC.isPermitted(unregKey));
    a.check("04", regFC.isPermitted(regKey));
}

/** Test worksOn(), generic filter. */
AFL_TEST("game.spec.FriendlyCode:worksOn:generic", a)
{
    afl::string::NullTranslator tx;
    FriendlyCode shipFC("sfc", "s,xxx", tx);
    FriendlyCode planetFC("pfc", "p,xxx", tx);
    FriendlyCode baseFC("bfc", "b,xxx", tx);
    FriendlyCode genericFC("gfc", "spb,xxx", tx);
    FriendlyCode alchemyFC("afc", "sa,xxx", tx);
    FriendlyCode capitalFC("cfc", "sc,xxx", tx);
    FriendlyCode bigFC("Bfc", "sca,xxx", tx);

    // Null filter
    {
        FriendlyCode::Filter f;
        a.check("01", !shipFC.worksOn(f));
        a.check("02", !planetFC.worksOn(f));
        a.check("03", !baseFC.worksOn(f));
        a.check("04", !genericFC.worksOn(f));
        a.check("05", !alchemyFC.worksOn(f));
        a.check("06", !capitalFC.worksOn(f));
        a.check("07", !bigFC.worksOn(f));
    }

    // Ship filter
    {
        FriendlyCode::Filter f(FriendlyCode::FlagSet_t() + FriendlyCode::ShipCode, 1);
        a.check("11",  shipFC.worksOn(f));
        a.check("12", !planetFC.worksOn(f));
        a.check("13", !baseFC.worksOn(f));
        a.check("14",  genericFC.worksOn(f));
        a.check("15", !alchemyFC.worksOn(f));
        a.check("16", !capitalFC.worksOn(f));
        a.check("17", !bigFC.worksOn(f));
    }

    // Alchemy ship filter
    {
        FriendlyCode::Filter f(FriendlyCode::FlagSet_t() + FriendlyCode::ShipCode + FriendlyCode::AlchemyShipCode, 1);
        a.check("21",  shipFC.worksOn(f));
        a.check("22", !planetFC.worksOn(f));
        a.check("23", !baseFC.worksOn(f));
        a.check("24",  genericFC.worksOn(f));
        a.check("25",  alchemyFC.worksOn(f));
        a.check("26", !capitalFC.worksOn(f));
        a.check("27", !bigFC.worksOn(f));
    }

    // Capital ship filter
    {
        FriendlyCode::Filter f(FriendlyCode::FlagSet_t() + FriendlyCode::ShipCode + FriendlyCode::CapitalShipCode, 1);
        a.check("31",  shipFC.worksOn(f));
        a.check("32", !planetFC.worksOn(f));
        a.check("33", !baseFC.worksOn(f));
        a.check("34",  genericFC.worksOn(f));
        a.check("35", !alchemyFC.worksOn(f));
        a.check("36",  capitalFC.worksOn(f));
        a.check("37", !bigFC.worksOn(f));
    }

    // Capital alchemy ship filter
    {
        FriendlyCode::Filter f(FriendlyCode::FlagSet_t() + FriendlyCode::ShipCode + FriendlyCode::AlchemyShipCode + FriendlyCode::CapitalShipCode, 1);
        a.check("41",  shipFC.worksOn(f));
        a.check("42", !planetFC.worksOn(f));
        a.check("43", !baseFC.worksOn(f));
        a.check("44",  genericFC.worksOn(f));
        a.check("45",  alchemyFC.worksOn(f));
        a.check("46",  capitalFC.worksOn(f));
        a.check("47",  bigFC.worksOn(f));
    }

    // Planet filter
    {
        FriendlyCode::Filter f(FriendlyCode::FlagSet_t() + FriendlyCode::PlanetCode, 1);
        a.check("51", !shipFC.worksOn(f));
        a.check("52",  planetFC.worksOn(f));
        a.check("53", !baseFC.worksOn(f));
        a.check("54",  genericFC.worksOn(f));
        a.check("55", !alchemyFC.worksOn(f));
        a.check("56", !capitalFC.worksOn(f));
        a.check("57", !bigFC.worksOn(f));
    }

    // Starbase + planet filter
    {
        FriendlyCode::Filter f(FriendlyCode::FlagSet_t() + FriendlyCode::PlanetCode + FriendlyCode::StarbaseCode, 1);
        a.check("61", !shipFC.worksOn(f));
        a.check("62",  planetFC.worksOn(f));
        a.check("63",  baseFC.worksOn(f));
        a.check("64",  genericFC.worksOn(f));
        a.check("65", !alchemyFC.worksOn(f));
        a.check("66", !capitalFC.worksOn(f));
        a.check("67", !bigFC.worksOn(f));
    }

    // All types filter
    {
        FriendlyCode::Filter f(FriendlyCode::FlagSet_t() + FriendlyCode::PlanetCode + FriendlyCode::StarbaseCode + FriendlyCode::ShipCode, 1);
        a.check("71",  shipFC.worksOn(f));
        a.check("72",  planetFC.worksOn(f));
        a.check("73",  baseFC.worksOn(f));
        a.check("74",  genericFC.worksOn(f));
        a.check("75", !alchemyFC.worksOn(f));
        a.check("76", !capitalFC.worksOn(f));
        a.check("77", !bigFC.worksOn(f));
    }
}
