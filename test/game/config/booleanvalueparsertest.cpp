/**
  *  \file test/game/config/booleanvalueparsertest.cpp
  *  \brief Test for game::config::BooleanValueParser
  */

#include "game/config/booleanvalueparser.hpp"

#include "afl/test/testrunner.hpp"

AFL_TEST("game.config.BooleanValueParser", a)
{
    // ex UtilConfTestSuite::testValueBoolParser
    game::config::BooleanValueParser bvp;

    // Test some values
    a.checkEqual("01. parse", bvp.parse("no"), 0);
    a.checkEqual("02. parse", bvp.parse("yes"), 1);
    a.checkEqual("03. parse", bvp.parse("allies"), 2);
    a.checkEqual("04. parse", bvp.parse("external"), 3);
    a.checkEqual("05. parse", bvp.parse("true"), 1);
    a.checkEqual("06. parse", bvp.parse("false"), 0);

    // Short forms
    a.checkEqual("11. parse", bvp.parse("n"), 0);
    a.checkEqual("12. parse", bvp.parse("y"), 1);
    a.checkEqual("13. parse", bvp.parse("a"), 2);
    a.checkEqual("14. parse", bvp.parse("e"), 3);
    a.checkEqual("15. parse", bvp.parse("t"), 1);
    a.checkEqual("16. parse", bvp.parse("f"), 0);

    // Case
    a.checkEqual("21. parse", bvp.parse("NO"), 0);
    a.checkEqual("22. parse", bvp.parse("YES"), 1);
    a.checkEqual("23. parse", bvp.parse("ALL"), 2);
    a.checkEqual("24. parse", bvp.parse("EXT"), 3);
    a.checkEqual("25. parse", bvp.parse("TRU"), 1);
    a.checkEqual("26. parse", bvp.parse("FAL"), 0);

    // Numeric
    a.checkEqual("31. parse", bvp.parse("0"), 0);
    a.checkEqual("32. parse", bvp.parse("1"), 1);
    a.checkEqual("33. parse", bvp.parse("10"), 10);

    // Error, treated as 1
    a.checkEqual("41. parse", bvp.parse("Whateverest"), 1);

    // Reverse conversion
    a.checkEqual("51. toString", bvp.toString(0), "No");
    a.checkEqual("52. toString", bvp.toString(1), "Yes");
    a.checkEqual("53. toString", bvp.toString(2), "Allies");
    a.checkEqual("54. toString", bvp.toString(3), "External");
}
