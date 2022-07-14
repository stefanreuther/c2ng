/**
  *  \file u/t_game_spec_friendlycode.cpp
  *  \brief Test for game::spec::FriendlyCode
  */

#include <stdexcept>
#include "game/spec/friendlycode.hpp"

#include "t_game_spec.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/map/configuration.hpp"
#include "game/map/minefield.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/playerlist.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/registrationkey.hpp"

/** Test friendly code constructors. */
void
TestGameSpecFriendlyCode::testFCode()
{
    // ex GameFcodeTestSuite::testFCode
    afl::string::NullTranslator tx;
    game::spec::FriendlyCode mkt("mkt", "sc,make torps", tx);
    game::spec::FriendlyCode lfm("lfm", "sc+9ab,make fighters", tx);
    game::spec::FriendlyCode att("ATT", "p,attack", tx);

    game::PlayerList list;

    TS_ASSERT_EQUALS(mkt.getCode(), "mkt");
    TS_ASSERT(mkt.getRaces().contains(1));
    TS_ASSERT(mkt.getRaces().contains(2));
    TS_ASSERT(mkt.getRaces().contains(10));
    TS_ASSERT_EQUALS(mkt.getDescription(list, tx), "make torps");

    TS_ASSERT(!lfm.getRaces().contains(1));
    TS_ASSERT(!lfm.getRaces().contains(8));
    TS_ASSERT(lfm.getRaces().contains(9));
    TS_ASSERT(lfm.getRaces().contains(10));
    TS_ASSERT(lfm.getRaces().contains(11));
}

/** Test constructor failures. */
void
TestGameSpecFriendlyCode::testFCodeFail()
{
    afl::string::NullTranslator tx;

    // Player character out of range
    TS_ASSERT_THROWS(game::spec::FriendlyCode("xy0", "+0,hi", tx), std::exception);
    TS_ASSERT_THROWS(game::spec::FriendlyCode("xyz", "+z,hi", tx), std::exception);

    // Missing description
    TS_ASSERT_THROWS(game::spec::FriendlyCode("xyz", "", tx), std::exception);
    TS_ASSERT_THROWS(game::spec::FriendlyCode("xyz", "p", tx), std::exception);
}

/** Test initial state getters. */
void
TestGameSpecFriendlyCode::testData()
{
    afl::string::NullTranslator tx;
    game::PlayerList list;
    game::spec::FriendlyCode testee;
    TS_ASSERT_EQUALS(testee.getCode(), "");
    TS_ASSERT_EQUALS(testee.getDescription(list, tx), "");
    TS_ASSERT(testee.getFlags().empty());
    TS_ASSERT(testee.getRaces().empty());
}

/** Test getDescription(). */
void
TestGameSpecFriendlyCode::testDescription()
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
    TS_ASSERT_EQUALS(game::spec::FriendlyCode("xyz",",[%3]", tx).getDescription(list, tx), "[Short]");
    TS_ASSERT_EQUALS(game::spec::FriendlyCode("xyz",",[%-3]", tx).getDescription(list, tx), "[Adj]");
    TS_ASSERT_EQUALS(game::spec::FriendlyCode("xyz",",[%2]", tx).getDescription(list, tx), "[2]");
    TS_ASSERT_EQUALS(game::spec::FriendlyCode("xyz",",[%-2]", tx).getDescription(list, tx), "[2]");
}

/** Test worksOn(). */
void
TestGameSpecFriendlyCode::testWorksOn()
{
    // Environment
    game::config::HostConfiguration config;
    config.setDefaultValues();

    afl::sys::Log log;
    afl::string::NullTranslator tx;

    // Some fcodes
    game::spec::FriendlyCode planetFC("pfc", "p,xxx", tx);
    game::spec::FriendlyCode baseFC("bfc", "b,xxx", tx);
    game::spec::FriendlyCode shipFC("sfc", "s,xxx", tx);
    game::spec::FriendlyCode fedFC("ffc", "p+1,xxx", tx);

    // Fed planet
    {
        game::map::Planet p(9);
        p.setOwner(1);
        p.setPlayability(p.ReadOnly);
        TS_ASSERT(planetFC.worksOn(p, config));
        TS_ASSERT(!baseFC.worksOn(p, config));
        TS_ASSERT(!shipFC.worksOn(p, config));
        TS_ASSERT(fedFC.worksOn(p, config));

        const game::map::Object& obj = p;
        const game::spec::ShipList shipList;
        const game::UnitScoreDefinitionList scoreDefinitions;
        TS_ASSERT(planetFC.worksOn(obj, scoreDefinitions, shipList, config));
        TS_ASSERT(!baseFC.worksOn(obj, scoreDefinitions, shipList, config));
        TS_ASSERT(!shipFC.worksOn(obj, scoreDefinitions, shipList, config));
        TS_ASSERT(fedFC.worksOn(obj, scoreDefinitions, shipList, config));
    }

    // Lizard planet
    {
        game::map::Planet p(9);
        p.setOwner(2);
        p.setPlayability(p.ReadOnly);
        TS_ASSERT(planetFC.worksOn(p, config));
        TS_ASSERT(!baseFC.worksOn(p, config));
        TS_ASSERT(!shipFC.worksOn(p, config));
        TS_ASSERT(!fedFC.worksOn(p, config));
    }

    // Unknown planet
    {
        game::map::Planet p(9);
        TS_ASSERT(!planetFC.worksOn(p, config));
        TS_ASSERT(!baseFC.worksOn(p, config));
        TS_ASSERT(!shipFC.worksOn(p, config));
        TS_ASSERT(!fedFC.worksOn(p, config));
    }

    // Lizard base
    {
        game::map::Planet p(9);
        p.setOwner(2);
        p.setPosition(game::map::Point(2000, 2000));
        p.addPlanetSource(game::PlayerSet_t(2));
        p.addBaseSource(game::PlayerSet_t(2));
        p.setPlayability(p.ReadOnly);
        p.internalCheck(game::map::Configuration(), tx, log);
        TS_ASSERT(planetFC.worksOn(p, config));
        TS_ASSERT(baseFC.worksOn(p, config));
        TS_ASSERT(!shipFC.worksOn(p, config));
        TS_ASSERT(!fedFC.worksOn(p, config));
    }

    // Minefield
    {
        game::map::Minefield m(90);
        const game::spec::ShipList shipList;
        const game::UnitScoreDefinitionList scoreDefinitions;
        m.addReport(game::map::Point(2000, 2000), 2, m.IsMine, m.RadiusKnown, 100, 5, m.MinefieldLaid);
        m.setPlayability(m.ReadOnly);
        TS_ASSERT(!planetFC.worksOn(m, scoreDefinitions, shipList, config));
        TS_ASSERT(!baseFC.worksOn(m, scoreDefinitions, shipList, config));
        TS_ASSERT(!shipFC.worksOn(m, scoreDefinitions, shipList, config));
        TS_ASSERT(!fedFC.worksOn(m, scoreDefinitions, shipList, config));
    }
}

/** Test worksOn(), for ships. */
void
TestGameSpecFriendlyCode::testWorksOnShip()
{
    // Environment
    game::UnitScoreDefinitionList scoreDefinitions;
    game::spec::ShipList shipList;
    game::config::HostConfiguration config;
    config.setDefaultValues();

    const int HULL_NR = 12;
    shipList.hulls().create(HULL_NR);

    afl::sys::Log log;
    afl::string::NullTranslator tx;

    // Some fcodes
    game::spec::FriendlyCode planetFC("pfc", "p,xxx", tx);
    game::spec::FriendlyCode shipFC("sfc", "s,xxx", tx);
    game::spec::FriendlyCode fedFC("ffc", "s+1,xxx", tx);
    game::spec::FriendlyCode capFC("cfc", "sc,xxx", tx);
    game::spec::FriendlyCode alchemyFC("afc", "sa,xxx", tx);

    // Fed ship
    {
        game::map::Ship sh(9);
        sh.setOwner(1);
        sh.setHull(HULL_NR);
        sh.setPlayability(sh.ReadOnly);
        TS_ASSERT(!planetFC.worksOn(sh, scoreDefinitions, shipList, config));
        TS_ASSERT(shipFC.worksOn(sh, scoreDefinitions, shipList, config));
        TS_ASSERT(fedFC.worksOn(sh, scoreDefinitions, shipList, config));
        TS_ASSERT(!capFC.worksOn(sh, scoreDefinitions, shipList, config));
        TS_ASSERT(!alchemyFC.worksOn(sh, scoreDefinitions, shipList, config));

        const game::map::Object& obj = sh;
        TS_ASSERT(!planetFC.worksOn(obj, scoreDefinitions, shipList, config));
        TS_ASSERT(shipFC.worksOn(obj, scoreDefinitions, shipList, config));
        TS_ASSERT(fedFC.worksOn(obj, scoreDefinitions, shipList, config));
        TS_ASSERT(!capFC.worksOn(obj, scoreDefinitions, shipList, config));
        TS_ASSERT(!alchemyFC.worksOn(obj, scoreDefinitions, shipList, config));
    }

    // Lizard warship
    {
        game::map::Ship sh(9);
        sh.setOwner(2);
        sh.setHull(HULL_NR);
        sh.setPlayability(sh.ReadOnly);
        sh.setNumBays(1);
        TS_ASSERT(shipFC.worksOn(sh, scoreDefinitions, shipList, config));
        TS_ASSERT(!fedFC.worksOn(sh, scoreDefinitions, shipList, config));
        TS_ASSERT(capFC.worksOn(sh, scoreDefinitions, shipList, config));
        TS_ASSERT(!alchemyFC.worksOn(sh, scoreDefinitions, shipList, config));
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
        TS_ASSERT(shipFC.worksOn(sh, scoreDefinitions, shipList, config));
        TS_ASSERT(!fedFC.worksOn(sh, scoreDefinitions, shipList, config));
        TS_ASSERT(capFC.worksOn(sh, scoreDefinitions, shipList, config));
        TS_ASSERT(alchemyFC.worksOn(sh, scoreDefinitions, shipList, config));
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
        TS_ASSERT(!fedFC.worksOn(sh, scoreDefinitions, shipList, config));
    }
}

/** Test isPermitted(). */
void
TestGameSpecFriendlyCode::testIsPermitted()
{
    afl::string::NullTranslator tx;
    game::spec::FriendlyCode unregFC("ufc", "s,xxx", tx);
    game::spec::FriendlyCode regFC("rfc", "sr,xxx", tx);

    game::test::RegistrationKey unregKey(game::RegistrationKey::Unregistered, 6);
    game::test::RegistrationKey regKey(game::RegistrationKey::Registered, 10);

    TS_ASSERT(unregFC.isPermitted(unregKey));
    TS_ASSERT(unregFC.isPermitted(regKey));
    TS_ASSERT(!regFC.isPermitted(unregKey));
    TS_ASSERT(regFC.isPermitted(regKey));
}

