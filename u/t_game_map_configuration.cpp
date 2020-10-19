/**
  *  \file u/t_game_map_configuration.cpp
  *  \brief Test for game::map::Configuration
  */

#include "game/map/configuration.hpp"

#include "t_game_map.hpp"

using game::map::Point;

void
TestGameMapConfiguration::testFlat()
{
    // ex GameCoordTestSuite::testFlat
    game::map::Configuration cc;

    // Configure map to not-wrapped, standard size
    cc.setConfiguration(cc.Flat, Point(2000, 2000), Point(2000, 2000));
    TS_ASSERT(cc.getMinimumCoordinates() == Point(1000, 1000));
    TS_ASSERT(cc.getMaximumCoordinates() == Point(3000, 3000));

    // Normalizing does not modify points
    TS_ASSERT(cc.getSimpleCanonicalLocation(Point(10, 20)) == Point(10, 20));
    TS_ASSERT(cc.getCanonicalLocation(Point(10, 20)) == Point(10, 20));
    TS_ASSERT(cc.getSimpleNearestAlias(Point(10, 20), Point(2900, 2900)) == Point(10, 20));
    TS_ASSERT(cc.getSimpleNearestAlias(Point(10, 20), Point(1100, 2900)) == Point(10, 20));
    TS_ASSERT(cc.getSimpleNearestAlias(Point(10, 20), Point(2900, 1100)) == Point(10, 20));
    TS_ASSERT(cc.getSimpleNearestAlias(Point(10, 20), Point(1100, 1100)) == Point(10, 20));

    TS_ASSERT(cc.isOnMap(Point(10, 20)));

    // Sector numbers known for all points in [1000,3000)
    // Sectors are numbered
    //   100 110 120 130 ... 190 200 210 ...
    //   101 111 121 131 ... 191 201 211 ...
    //   ... ... ...
    //   109 119 129 139 ... 199 209 219 ...
    //   300 310 320 330 ... 390 400 410 ...
    //   ... ... ...
    //   309 319 329 339 ... 399 409 419 ...
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1000, 1000)), 309);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1099, 1099)), 309);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1111, 1000)), 319);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1222, 1000)), 329);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1111, 1111)), 318);

    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(2000, 1000)), 409);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1000, 2000)), 109);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(2000, 2000)), 209);

    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(2999, 2999)), 290);

    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1000)), 359);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1100)), 358);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1200)), 357);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1300)), 356);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1400)), 355);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1500)), 354);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1600)), 353);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1700)), 352);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1800)), 351);

    // Check that parsed sector numbers are centered within their respective sector
    for (int i = 100; i < 500; ++i) {
        Point p;
        TS_ASSERT(cc.parseSectorNumber(i, p));
        TS_ASSERT_EQUALS(cc.getSectorNumber(p), i);
        TS_ASSERT_EQUALS(p.getX() % 100, 50);
        TS_ASSERT_EQUALS(p.getY() % 100, 50);
    }

    Point p;
    TS_ASSERT(!cc.parseSectorNumber(0, p));
    TS_ASSERT(!cc.parseSectorNumber(-1, p));
    TS_ASSERT(!cc.parseSectorNumber(99, p));
    TS_ASSERT(!cc.parseSectorNumber(500, p));
    TS_ASSERT(!cc.parseSectorNumber(501, p));

    // Some out-of-range values
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(999,  999)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1999, 999)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(999,  1999)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1999, 3001)), 0);

    // Distance
    TS_ASSERT_EQUALS(cc.getSquaredDistance(Point(1000, 1000), Point(1003, 1004)), 25);
}

void
TestGameMapConfiguration::testFlatSmall()
{
    // ex GameCoordTestSuite::testFlatSmall()
    game::map::Configuration cc;

    // Configure map to not-wrapped, smaller size
    cc.setConfiguration(cc.Flat, Point(2000, 2000), Point(1000, 1000));
    TS_ASSERT(cc.getMinimumCoordinates() == Point(1500, 1500));
    TS_ASSERT(cc.getMaximumCoordinates() == Point(2500, 2500));

    // Normalizing still does not modify points
    TS_ASSERT(cc.getSimpleCanonicalLocation(Point(10, 20)) == Point(10, 20));
    TS_ASSERT(cc.getCanonicalLocation(Point(10, 20)) == Point(10, 20));
    TS_ASSERT(cc.getSimpleNearestAlias(Point(10, 20), Point(2900, 2900)) == Point(10, 20));
    TS_ASSERT(cc.getSimpleNearestAlias(Point(10, 20), Point(1100, 2900)) == Point(10, 20));
    TS_ASSERT(cc.getSimpleNearestAlias(Point(10, 20), Point(2900, 1100)) == Point(10, 20));
    TS_ASSERT(cc.getSimpleNearestAlias(Point(10, 20), Point(1100, 1100)) == Point(10, 20));

    TS_ASSERT(cc.isOnMap(Point(10, 20)));

    // Sector numbers still known for all points on map:
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1000, 1000)), 309);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1099, 1099)), 309);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1111, 1000)), 319);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1222, 1000)), 329);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1111, 1111)), 318);

    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(2000, 1000)), 409);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1000, 2000)), 109);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(2000, 2000)), 209);

    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(2999, 2999)), 290);

    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1000)), 359);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1100)), 358);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1200)), 357);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1300)), 356);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1400)), 355);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1500)), 354);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1600)), 353);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1700)), 352);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1800)), 351);

    // Check that parsed sector numbers are centered within their respective sector
    for (int i = 100; i < 500; ++i) {
        Point p;
        TS_ASSERT(cc.parseSectorNumber(i, p));
        TS_ASSERT_EQUALS(cc.getSectorNumber(p), i);
        TS_ASSERT_EQUALS(p.getX() % 100, 50);
        TS_ASSERT_EQUALS(p.getY() % 100, 50);
    }

    Point p;
    TS_ASSERT(!cc.parseSectorNumber(0, p));
    TS_ASSERT(!cc.parseSectorNumber(-1, p));
    TS_ASSERT(!cc.parseSectorNumber(99, p));
    TS_ASSERT(!cc.parseSectorNumber(500, p));
    TS_ASSERT(!cc.parseSectorNumber(501, p));

    // Some out-of-range values
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(999,  999)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1999, 999)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(999,  1999)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1999, 3001)), 0);
}

void
TestGameMapConfiguration::testFlatOffset()
{
    // ex GameCoordTestSuite::testFlatOffset()
    game::map::Configuration cc;

    // Configure map to not-wrapped, smaller size and not centered at 2000,2000
    cc.setConfiguration(cc.Flat, Point(1750, 2500), Point(1500, 1000));
    TS_ASSERT_EQUALS(cc.getMinimumCoordinates().getX(), 1000);
    TS_ASSERT_EQUALS(cc.getMinimumCoordinates().getY(), 2000);
    TS_ASSERT_EQUALS(cc.getMaximumCoordinates().getX(), 2500);
    TS_ASSERT_EQUALS(cc.getMaximumCoordinates().getY(), 3000);

    // Normalizing still does not modify points
    TS_ASSERT(cc.getSimpleCanonicalLocation(Point(10, 20)) == Point(10, 20));
    TS_ASSERT(cc.getCanonicalLocation(Point(10, 20)) == Point(10, 20));
    TS_ASSERT(cc.getSimpleNearestAlias(Point(10, 20), Point(2900, 2900)) == Point(10, 20));
    TS_ASSERT(cc.getSimpleNearestAlias(Point(10, 20), Point(1100, 2900)) == Point(10, 20));
    TS_ASSERT(cc.getSimpleNearestAlias(Point(10, 20), Point(2900, 1100)) == Point(10, 20));
    TS_ASSERT(cc.getSimpleNearestAlias(Point(10, 20), Point(1100, 1100)) == Point(10, 20));

    TS_ASSERT(cc.isOnMap(Point(10, 20)));

    // Sector numbers still known for all points on map, but offset
    // relative to new center at 1750,2500 instead of 2000,2000.
    // Those are now out-of-range:
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1000, 1000)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1099, 1099)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1111, 1000)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1222, 1000)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1111, 1111)), 0);

    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(2000, 1000)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1000, 2000)), 324);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(2000, 2000)), 424);

    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(2999, 2999)), 0);

    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1000)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1100)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1200)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1300)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1400)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1500)), 379);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1600)), 378);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1700)), 377);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1800)), 376);

    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(999,   999)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1999,  999)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(999,  1999)), 325);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1999, 3001)), 224);

    // Check that parsed sector numbers are centered within their respective sector
    // Sectors are now centered around (xx00,xx50), not (xx00,xx50)
    for (int i = 100; i < 500; ++i) {
        Point p;
        TS_ASSERT(cc.parseSectorNumber(i, p));
        TS_ASSERT_EQUALS(cc.getSectorNumber(p), i);
        TS_ASSERT_EQUALS(p.getX() % 100, 0);
        TS_ASSERT_EQUALS(p.getY() % 100, 50);
    }

    Point p;
    TS_ASSERT(!cc.parseSectorNumber(0, p));
    TS_ASSERT(!cc.parseSectorNumber(-1, p));
    TS_ASSERT(!cc.parseSectorNumber(99, p));
    TS_ASSERT(!cc.parseSectorNumber(500, p));
    TS_ASSERT(!cc.parseSectorNumber(501, p));
}

void
TestGameMapConfiguration::testWrapped()
{
    // ex GameCoordTestSuite::testWrapped
    game::map::Configuration cc;

    // Configure map to wrapped, standard size
    cc.setConfiguration(cc.Wrapped, Point(2000, 2000), Point(2000, 2000));

    // Normalizing
    TS_ASSERT(cc.getSimpleCanonicalLocation(Point(10, 20)) == Point(2010, 2020));
    TS_ASSERT(cc.getCanonicalLocation(Point(10, 20)) == Point(2010, 2020));
    TS_ASSERT(cc.getSimpleNearestAlias(Point(10, 20), Point(2900, 2900)) == Point(2010, 2020));
    TS_ASSERT(cc.getSimpleNearestAlias(Point(10, 20), Point(1100, 2900)) == Point(2010, 2020));
    TS_ASSERT(cc.getSimpleNearestAlias(Point(10, 20), Point(2900, 1100)) == Point(2010, 2020));
    TS_ASSERT(cc.getSimpleNearestAlias(Point(10, 20), Point(1100, 1100)) == Point(2010, 2020));

    TS_ASSERT(cc.getSimpleNearestAlias(Point(1010, 1020), Point(2900, 2900)) == Point(3010, 3020));
    TS_ASSERT(cc.getSimpleNearestAlias(Point(1010, 1020), Point(1100, 2900)) == Point(1010, 3020));
    TS_ASSERT(cc.getSimpleNearestAlias(Point(1010, 1020), Point(2900, 1100)) == Point(3010, 1020));
    TS_ASSERT(cc.getSimpleNearestAlias(Point(1010, 1020), Point(1100, 1100)) == Point(1010, 1020));

    TS_ASSERT(!cc.isOnMap(Point(10, 20)));

    // Sector numbers known for all points in [1000,3000), same as in testFlat()
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1000, 1000)), 309);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1099, 1099)), 309);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1111, 1000)), 319);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1222, 1000)), 329);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1111, 1111)), 318);

    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(2000, 1000)), 409);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1000, 2000)), 109);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(2000, 2000)), 209);

    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(2999, 2999)), 290);

    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1000)), 359);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1100)), 358);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1200)), 357);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1300)), 356);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1400)), 355);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1500)), 354);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1600)), 353);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1700)), 352);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1800)), 351);

    // Check that parsed sector numbers are centered within their respective sector
    for (int i = 100; i < 500; ++i) {
        Point p;
        TS_ASSERT(cc.parseSectorNumber(i, p));
        TS_ASSERT_EQUALS(cc.getSectorNumber(p), i);
        TS_ASSERT_EQUALS(p.getX() % 100, 50);
        TS_ASSERT_EQUALS(p.getY() % 100, 50);
        TS_ASSERT_EQUALS(p, cc.getSimpleCanonicalLocation(p));
        TS_ASSERT_EQUALS(p, cc.getCanonicalLocation(p));
    }

    Point p;
    TS_ASSERT(!cc.parseSectorNumber(0, p));
    TS_ASSERT(!cc.parseSectorNumber(-1, p));
    TS_ASSERT(!cc.parseSectorNumber(99, p));
    TS_ASSERT(!cc.parseSectorNumber(500, p));
    TS_ASSERT(!cc.parseSectorNumber(501, p));

    // Some out-of-range values
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(999,  999)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1999, 999)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(999,  1999)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1999, 3001)), 0);


    // Distance
    TS_ASSERT_EQUALS(cc.getSquaredDistance(Point(1000, 1000), Point(1003, 1004)), 25);
    TS_ASSERT_EQUALS(cc.getSquaredDistance(Point(1000, 1000), Point(3003, 3004)), 25);
    TS_ASSERT_EQUALS(cc.getSquaredDistance(Point(3000, 3000), Point(1003, 1004)), 25);
    TS_ASSERT_EQUALS(cc.getSquaredDistance(Point(3000, 3000), Point(3003, 3004)), 25);
}

void
TestGameMapConfiguration::testWrappedSmall()
{
    // ex GameCoordTestSuite::testWrappedSmall

    game::map::Configuration cc;

    // Configure map to wrapped, small size
    cc.setConfiguration(cc.Wrapped, Point(2000, 2000), Point(1000, 1000));

    // Sector numbers known for all points in [1500,2500), but still numbered as in normal map.
    // Parts are out of range.
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1000, 1000)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1099, 1099)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1111, 1000)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1222, 1000)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1111, 1111)), 0);

    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(2000, 1000)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1000, 2000)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(2000, 2000)), 209);

    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(2999, 2999)), 0);

    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1000)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1100)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1200)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1300)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1400)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1500)), 354);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1600)), 353);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1700)), 352);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1500, 1800)), 351);
}

void
TestGameMapConfiguration::testCircular()
{
    game::map::Configuration cc;
    cc.setConfiguration(cc.Circular, Point(2000, 2000), Point(1000, 1000));

    // Test isOnMap:
    TS_ASSERT_EQUALS(cc.isOnMap(Point(2000, 2000)), true);     // clearly inside
    TS_ASSERT_EQUALS(cc.isOnMap(Point(3000, 2000)), true);     // at edge
    TS_ASSERT_EQUALS(cc.isOnMap(Point(2000, 3000)), true);     // at edge
    TS_ASSERT_EQUALS(cc.isOnMap(Point(3000, 3000)), false);    // clearly outside
    TS_ASSERT_EQUALS(cc.isOnMap(Point(2001, 3000)), false);    // barely outside

    // Test getCanonicalLocation:
    TS_ASSERT_EQUALS(cc.getCanonicalLocation(Point(2000, 2000)), Point(2000, 2000));
    TS_ASSERT_EQUALS(cc.getCanonicalLocation(Point(3000, 2000)), Point(3000, 2000));
    TS_ASSERT_EQUALS(cc.getCanonicalLocation(Point(2000, 3000)), Point(2000, 3000));
    TS_ASSERT_EQUALS(cc.getCanonicalLocation(Point(3000, 3000)), Point(1586, 1586));
    TS_ASSERT_EQUALS(cc.getCanonicalLocation(Point(2001, 3000)), Point(1999, 1000));

    // Some more points (cross-checked against pwrap)
    TS_ASSERT_EQUALS(cc.getCanonicalLocation(Point(2100, 3000)), Point(1901, 1010));
    TS_ASSERT_EQUALS(cc.getCanonicalLocation(Point(2100, 3100)), Point(1919, 1108));
    TS_ASSERT_EQUALS(cc.getCanonicalLocation(Point(2102, 3100)), Point(1917, 1109));
    TS_ASSERT_EQUALS(cc.getCanonicalLocation(Point(1300, 1200)), Point(2617, 2705));
    TS_ASSERT_EQUALS(cc.getCanonicalLocation(Point(3027, 2286)), Point(1100, 1749));

    // Test getPointAlias:
    Point result;

    // - Center cannot be mapped outside ("too far inside" case)
    TS_ASSERT_EQUALS(cc.getPointAlias(Point(2000, 2000), result, 1, true), false);

    // - Edge cannot be mapped outside
    TS_ASSERT_EQUALS(cc.getPointAlias(Point(3000, 2000), result, 1, true), false);

    // - Barely outside cannot be mapped outside because its inverse is outside again
    TS_ASSERT_EQUALS(cc.getPointAlias(Point(1999, 1000), result, 1, true), false);

    // - More points that successfully map:
    TS_ASSERT_EQUALS(cc.getPointAlias(Point(1901, 1010), result, 1, true), true);
    TS_ASSERT_EQUALS(result, Point(2100, 3000));

    TS_ASSERT_EQUALS(cc.getPointAlias(Point(1919, 1108), result, 1, true), true);
    TS_ASSERT_EQUALS(result, Point(2100, 3100));

    TS_ASSERT_EQUALS(cc.getPointAlias(Point(1917, 1109), result, 1, true), true);
    TS_ASSERT_EQUALS(result, Point(2103, 3100));  // note different result than tried in forward mappint above!

    TS_ASSERT_EQUALS(cc.getPointAlias(Point(2617, 2705), result, 1, true), true);
    TS_ASSERT_EQUALS(result, Point(1300, 1200));

    // This is a point where we need to search for the actual match:
    TS_ASSERT_EQUALS(cc.getPointAlias(Point(1100, 1749), result, 1, true), true);
    TS_ASSERT_EQUALS(result, Point(3027, 2286));
}

