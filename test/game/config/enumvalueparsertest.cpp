/**
  *  \file test/game/config/enumvalueparsertest.cpp
  *  \brief Test for game::config::EnumValueParser
  */

#include "game/config/enumvalueparser.hpp"

#include "afl/test/testrunner.hpp"
#include <stdexcept>

/** Simple test. */
AFL_TEST("game.config.EnumValueParser:1", a)
{
    game::config::EnumValueParser t("one,two,three");

    a.checkEqual("01. parse", t.parse("one"), 0);
    a.checkEqual("02. parse", t.parse("two"), 1);
    a.checkEqual("03. parse", t.parse("three"), 2);

    a.checkEqual("11. parse", t.parse("ONE"), 0);
    a.checkEqual("12. parse", t.parse("TWO"), 1);
    a.checkEqual("13. parse", t.parse("THREE"), 2);

    AFL_CHECK_THROWS(a("21. parse"), t.parse(""), std::exception);
    AFL_CHECK_THROWS(a("22. parse"), t.parse("on"), std::exception);
    AFL_CHECK_THROWS(a("23. parse"), t.parse("ones"), std::exception);
    AFL_CHECK_THROWS(a("24. parse"), t.parse("four"), std::exception);

    a.checkEqual("31. toString", t.toString(0), "one");
    a.checkEqual("32. toString", t.toString(1), "two");
    a.checkEqual("33. toString", t.toString(2), "three");
    a.checkEqual("34. toString", t.toString(3), "3");
    a.checkEqual("35. toString", t.toString(3000), "3000");
    a.checkEqual("36. toString", t.toString(2000000000), "2000000000");
    a.checkEqual("37. toString", t.toString(-1), "-1");

    a.checkEqual("41. parse", t.parse("3000"), 3000);
    a.checkEqual("42. parse", t.parse("3"), 3);
}

/** Another test. */
AFL_TEST("game.config.EnumValueParser:2", a)
{
    game::config::EnumValueParser t("One,Two,Three");

    a.checkEqual("01. parse", t.parse("one"), 0);
    a.checkEqual("02. parse", t.parse("two"), 1);
    a.checkEqual("03. parse", t.parse("three"), 2);

    a.checkEqual("11. parse", t.parse("ONE"), 0);
    a.checkEqual("12. parse", t.parse("TWO"), 1);
    a.checkEqual("13. parse", t.parse("THREE"), 2);

    AFL_CHECK_THROWS(a("21. parse"), t.parse(""), std::exception);

    a.checkEqual("31. toString", t.toString(0), "One");
    a.checkEqual("32. toString", t.toString(1), "Two");
    a.checkEqual("33. toString", t.toString(2), "Three");
    a.checkEqual("34. toString", t.toString(3), "3");
}
