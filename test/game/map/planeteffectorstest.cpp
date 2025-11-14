/**
  *  \file test/game/map/planeteffectorstest.cpp
  *  \brief Test for game::map::PlanetEffectors
  */

#include "game/map/planeteffectors.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/hostversion.hpp"

using afl::base::Ref;
using game::config::HostConfiguration;
using game::map::PlanetEffectors;

AFL_TEST("game.map.PlanetEffectors:basics", a)
{
    PlanetEffectors t;
    a.checkEqual("01. get", t.get(PlanetEffectors::HeatsTo50), 0);
    a.checkEqual("02. getNumTerraformers", t.getNumTerraformers(), 0);

    PlanetEffectors t2;
    a.checkEqual("11. eq", t == t2, true);
    a.checkEqual("12. ne", t != t2, false);

    t.set(PlanetEffectors::HeatsTo50, 3);
    t.set(PlanetEffectors::HeatsTo100, 5);
    t.add(PlanetEffectors::HeatsTo50, 1);
    a.checkEqual("21. get", t.get(PlanetEffectors::HeatsTo50), 4);
    a.checkEqual("22. getNumTerraformers", t.getNumTerraformers(), 9);

    a.checkEqual("31. eq", t == t2, false);
    a.checkEqual("32. ne", t != t2, true);
}

AFL_TEST("game.map.PlanetEffectors:describe:none", a)
{
    afl::string::NullTranslator tx;
    Ref<HostConfiguration> config = HostConfiguration::create();
    PlanetEffectors t;
    a.checkEqual("describe", t.describe(tx, 3, *config), "No ship effects considered");
}

AFL_TEST("game.map.PlanetEffectors:describe:hiss", a)
{
    afl::string::NullTranslator tx;
    Ref<HostConfiguration> config = HostConfiguration::create();
    PlanetEffectors t;
    t.set(PlanetEffectors::Hiss, 3);
    a.checkEqual("describe", t.describe(tx, 3, *config), "3 ships hissing (+15)");
}

AFL_TEST("game.map.PlanetEffectors:describe:terraform", a)
{
    afl::string::NullTranslator tx;
    Ref<HostConfiguration> config = HostConfiguration::create();
    PlanetEffectors t;
    t.set(PlanetEffectors::HeatsTo50, 3);
    t.set(PlanetEffectors::HeatsTo100, 2);
    a.checkEqual("describe", t.describe(tx, 3, *config), "5 ships terraforming");
}

AFL_TEST("game.map.PlanetEffectors:describe:hiss+terraform", a)
{
    afl::string::NullTranslator tx;
    Ref<HostConfiguration> config = HostConfiguration::create();
    PlanetEffectors t;
    t.set(PlanetEffectors::Hiss, 4);
    t.set(PlanetEffectors::HeatsTo50, 3);
    t.set(PlanetEffectors::HeatsTo100, 2);
    a.checkEqual("describe", t.describe(tx, 3, *config), "4 ships hissing, 5 ships terraforming");
}
