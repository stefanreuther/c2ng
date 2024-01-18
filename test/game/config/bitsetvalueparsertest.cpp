/**
  *  \file test/game/config/bitsetvalueparsertest.cpp
  *  \brief Test for game::config::BitsetValueParser
  */

#include "game/config/bitsetvalueparser.hpp"

#include "afl/test/testrunner.hpp"

AFL_TEST("game.config.BitsetValueParser", a)
{
    // ex UtilConfTestSuite::testValueBitsetParser
    game::config::BitsetValueParser bvp("one,two,three,four,five");

    // Single values
    a.checkEqual("01. parse", bvp.parse(""), 0);
    a.checkEqual("02. parse", bvp.parse("one"), 1);
    a.checkEqual("03. parse", bvp.parse("two"), 2);
    a.checkEqual("04. parse", bvp.parse("three"), 4);
    a.checkEqual("05. parse", bvp.parse("four"), 8);
    a.checkEqual("06. parse", bvp.parse("five"), 16);

    // Multiple values
    a.checkEqual("11. parse", bvp.parse("one,two"), 3);
    a.checkEqual("12. parse", bvp.parse("two,three,four"), 14);
    a.checkEqual("13. parse", bvp.parse("five,three"), 20);
    a.checkEqual("14. parse", bvp.parse("one,one,one,one"), 1);
    a.checkEqual("15. parse", bvp.parse("five,,,,,,,,"), 16);

    // Numerical values
    a.checkEqual("21. parse", bvp.parse("one,120"), 121);
    a.checkEqual("22. parse", bvp.parse("one,121"), 121);
    a.checkEqual("23. parse", bvp.parse("121,one"), 121);

    // Reverse conversion
    a.checkEqual("31. toString", bvp.toString(0), "");
    a.checkEqual("32. toString", bvp.toString(1), "one");
    a.checkEqual("33. toString", bvp.toString(2), "two");
    a.checkEqual("34. toString", bvp.toString(3), "one,two");
    a.checkEqual("35. toString", bvp.toString(4), "three");
    a.checkEqual("36. toString", bvp.toString(32), "");
}
