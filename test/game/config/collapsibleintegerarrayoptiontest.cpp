/**
  *  \file test/game/config/collapsibleintegerarrayoptiontest.cpp
  *  \brief Test for game::config::CollapsibleIntegerArrayOption
  */

#include "game/config/collapsibleintegerarrayoption.hpp"

#include "afl/test/testrunner.hpp"
#include "game/config/integervalueparser.hpp"

AFL_TEST("game.config.CollapsibleIntegerArrayOption", a)
{
    game::config::IntegerValueParser p;
    game::config::CollapsibleIntegerArrayOption<4> testee(p);

    a.checkEqual("01. index", testee(1), 0);
    a.check("02. isAllTheSame", testee.isAllTheSame());
    a.checkEqual("03. toString", testee.toString(), "0");

    testee.set("1,2,3,4");
    a.checkEqual("11. toString", testee.toString(), "1,2,3,4");
    a.checkEqual("12. index", testee(1), 1);
    a.checkEqual("13. index", testee(2), 2);
    a.checkEqual("14. index", testee(3), 3);
    a.checkEqual("15. index", testee(4), 4);

    testee.set(2, 3);
    testee.set(4, 3);
    a.checkEqual("21. toString", testee.toString(), "1,3,3,3");
    a.checkEqual("22. index", testee(1), 1);
    a.checkEqual("23. index", testee(2), 3);
    a.checkEqual("24. index", testee(3), 3);
    a.checkEqual("25. index", testee(4), 3);

    testee.set(1, 3);
    a.checkEqual("31. toString", testee.toString(), "3");
    a.checkEqual("32. index", testee(1), 3);
    a.checkEqual("33. index", testee(2), 3);
    a.checkEqual("34. index", testee(3), 3);
    a.checkEqual("35. index", testee(4), 3);

    testee.set(9);
    a.checkEqual("41. toString", testee.toString(), "9");
    a.checkEqual("42. index", testee(1), 9);
    a.checkEqual("43. index", testee(2), 9);
    a.checkEqual("44. index", testee(3), 9);
    a.checkEqual("45. index", testee(4), 9);

    testee.set("4");
    a.checkEqual("51. toString", testee.toString(), "4");
    a.checkEqual("52. index", testee(1), 4);
    a.checkEqual("53. index", testee(2), 4);
    a.checkEqual("54. index", testee(3), 4);
    a.checkEqual("55. index", testee(4), 4);
}
