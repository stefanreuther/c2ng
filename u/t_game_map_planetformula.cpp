/**
  *  \file u/t_game_map_planetformula.cpp
  *  \brief Test for game::map::PlanetFormula
  */

#include <cstdio>
#include <stdio.h>
#include "game/map/planetformula.hpp"

#include "t_game_map.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/map/planet.hpp"
#include "game/hostversion.hpp"



/** Test getColonistChange(). */
void
TestGameMapPlanetFormula::testGetColonistChange()
{
    const int OWNER = 7;
    const int TEMP = 50;

    game::map::Planet p(39);
    p.setOwner(OWNER);
    p.setTemperature(TEMP);
    p.setCargo(game::Element::Colonists, 10000);
    p.setColonistHappiness(100);
    p.setColonistTax(0);
    p.setCargo(game::Element::Supplies, 0);
    p.setNativeHappiness(100);
    p.setNativeRace(0);
    p.setNatives(0);
    p.setNativeGovernment(0);
    p.setNumBuildings(game::FactoryBuilding, 50);
    p.setNumBuildings(game::MineBuilding, 20);
    p.setNumBuildings(game::DefenseBuilding, 0);

    game::config::HostConfiguration config;
    game::HostVersion host(game::HostVersion::PHost, MKVERSION(3,0,0));

    // Parameterized and non-parameterized version must agree. We had a typo here.
    TS_ASSERT_EQUALS(getColonistChange(p, config, host, 0,  70).orElse(-777), 8);
    TS_ASSERT_EQUALS(getColonistChange(p, config, host)        .orElse(-777), 8);

    // Increasing buildings by 300 drops happiness by 1.
    TS_ASSERT_EQUALS(getColonistChange(p, config, host, 0, 370).orElse(-777), 7);
    TS_ASSERT_EQUALS(getColonistChange(p, config, host, 0, 670).orElse(-777), 6);
}


/*
 *  Tax Series: Test all tax rates for a given planet
 *
 *  Equivalent to c2hosttest/planet/03_tax
 */

namespace {
    const int8_t TAX_SERIES_HAPPINESS[50] = {
        89, 89, 88, 87, 86, 85, 85, 84, 83, 82,
        81, 81, 80, 80, 79, 78, 78, 77, 76, 75,
        74, 74, 73, 72, 71, 70, 70, 69, 68, 67,
        66, 66, 65, 64, 63, 62, 62, 61, 60, 59,
        58, 58, 57, 56, 55, 54, 54, 53, 52, 51
    };
    const int8_t TAX_SERIES_INCOME_THOST[50] = {
        0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        4, 6, 6, 6, 6, 6, 6, 6, 6, 6, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 10, 10, 10, 10
    };
    const int8_t TAX_SERIES_INCOME_PHOST[50] = {
        0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 10, 10, 10, 10, 10
    };

    void doTaxSeries(const game::HostVersion host, const int8_t (&expectedHappiness)[50], const int8_t (&expectedIncome)[50])
    {
        const game::config::HostConfiguration config;
        for (int tax = 0; tax < 50; ++tax) {
            game::map::Planet p(66);
            p.setOwner(1);                  // test set is built for Feds
            p.setColonistHappiness(80);
            p.setCargo(game::Element::Colonists, 100);
            p.setColonistTax(tax);
            p.setCargo(game::Element::Supplies, 0);
            p.setCargo(game::Element::Money, 0);
            p.setTemperature(50);
            p.setNativeHappiness(80);
            p.setNumBuildings(game::FactoryBuilding, 0);
            p.setNumBuildings(game::MineBuilding, 0);
            p.setNumBuildings(game::DefenseBuilding, 0);

            char name[50];
            std::sprintf(name, "tax=%d", tax);
            TSM_ASSERT_EQUALS(name, getColonistChange(p, config, host).orElse(-777) + 80, expectedHappiness[tax]);
            TSM_ASSERT_EQUALS(name, getColonistDue(p, config, host, tax).orElse(-777),    expectedIncome[tax]);
        }
    }
}

void
TestGameMapPlanetFormula::testTaxSeriesTHost()
{
    doTaxSeries(game::HostVersion(game::HostVersion::Host, MKVERSION(3, 22, 40)), TAX_SERIES_HAPPINESS, TAX_SERIES_INCOME_THOST);
}

void
TestGameMapPlanetFormula::testTaxSeriesPHost()
{
    doTaxSeries(game::HostVersion(game::HostVersion::PHost, MKVERSION(4, 0, 0)), TAX_SERIES_HAPPINESS, TAX_SERIES_INCOME_PHOST);
}


/*
 *  Temperature Series: test one situation for all temperatures
 *
 *  Equivalent to c2hosttest/planet/04_tax_temp
 */

namespace {
    const int8_t TEMP_SERIES_THOST_FED[101] = {
        86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86,
        86, 86, 86, 86, 86, 86, 86, 86, 86, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87,
        87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 86, 86, 86, 86, 86, 86, 86, 86,
        86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86
    };

    const int8_t TEMP_SERIES_PHOST_FED[101] = {
        85, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86,
        86, 86, 86, 86, 86, 86, 86, 86, 86, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87,
        87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 86, 86, 86, 86, 86, 86, 86, 86,
        86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 85
    };

    const int8_t TEMP_SERIES_THOST_CRY[101] = {
        84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 85, 85, 85, 85, 85, 85, 85, 85,
        85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
        86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86,
        86, 86, 86, 86, 86, 86, 86, 86, 86, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87
    };

    const int8_t TEMP_SERIES_PHOST_CRY[101] = {
        85, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86,
        86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86,
        86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 87, 87, 87, 87, 87, 87, 87, 87,
        87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 87
    };

    void doTemperatureSeries(const game::HostVersion host, const int planetOwner, const int8_t (&expectedHappiness)[101])
    {
        const game::config::HostConfiguration config;
        for (int temp = 0; temp <= 100; ++temp) {
            game::map::Planet p(12);
            p.setOwner(planetOwner);
            p.setColonistHappiness(80);
            p.setCargo(game::Element::Colonists, 100);
            p.setColonistTax(3);
            p.setCargo(game::Element::Supplies, 0);
            p.setCargo(game::Element::Money, 0);
            p.setTemperature(temp);
            p.setNumBuildings(game::MineBuilding, 0);
            p.setNumBuildings(game::FactoryBuilding, 0);
            p.setNumBuildings(game::DefenseBuilding, 0);

            char name[50];
            std::sprintf(name, "temp=%d", temp);
            TSM_ASSERT_EQUALS(name, getColonistChange(p, config, host).orElse(-777) + 80, expectedHappiness[temp]);
        }
    }
}

void
TestGameMapPlanetFormula::testTemperatureSeriesFedTHost()
{
    doTemperatureSeries(game::HostVersion(game::HostVersion::Host, MKVERSION(3,22,40)), 1, TEMP_SERIES_THOST_FED);
}

void
TestGameMapPlanetFormula::testTemperatureSeriesFedPHost()
{
    doTemperatureSeries(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0)), 1, TEMP_SERIES_PHOST_FED);
}

void
TestGameMapPlanetFormula::testTemperatureSeriesCryTHost()
{
    doTemperatureSeries(game::HostVersion(game::HostVersion::Host, MKVERSION(3,22,40)), 7, TEMP_SERIES_THOST_CRY);
}

void
TestGameMapPlanetFormula::testTemperatureSeriesCryPHost()
{
    doTemperatureSeries(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0)), 7, TEMP_SERIES_PHOST_CRY);
}

/*
 *  Temperature Series: test one situation for different numbers of buildings
 *
 *  Equivalent to c2hosttest/planet/05_tax_building
 */

namespace {
    void doBuildingSeries(const game::HostVersion host, const int cutoff)
    {
        const game::config::HostConfiguration config;
        for (int n = 0; n < 400; ++n) {
            game::map::Planet p(99);
            p.setOwner(1);
            p.setColonistHappiness(80);
            p.setCargo(game::Element::Colonists, 100);
            p.setColonistTax(3);
            p.setCargo(game::Element::Supplies, 0);
            p.setCargo(game::Element::Money, 0);
            p.setTemperature(70);

            int half = n/2;
            p.setNumBuildings(game::MineBuilding, half);
            p.setNumBuildings(game::FactoryBuilding, n-half);
            p.setNumBuildings(game::DefenseBuilding, 0);

            const int expectedHappiness = n < cutoff ? 86 : 85;

            char name[50];
            std::sprintf(name, "mifa=%d", n);

            TSM_ASSERT_EQUALS(name, getColonistChange(p, config, host).orElse(-777) + 80, expectedHappiness);
        }
    }
}

void
TestGameMapPlanetFormula::testBuildingSeriesTHost()
{
    doBuildingSeries(game::HostVersion(game::HostVersion::Host, MKVERSION(3,22,40)), 273);
}

void
TestGameMapPlanetFormula::testBuildingSeriesPHost()
{
    doBuildingSeries(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0)), 269);
}

/*
 *  Native Tax Series
 *
 *  Equivalent to c2hosttest/planet/06_ntax
 */

namespace {
    const int8_t NTAX_EXPECT_HAPPY[50] = {
        85, 84, 84, 83, 82, 81, 80, 80, 79, 79,
        78, 77, 76, 75, 74, 74, 73, 72, 71, 70,
        69, 68, 68, 67, 66, 65, 64, 63, 62, 62,
        61, 60, 59, 58, 57, 57, 56, 55, 54, 53,
        52, 51, 51, 50, 49, 48, 47, 46, 45, 45
    };

    const int8_t NTAX_EXPECT_INCOME[50] = {
        0, 0, 0, 2, 2, 2, 2, 2, 4, 4, 4, 4, 4, 6, 6, 6, 6, 6, 8, 8, 8, 8, 8, 10, 10, 10, 10, 10,
        12, 12, 12, 12, 12, 14, 14, 14, 14, 14, 16, 16, 16, 16, 16, 18, 18, 18, 18, 18, 20, 20
    };

    void doNativeTaxSeries(const game::HostVersion host, const int8_t (&expectedHappiness)[50], const int8_t (&expectedIncome)[50])
    {
        const game::config::HostConfiguration config;
        for (int tax = 0; tax < 50; ++tax) {
            game::map::Planet p(66);
            p.setOwner(1);                  // test set is built for Feds
            p.setColonistHappiness(80);
            p.setCargo(game::Element::Colonists, 100);
            p.setColonistTax(0);
            p.setCargo(game::Element::Supplies, 0);
            p.setCargo(game::Element::Money, 0);
            p.setTemperature(50);
            p.setNativeHappiness(80);
            p.setNativeGovernment(2);
            p.setNativeRace(1);             // Humanoids
            p.setNatives(500);
            p.setNativeTax(tax);
            p.setNumBuildings(game::FactoryBuilding, 0);
            p.setNumBuildings(game::MineBuilding, 0);
            p.setNumBuildings(game::DefenseBuilding, 0);

            char name[50];
            std::sprintf(name, "tax=%d", tax);
            TSM_ASSERT_EQUALS(name, getNativeChange(p, host).orElse(-777) + 80,      expectedHappiness[tax]);
            TSM_ASSERT_EQUALS(name, getNativeDue(p, config, host, tax).orElse(-777), expectedIncome[tax]);
        }
    }
}

void
TestGameMapPlanetFormula::testNativeTaxSeriesTHost()
{
    doNativeTaxSeries(game::HostVersion(game::HostVersion::Host, MKVERSION(3,22,40)), NTAX_EXPECT_HAPPY, NTAX_EXPECT_INCOME);
}

void
TestGameMapPlanetFormula::testNativeTaxSeriesPHost()
{
    doNativeTaxSeries(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,5)), NTAX_EXPECT_HAPPY, NTAX_EXPECT_INCOME);
}

/*
 *  Native Tax Building Series
 *
 *  Equivalent to c2hosttest/planet/07_ntax_building
 */

namespace {
    void doNativeBuildingSeries(const game::HostVersion host, const int num84, const int num83)
    {
        for (int n = 0; n < 400; ++n) {
            game::map::Planet p(99);
            p.setOwner(1);
            p.setColonistHappiness(80);
            p.setCargo(game::Element::Colonists, 10000);
            p.setColonistTax(0);
            p.setCargo(game::Element::Supplies, 0);
            p.setCargo(game::Element::Money, 0);
            p.setTemperature(70);
            p.setNatives(100);
            p.setNativeHappiness(80);
            p.setNativeRace(1);
            p.setNativeGovernment(4);
            p.setNativeTax(3);

            int half = n/2;
            p.setNumBuildings(game::MineBuilding, half);
            p.setNumBuildings(game::FactoryBuilding, n-half);
            p.setNumBuildings(game::DefenseBuilding, 0);

            const int expectedHappiness = n < num84 ? 84 : n < num84+num83 ? 83 : 82;

            char name[50];
            std::sprintf(name, "mifa=%d", n);

            TSM_ASSERT_EQUALS(name, getNativeChange(p, host).orElse(-777) + 80, expectedHappiness);
        }
    }
}

void
TestGameMapPlanetFormula::testNativeTaxBuildingSeriesTHost()
{
    doNativeBuildingSeries(game::HostVersion(game::HostVersion::Host, MKVERSION(3,22,40)), 72, 200);
}

void
TestGameMapPlanetFormula::testNativeTaxBuildingSeriesPHost()
{
    doNativeBuildingSeries(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,5)), 71, 200);
}

/*
 *  Building Limit Series
 *
 *  Equivalent to c2hosttest/planet/08_buildings
 */

void
TestGameMapPlanetFormula::testBuildingLimitSeries()
{
    static const int LIMIT = 400;
    static const uint8_t EXPECT_MINES[LIMIT] = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80,
        81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120,
        121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160,
        161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200,
        201, 201, 202, 202, 202, 202, 203, 203, 203, 203, 203, 203, 204, 204, 204, 204, 204, 204, 204, 204, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206,
        206, 206, 207, 207, 207, 207, 207, 207, 207, 207, 207, 207, 207, 207, 207, 207, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 208, 209, 209, 209, 209, 209, 209, 209, 209,
        209, 209, 209, 209, 209, 209, 209, 209, 209, 209, 210, 210, 210, 210, 210, 210, 210, 210, 210, 210, 210, 210, 210, 210, 210, 210, 210, 210, 210, 210, 211, 211, 211, 211, 211, 211, 211, 211, 211, 211,
        211, 211, 211, 211, 211, 211, 211, 211, 211, 211, 211, 211, 212, 212, 212, 212, 212, 212, 212, 212, 212, 212, 212, 212, 212, 212, 212, 212, 212, 212, 212, 212, 212, 212, 212, 212, 213, 213, 213, 213,
        213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 213, 214, 214, 214, 214, 214, 214, 214, 214, 214, 214, 214, 214, 214, 214, 214, 214, 214, 214
    };
    static const uint8_t EXPECT_FACTORIES[] = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80,
        81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 101, 102, 102, 102, 102, 103, 103, 103, 103, 103, 103, 104, 104, 104, 104, 104, 104, 104, 104,
        105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 106, 106, 106, 106, 106, 106, 106, 106, 106, 106, 106, 106, 107, 107, 107, 107, 107, 107, 107, 107, 107, 107, 107, 107, 107, 107, 108, 108, 108, 108,
        108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 108, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 109, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110,
        110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 112, 112, 112, 112, 112, 112, 112, 112,
        112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 112, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113,
        113, 113, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115,
        115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 115, 116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 116,
        116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 116, 117, 117, 117, 117, 117, 117, 117, 117, 117, 117, 117, 117, 117, 117, 117, 117, 117, 117, 117, 117, 117, 117, 117, 117, 117, 117, 117, 117
    };
    static const uint8_t EXPECT_DEFENSE[] = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 51, 52, 52, 52, 52, 53, 53, 53, 53, 53, 53, 54, 54, 54, 54, 54, 54, 54, 54, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
        56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
        58, 58, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60,
        61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
        62, 62, 62, 62, 62, 62, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
        65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
        66, 66, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 67, 68, 68, 68, 68,
        68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 69, 69, 69, 69, 69, 69, 69, 69
    };

    for (int i = 0; i < LIMIT; ++i) {
        const game::config::HostConfiguration fig;

        const int clans = i+1;

        game::map::Planet p(42);
        p.setOwner(1);
        p.setCargo(game::Element::Colonists, 10000);

        // 4-argument version
        TS_ASSERT_EQUALS(game::map::getMaxBuildings(p, game::MineBuilding,    fig, clans).orElse(-1), EXPECT_MINES[i]);
        TS_ASSERT_EQUALS(game::map::getMaxBuildings(p, game::FactoryBuilding, fig, clans).orElse(-1), EXPECT_FACTORIES[i]);
        TS_ASSERT_EQUALS(game::map::getMaxBuildings(p, game::DefenseBuilding, fig, clans).orElse(-1), EXPECT_DEFENSE[i]);

        // 3-argument version
        p.setCargo(game::Element::Colonists, clans);
        TS_ASSERT_EQUALS(game::map::getMaxBuildings(p, game::MineBuilding,    fig).orElse(-1), EXPECT_MINES[i]);
        TS_ASSERT_EQUALS(game::map::getMaxBuildings(p, game::FactoryBuilding, fig).orElse(-1), EXPECT_FACTORIES[i]);
        TS_ASSERT_EQUALS(game::map::getMaxBuildings(p, game::DefenseBuilding, fig).orElse(-1), EXPECT_DEFENSE[i]);
    }
}

