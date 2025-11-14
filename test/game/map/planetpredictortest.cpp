/**
  *  \file test/game/map/planetpredictortest.cpp
  *  \brief Test for game::map::PlanetPredictor
  */

#include "game/map/planetpredictor.hpp"

#include "afl/string/format.hpp"
#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/hostversion.hpp"
#include "game/map/planet.hpp"
#include "game/map/planeteffectors.hpp"

using afl::string::Format;

namespace {
    game::map::Planet makePlanet()
    {
        game::map::Planet p(39);
        p.setOwner(1);
        p.setTemperature(50);
        p.setCargo(game::Element::Colonists, 10000);
        p.setColonistHappiness(100);
        p.setColonistTax(0);
        p.setCargo(game::Element::Supplies, 0);
        p.setNativeHappiness(100);
        p.setNativeRace(0);
        p.setNatives(0);
        p.setNativeGovernment(0);
        return p;
    }

    void testGrowth(afl::test::Assert a, const game::map::Planet& tpl, int32_t expect, const game::HostVersion& host)
    {
        game::map::Planet p(tpl);

        afl::base::Ref<game::config::HostConfiguration> config = game::config::HostConfiguration::create();
        config->setDefaultValues();
        (*config)[game::config::HostConfiguration::ClimateDeathRate].set(0);

        game::map::PlanetPredictor pp(p);
        AFL_CHECK_SUCCEEDS(a, pp.computeTurn(game::map::PlanetEffectors(), game::UnitScoreDefinitionList(), *config, host));
        a.checkEqual("Colonists", pp.planet().getCargo(game::Element::Colonists).orElse(0), expect);
    }

    void testGrowthNatives(afl::test::Assert a, const game::map::Planet& tpl, int32_t expect, const game::HostVersion& host)
    {
        game::map::Planet p(tpl);

        afl::base::Ref<game::config::HostConfiguration> config = game::config::HostConfiguration::create();
        config->setDefaultValues();
        (*config)[game::config::HostConfiguration::ClimateDeathRate].set(0);

        game::map::PlanetPredictor pp(p);
        AFL_CHECK_SUCCEEDS(a, pp.computeTurn(game::map::PlanetEffectors(), game::UnitScoreDefinitionList(), *config, host));
        a.checkEqual("Natives", pp.planet().getNatives().orElse(0), expect);
    }
}

/** Test growth, PHost version. */
AFL_TEST("game.map.PlanetPredictor:colonist-growth:phost", a)
{
    static const int32_t expect[] = {
        10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000,
        10227, 10241, 10255, 10268, 10281, 10294, 10306, 10319, 10331, 10342, 10354, 10364, 10375, 10385, 10395,
        10405, 10414, 10422, 10430, 10438, 10446, 10452, 10459, 10465, 10470, 10476, 10480, 10484, 10488, 10491,
        10494, 10496, 10498, 10499, 10500, 10500, 10500, 10499, 10498, 10496, 10494, 10491, 10488, 10484, 10480,
        10476, 10470, 10465, 10459, 10452, 10446, 10438, 10430, 10422, 10414, 10405, 10395, 10385, 10375, 10364,
        10354, 10342, 10331, 10319, 10306, 10294, 10281, 10268, 10255, 10241, 10000, 10000, 10000, 10000, 10000,
        10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000
    };
    for (int i = 0; i <= 100; ++i) {
        game::map::Planet pl(makePlanet());
        pl.setOwner(1);
        pl.setTemperature(i);
        testGrowth(a(Format("temp=%d", i)), pl, expect[i], game::HostVersion(game::HostVersion::PHost, MKVERSION(3, 4, 5)));
    }
}

/** Test growth, Host version. */
AFL_TEST("game.map.PlanetPredictor:colonist-growth:host", a)
{
    static const int32_t expect[] = {
        10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000,
        10228, 10241, 10255, 10268, 10282, 10294, 10307, 10319, 10331, 10343, 10354, 10365, 10375, 10386, 10395,
        10405, 10414, 10422, 10431, 10438, 10446, 10453, 10459, 10465, 10471, 10476, 10480, 10484, 10488, 10491,
        10494, 10496, 10498, 10499, 10500, 10500, 10500, 10499, 10498, 10496, 10494, 10491, 10488, 10484, 10480,
        10475, 10470, 10465, 10459, 10452, 10445, 10438, 10430, 10422, 10413, 10404, 10395, 10385, 10375, 10364,
        10353, 10342, 10331, 10319, 10306, 10294, 10281, 10268, 10254, 10241, 10000, 10000, 10000, 10000, 10000,
        10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000
    };
    for (int i = 0; i <= 100; ++i) {
        game::map::Planet pl(makePlanet());
        pl.setOwner(1);
        pl.setTemperature(i);
        testGrowth(a(Format("temp=%d", i)), pl, expect[i], game::HostVersion(game::HostVersion::Host, MKVERSION(3, 22, 40)));
    }
}

/** Test growth, Tholian, PHost. */
AFL_TEST("game.map.PlanetPredictor:colonist-growth:phost:tholian", a)
{
    static const int32_t expect[] = {
        10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000,
        10055, 10060, 10065, 10070, 10075, 10080, 10085, 10090, 10095, 10100, 10105,
        10110, 10115, 10120, 10125, 10130, 10135, 10140, 10145, 10150, 10155, 10160,
        10165, 10170, 10175, 10180, 10185, 10190, 10195, 10200, 10205, 10210, 10215,
        10220, 10225, 10230, 10235, 10240, 10245, 10250, 10255, 10260, 10265, 10270,
        10275, 10280, 10285, 10290, 10295, 10300, 10305, 10310, 10315, 10320, 10325,
        10330, 10335, 10340, 10345, 10350, 10355, 10360, 10365, 10370, 10375, 10380,
        10385, 10390, 10395, 10400, 10405, 10410, 10415, 10420, 10425, 10430, 10435,
        10440, 10445, 10450, 10455, 10460, 10465, 10470, 10475, 10480, 10485, 10490,
        10495, 10500
    };
    for (int i = 0; i <= 100; ++i) {
        game::map::Planet pl(makePlanet());
        pl.setOwner(7);
        pl.setTemperature(i);
        testGrowth(a(Format("temp=%d", i)), pl, expect[i], game::HostVersion(game::HostVersion::PHost, MKVERSION(3, 4, 5)));
    }
}

/** Test growth, Tholian, THost. */
AFL_TEST("game.map.PlanetPredictor:colonist-growth:host:tholian", a)
{
    // ex GamePlanetPredTestSuite::testGrowthTholianTim()
    static const int32_t expect[] = {
        0, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 10000, 10000, 10000, 10000,
        10075, 10080, 10085, 10090, 10095, 10100, 10105, 10110, 10115, 10120, 10125,
        10130, 10135, 10140, 10145, 10150, 10155, 10160, 10165, 10170, 10175, 10180,
        10185, 10190, 10195, 10200, 10205, 10210, 10215, 10220, 10225, 10230, 10235,
        10240, 10245, 10250, 10255, 10260, 10265, 10270, 10275, 10280, 10285, 10290,
        10295, 10300, 10305, 10310, 10315, 10320, 10325, 10330, 10335, 10340, 10345,
        10350, 10355, 10360, 10365, 10370, 10375, 10380, 10385, 10390, 10395, 10400,
        10405, 10410, 10415, 10420, 10425, 10430, 10435, 10440, 10445, 10450, 10455,
        10460, 10465, 10470, 10475, 10480, 10485, 10490, 10495, 10500
    };
    for (int i = 0; i <= 100; ++i) {
        game::map::Planet pl(makePlanet());
        pl.setOwner(7);
        pl.setTemperature(i);
        testGrowth(a(Format("temp=%d", i)), pl, expect[i], game::HostVersion(game::HostVersion::Host, MKVERSION(3, 22, 40)));
    }
}

/** Test growth, Host version, close to maximum population. */
AFL_TEST("game.map.PlanetPredictor:colonist-growth:host:max", a)
{
    // ex GamePlanetPredTestSuite::testGrowthMaxTim()
    static const int32_t expect[101] = {
        99999, 99999, 99999, 99999, 99999, 99999, 99999, 99999, 99999, 99999, 99999, 99999, 99999, 99999, 99999,
        45520, 48293, 51018, 53693, 56315, 58882, 61390, 63838, 66223, 68543, 70795, 72977, 75088, 77124, 79085,
        80967, 82770, 84491, 86128, 87681, 89148, 90526, 91815, 93014, 94121, 95135, 96056, 96881, 97611, 98245,
        98782, 99222, 99564, 99808, 99953,
        100000,
        99948, 99798, 99549, 99202, 98758,
        98216, 97577, 96842, 96011, 95086, 94067, 92955, 91752, 90458, 89075, 87605, 86047, 84405, 82680, 80874,
        78987, 77023, 74983, 72869, 70683, 68427, 66104, 63715, 61264, 58753, 56183, 53558, 50881, 48153, 99999,
        99999, 99999, 99999, 99999, 99999, 99999, 99999, 99999, 99999, 99999, 99999, 99999, 99999, 99999, 99999
    };
    for (int i = 0; i <= 100; ++i) {
        game::map::Planet pl(makePlanet());
        pl.setOwner(1);
        pl.setTemperature(i);
        pl.setCargo(game::Element::Colonists, 99999);
        testGrowth(a(Format("temp=%d", i)), pl, expect[i], game::HostVersion(game::HostVersion::Host, MKVERSION(3, 22, 40)));
    }
}

AFL_TEST("game.map.PlanetPredictor:colonist-growth:host:rebel", a)
{
    // ex GamePlanetPredTestSuite::testGrowthRebelTim()
    static const int32_t expect[101] = {
        90000, 90000, 90000, 90000, 90000, 90000, 90000, 90000, 90000, 90000, 90000, 90000, 90000, 90000, 90000,
        90000, 90000, 90000, 90000, 90000, 58882, 61390, 63838, 66223, 68543, 70795, 72977, 75088, 77124, 79085,
        80967, 82770, 84491, 86128, 87681, 89148, 90526, 91815, 93014, 94121, 95135, 96056, 96881, 97318, 97334,
        97346, 97356, 97364, 97370, 97374, 97375, 97374, 97370, 97364, 97356, 97346, 97332, 97318, 96842, 96011,
        95086, 94067, 92955, 91752, 90458, 89075, 87605, 86047, 84405, 82680, 80874, 78987, 77023, 74983, 72869,
        70683, 68427, 66104, 63715, 61264, 58753, 56183, 53558, 50881, 48153, 95000, 95000, 95000, 95000, 95000,
        95000, 95000, 95000, 95000, 95000, 95000, 95000, 95000, 95000, 95000, 95000
    };
    for (int i = 0; i <= 100; ++i) {
        game::map::Planet pl(makePlanet());
        pl.setOwner(10);
        pl.setTemperature(i);
        pl.setCargo(game::Element::Colonists, 95000);
        testGrowth(a(Format("temp=%d", i)), pl, expect[i], game::HostVersion(game::HostVersion::Host, MKVERSION(3, 22, 40)));
    }
}

AFL_TEST("game.map.PlanetPredictor:native-growth:host:humanoid", a)
{
    // ex GamePlanetPredTestSuite::testGrowthHumanoidTim()
    static const int32_t expect[101] = {
        10000, 10000, 10000, 10038, 10051, 10063, 10076, 10088, 10100, 10112, 10124, 10136, 10148, 10159, 10171,
        10182, 10193, 10204, 10215, 10225, 10236, 10246, 10255, 10265, 10274, 10283, 10292, 10300, 10308, 10316,
        10324, 10331, 10338, 10345, 10351, 10357, 10362, 10367, 10372, 10376, 10381, 10384, 10388, 10390, 10393,
        10395, 10397, 10398, 10399, 10400, 10400, 10400, 10399, 10398, 10397, 10395, 10393, 10390, 10387, 10384,
        10380, 10376, 10372, 10367, 10362, 10356, 10350, 10344, 10338, 10331, 10323, 10316, 10308, 10300, 10291,
        10283, 10274, 10264, 10255, 10245, 10235, 10225, 10214, 10204, 10193, 10182, 10170, 10159, 10147, 10135,
        10124, 10112, 10099, 10087, 10075, 10063, 10050, 10038, 10000, 10000, 10000
    };

    for (int i = 0; i <= 100; ++i) {
        game::map::Planet pl(makePlanet());
        pl.setTemperature(i);
        pl.setNativeGovernment(5);
        pl.setNativeRace(1);
        pl.setNativeHappiness(100);
        pl.setNatives(10000);
        testGrowthNatives(a(Format("temp=%d", i)), pl, expect[i], game::HostVersion(game::HostVersion::Host, MKVERSION(3, 22, 40)));
    }
}
