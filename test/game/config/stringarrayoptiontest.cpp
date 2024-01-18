/**
  *  \file test/game/config/stringarrayoptiontest.cpp
  *  \brief Test for game::config::StringArrayOption
  */

#include "game/config/stringarrayoption.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("game.config.StringArrayOption", a)
{
    game::config::StringArrayOption opt(1, 10);
    a.checkEqual("01. toString", opt.toString(), "");
    a.checkEqual("02. getFirstIndex", opt.getFirstIndex(), 1);
    a.checkEqual("03. getNumSlots", opt.getNumSlots(), 10);
    a.checkEqual("04. index", opt(1), "");

    opt.set("a, b, c");
    a.checkEqual("11. toString", opt.toString(), "a,b,c");
    a.checkEqual("12. index", opt(0), "");
    a.checkEqual("13. index", opt(1), "a");
    a.checkEqual("14. index", opt(2), "b");
    a.checkEqual("15. index", opt(3), "c");
    a.checkEqual("16. index", opt(4), "");

    opt.set(5, "x");
    a.checkEqual("21. toString", opt.toString(), "a,b,c,,x");
    a.checkEqual("22. index", opt(3), "c");
    a.checkEqual("23. index", opt(4), "");
    a.checkEqual("24. index", opt(5), "x");
    a.checkEqual("25. index", opt(6), "");
}
