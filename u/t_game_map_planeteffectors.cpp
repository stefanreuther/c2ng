/**
  *  \file u/t_game_map_planeteffectors.cpp
  *  \brief Test for game::map::PlanetEffectors
  */

#include "game/map/planeteffectors.hpp"

#include "t_game_map.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/hostversion.hpp"

using game::map::PlanetEffectors;

void
TestGameMapPlanetEffectors::testIt()
{
    PlanetEffectors t;
    TS_ASSERT_EQUALS(t.get(PlanetEffectors::HeatsTo50), 0);
    TS_ASSERT_EQUALS(t.getNumTerraformers(), 0);

    PlanetEffectors t2;
    TS_ASSERT_EQUALS(t == t2, true);
    TS_ASSERT_EQUALS(t != t2, false);

    t.set(PlanetEffectors::HeatsTo50, 3);
    t.set(PlanetEffectors::HeatsTo100, 5);
    t.add(PlanetEffectors::HeatsTo50, 1);
    TS_ASSERT_EQUALS(t.get(PlanetEffectors::HeatsTo50), 4);
    TS_ASSERT_EQUALS(t.getNumTerraformers(), 9);

    TS_ASSERT_EQUALS(t == t2, false);
    TS_ASSERT_EQUALS(t != t2, true);
}

/** Regression/coverage test for describe(). */
void
TestGameMapPlanetEffectors::testDescribe()
{
    afl::string::NullTranslator tx;
    game::config::HostConfiguration config;
    game::HostVersion host(game::HostVersion::PHost, MKVERSION(3,0,0));

    {
        PlanetEffectors t;
        TS_ASSERT_EQUALS(t.describe(tx, 3, config, host), "No ship effects considered");
    }

    {
        PlanetEffectors t;
        t.set(PlanetEffectors::Hiss, 3);
        TS_ASSERT_EQUALS(t.describe(tx, 3, config, host), "3 ships hissing (+15)");
    }

    {
        PlanetEffectors t;
        t.set(PlanetEffectors::HeatsTo50, 3);
        t.set(PlanetEffectors::HeatsTo100, 2);
        TS_ASSERT_EQUALS(t.describe(tx, 3, config, host), "5 ships terraforming");
    }

    {
        PlanetEffectors t;
        t.set(PlanetEffectors::Hiss, 4);
        t.set(PlanetEffectors::HeatsTo50, 3);
        t.set(PlanetEffectors::HeatsTo100, 2);
        TS_ASSERT_EQUALS(t.describe(tx, 3, config, host), "4 ships hissing, 5 ships terraforming");
    }
}

