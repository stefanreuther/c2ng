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

using game::HostVersion;
using game::config::HostConfiguration;


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

    HostConfiguration config;
    HostVersion host(HostVersion::PHost, MKVERSION(3,0,0));

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

    void doTaxSeries(const HostVersion host, const int8_t (&expectedHappiness)[50], const int8_t (&expectedIncome)[50])
    {
        const HostConfiguration config;
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
    doTaxSeries(HostVersion(HostVersion::Host, MKVERSION(3, 22, 40)), TAX_SERIES_HAPPINESS, TAX_SERIES_INCOME_THOST);
}

void
TestGameMapPlanetFormula::testTaxSeriesPHost()
{
    doTaxSeries(HostVersion(HostVersion::PHost, MKVERSION(4, 0, 0)), TAX_SERIES_HAPPINESS, TAX_SERIES_INCOME_PHOST);
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

    void doTemperatureSeries(const HostVersion host, const int planetOwner, const int8_t (&expectedHappiness)[101])
    {
        const HostConfiguration config;
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
    doTemperatureSeries(HostVersion(HostVersion::Host, MKVERSION(3,22,40)), 1, TEMP_SERIES_THOST_FED);
}

void
TestGameMapPlanetFormula::testTemperatureSeriesFedPHost()
{
    doTemperatureSeries(HostVersion(HostVersion::PHost, MKVERSION(4,0,0)), 1, TEMP_SERIES_PHOST_FED);
}

void
TestGameMapPlanetFormula::testTemperatureSeriesCryTHost()
{
    doTemperatureSeries(HostVersion(HostVersion::Host, MKVERSION(3,22,40)), 7, TEMP_SERIES_THOST_CRY);
}

void
TestGameMapPlanetFormula::testTemperatureSeriesCryPHost()
{
    doTemperatureSeries(HostVersion(HostVersion::PHost, MKVERSION(4,0,0)), 7, TEMP_SERIES_PHOST_CRY);
}

/*
 *  Temperature Series: test one situation for different numbers of buildings
 *
 *  Equivalent to c2hosttest/planet/05_tax_building
 */

namespace {
    void doBuildingSeries(const HostVersion host, const int cutoff)
    {
        const HostConfiguration config;
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
    doBuildingSeries(HostVersion(HostVersion::Host, MKVERSION(3,22,40)), 273);
}

void
TestGameMapPlanetFormula::testBuildingSeriesPHost()
{
    doBuildingSeries(HostVersion(HostVersion::PHost, MKVERSION(4,0,0)), 269);
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

    void doNativeTaxSeries(const HostVersion host, const int8_t (&expectedHappiness)[50], const int8_t (&expectedIncome)[50])
    {
        const HostConfiguration config;
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
    doNativeTaxSeries(HostVersion(HostVersion::Host, MKVERSION(3,22,40)), NTAX_EXPECT_HAPPY, NTAX_EXPECT_INCOME);
}

void
TestGameMapPlanetFormula::testNativeTaxSeriesPHost()
{
    doNativeTaxSeries(HostVersion(HostVersion::PHost, MKVERSION(4,1,5)), NTAX_EXPECT_HAPPY, NTAX_EXPECT_INCOME);
}

/*
 *  Native Tax Building Series
 *
 *  Equivalent to c2hosttest/planet/07_ntax_building
 */

namespace {
    void doNativeBuildingSeries(const HostVersion host, const int num84, const int num83)
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
    doNativeBuildingSeries(HostVersion(HostVersion::Host, MKVERSION(3,22,40)), 72, 200);
}

void
TestGameMapPlanetFormula::testNativeTaxBuildingSeriesPHost()
{
    doNativeBuildingSeries(HostVersion(HostVersion::PHost, MKVERSION(4,0,5)), 71, 200);
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
        const HostConfiguration fig;

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

/*
 *  Maximum Colonists Series
 *
 *  Equivalent to c2hosttest/planet/14_maxpop
 */

namespace {
    /* The reference values have been obtained by running host with ClimateDeathRate=100,
       in the expectation that all excess population dies and only the nominal max population remains.
       This leads to a systematic error in PHost because it internally uses people, not clans, as unit of computation:
       PHost since 4.1/3.5 will come up with one more surviving clan because it truncates population loss:

            2006-12-27  Stefan Reuther  <Streu@gmx.de>

                    * planet.c (DoPlanetaryLimitChecks): climate deaths and reports through
                    message would often differ by one

       For example, if the planet allows 1050 colonists (=10 clans) and has 20 clans,
       PHost will compute a loss of 950 colonists, but truncate that to 9 clans, leaving 11 remaining.

       We compensate for this in the test by using this offset where needed in this test.
       As of 20201004, we do not compensate in production code, because
       (a) the effect can be attributed to CDR which is different in actual games, and
       (b) affected amounts are large enough such that +/- 1 clan doesn't matter as much as for an ice planet. */
    const int P = -1;

    void doMaxColonistSeries(HostVersion host, const HostConfiguration& config, int owner, const int32_t (&expect)[101])
    {
        for (int temp = 0; temp <= 100; ++temp) {
            game::map::Planet p(99);
            p.setOwner(owner);
            p.setColonistHappiness(100);
            p.setCargo(game::Element::Colonists, 99999);
            p.setColonistTax(0);
            p.setCargo(game::Element::Supplies, 0);
            p.setCargo(game::Element::Money, 0);
            p.setTemperature(temp);
            p.setNatives(0);
            p.setNativeHappiness(100);
            p.setNativeRace(0);
            p.setNativeGovernment(0);
            p.setNativeTax(0);

            char name[50];
            std::sprintf(name, "temp=%d", temp);

            TSM_ASSERT_EQUALS(name, getMaxSupportedColonists(p, config, host).orElse(-777), expect[temp]);
        }
    }
}

void
TestGameMapPlanetFormula::testMaxColonistSeriesNormalTHost()
{
    static const int32_t EXPECT[] = {
            3,      5,      7,      9,     11,     13,     15,     17,     19,     21,     23,     25,     27,     29,     31,
        45520,  48293,  51018,  53693,  56315,  58882,  61390,  63838,  66223,  68543,  70795,  72977,  75088,  77124,  79085,
        80967,  82770,  84491,  86128,  87681,  89148,  90526,  91815,  93014,  94121,  95135,  96056,  96881,  97611,  98245,
        98782,  99222,  99564,  99808,  99953, 100000,  99948,  99798,  99549,  99202,  98758,  98216,  97577,  96842,  96011,
        95086,  94067,  92955,  91752,  90458,  89075,  87605,  86047,  84405,  82680,  80874,  78987,  77023,  74983,  72869,
        70683,  68427,  66104,  63715,  61264,  58753,  56183,  53558,  50881,  48153,     31,     29,     27,     25,     23,
           21,     19,     17,     15,     13,     11,      9,      7,      5,      3,      1
    };
    HostConfiguration config;
    config[HostConfiguration::ClimateLimitsPopulation].set(1);

    doMaxColonistSeries(HostVersion(HostVersion::Host, MKVERSION(3,22,46)), config, 3, EXPECT);
}

void
TestGameMapPlanetFormula::testMaxColonistSeriesNormalPHost()
{
    static const int32_t EXPECT[] = {
              3,       13,       23,       33,       43,       53,       63,       73,       83,       93,      103,      113,      123,      133,      143,
        P+45400,  P+48176,  P+50905,  P+53583,  P+56209,  P+58779,  P+61291,  P+63743,  P+66132,  P+68455,  P+70711,  P+72897,  P+75012,  P+77052,  P+79016,
        P+80902,  P+82709,  P+84433,  P+86075,  P+87631,  P+89101,  P+90483,  P+91776,  P+92978,  P+94089,  P+95106,  P+96030,  P+96859,  P+97592,  P+98229,
        P+98769,  P+99212,  P+99557,  P+99803,  P+99951,   100000,  P+99951,  P+99803,  P+99557,  P+99212,  P+98769,  P+98229,  P+97592,  P+96859,  P+96030,
        P+95106,  P+94089,  P+92978,  P+91776,  P+90483,  P+89101,  P+87631,  P+86075,  P+84433,  P+82709,  P+80902,  P+79016,  P+77052,  P+75012,  P+72897,
        P+70711,  P+68455,  P+66132,  P+63743,  P+61291,  P+58779,  P+56209,  P+53583,  P+50905,  P+48176,      151,      141,      131,      121,      111,
            101,       91,       81,       71,       61,       51,       41,       31,       21,       11,        1
    };
    HostConfiguration config;
    config[HostConfiguration::ClimateLimitsPopulation].set(1);

    doMaxColonistSeries(HostVersion(HostVersion::PHost, MKVERSION(4,1,0)), config, 3, EXPECT);
}

void
TestGameMapPlanetFormula::testMaxColonistSeriesRebelTHost()
{
    static const int32_t EXPECT[] = {
        90000,  90000,  90000,  90000,  90000,  90000,  90000,  90000,  90000,  90000,  90000,  90000,  90000,  90000,  90000,
        90000,  90000,  90000,  90000,  90000,  58882,  61390,  63838,  66223,  68543,  70795,  72977,  75088,  77124,  79085,
        80967,  82770,  84491,  86128,  87681,  89148,  90526,  91815,  93014,  94121,  95135,  96056,  96881,  97611,  98245,
        98782,  99222,  99564,  99808,  99953, 100000,  99948,  99798,  99549,  99202,  98758,  98216,  97577,  96842,  96011,
        95086,  94067,  92955,  91752,  90458,  89075,  87605,  86047,  84405,  82680,  80874,  78987,  77023,  74983,  72869,
        70683,  68427,  66104,  63715,  61264,  58753,  56183,  53558,  50881,  48153,     60,     60,     60,     60,     60,
           60,     60,     60,     60,     60,     60,     60,     60,     60,     60,     60
    };
    HostConfiguration config;
    config[HostConfiguration::ClimateLimitsPopulation].set(1);

    doMaxColonistSeries(HostVersion(HostVersion::Host, MKVERSION(3,22,46)), config, 10, EXPECT);
}

void
TestGameMapPlanetFormula::testMaxColonistSeriesRebelPHost()
{
    static const int32_t EXPECT[] = {
          90000,    90000,    90000,    90000,    90000,    90000,    90000,    90000,    90000,    90000,    90000,    90000,    90000,    90000,    90000,
          90000,    90000,    90000,    90000,    90000,  P+58779,  P+61291,  P+63743,  P+66132,  P+68455,  P+70711,  P+72897,  P+75012,  P+77052,  P+79016,
        P+80902,  P+82709,  P+84433,  P+86075,  P+87631,  P+89101,  P+90483,  P+91776,  P+92978,  P+94089,  P+95106,  P+96030,  P+96859,  P+97592,  P+98229,
        P+98769,  P+99212,  P+99557,  P+99803,  P+99951,   100000,  P+99951,  P+99803,  P+99557,  P+99212,  P+98769,  P+98229,  P+97592,  P+96859,  P+96030,
        P+95106,  P+94089,  P+92978,  P+91776,  P+90483,  P+89101,  P+87631,  P+86075,  P+84433,  P+82709,  P+80902,  P+79016,  P+77052,  P+75012,  P+72897,
        P+70711,  P+68455,  P+66132,  P+63743,  P+61291,  P+58779,  P+56209,  P+53583,  P+50905,  P+48176,      151,      141,      131,      121,      111,
            101,       91,       81,       71,       61,       60,       60,       60,       60,       60,       60
    };
    HostConfiguration config;
    config[HostConfiguration::ClimateLimitsPopulation].set(1);

    doMaxColonistSeries(HostVersion(HostVersion::PHost, MKVERSION(4,1,0)), config, 10, EXPECT);
}

void
TestGameMapPlanetFormula::testMaxColonistSeriesKlingonTHost()
{
    static const int32_t EXPECT[] = {
            3,      5,      7,      9,     11,     13,     15,     17,     19,     21,     23,     25,     27,     29,     31,
        45520,  48293,  51018,  53693,  56315,  58882,  61390,  63838,  66223,  68543,  70795,  72977,  75088,  77124,  79085,
        80967,  82770,  84491,  86128,  87681,  89148,  90526,  91815,  93014,  94121,  95135,  96056,  96881,  97611,  98245,
        98782,  99222,  99564,  99808,  99953, 100000,  99948,  99798,  99549,  99202,  98758,  98216,  97577,  96842,  96011,
        95086,  94067,  92955,  91752,  90458,  89075,  87605,  86047,  84405,  82680,  80874,  78987,  77023,  74983,  72869,
        70683,  68427,  66104,  63715,  61264,  58753,  56183,  53558,  50881,  48153,     60,     60,     60,     60,     60,
           60,     60,     60,     60,     60,     60,     60,     60,     60,     60,     60
    };
    HostConfiguration config;
    config[HostConfiguration::ClimateLimitsPopulation].set(1);

    doMaxColonistSeries(HostVersion(HostVersion::Host, MKVERSION(3,22,46)), config, 4, EXPECT);
}

void
TestGameMapPlanetFormula::testMaxColonistSeriesKlingonPHost()
{
    static const int32_t EXPECT[] = {
              3,       13,       23,       33,       43,       53,       63,       73,       83,       93,      103,      113,      123,      133,      143,
        P+45400,  P+48176,  P+50905,  P+53583,  P+56209,  P+58779,  P+61291,  P+63743,  P+66132,  P+68455,  P+70711,  P+72897,  P+75012,  P+77052,  P+79016,
        P+80902,  P+82709,  P+84433,  P+86075,  P+87631,  P+89101,  P+90483,  P+91776,  P+92978,  P+94089,  P+95106,  P+96030,  P+96859,  P+97592,  P+98229,
        P+98769,  P+99212,  P+99557,  P+99803,  P+99951,   100000,  P+99951,  P+99803,  P+99557,  P+99212,  P+98769,  P+98229,  P+97592,  P+96859,  P+96030,
        P+95106,  P+94089,  P+92978,  P+91776,  P+90483,  P+89101,  P+87631,  P+86075,  P+84433,  P+82709,  P+80902,  P+79016,  P+77052,  P+75012,  P+72897,
        P+70711,  P+68455,  P+66132,  P+63743,  P+61291,  P+58779,  P+56209,  P+53583,  P+50905,  P+48176,      151,      141,      131,      121,      111,
            101,       91,       81,       71,       61,       60,       60,       60,       60,       60,       60
    };
    HostConfiguration config;
    config[HostConfiguration::ClimateLimitsPopulation].set(1);

    doMaxColonistSeries(HostVersion(HostVersion::PHost, MKVERSION(4,1,0)), config, 4, EXPECT);
}

void
TestGameMapPlanetFormula::testMaxColonistSeriesCrystalTHost()
{
    static const int32_t EXPECT[] = {
            0,   1000,   2000,   3000,   4000,   5000,   6000,   7000,   8000,   9000,  10000,  11000,  12000,  13000,  14000,
        15000,  16000,  17000,  18000,  19000,  20000,  21000,  22000,  23000,  24000,  25000,  26000,  27000,  28000,  29000,
        30000,  31000,  32000,  33000,  34000,  35000,  36000,  37000,  38000,  39000,  40000,  41000,  42000,  43000,  44000,
        45000,  46000,  47000,  48000,  49000,  50000,  51000,  52000,  53000,  54000,  55000,  56000,  57000,  58000,  59000,
        60000,  61000,  62000,  63000,  64000,  65000,  66000,  67000,  68000,  69000,  70000,  71000,  72000,  73000,  74000,
        75000,  76000,  77000,  78000,  79000,  80000,  81000,  82000,  83000,  84000,  85000,  86000,  87000,  88000,  89000,
        90000,  91000,  92000,  93000,  94000,  95000,  96000,  97000,  98000,  99000, 100000
    };
    HostConfiguration config;
    config[HostConfiguration::ClimateLimitsPopulation].set(1);

    doMaxColonistSeries(HostVersion(HostVersion::Host, MKVERSION(3,22,46)), config, 7, EXPECT);
}

void
TestGameMapPlanetFormula::testMaxColonistSeriesCrystalPHost()
{
    static const int32_t EXPECT[] = {
            1,   1000,   2000,   3000,   4000,   5000,   6000,   7000,   8000,   9000,  10000,  11000,  12000,  13000,  14000,
        15000,  16000,  17000,  18000,  19000,  20000,  21000,  22000,  23000,  24000,  25000,  26000,  27000,  28000,  29000,
        30000,  31000,  32000,  33000,  34000,  35000,  36000,  37000,  38000,  39000,  40000,  41000,  42000,  43000,  44000,
        45000,  46000,  47000,  48000,  49000,  50000,  51000,  52000,  53000,  54000,  55000,  56000,  57000,  58000,  59000,
        60000,  61000,  62000,  63000,  64000,  65000,  66000,  67000,  68000,  69000,  70000,  71000,  72000,  73000,  74000,
        75000,  76000,  77000,  78000,  79000,  80000,  81000,  82000,  83000,  84000,  85000,  86000,  87000,  88000,  89000,
        90000,  91000,  92000,  93000,  94000,  95000,  96000,  97000,  98000,  99000, 100000
    };
    HostConfiguration config;
    config[HostConfiguration::ClimateLimitsPopulation].set(1);

    doMaxColonistSeries(HostVersion(HostVersion::PHost, MKVERSION(4,1,0)), config, 7, EXPECT);
}

void
TestGameMapPlanetFormula::testMaxColonistSeriesCrystalSinTemp()
{
    static const int32_t EXPECT[] = {
              3,       13  ,     23,       33,       43,       53,       63,       73,       83,       93,      103,      113,      123,      133,      143,
        P+23345,  P+24869,  P+26388,  P+27900,  P+29405,  P+30902,  P+32392,  P+33874,  P+35348,  P+36813,  P+38269,  P+39715,  P+41152,  P+42578,  P+43994,
        P+45400,  P+46793,  P+48176,  P+49546,  P+50905,  P+52250,  P+53583,  P+54903,  P+56209,  P+57501,  P+58779,  P+60043,  P+61291,  P+62525,  P+63743,
        P+64945,  P+66132,  P+67302,  P+68455,  P+69592,  P+70711,  P+71813,  P+72897,  P+73964,  P+75012,  P+76041,  P+77052,  P+78044,  P+79016,  P+79969,
        P+80902,  P+81815,  P+82709,  P+83581,  P+84433,  P+85265,  P+86075,  P+86864,  P+87631,  P+88377,  P+89101,  P+89803,  P+90483,  P+91141,  P+91776,
        P+92388,  P+92978,  P+93545,  P+94089,  P+94609,  P+95106,  P+95580,  P+96030,  P+96456,  P+96859,  P+97237,  P+97592,  P+97923,  P+98229,  P+98511,
        P+98769,  P+99003,  P+99212,  P+99397,  P+99557,  P+99692,  P+99803,  P+99889,  P+99951,  P+99988,   100000
    };
    HostConfiguration config;
    config[HostConfiguration::ClimateLimitsPopulation].set(1);
    config[HostConfiguration::CrystalSinTempBehavior].set(1);

    doMaxColonistSeries(HostVersion(HostVersion::PHost, MKVERSION(4,1,0)), config, 7, EXPECT);
}
