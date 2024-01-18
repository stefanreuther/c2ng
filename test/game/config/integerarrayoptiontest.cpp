/**
  *  \file test/game/config/integerarrayoptiontest.cpp
  *  \brief Test for game::config::IntegerArrayOption
  */

#include "game/config/integerarrayoption.hpp"

#include "afl/test/testrunner.hpp"
#include "game/config/integervalueparser.hpp"

AFL_TEST("game.config.IntegerArrayOption", a)
{
    game::config::IntegerValueParser vp;
    game::config::IntegerArrayOption<5> one(vp);

    // Verify initial state
    a.checkEqual("01. size", one.getArray().size(), 5U);
    a.checkEqual("02. at", *one.getArray().at(0), 0);
    a.checkEqual("03. at", *one.getArray().at(4), 0);
    a.checkEqual("04. toString", one.toString(), "0,0,0,0,0");
    a.checkEqual("05. parser", &one.parser(), &vp);
    a.checkEqual("06. index", one(1), 0);
    a.check("07. isAllTheSame", one.isAllTheSame());

    // Modify
    one.set("3,     1, 4, 1, 5");
    a.checkEqual("11. index", one(1), 3);
    a.checkEqual("12. index", one(2), 1);
    a.checkEqual("13. index", one(3), 4);
    a.checkEqual("14. index", one(4), 1);
    a.checkEqual("15. index", one(5), 5);

    a.checkEqual("21. index", one(0), 5);
    a.checkEqual("22. index", one(6), 5);
    a.checkEqual("23. index", one(1000), 5);
    a.checkEqual("24. index", one(-1), 5);

    a.checkEqual("31. toString", one.toString(), "3,1,4,1,5");

    // Another one
    static const int32_t init[] = { 3, 2, 1, 6, 8 };
    game::config::IntegerArrayOption<5> two(vp, init);
    a.checkEqual("41. toString", two.toString(), "3,2,1,6,8");

    two.copyFrom(one);
    a.checkEqual("51. toString", one.toString(), "3,1,4,1,5");
}
