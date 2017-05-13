/**
  *  \file u/t_game_spec_friendlycode.cpp
  *  \brief Test for game::spec::FriendlyCode
  */

#include <stdexcept>
#include <stdio.h>
#include "game/spec/friendlycode.hpp"

#include "t_game_spec.hpp"
#include "game/playerlist.hpp"
#include "game/map/planet.hpp"
#include "afl/sys/log.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/map/configuration.hpp"
#include "game/map/minefield.hpp"

/** Test friendly code constructors. */
void
TestGameSpecFriendlyCode::testFCode()
{
    // ex GameFcodeTestSuite::testFCode
    game::spec::FriendlyCode mkt("mkt", "sc,make torps");
    game::spec::FriendlyCode lfm("lfm", "sc+9ab,make fighters");
    game::spec::FriendlyCode att("ATT", "p,attack");

    game::PlayerList list;

    TS_ASSERT_EQUALS(mkt.getCode(), "mkt");
    TS_ASSERT(mkt.getRaces().contains(1));
    TS_ASSERT(mkt.getRaces().contains(2));
    TS_ASSERT(mkt.getRaces().contains(10));
    TS_ASSERT_EQUALS(mkt.getDescription(list), "make torps");

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
    // Player character out of range
    TS_ASSERT_THROWS(game::spec::FriendlyCode("xy0", "+0,hi"), std::exception);
    TS_ASSERT_THROWS(game::spec::FriendlyCode("xyz", "+z,hi"), std::exception);

    // Missing description
    TS_ASSERT_THROWS(game::spec::FriendlyCode("xyz", ""), std::exception);
    TS_ASSERT_THROWS(game::spec::FriendlyCode("xyz", "p"), std::exception);
}

/** Test initial state getters. */
void
TestGameSpecFriendlyCode::testData()
{
    game::PlayerList list;
    game::spec::FriendlyCode testee;
    TS_ASSERT_EQUALS(testee.getCode(), "");
    TS_ASSERT_EQUALS(testee.getDescription(list), "");
    TS_ASSERT(testee.getFlags().empty());
    TS_ASSERT(testee.getRaces().empty());
}

/** Test getDescription(). */
void
TestGameSpecFriendlyCode::testDescription()
{
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
    TS_ASSERT_EQUALS(game::spec::FriendlyCode("xyz",",[%3]").getDescription(list), "[Short]");
    TS_ASSERT_EQUALS(game::spec::FriendlyCode("xyz",",[%-3]").getDescription(list), "[Adj]");
    TS_ASSERT_EQUALS(game::spec::FriendlyCode("xyz",",[%2]").getDescription(list), "[2]");
    TS_ASSERT_EQUALS(game::spec::FriendlyCode("xyz",",[%-2]").getDescription(list), "[2]");
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
    game::spec::FriendlyCode planetFC("pfc", "p,xxx");
    game::spec::FriendlyCode baseFC("bfc", "b,xxx");
    game::spec::FriendlyCode shipFC("sfc", "s,xxx");
    game::spec::FriendlyCode fedFC("ffc", "p+1,xxx");

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
        TS_ASSERT(planetFC.worksOn(obj, config));
        TS_ASSERT(!baseFC.worksOn(obj, config));
        TS_ASSERT(!shipFC.worksOn(obj, config));
        TS_ASSERT(fedFC.worksOn(obj, config));
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
        m.addReport(game::map::Point(2000, 2000), 2, m.IsMine, m.RadiusKnown, 100, 5, m.MinefieldLaid);
        m.setPlayability(m.ReadOnly);
        TS_ASSERT(!planetFC.worksOn(m, config));
        TS_ASSERT(!baseFC.worksOn(m, config));
        TS_ASSERT(!shipFC.worksOn(m, config));
        TS_ASSERT(!fedFC.worksOn(m, config));
    }
}

