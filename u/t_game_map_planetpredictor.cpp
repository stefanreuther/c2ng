/**
  *  \file u/t_game_map_planetpredictor.cpp
  *  \brief Test for game::map::PlanetPredictor
  */

#include "game/map/planetpredictor.hpp"

#include "t_game_map.hpp"
#include "afl/string/format.hpp"
#include "game/map/planet.hpp"
#include "game/hostversion.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/map/planeteffectors.hpp"

namespace {
    void testGrowth(int owner, int temp, int32_t expect, const game::HostVersion& host, String_t name)
    {
        game::map::Planet p(39);
        p.setOwner(owner);
        p.setTemperature(temp);
        p.setCargo(game::Element::Colonists, 10000);
        p.setColonistHappiness(100);
        p.setColonistTax(0);
        p.setCargo(game::Element::Supplies, 0);
        p.setNativeHappiness(100);
        p.setNativeRace(0);
        p.setNatives(0);
        p.setNativeGovernment(0);

        game::config::HostConfiguration config;
        config.setDefaultValues();
        config[config.ClimateDeathRate].set(0);

        game::map::PlanetPredictor pp(p);
        TSM_ASSERT_THROWS_NOTHING(name.c_str(), pp.computeTurn(game::map::PlanetEffectors(), config, host));
        TSM_ASSERT_EQUALS(name.c_str(), pp.planet().getCargo(game::Element::Colonists).orElse(0), expect);
    }
}

/** Test growth, PHost version. */
void
TestGameMapPlanetPredictor::testPHost()
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
        testGrowth(1, i, expect[i], game::HostVersion(game::HostVersion::PHost, MKVERSION(3, 4, 5)), afl::string::Format("PHost temp %d", i));
    }
}

/** Test growth, Host version. */
void
TestGameMapPlanetPredictor::testHost()
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
        testGrowth(1, i, expect[i], game::HostVersion(game::HostVersion::Host, MKVERSION(3, 22, 40)), afl::string::Format("Host temp %d", i));
    }
}

/** Test growth, Tholian, PHost. */
void
TestGameMapPlanetPredictor::testGrowthPHostTholian()
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
        testGrowth(7, i, expect[i], game::HostVersion(game::HostVersion::PHost, MKVERSION(3, 4, 5)), afl::string::Format("PHost Tholian temp %d", i));
    }
}

/** Test growth, Tholian, THost. */
void
TestGameMapPlanetPredictor::testGrowthHostTholian()
{
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
        testGrowth(7, i, expect[i], game::HostVersion(game::HostVersion::Host, MKVERSION(3, 22, 40)), afl::string::Format("Host Tholian temp %d", i));
    }
}

