/**
  *  \file u/t_game_map_configuration.cpp
  *  \brief Test for game::map::Configuration
  */

#include "game/map/configuration.hpp"

#include "t_game_map.hpp"

using game::map::Point;
using game::config::HostConfiguration;
using game::config::UserConfiguration;
using game::config::ConfigurationOption;

/** Test flat, default map.
    Verifies common operations transformations for default map. */
void
TestGameMapConfiguration::testFlat()
{
    // ex GameCoordTestSuite::testFlat
    game::map::Configuration cc;

    // Configure map to not-wrapped, standard size
    cc.setConfiguration(cc.Flat, Point(2000, 2000), Point(2000, 2000));
    TS_ASSERT(cc.getMinimumCoordinates() == Point(1000, 1000));
    TS_ASSERT(cc.getMaximumCoordinates() == Point(3000, 3000));
    TS_ASSERT(cc.getCenter() == Point(2000, 2000));
    TS_ASSERT(cc.getSize() == Point(2000, 2000));

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

    TS_ASSERT(cc.parseSectorNumber("100", p));
    TS_ASSERT_EQUALS(p.getX(), 1050);
    TS_ASSERT_EQUALS(p.getY(), 2950);

    TS_ASSERT(cc.parseSectorNumber("200", p));
    TS_ASSERT_EQUALS(p.getX(), 2050);
    TS_ASSERT_EQUALS(p.getY(), 2950);

    // Some out-of-range values
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(999,  999)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1999, 999)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(999,  1999)), 0);
    TS_ASSERT_EQUALS(cc.getSectorNumber(Point(1999, 3001)), 0);

    // Distance
    TS_ASSERT_EQUALS(cc.getSquaredDistance(Point(1000, 1000), Point(1003, 1004)), 25);
}

/** Test image transformations in flat, default map.
    Verifies image transformations for default map. */
void
TestGameMapConfiguration::testFlatImage()
{
    game::map::Configuration cc;

    // Configure map to not-wrapped, standard size
    cc.setConfiguration(cc.Flat, Point(2000, 2000), Point(2000, 2000));
    TS_ASSERT_EQUALS(cc.getNumRectangularImages(), 1);
    TS_ASSERT_EQUALS(cc.getNumPointImages(), 1);

    // Point alias
    Point out;
    TS_ASSERT_EQUALS(cc.getPointAlias(Point(1500, 1400), out, 0, true), true);
    TS_ASSERT_EQUALS(out, Point(1500, 1400));
    TS_ASSERT_EQUALS(cc.getPointAlias(Point(1500, 1400), out, 1, true), false);

    TS_ASSERT_EQUALS(cc.getSimplePointAlias(Point(1500, 1400), 0), Point(1500, 1400));
}

/** Test flat, small map.
    Verifies common operations transformations.
    Note that outside points have no sector number.*/
void
TestGameMapConfiguration::testFlatSmall()
{
    // ex GameCoordTestSuite::testFlatSmall()
    game::map::Configuration cc;

    // Configure map to not-wrapped, smaller size
    cc.setConfiguration(cc.Flat, Point(2000, 2000), Point(1000, 1000));
    TS_ASSERT(cc.getMinimumCoordinates() == Point(1500, 1500));
    TS_ASSERT(cc.getMaximumCoordinates() == Point(2500, 2500));
    TS_ASSERT(cc.getCenter() == Point(2000, 2000));
    TS_ASSERT(cc.getSize() == Point(1000, 1000));

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

/** Test nonstandard, small map.
    Verifies common operations transformations.
    Note that sectors are offset. */
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
    TS_ASSERT(cc.getCenter() == Point(1750, 2500));
    TS_ASSERT(cc.getSize() == Point(1500, 1000));

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

/** Test image transformations in nonstandard map.
    Verifies image transformations. */
void
TestGameMapConfiguration::testFlatOffsetImage()
{
    game::map::Configuration cc;

    // Configure map to not-wrapped, smaller size and not centered at 2000,2000
    cc.setConfiguration(cc.Flat, Point(1750, 2500), Point(1500, 1000));
    TS_ASSERT_EQUALS(cc.getNumRectangularImages(), 1);
    TS_ASSERT_EQUALS(cc.getNumPointImages(), 1);

    // Point alias
    Point out;
    TS_ASSERT_EQUALS(cc.getPointAlias(Point(1500, 1400), out, 0, true), true);
    TS_ASSERT_EQUALS(out, Point(1500, 1400));
    TS_ASSERT_EQUALS(cc.getPointAlias(Point(1500, 1400), out, 1, true), false);

    TS_ASSERT_EQUALS(cc.getSimplePointAlias(Point(1500, 1400), 0), Point(1500, 1400));
}

/** Test standard, wrapped map.
    Verifies common operations transformations. */
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
    TS_ASSERT(cc.getSimpleCanonicalLocation(Point(3010, 3020)) == Point(1010, 1020));
    TS_ASSERT(cc.getCanonicalLocation(Point(3010, 3020)) == Point(1010, 1020));
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

/** Test image transformations in wrapped map. */
void
TestGameMapConfiguration::testWrappedImage()
{
    // ex GameCoordTestSuite::testWrapped
    game::map::Configuration cc;

    // Configure map to wrapped, standard size
    cc.setConfiguration(cc.Wrapped, Point(2000, 2000), Point(2000, 2000));
    TS_ASSERT_EQUALS(cc.getNumRectangularImages(), 9);
    TS_ASSERT_EQUALS(cc.getNumPointImages(), 9);

    // Point alias
    Point out;
    TS_ASSERT_EQUALS(cc.getPointAlias(Point(1500, 1500), out, 0, true), true);
    TS_ASSERT_EQUALS(out, Point(1500, 1500));
    TS_ASSERT_EQUALS(cc.getPointAlias(Point(1500, 1500), out, 1, true), true);
    TS_ASSERT_EQUALS(out, Point(-500, -500));
    TS_ASSERT_EQUALS(cc.getPointAlias(Point(1500, 1500), out, 7, true), true);
    TS_ASSERT_EQUALS(out, Point(1500, 3500));
    TS_ASSERT_EQUALS(cc.getPointAlias(Point(1500, 1500), out, 8, true), true);
    TS_ASSERT_EQUALS(out, Point(3500, 3500));

    TS_ASSERT_EQUALS(cc.getPointAlias(Point(500, 500), out, 0, true), false);

    // Simple point alias
    TS_ASSERT_EQUALS(cc.getSimplePointAlias(Point(1500, 1500), 0), Point(1500, 1500));
    TS_ASSERT_EQUALS(cc.getSimplePointAlias(Point(1500, 1500), 1), Point(-500, -500));
    TS_ASSERT_EQUALS(cc.getSimplePointAlias(Point(1500, 1500), 7), Point(1500, 3500));
    TS_ASSERT_EQUALS(cc.getSimplePointAlias(Point(1500, 1500), 8), Point(3500, 3500));

    // Simple point alias, error cases
    // For out-of-range parameters, getSimplePointAlias returns the original point
    TS_ASSERT_EQUALS(cc.getSimplePointAlias(Point( 500,  500), 0),   Point(500, 500));
    TS_ASSERT_EQUALS(cc.getSimplePointAlias(Point(1500, 1500), -1),  Point(1500, 1500));
    TS_ASSERT_EQUALS(cc.getSimplePointAlias(Point(1500, 1500), 888), Point(1500, 1500));
}

/** Test small, wrapped map.
    Verifies common operations transformations.
    Note out-of-range points. */
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

/** Test circular map.
    Verifies common operations transformations. */
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
}

/** Test image transformations in circular map.
    Focus on inside-out transformation (getPointAlias(1)). */
void
TestGameMapConfiguration::testCircularImage()
{
    game::map::Configuration cc;
    cc.setConfiguration(cc.Circular, Point(2000, 2000), Point(1000, 1000));
    TS_ASSERT_EQUALS(cc.getNumRectangularImages(), 1);
    TS_ASSERT_EQUALS(cc.getNumPointImages(), 2);

    // Other circular config values have sensible defaults:
    TS_ASSERT(cc.getCircularPrecision() > 0);
    TS_ASSERT(cc.getCircularExcess() > 500);

    // Test getPointAlias:
    Point result;

    // - Center cannot be mapped outside ("too far inside" case)
    TS_ASSERT_EQUALS(cc.getPointAlias(Point(2000, 2000), result, 1, true), false);
    TS_ASSERT_EQUALS(cc.getPointAlias(Point(2000, 2000), result, 1, false), false);

    // - Edge cannot be mapped outside
    TS_ASSERT_EQUALS(cc.getPointAlias(Point(3000, 2000), result, 1, true), false);
    //   Inexact mapping WILL map it!
    TS_ASSERT_EQUALS(cc.getPointAlias(Point(3000, 2000), result, 1, false), true);
    TS_ASSERT_EQUALS(result, Point(1000, 2000));

    // - Barely outside cannot be mapped outside because its inverse is outside again
    TS_ASSERT_EQUALS(cc.getPointAlias(Point(1999, 1000), result, 1, true), false);
    TS_ASSERT_EQUALS(cc.getPointAlias(Point(1999, 1000), result, 1, false), false);

    // - More points that successfully map:
    TS_ASSERT_EQUALS(cc.getPointAlias(Point(1901, 1010), result, 1, true), true);
    TS_ASSERT_EQUALS(result, Point(2100, 3000));
    TS_ASSERT_EQUALS(cc.getPointAlias(Point(1901, 1010), result, 1, false), true);
    TS_ASSERT_EQUALS(result, Point(2100, 3000));

    TS_ASSERT_EQUALS(cc.getPointAlias(Point(1919, 1108), result, 1, true), true);
    TS_ASSERT_EQUALS(result, Point(2100, 3100));
    TS_ASSERT_EQUALS(cc.getPointAlias(Point(1919, 1108), result, 1, false), true);
    TS_ASSERT_EQUALS(result, Point(2100, 3100));

    TS_ASSERT_EQUALS(cc.getPointAlias(Point(1917, 1109), result, 1, true), true);
    TS_ASSERT_EQUALS(result, Point(2103, 3100));  // note different result than tried in forward mapping above!

    TS_ASSERT_EQUALS(cc.getPointAlias(Point(2617, 2705), result, 1, true), true);
    TS_ASSERT_EQUALS(result, Point(1300, 1200));

    // This is a point where we need to search for the actual match. Inexact mapping yields a different point.
    TS_ASSERT_EQUALS(cc.getPointAlias(Point(1100, 1749), result, 1, true), true);
    TS_ASSERT_EQUALS(result, Point(3027, 2286));
    TS_ASSERT_EQUALS(cc.getPointAlias(Point(1100, 1749), result, 1, false), true);
    TS_ASSERT_EQUALS(result, Point(3026, 2286));

    // Simple point alias: Circular has no simple alias
    TS_ASSERT_EQUALS(cc.getSimplePointAlias(Point(2000, 2000), 0), Point(2000, 2000));
    TS_ASSERT_EQUALS(cc.getSimplePointAlias(Point(2000, 2000), 1), Point(2000, 2000));
    TS_ASSERT_EQUALS(cc.getSimplePointAlias(Point(2000, 2000), 10000), Point(2000, 2000));

    TS_ASSERT_EQUALS(cc.getSimplePointAlias(Point(3000, 3000), 0), Point(3000, 3000));
}

/** Test initialisation from default configuration. */
void
TestGameMapConfiguration::testInitFromConfig()
{
    HostConfiguration config;
    UserConfiguration pref;

    game::map::Configuration testee;
    testee.initFromConfiguration(config, pref);

    TS_ASSERT_EQUALS(testee.isSetFromHostConfiguration(), false);
    TS_ASSERT_EQUALS(testee.getMode(), game::map::Configuration::Flat);
    TS_ASSERT_EQUALS(testee.getCenter(), Point(2000, 2000));
    TS_ASSERT_EQUALS(testee.getSize(), Point(2000, 2000));
}

/** Test initialisation from wrapped map configuration. */
void
TestGameMapConfiguration::testInitFromConfigWrap()
{
    HostConfiguration config;
    UserConfiguration pref;
    config.setOption("AllowWraparoundMap", "Yes", ConfigurationOption::Game);
    config.setOption("WraparoundRectangle", "1000,1010,3000,3020", ConfigurationOption::Game);
    TS_ASSERT_EQUALS(config[HostConfiguration::AllowWraparoundMap](), 1);
    TS_ASSERT_EQUALS(config[HostConfiguration::WraparoundRectangle](3), 3000);

    game::map::Configuration testee;
    testee.initFromConfiguration(config, pref);

    TS_ASSERT_EQUALS(testee.isSetFromHostConfiguration(), true);
    TS_ASSERT_EQUALS(testee.getMode(), game::map::Configuration::Wrapped);
    TS_ASSERT_EQUALS(testee.getCenter(), Point(2000, 2015));
    TS_ASSERT_EQUALS(testee.getSize(), Point(2000, 2010));
}

/** Test initialisation from invalid wrapped map configuration. */
void
TestGameMapConfiguration::testInitFromBadConfig()
{
    HostConfiguration config;
    UserConfiguration pref;
    config.setOption("AllowWraparoundMap", "Yes", ConfigurationOption::Game);
    config.setOption("WraparoundRectangle", "1000,1010,1020,1030", ConfigurationOption::Game);

    game::map::Configuration testee;
    testee.initFromConfiguration(config, pref);

    TS_ASSERT_EQUALS(testee.isSetFromHostConfiguration(), false);
    TS_ASSERT_EQUALS(testee.getMode(), game::map::Configuration::Wrapped);
    TS_ASSERT_EQUALS(testee.getCenter(), Point(1010, 1020));
    TS_ASSERT_EQUALS(testee.getSize(), Point(2000, 2000));
}

/** Test saveToConfiguration.
    Saving a default configuration should not set any option in UserConfiguration. */
void
TestGameMapConfiguration::testSaveToConfig()
{
    UserConfiguration pref;

    game::map::Configuration testee;
    testee.saveToConfiguration(pref);

    afl::base::Ref<UserConfiguration::Enumerator_t> e(pref.getOptions());
    UserConfiguration::OptionInfo_t info;
    while (e->getNextElement(info)) {
        TS_ASSERT(info.second != 0);
        TS_ASSERT_EQUALS(info.second->getSource(), ConfigurationOption::Default);
    }
}

/** Test saveToConfiguration, wrapped map.
    This should produce a single Chart.Geo.Mode entry because other values are standard. */
void
TestGameMapConfiguration::testSaveToConfigWrap()
{
    UserConfiguration pref;

    game::map::Configuration testee;
    testee.setConfiguration(testee.Wrapped, Point(2000, 2000), Point(2000, 2000));
    testee.saveToConfiguration(pref);

    afl::base::Ref<UserConfiguration::Enumerator_t> e(pref.getOptions());
    UserConfiguration::OptionInfo_t info;
    while (e->getNextElement(info)) {
        TS_ASSERT(info.second != 0);
        if (info.first == "Chart.Geo.Mode") {
            TS_ASSERT_EQUALS(info.second->getSource(), ConfigurationOption::Game);
            TS_ASSERT_EQUALS(info.second->toString(), "wrapped");
        } else {
            TS_ASSERT_EQUALS(info.second->getSource(), ConfigurationOption::Default);
        }
    }
}

/** Test saveToConfiguration, full set.
    Configure some more values to force other values to be generated. */
void
TestGameMapConfiguration::testSaveToConfigFull()
{
    UserConfiguration pref;

    game::map::Configuration testee;
    testee.setConfiguration(testee.Wrapped, Point(1800, 1900), Point(2000, 2100));
    testee.setCircularExcess(200);
    testee.setCircularPrecision(7);
    testee.saveToConfiguration(pref);

    ConfigurationOption* opt = pref.getOptionByName("Chart.Geo.Mode");
    TS_ASSERT(opt);
    TS_ASSERT_EQUALS(opt->getSource(), ConfigurationOption::Game);
    TS_ASSERT_EQUALS(opt->toString(), "wrapped");

    opt = pref.getOptionByName("Chart.Geo.Center");
    TS_ASSERT(opt);
    TS_ASSERT_EQUALS(opt->getSource(), ConfigurationOption::Game);
    TS_ASSERT_EQUALS(opt->toString(), "1800,1900");

    opt = pref.getOptionByName("Chart.Geo.Size");
    TS_ASSERT(opt);
    TS_ASSERT_EQUALS(opt->getSource(), ConfigurationOption::Game);
    TS_ASSERT_EQUALS(opt->toString(), "2000,2100");

    opt = pref.getOptionByName("Chart.Circle.Precision");
    TS_ASSERT(opt);
    // TS_ASSERT_EQUALS(opt->getSource(), ConfigurationOption::Game); - no, this is a user option
    TS_ASSERT_EQUALS(opt->toString(), "7");

    opt = pref.getOptionByName("Chart.Circle.Outside");
    TS_ASSERT(opt);
    TS_ASSERT_EQUALS(opt->getSource(), ConfigurationOption::Game);
    TS_ASSERT_EQUALS(opt->toString(), "200");
}

/** Test saveToConfiguration.
    Saving a default configuration should not set any option in UserConfiguration.
    However, an option that was previously set in Game scope remains there. */
void
TestGameMapConfiguration::testSaveToConfigUser()
{
    UserConfiguration pref;
    pref.setOption("Chart.Geo.Mode", "flat", ConfigurationOption::Game);

    game::map::Configuration testee;
    testee.setConfiguration(testee.Flat, Point(2000, 2000), Point(2000, 2000));
    testee.saveToConfiguration(pref);

    afl::base::Ref<UserConfiguration::Enumerator_t> e(pref.getOptions());
    UserConfiguration::OptionInfo_t info;

    ConfigurationOption* opt = pref.getOptionByName("Chart.Geo.Mode");
    TS_ASSERT(opt);
    TS_ASSERT_EQUALS(opt->getSource(), ConfigurationOption::Game);
    TS_ASSERT_EQUALS(opt->toString(), "flat");
}

