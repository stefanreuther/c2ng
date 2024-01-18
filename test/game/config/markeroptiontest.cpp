/**
  *  \file test/game/config/markeroptiontest.cpp
  *  \brief Test for game::config::MarkerOption
  */

#include "game/config/markeroption.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("game.config.MarkerOption", a)
{
    // Check initial values
    game::config::MarkerOption testee(7, 3);
    a.checkEqual("01. markerKind", testee().markerKind, 7);
    a.checkEqual("02. color", testee().color, 3);
    a.checkEqual("03. note", testee().note, "");
    a.checkEqual("04. toString", testee.toString(), "7,3,");

    // Check typed setters
    testee.set(game::config::MarkerOption::Data(1, 2, "x"));
    a.checkEqual("11. markerKind", testee().markerKind, 1);
    a.checkEqual("12. color", testee().color, 2);
    a.checkEqual("13. note", testee().note, "x");
    a.checkEqual("14. toString", testee.toString(), "1,2,x");

    // Check public setter
    testee.set("4, 5, hi");
    a.checkEqual("21. markerKind", testee().markerKind, 4);
    a.checkEqual("22. color", testee().color, 5);
    a.checkEqual("23. note", testee().note, "hi");
    a.checkEqual("24. toString", testee.toString(), "4,5,hi");

    // Check invalid setter -> does not change anything
    testee.set("10000,4,x");
    a.checkEqual("31. value after set", testee().markerKind, 4);
    testee.set("1,10000,x");
    a.checkEqual("32. value after set", testee().markerKind, 4);
    testee.set("a,b,c");
    a.checkEqual("33. value after set", testee().markerKind, 4);
    testee.set("1");
    a.checkEqual("34. value after set", testee().markerKind, 4);
}
