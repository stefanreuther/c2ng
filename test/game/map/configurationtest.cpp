/**
  *  \file test/game/map/configurationtest.cpp
  *  \brief Test for game::map::Configuration
  */

#include "game/map/configuration.hpp"
#include "afl/test/testrunner.hpp"

using afl::base::Ref;
using game::map::Point;
using game::config::HostConfiguration;
using game::config::UserConfiguration;
using game::config::ConfigurationOption;

/** Test flat, default map.
    Verifies common operations transformations for default map. */
AFL_TEST("game.map.Configuration:flat", a)
{
    // ex GameCoordTestSuite::testFlat
    game::map::Configuration cc;

    // Configure map to not-wrapped, standard size
    cc.setConfiguration(cc.Flat, Point(2000, 2000), Point(2000, 2000));
    a.check("01. getMinimumCoordinates", cc.getMinimumCoordinates() == Point(1000, 1000));
    a.check("02. getMaximumCoordinates", cc.getMaximumCoordinates() == Point(3000, 3000));
    a.check("03. getCenter", cc.getCenter() == Point(2000, 2000));
    a.check("04. getSize", cc.getSize() == Point(2000, 2000));

    // Normalizing does not modify points
    a.check("11. getSimpleCanonicalLocation", cc.getSimpleCanonicalLocation(Point(10, 20)) == Point(10, 20));
    a.check("12. getCanonicalLocation", cc.getCanonicalLocation(Point(10, 20)) == Point(10, 20));
    a.check("13. getSimpleNearestAlias", cc.getSimpleNearestAlias(Point(10, 20), Point(2900, 2900)) == Point(10, 20));
    a.check("14. getSimpleNearestAlias", cc.getSimpleNearestAlias(Point(10, 20), Point(1100, 2900)) == Point(10, 20));
    a.check("15. getSimpleNearestAlias", cc.getSimpleNearestAlias(Point(10, 20), Point(2900, 1100)) == Point(10, 20));
    a.check("16. getSimpleNearestAlias", cc.getSimpleNearestAlias(Point(10, 20), Point(1100, 1100)) == Point(10, 20));

    a.check("21. limitUserLocation", cc.limitUserLocation(Point(10, 20)) == Point(10, 20));
    a.check("22. limitUserLocation", cc.limitUserLocation(Point(1000, 2000)) == Point(1000, 2000));
    a.check("23. limitUserLocation", cc.limitUserLocation(Point(15000, 9000)) == Point(10000, 9000));

    a.check("31. isOnMap", cc.isOnMap(Point(10, 20)));

    // Sector numbers known for all points in [1000,3000)
    // Sectors are numbered
    //   100 110 120 130 ... 190 200 210 ...
    //   101 111 121 131 ... 191 201 211 ...
    //   ... ... ...
    //   109 119 129 139 ... 199 209 219 ...
    //   300 310 320 330 ... 390 400 410 ...
    //   ... ... ...
    //   309 319 329 339 ... 399 409 419 ...
    a.checkEqual("41. getSectorNumber", cc.getSectorNumber(Point(1000, 1000)), 309);
    a.checkEqual("42. getSectorNumber", cc.getSectorNumber(Point(1099, 1099)), 309);
    a.checkEqual("43. getSectorNumber", cc.getSectorNumber(Point(1111, 1000)), 319);
    a.checkEqual("44. getSectorNumber", cc.getSectorNumber(Point(1222, 1000)), 329);
    a.checkEqual("45. getSectorNumber", cc.getSectorNumber(Point(1111, 1111)), 318);

    a.checkEqual("51. getSectorNumber", cc.getSectorNumber(Point(2000, 1000)), 409);
    a.checkEqual("52. getSectorNumber", cc.getSectorNumber(Point(1000, 2000)), 109);
    a.checkEqual("53. getSectorNumber", cc.getSectorNumber(Point(2000, 2000)), 209);

    a.checkEqual("61. getSectorNumber", cc.getSectorNumber(Point(2999, 2999)), 290);

    a.checkEqual("71. getSectorNumber", cc.getSectorNumber(Point(1500, 1000)), 359);
    a.checkEqual("72. getSectorNumber", cc.getSectorNumber(Point(1500, 1100)), 358);
    a.checkEqual("73. getSectorNumber", cc.getSectorNumber(Point(1500, 1200)), 357);
    a.checkEqual("74. getSectorNumber", cc.getSectorNumber(Point(1500, 1300)), 356);
    a.checkEqual("75. getSectorNumber", cc.getSectorNumber(Point(1500, 1400)), 355);
    a.checkEqual("76. getSectorNumber", cc.getSectorNumber(Point(1500, 1500)), 354);
    a.checkEqual("77. getSectorNumber", cc.getSectorNumber(Point(1500, 1600)), 353);
    a.checkEqual("78. getSectorNumber", cc.getSectorNumber(Point(1500, 1700)), 352);
    a.checkEqual("79. getSectorNumber", cc.getSectorNumber(Point(1500, 1800)), 351);

    // Check that parsed sector numbers are centered within their respective sector
    for (int i = 100; i < 500; ++i) {
        Point p;
        a.check("81. parseSectorNumber", cc.parseSectorNumber(i, p));
        a.checkEqual("82. getSectorNumber", cc.getSectorNumber(p), i);
        a.checkEqual("83. x", p.getX() % 100, 50);
        a.checkEqual("84. y", p.getY() % 100, 50);
    }

    Point p;
    a.check("91. parseSectorNumber", !cc.parseSectorNumber(0, p));
    a.check("92. parseSectorNumber", !cc.parseSectorNumber(-1, p));
    a.check("93. parseSectorNumber", !cc.parseSectorNumber(99, p));
    a.check("94. parseSectorNumber", !cc.parseSectorNumber(500, p));
    a.check("95. parseSectorNumber", !cc.parseSectorNumber(501, p));

    a.check("101. parseSectorNumber", cc.parseSectorNumber("100", p));
    a.checkEqual("102. x", p.getX(), 1050);
    a.checkEqual("103. y", p.getY(), 2950);

    a.check("111. parseSectorNumber", cc.parseSectorNumber("200", p));
    a.checkEqual("112. x", p.getX(), 2050);
    a.checkEqual("113. y", p.getY(), 2950);

    // Some out-of-range values
    a.checkEqual("121. getSectorNumber", cc.getSectorNumber(Point(999,  999)), 0);
    a.checkEqual("122. getSectorNumber", cc.getSectorNumber(Point(1999, 999)), 0);
    a.checkEqual("123. getSectorNumber", cc.getSectorNumber(Point(999,  1999)), 0);
    a.checkEqual("124. getSectorNumber", cc.getSectorNumber(Point(1999, 3001)), 0);

    // Distance
    a.checkEqual("131. getSquaredDistance", cc.getSquaredDistance(Point(1000, 1000), Point(1003, 1004)), 25);

    // Comparison
    a.checkEqual("141. eq", cc == cc, true);
    a.checkEqual("142. ne", cc != cc, false);
    a.checkEqual("143. eq", cc == game::map::Configuration(), true);
    a.checkEqual("144. ne", cc != game::map::Configuration(), false);
}

/** Test image transformations in flat, default map.
    Verifies image transformations for default map. */
AFL_TEST("game.map.Configuration:flat:image", a)
{
    game::map::Configuration cc;

    // Configure map to not-wrapped, standard size
    cc.setConfiguration(cc.Flat, Point(2000, 2000), Point(2000, 2000));
    a.checkEqual("01. getNumRectangularImages", cc.getNumRectangularImages(), 1);
    a.checkEqual("02. getNumPointImages", cc.getNumPointImages(), 1);

    // Point alias
    Point out;
    a.checkEqual("11. getPointAlias", cc.getPointAlias(Point(1500, 1400), out, 0, true), true);
    a.checkEqual("12. out", out, Point(1500, 1400));
    a.checkEqual("13. getPointAlias", cc.getPointAlias(Point(1500, 1400), out, 1, true), false);

    a.checkEqual("21. getSimplePointAlias", cc.getSimplePointAlias(Point(1500, 1400), 0), Point(1500, 1400));
}

/** Test flat, small map.
    Verifies common operations transformations.
    Note that outside points have no sector number.*/
AFL_TEST("game.map.Configuration:flat:small", a)
{
    // ex GameCoordTestSuite::testFlatSmall()
    game::map::Configuration cc;

    // Configure map to not-wrapped, smaller size
    cc.setConfiguration(cc.Flat, Point(2000, 2000), Point(1000, 1000));
    a.check("01. getMinimumCoordinates", cc.getMinimumCoordinates() == Point(1500, 1500));
    a.check("02. getMaximumCoordinates", cc.getMaximumCoordinates() == Point(2500, 2500));
    a.check("03. getCenter", cc.getCenter() == Point(2000, 2000));
    a.check("04. getSize", cc.getSize() == Point(1000, 1000));

    // Normalizing still does not modify points
    a.check("11. getSimpleCanonicalLocation", cc.getSimpleCanonicalLocation(Point(10, 20)) == Point(10, 20));
    a.check("12. getCanonicalLocation", cc.getCanonicalLocation(Point(10, 20)) == Point(10, 20));
    a.check("13. getSimpleNearestAlias", cc.getSimpleNearestAlias(Point(10, 20), Point(2900, 2900)) == Point(10, 20));
    a.check("14. getSimpleNearestAlias", cc.getSimpleNearestAlias(Point(10, 20), Point(1100, 2900)) == Point(10, 20));
    a.check("15. getSimpleNearestAlias", cc.getSimpleNearestAlias(Point(10, 20), Point(2900, 1100)) == Point(10, 20));
    a.check("16. getSimpleNearestAlias", cc.getSimpleNearestAlias(Point(10, 20), Point(1100, 1100)) == Point(10, 20));

    a.check("21. isOnMap", cc.isOnMap(Point(10, 20)));

    // Sector numbers still known for all points on map:
    a.checkEqual("31. getSectorNumber", cc.getSectorNumber(Point(1000, 1000)), 309);
    a.checkEqual("32. getSectorNumber", cc.getSectorNumber(Point(1099, 1099)), 309);
    a.checkEqual("33. getSectorNumber", cc.getSectorNumber(Point(1111, 1000)), 319);
    a.checkEqual("34. getSectorNumber", cc.getSectorNumber(Point(1222, 1000)), 329);
    a.checkEqual("35. getSectorNumber", cc.getSectorNumber(Point(1111, 1111)), 318);

    a.checkEqual("41. getSectorNumber", cc.getSectorNumber(Point(2000, 1000)), 409);
    a.checkEqual("42. getSectorNumber", cc.getSectorNumber(Point(1000, 2000)), 109);
    a.checkEqual("43. getSectorNumber", cc.getSectorNumber(Point(2000, 2000)), 209);

    a.checkEqual("51. getSectorNumber", cc.getSectorNumber(Point(2999, 2999)), 290);

    a.checkEqual("61. getSectorNumber", cc.getSectorNumber(Point(1500, 1000)), 359);
    a.checkEqual("62. getSectorNumber", cc.getSectorNumber(Point(1500, 1100)), 358);
    a.checkEqual("63. getSectorNumber", cc.getSectorNumber(Point(1500, 1200)), 357);
    a.checkEqual("64. getSectorNumber", cc.getSectorNumber(Point(1500, 1300)), 356);
    a.checkEqual("65. getSectorNumber", cc.getSectorNumber(Point(1500, 1400)), 355);
    a.checkEqual("66. getSectorNumber", cc.getSectorNumber(Point(1500, 1500)), 354);
    a.checkEqual("67. getSectorNumber", cc.getSectorNumber(Point(1500, 1600)), 353);
    a.checkEqual("68. getSectorNumber", cc.getSectorNumber(Point(1500, 1700)), 352);
    a.checkEqual("69. getSectorNumber", cc.getSectorNumber(Point(1500, 1800)), 351);

    // Check that parsed sector numbers are centered within their respective sector
    for (int i = 100; i < 500; ++i) {
        Point p;
        a.check("71. parseSectorNumber", cc.parseSectorNumber(i, p));
        a.checkEqual("72. getSectorNumber", cc.getSectorNumber(p), i);
        a.checkEqual("73. x", p.getX() % 100, 50);
        a.checkEqual("74. y", p.getY() % 100, 50);
    }

    Point p;
    a.check("81. parseSectorNumber", !cc.parseSectorNumber(0, p));
    a.check("82. parseSectorNumber", !cc.parseSectorNumber(-1, p));
    a.check("83. parseSectorNumber", !cc.parseSectorNumber(99, p));
    a.check("84. parseSectorNumber", !cc.parseSectorNumber(500, p));
    a.check("85. parseSectorNumber", !cc.parseSectorNumber(501, p));

    // Some out-of-range values
    a.checkEqual("91. getSectorNumber", cc.getSectorNumber(Point(999,  999)), 0);
    a.checkEqual("92. getSectorNumber", cc.getSectorNumber(Point(1999, 999)), 0);
    a.checkEqual("93. getSectorNumber", cc.getSectorNumber(Point(999,  1999)), 0);
    a.checkEqual("94. getSectorNumber", cc.getSectorNumber(Point(1999, 3001)), 0);

    // Comparison to default
    a.checkEqual("101. eq", cc == game::map::Configuration(), false);
    a.checkEqual("102. ne", cc != game::map::Configuration(), true);
}

/** Test nonstandard, small map.
    Verifies common operations transformations.
    Note that sectors are offset. */
AFL_TEST("game.map.Configuration:flat:offset", a)
{
    // ex GameCoordTestSuite::testFlatOffset()
    game::map::Configuration cc;

    // Configure map to not-wrapped, smaller size and not centered at 2000,2000
    cc.setConfiguration(cc.Flat, Point(1750, 2500), Point(1500, 1000));
    a.checkEqual("01. getMinimumCoordinates", cc.getMinimumCoordinates().getX(), 1000);
    a.checkEqual("02. getMinimumCoordinates", cc.getMinimumCoordinates().getY(), 2000);
    a.checkEqual("03. getMaximumCoordinates", cc.getMaximumCoordinates().getX(), 2500);
    a.checkEqual("04. getMaximumCoordinates", cc.getMaximumCoordinates().getY(), 3000);
    a.check("05. getCenter", cc.getCenter() == Point(1750, 2500));
    a.check("06. getSize", cc.getSize() == Point(1500, 1000));

    // Normalizing still does not modify points
    a.check("11. getSimpleCanonicalLocation", cc.getSimpleCanonicalLocation(Point(10, 20)) == Point(10, 20));
    a.check("12. getCanonicalLocation", cc.getCanonicalLocation(Point(10, 20)) == Point(10, 20));
    a.check("13. getSimpleNearestAlias", cc.getSimpleNearestAlias(Point(10, 20), Point(2900, 2900)) == Point(10, 20));
    a.check("14. getSimpleNearestAlias", cc.getSimpleNearestAlias(Point(10, 20), Point(1100, 2900)) == Point(10, 20));
    a.check("15. getSimpleNearestAlias", cc.getSimpleNearestAlias(Point(10, 20), Point(2900, 1100)) == Point(10, 20));
    a.check("16. getSimpleNearestAlias", cc.getSimpleNearestAlias(Point(10, 20), Point(1100, 1100)) == Point(10, 20));

    a.check("21. isOnMap", cc.isOnMap(Point(10, 20)));

    // Sector numbers still known for all points on map, but offset
    // relative to new center at 1750,2500 instead of 2000,2000.
    // Those are now out-of-range:
    a.checkEqual("31. getSectorNumber", cc.getSectorNumber(Point(1000, 1000)), 0);
    a.checkEqual("32. getSectorNumber", cc.getSectorNumber(Point(1099, 1099)), 0);
    a.checkEqual("33. getSectorNumber", cc.getSectorNumber(Point(1111, 1000)), 0);
    a.checkEqual("34. getSectorNumber", cc.getSectorNumber(Point(1222, 1000)), 0);
    a.checkEqual("35. getSectorNumber", cc.getSectorNumber(Point(1111, 1111)), 0);

    a.checkEqual("41. getSectorNumber", cc.getSectorNumber(Point(2000, 1000)), 0);
    a.checkEqual("42. getSectorNumber", cc.getSectorNumber(Point(1000, 2000)), 324);
    a.checkEqual("43. getSectorNumber", cc.getSectorNumber(Point(2000, 2000)), 424);

    a.checkEqual("51. getSectorNumber", cc.getSectorNumber(Point(2999, 2999)), 0);

    a.checkEqual("61. getSectorNumber", cc.getSectorNumber(Point(1500, 1000)), 0);
    a.checkEqual("62. getSectorNumber", cc.getSectorNumber(Point(1500, 1100)), 0);
    a.checkEqual("63. getSectorNumber", cc.getSectorNumber(Point(1500, 1200)), 0);
    a.checkEqual("64. getSectorNumber", cc.getSectorNumber(Point(1500, 1300)), 0);
    a.checkEqual("65. getSectorNumber", cc.getSectorNumber(Point(1500, 1400)), 0);
    a.checkEqual("66. getSectorNumber", cc.getSectorNumber(Point(1500, 1500)), 379);
    a.checkEqual("67. getSectorNumber", cc.getSectorNumber(Point(1500, 1600)), 378);
    a.checkEqual("68. getSectorNumber", cc.getSectorNumber(Point(1500, 1700)), 377);
    a.checkEqual("69. getSectorNumber", cc.getSectorNumber(Point(1500, 1800)), 376);

    a.checkEqual("71. getSectorNumber", cc.getSectorNumber(Point(999,   999)), 0);
    a.checkEqual("72. getSectorNumber", cc.getSectorNumber(Point(1999,  999)), 0);
    a.checkEqual("73. getSectorNumber", cc.getSectorNumber(Point(999,  1999)), 325);
    a.checkEqual("74. getSectorNumber", cc.getSectorNumber(Point(1999, 3001)), 224);

    // Check that parsed sector numbers are centered within their respective sector
    // Sectors are now centered around (xx00,xx50), not (xx00,xx50)
    for (int i = 100; i < 500; ++i) {
        Point p;
        a.check("81. parseSectorNumber", cc.parseSectorNumber(i, p));
        a.checkEqual("82. getSectorNumber", cc.getSectorNumber(p), i);
        a.checkEqual("83. x", p.getX() % 100, 0);
        a.checkEqual("84. y", p.getY() % 100, 50);
    }

    Point p;
    a.check("91. parseSectorNumber", !cc.parseSectorNumber(0, p));
    a.check("92. parseSectorNumber", !cc.parseSectorNumber(-1, p));
    a.check("93. parseSectorNumber", !cc.parseSectorNumber(99, p));
    a.check("94. parseSectorNumber", !cc.parseSectorNumber(500, p));
    a.check("95. parseSectorNumber", !cc.parseSectorNumber(501, p));

    // Comparison to default
    a.checkEqual("101. eq", cc == game::map::Configuration(), false);
    a.checkEqual("102. ne", cc != game::map::Configuration(), true);
}

/** Test image transformations in nonstandard map.
    Verifies image transformations. */
AFL_TEST("game.map.Configuration:flat:offset:image", a)
{
    game::map::Configuration cc;

    // Configure map to not-wrapped, smaller size and not centered at 2000,2000
    cc.setConfiguration(cc.Flat, Point(1750, 2500), Point(1500, 1000));
    a.checkEqual("01. getNumRectangularImages", cc.getNumRectangularImages(), 1);
    a.checkEqual("02. getNumPointImages", cc.getNumPointImages(), 1);

    // Point alias
    Point out;
    a.checkEqual("11. getPointAlias", cc.getPointAlias(Point(1500, 1400), out, 0, true), true);
    a.checkEqual("12. out", out, Point(1500, 1400));
    a.checkEqual("13. getPointAlias", cc.getPointAlias(Point(1500, 1400), out, 1, true), false);

    a.checkEqual("21. getSimplePointAlias", cc.getSimplePointAlias(Point(1500, 1400), 0), Point(1500, 1400));
}

/** Test standard, wrapped map.
    Verifies common operations transformations. */
AFL_TEST("game.map.Configuration:wrapped", a)
{
    // ex GameCoordTestSuite::testWrapped
    game::map::Configuration cc;

    // Configure map to wrapped, standard size
    cc.setConfiguration(cc.Wrapped, Point(2000, 2000), Point(2000, 2000));

    // Normalizing
    a.check("01. getSimpleCanonicalLocation", cc.getSimpleCanonicalLocation(Point(10, 20)) == Point(2010, 2020));
    a.check("02. getCanonicalLocation", cc.getCanonicalLocation(Point(10, 20)) == Point(2010, 2020));
    a.check("03. getSimpleCanonicalLocation", cc.getSimpleCanonicalLocation(Point(3010, 3020)) == Point(1010, 1020));
    a.check("04. getCanonicalLocation", cc.getCanonicalLocation(Point(3010, 3020)) == Point(1010, 1020));
    a.check("05. getSimpleNearestAlias", cc.getSimpleNearestAlias(Point(10, 20), Point(2900, 2900)) == Point(2010, 2020));
    a.check("06. getSimpleNearestAlias", cc.getSimpleNearestAlias(Point(10, 20), Point(1100, 2900)) == Point(2010, 2020));
    a.check("07. getSimpleNearestAlias", cc.getSimpleNearestAlias(Point(10, 20), Point(2900, 1100)) == Point(2010, 2020));
    a.check("08. getSimpleNearestAlias", cc.getSimpleNearestAlias(Point(10, 20), Point(1100, 1100)) == Point(2010, 2020));

    a.check("11. getSimpleNearestAlias", cc.getSimpleNearestAlias(Point(1010, 1020), Point(2900, 2900)) == Point(3010, 3020));
    a.check("12. getSimpleNearestAlias", cc.getSimpleNearestAlias(Point(1010, 1020), Point(1100, 2900)) == Point(1010, 3020));
    a.check("13. getSimpleNearestAlias", cc.getSimpleNearestAlias(Point(1010, 1020), Point(2900, 1100)) == Point(3010, 1020));
    a.check("14. getSimpleNearestAlias", cc.getSimpleNearestAlias(Point(1010, 1020), Point(1100, 1100)) == Point(1010, 1020));

    a.check("21. limitUserLocation", cc.limitUserLocation(Point(10, 20)) == Point(2010, 2020));
    a.check("22. limitUserLocation", cc.limitUserLocation(Point(1000, 2000)) == Point(1000, 2000));
    a.check("23. limitUserLocation", cc.limitUserLocation(Point(3010, 3020)) == Point(1010, 1020));

    a.check("31. isOnMap", !cc.isOnMap(Point(10, 20)));

    // Sector numbers known for all points in [1000,3000), same as in testFlat()
    a.checkEqual("41. getSectorNumber", cc.getSectorNumber(Point(1000, 1000)), 309);
    a.checkEqual("42. getSectorNumber", cc.getSectorNumber(Point(1099, 1099)), 309);
    a.checkEqual("43. getSectorNumber", cc.getSectorNumber(Point(1111, 1000)), 319);
    a.checkEqual("44. getSectorNumber", cc.getSectorNumber(Point(1222, 1000)), 329);
    a.checkEqual("45. getSectorNumber", cc.getSectorNumber(Point(1111, 1111)), 318);

    a.checkEqual("51. getSectorNumber", cc.getSectorNumber(Point(2000, 1000)), 409);
    a.checkEqual("52. getSectorNumber", cc.getSectorNumber(Point(1000, 2000)), 109);
    a.checkEqual("53. getSectorNumber", cc.getSectorNumber(Point(2000, 2000)), 209);

    a.checkEqual("61. getSectorNumber", cc.getSectorNumber(Point(2999, 2999)), 290);

    a.checkEqual("71. getSectorNumber", cc.getSectorNumber(Point(1500, 1000)), 359);
    a.checkEqual("72. getSectorNumber", cc.getSectorNumber(Point(1500, 1100)), 358);
    a.checkEqual("73. getSectorNumber", cc.getSectorNumber(Point(1500, 1200)), 357);
    a.checkEqual("74. getSectorNumber", cc.getSectorNumber(Point(1500, 1300)), 356);
    a.checkEqual("75. getSectorNumber", cc.getSectorNumber(Point(1500, 1400)), 355);
    a.checkEqual("76. getSectorNumber", cc.getSectorNumber(Point(1500, 1500)), 354);
    a.checkEqual("77. getSectorNumber", cc.getSectorNumber(Point(1500, 1600)), 353);
    a.checkEqual("78. getSectorNumber", cc.getSectorNumber(Point(1500, 1700)), 352);
    a.checkEqual("79. getSectorNumber", cc.getSectorNumber(Point(1500, 1800)), 351);

    // Check that parsed sector numbers are centered within their respective sector
    for (int i = 100; i < 500; ++i) {
        Point p;
        a.check("81. parseSectorNumber", cc.parseSectorNumber(i, p));
        a.checkEqual("82. getSectorNumber", cc.getSectorNumber(p), i);
        a.checkEqual("83. x", p.getX() % 100, 50);
        a.checkEqual("84. y", p.getY() % 100, 50);
        a.checkEqual("85. getSimpleCanonicalLocation", p, cc.getSimpleCanonicalLocation(p));
        a.checkEqual("86. getCanonicalLocation", p, cc.getCanonicalLocation(p));
    }

    Point p;
    a.check("91. parseSectorNumber", !cc.parseSectorNumber(0, p));
    a.check("92. parseSectorNumber", !cc.parseSectorNumber(-1, p));
    a.check("93. parseSectorNumber", !cc.parseSectorNumber(99, p));
    a.check("94. parseSectorNumber", !cc.parseSectorNumber(500, p));
    a.check("95. parseSectorNumber", !cc.parseSectorNumber(501, p));

    // Some out-of-range values
    a.checkEqual("101. getSectorNumber", cc.getSectorNumber(Point(999,  999)), 0);
    a.checkEqual("102. getSectorNumber", cc.getSectorNumber(Point(1999, 999)), 0);
    a.checkEqual("103. getSectorNumber", cc.getSectorNumber(Point(999,  1999)), 0);
    a.checkEqual("104. getSectorNumber", cc.getSectorNumber(Point(1999, 3001)), 0);

    // Distance
    a.checkEqual("111. getSquaredDistance", cc.getSquaredDistance(Point(1000, 1000), Point(1003, 1004)), 25);
    a.checkEqual("112. getSquaredDistance", cc.getSquaredDistance(Point(1000, 1000), Point(3003, 3004)), 25);
    a.checkEqual("113. getSquaredDistance", cc.getSquaredDistance(Point(3000, 3000), Point(1003, 1004)), 25);
    a.checkEqual("114. getSquaredDistance", cc.getSquaredDistance(Point(3000, 3000), Point(3003, 3004)), 25);

    // Comparison to default
    a.checkEqual("121. eq", cc == game::map::Configuration(), false);
    a.checkEqual("122. ne", cc != game::map::Configuration(), true);
}

/** Test image transformations in wrapped map. */
AFL_TEST("game.map.Configuration:wrapped:image", a)
{
    // ex GameCoordTestSuite::testWrapped
    game::map::Configuration cc;

    // Configure map to wrapped, standard size
    cc.setConfiguration(cc.Wrapped, Point(2000, 2000), Point(2000, 2000));
    a.checkEqual("01. getNumRectangularImages", cc.getNumRectangularImages(), 9);
    a.checkEqual("02. getNumPointImages", cc.getNumPointImages(), 9);

    // Point alias
    Point out;
    a.checkEqual("11. getPointAlias", cc.getPointAlias(Point(1500, 1500), out, 0, true), true);
    a.checkEqual("12. out", out, Point(1500, 1500));
    a.checkEqual("13. getPointAlias", cc.getPointAlias(Point(1500, 1500), out, 1, true), true);
    a.checkEqual("14. out", out, Point(-500, -500));
    a.checkEqual("15. getPointAlias", cc.getPointAlias(Point(1500, 1500), out, 7, true), true);
    a.checkEqual("16. out", out, Point(1500, 3500));
    a.checkEqual("17. getPointAlias", cc.getPointAlias(Point(1500, 1500), out, 8, true), true);
    a.checkEqual("18. out", out, Point(3500, 3500));

    a.checkEqual("21. getPointAlias", cc.getPointAlias(Point(500, 500), out, 0, true), false);

    // Simple point alias
    a.checkEqual("31. getSimplePointAlias", cc.getSimplePointAlias(Point(1500, 1500), 0), Point(1500, 1500));
    a.checkEqual("32. getSimplePointAlias", cc.getSimplePointAlias(Point(1500, 1500), 1), Point(-500, -500));
    a.checkEqual("33. getSimplePointAlias", cc.getSimplePointAlias(Point(1500, 1500), 7), Point(1500, 3500));
    a.checkEqual("34. getSimplePointAlias", cc.getSimplePointAlias(Point(1500, 1500), 8), Point(3500, 3500));

    // Simple point alias, error cases
    // For out-of-range parameters, getSimplePointAlias returns the original point
    a.checkEqual("41. getSimplePointAlias", cc.getSimplePointAlias(Point( 500,  500), 0),   Point(500, 500));
    a.checkEqual("42. getSimplePointAlias", cc.getSimplePointAlias(Point(1500, 1500), -1),  Point(1500, 1500));
    a.checkEqual("43. getSimplePointAlias", cc.getSimplePointAlias(Point(1500, 1500), 888), Point(1500, 1500));
}

/** Test small, wrapped map.
    Verifies common operations transformations.
    Note out-of-range points. */
AFL_TEST("game.map.Configuration:wrapped:small", a)
{
    // ex GameCoordTestSuite::testWrappedSmall

    game::map::Configuration cc;

    // Configure map to wrapped, small size
    cc.setConfiguration(cc.Wrapped, Point(2000, 2000), Point(1000, 1000));

    // Sector numbers known for all points in [1500,2500), but still numbered as in normal map.
    // Parts are out of range.
    a.checkEqual("01. getSectorNumber", cc.getSectorNumber(Point(1000, 1000)), 0);
    a.checkEqual("02. getSectorNumber", cc.getSectorNumber(Point(1099, 1099)), 0);
    a.checkEqual("03. getSectorNumber", cc.getSectorNumber(Point(1111, 1000)), 0);
    a.checkEqual("04. getSectorNumber", cc.getSectorNumber(Point(1222, 1000)), 0);
    a.checkEqual("05. getSectorNumber", cc.getSectorNumber(Point(1111, 1111)), 0);

    a.checkEqual("11. getSectorNumber", cc.getSectorNumber(Point(2000, 1000)), 0);
    a.checkEqual("12. getSectorNumber", cc.getSectorNumber(Point(1000, 2000)), 0);
    a.checkEqual("13. getSectorNumber", cc.getSectorNumber(Point(2000, 2000)), 209);

    a.checkEqual("21. getSectorNumber", cc.getSectorNumber(Point(2999, 2999)), 0);

    a.checkEqual("31. getSectorNumber", cc.getSectorNumber(Point(1500, 1000)), 0);
    a.checkEqual("32. getSectorNumber", cc.getSectorNumber(Point(1500, 1100)), 0);
    a.checkEqual("33. getSectorNumber", cc.getSectorNumber(Point(1500, 1200)), 0);
    a.checkEqual("34. getSectorNumber", cc.getSectorNumber(Point(1500, 1300)), 0);
    a.checkEqual("35. getSectorNumber", cc.getSectorNumber(Point(1500, 1400)), 0);
    a.checkEqual("36. getSectorNumber", cc.getSectorNumber(Point(1500, 1500)), 354);
    a.checkEqual("37. getSectorNumber", cc.getSectorNumber(Point(1500, 1600)), 353);
    a.checkEqual("38. getSectorNumber", cc.getSectorNumber(Point(1500, 1700)), 352);
    a.checkEqual("39. getSectorNumber", cc.getSectorNumber(Point(1500, 1800)), 351);

    // Comparison to default
    a.checkEqual("41. eq", cc == game::map::Configuration(), false);
    a.checkEqual("42. ne", cc != game::map::Configuration(), true);
}

/** Test circular map.
    Verifies common operations transformations. */
AFL_TEST("game.map.Configuration:circular", a)
{
    game::map::Configuration cc;
    cc.setConfiguration(cc.Circular, Point(2000, 2000), Point(1000, 1000));

    // Test isOnMap:
    a.checkEqual("01. isOnMap", cc.isOnMap(Point(2000, 2000)), true);     // clearly inside
    a.checkEqual("02. isOnMap", cc.isOnMap(Point(3000, 2000)), true);     // at edge
    a.checkEqual("03. isOnMap", cc.isOnMap(Point(2000, 3000)), true);     // at edge
    a.checkEqual("04. isOnMap", cc.isOnMap(Point(3000, 3000)), false);    // clearly outside
    a.checkEqual("05. isOnMap", cc.isOnMap(Point(2001, 3000)), false);    // barely outside

    // Test getCanonicalLocation:
    a.checkEqual("11. getCanonicalLocation", cc.getCanonicalLocation(Point(2000, 2000)), Point(2000, 2000));
    a.checkEqual("12. getCanonicalLocation", cc.getCanonicalLocation(Point(3000, 2000)), Point(3000, 2000));
    a.checkEqual("13. getCanonicalLocation", cc.getCanonicalLocation(Point(2000, 3000)), Point(2000, 3000));
    a.checkEqual("14. getCanonicalLocation", cc.getCanonicalLocation(Point(3000, 3000)), Point(1586, 1586));
    a.checkEqual("15. getCanonicalLocation", cc.getCanonicalLocation(Point(2001, 3000)), Point(1999, 1000));

    // Some more points (cross-checked against pwrap)
    a.checkEqual("21. getCanonicalLocation", cc.getCanonicalLocation(Point(2100, 3000)), Point(1901, 1010));
    a.checkEqual("22. getCanonicalLocation", cc.getCanonicalLocation(Point(2100, 3100)), Point(1919, 1108));
    a.checkEqual("23. getCanonicalLocation", cc.getCanonicalLocation(Point(2102, 3100)), Point(1917, 1109));
    a.checkEqual("24. getCanonicalLocation", cc.getCanonicalLocation(Point(1300, 1200)), Point(2617, 2705));
    a.checkEqual("25. getCanonicalLocation", cc.getCanonicalLocation(Point(3027, 2286)), Point(1100, 1749));

    // Comparison to default
    a.checkEqual("31. eq", cc == game::map::Configuration(), false);
    a.checkEqual("32. ne", cc != game::map::Configuration(), true);
}

/** Test image transformations in circular map.
    Focus on inside-out transformation (getPointAlias(1)). */
AFL_TEST("game.map.Configuration:circular:image", a)
{
    game::map::Configuration cc;
    cc.setConfiguration(cc.Circular, Point(2000, 2000), Point(1000, 1000));
    a.checkEqual("01. getNumRectangularImages", cc.getNumRectangularImages(), 1);
    a.checkEqual("02. getNumPointImages", cc.getNumPointImages(), 2);

    // Other circular config values have sensible defaults:
    a.check("11. getCircularPrecision", cc.getCircularPrecision() > 0);
    a.check("12. getCircularExcess", cc.getCircularExcess() > 500);

    // Test getPointAlias:
    Point result;

    // - Center cannot be mapped outside ("too far inside" case)
    a.checkEqual("21. getPointAlias", cc.getPointAlias(Point(2000, 2000), result, 1, true), false);
    a.checkEqual("22. getPointAlias", cc.getPointAlias(Point(2000, 2000), result, 1, false), false);

    // - Edge cannot be mapped outside
    a.checkEqual("31. getPointAlias", cc.getPointAlias(Point(3000, 2000), result, 1, true), false);
    //   Inexact mapping WILL map it!
    a.checkEqual("32. getPointAlias", cc.getPointAlias(Point(3000, 2000), result, 1, false), true);
    a.checkEqual("33. result", result, Point(1000, 2000));

    // - Barely outside cannot be mapped outside because its inverse is outside again
    a.checkEqual("41. getPointAlias", cc.getPointAlias(Point(1999, 1000), result, 1, true), false);
    a.checkEqual("42. getPointAlias", cc.getPointAlias(Point(1999, 1000), result, 1, false), false);

    // - More points that successfully map:
    a.checkEqual("51. getPointAlias", cc.getPointAlias(Point(1901, 1010), result, 1, true), true);
    a.checkEqual("52. result", result, Point(2100, 3000));
    a.checkEqual("53. getPointAlias", cc.getPointAlias(Point(1901, 1010), result, 1, false), true);
    a.checkEqual("54. result", result, Point(2100, 3000));

    a.checkEqual("61. getPointAlias", cc.getPointAlias(Point(1919, 1108), result, 1, true), true);
    a.checkEqual("62. result", result, Point(2100, 3100));
    a.checkEqual("63. getPointAlias", cc.getPointAlias(Point(1919, 1108), result, 1, false), true);
    a.checkEqual("64. result", result, Point(2100, 3100));

    a.checkEqual("71. getPointAlias", cc.getPointAlias(Point(1917, 1109), result, 1, true), true);
    a.checkEqual("72. result", result, Point(2103, 3100));  // note different result than tried in forward mapping above!

    a.checkEqual("81. getPointAlias", cc.getPointAlias(Point(2617, 2705), result, 1, true), true);
    a.checkEqual("82. result", result, Point(1300, 1200));

    // This is a point where we need to search for the actual match. Inexact mapping yields a different point.
    a.checkEqual("91. getPointAlias", cc.getPointAlias(Point(1100, 1749), result, 1, true), true);
    a.checkEqual("92. result", result, Point(3027, 2286));
    a.checkEqual("93. getPointAlias", cc.getPointAlias(Point(1100, 1749), result, 1, false), true);
    a.checkEqual("94. result", result, Point(3026, 2286));

    // Simple point alias: Circular has no simple alias
    a.checkEqual("101. getSimplePointAlias", cc.getSimplePointAlias(Point(2000, 2000), 0), Point(2000, 2000));
    a.checkEqual("102. getSimplePointAlias", cc.getSimplePointAlias(Point(2000, 2000), 1), Point(2000, 2000));
    a.checkEqual("103. getSimplePointAlias", cc.getSimplePointAlias(Point(2000, 2000), 10000), Point(2000, 2000));

    a.checkEqual("111. getSimplePointAlias", cc.getSimplePointAlias(Point(3000, 3000), 0), Point(3000, 3000));
}

/** Test initialisation from default configuration. */
AFL_TEST("game.map.Configuration:initFromConfiguration:default", a)
{
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    Ref<UserConfiguration> rpref = UserConfiguration::create();
    UserConfiguration& pref = *rpref;

    game::map::Configuration testee;
    testee.initFromConfiguration(config, pref);

    a.checkEqual("01. isSetFromHostConfiguration", testee.isSetFromHostConfiguration(), false);
    a.checkEqual("02. getMode", testee.getMode(), game::map::Configuration::Flat);
    a.checkEqual("03. getCenter", testee.getCenter(), Point(2000, 2000));
    a.checkEqual("04. getSize", testee.getSize(), Point(2000, 2000));
}

/** Test initialisation from wrapped map configuration. */
AFL_TEST("game.map.Configuration:initFromConfiguration:wrap", a)
{
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    Ref<UserConfiguration> rpref = UserConfiguration::create();
    UserConfiguration& pref = *rpref;
    config.setOption("AllowWraparoundMap", "Yes", ConfigurationOption::Game);
    config.setOption("WraparoundRectangle", "1000,1010,3000,3020", ConfigurationOption::Game);
    a.checkEqual("01. AllowWraparoundMap", config[HostConfiguration::AllowWraparoundMap](), 1);
    a.checkEqual("02. WraparoundRectangle", config[HostConfiguration::WraparoundRectangle](3), 3000);

    game::map::Configuration testee;
    testee.initFromConfiguration(config, pref);

    a.checkEqual("11. isSetFromHostConfiguration", testee.isSetFromHostConfiguration(), true);
    a.checkEqual("12. getMode", testee.getMode(), game::map::Configuration::Wrapped);
    a.checkEqual("13. getCenter", testee.getCenter(), Point(2000, 2015));
    a.checkEqual("14. getSize", testee.getSize(), Point(2000, 2010));
}

/** Test initialisation from invalid wrapped map configuration. */
AFL_TEST("game.map.Configuration:initFromConfiguration:bad", a)
{
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    Ref<UserConfiguration> rpref = UserConfiguration::create();
    UserConfiguration& pref = *rpref;
    config.setOption("AllowWraparoundMap", "Yes", ConfigurationOption::Game);
    config.setOption("WraparoundRectangle", "1000,1010,1020,1030", ConfigurationOption::Game);

    game::map::Configuration testee;
    testee.initFromConfiguration(config, pref);

    a.checkEqual("01. isSetFromHostConfiguration", testee.isSetFromHostConfiguration(), false);
    a.checkEqual("02. getMode", testee.getMode(), game::map::Configuration::Wrapped);
    a.checkEqual("03. getCenter", testee.getCenter(), Point(1010, 1020));
    a.checkEqual("04. getSize", testee.getSize(), Point(2000, 2000));
}

/** Test saveToConfiguration.
    Saving a default configuration should not set any option in UserConfiguration. */
AFL_TEST("game.map.Configuration:saveToConfiguration:default", a)
{
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    Ref<UserConfiguration> rpref = UserConfiguration::create();
    UserConfiguration& pref = *rpref;

    game::map::Configuration testee;
    testee.saveToConfiguration(pref, config);

    afl::base::Ref<UserConfiguration::Enumerator_t> e(pref.getOptions());
    UserConfiguration::OptionInfo_t info;
    while (e->getNextElement(info)) {
        a.checkNonNull("01. option", info.second);
        a.checkEqual("02. source", info.second->getSource(), ConfigurationOption::Default);
    }
}

/** Test saveToConfiguration, wrapped map.
    This should produce a single Chart.Geo.Mode entry because other values are standard. */
AFL_TEST("game.map.Configuration:saveToConfiguration:wrap", a)
{
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    Ref<UserConfiguration> rpref = UserConfiguration::create();
    UserConfiguration& pref = *rpref;

    game::map::Configuration testee;
    testee.setConfiguration(testee.Wrapped, Point(2000, 2000), Point(2000, 2000));
    testee.saveToConfiguration(pref, config);

    afl::base::Ref<UserConfiguration::Enumerator_t> e(pref.getOptions());
    UserConfiguration::OptionInfo_t info;
    while (e->getNextElement(info)) {
        a.checkNonNull("01. option", info.second);
        if (info.first == "Chart.Geo.Mode") {
            a.checkEqual("02. geo source", info.second->getSource(), ConfigurationOption::Game);
            a.checkEqual("03. geo value", info.second->toString(), "wrapped");
        } else {
            a.checkEqual("04. source", info.second->getSource(), ConfigurationOption::Default);
        }
    }
}

/** Test saveToConfiguration, full set.
    Configure some more values to force other values to be generated. */
AFL_TEST("game.map.Configuration:saveToConfiguration:full", a)
{
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    Ref<UserConfiguration> rpref = UserConfiguration::create();
    UserConfiguration& pref = *rpref;

    game::map::Configuration testee;
    testee.setConfiguration(testee.Wrapped, Point(1800, 1900), Point(2000, 2100));
    testee.setCircularExcess(200);
    testee.setCircularPrecision(7);
    testee.saveToConfiguration(pref, config);

    ConfigurationOption* opt = pref.getOptionByName("Chart.Geo.Mode");
    a.check("01. opt", opt);
    a.checkEqual("02. getSource", opt->getSource(), ConfigurationOption::Game);
    a.checkEqual("03. toString", opt->toString(), "wrapped");

    opt = pref.getOptionByName("Chart.Geo.Center");
    a.check("11. opt", opt);
    a.checkEqual("12. getSource", opt->getSource(), ConfigurationOption::Game);
    a.checkEqual("13. toString", opt->toString(), "1800,1900");

    opt = pref.getOptionByName("Chart.Geo.Size");
    a.check("21. opt", opt);
    a.checkEqual("22. getSource", opt->getSource(), ConfigurationOption::Game);
    a.checkEqual("23. toString", opt->toString(), "2000,2100");

    opt = pref.getOptionByName("Chart.Circle.Precision");
    a.check("31. opt", opt);
    // a.checkEqual("32. getSource", opt->getSource(), ConfigurationOption::Game); - no, this is a user option
    a.checkEqual("33. toString", opt->toString(), "7");

    opt = pref.getOptionByName("Chart.Circle.Outside");
    a.check("41. opt", opt);
    a.checkEqual("42. getSource", opt->getSource(), ConfigurationOption::Game);
    a.checkEqual("43. toString", opt->toString(), "200");
}

/** Test saveToConfiguration.
    Saving a default configuration should not set any option in UserConfiguration.
    However, an option that was previously set in Game scope remains there. */
AFL_TEST("game.map.Configuration:saveToConfiguration:preserve-source", a)
{
    Ref<UserConfiguration> rpref = UserConfiguration::create();
    UserConfiguration& pref = *rpref;
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    pref.setOption("Chart.Geo.Mode", "flat", ConfigurationOption::Game);

    game::map::Configuration testee;
    testee.setConfiguration(testee.Flat, Point(2000, 2000), Point(2000, 2000));
    testee.saveToConfiguration(pref, config);

    ConfigurationOption* opt = pref.getOptionByName("Chart.Geo.Mode");
    a.check("01. opt", opt);
    a.checkEqual("02. getSource", opt->getSource(), ConfigurationOption::Game);
    a.checkEqual("03. toString", opt->toString(), "flat");
}

/** Test saveToConfiguration.
    Saving a default configuration should create a Chart.Geo.Mode if game has AllowWraparoundMap=1 */
AFL_TEST("game.map.Configuration:saveToConfiguration:wrap-default", a)
{
    Ref<UserConfiguration> rpref = UserConfiguration::create();
    UserConfiguration& pref = *rpref;
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    config[HostConfiguration::AllowWraparoundMap].set(1);

    game::map::Configuration testee;
    testee.saveToConfiguration(pref, config);

    ConfigurationOption* opt = pref.getOptionByName("Chart.Geo.Mode");
    a.check("01. opt", opt);
    a.checkEqual("02. getSource", opt->getSource(), ConfigurationOption::Game);
    a.checkEqual("03. toString", opt->toString(), "flat");
}
