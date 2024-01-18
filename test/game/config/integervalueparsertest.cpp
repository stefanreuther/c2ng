/**
  *  \file test/game/config/integervalueparsertest.cpp
  *  \brief Test for game::config::IntegerValueParser
  */

#include "game/config/integervalueparser.hpp"

#include "afl/test/testrunner.hpp"
#include <stdexcept>

AFL_TEST("game.config.IntegerValueParser", a)
{
    // ex UtilConfTestSuite::testValueIntParser
    game::config::IntegerValueParser ivp;

    // Some random values
    a.checkEqual("01. parse", ivp.parse("0"), 0);
    a.checkEqual("02. parse", ivp.parse("1"), 1);
    a.checkEqual("03. parse", ivp.parse("65535"), 65535);
    a.checkEqual("04. parse", ivp.parse("65536"), 65536);
    a.checkEqual("05. parse", ivp.parse("2147483647"), 2147483647);
    a.checkEqual("06. parse", ivp.parse("-1"), -1);
    a.checkEqual("07. parse", ivp.parse("-2147483648"), int32_t(-2147483648U));

    // Spacing etc.
    a.checkEqual("11. parse", ivp.parse(" 42"), 42);
    a.checkEqual("12. parse", ivp.parse(" 42      "), 42);
    a.checkEqual("13. parse", ivp.parse("42        "), 42);

    // Wrong stuff
    AFL_CHECK_THROWS(a("21. parse"), ivp.parse("x"), std::range_error);
    AFL_CHECK_THROWS(a("22. parse"), ivp.parse("x42"), std::range_error);
    // no longer an exception, as we want to parse things like '100%':
    // AFL_CHECK_THROWS(a("23. parse"), ivp.parse("42x"), std::range_error);
    a.checkEqual("24. parse", ivp.parse("42x"), 42);
    a.checkEqual("25. parse", ivp.parse("100%"), 100);

    // Reverse conversion
    a.checkEqual("31. toString", ivp.toString(0), "0");
    a.checkEqual("32. toString", ivp.toString(1), "1");
    a.checkEqual("33. toString", ivp.toString(65535), "65535");
    a.checkEqual("34. toString", ivp.toString(65536), "65536");
    a.checkEqual("35. toString", ivp.toString(2147483647), "2147483647");
    a.checkEqual("36. toString", ivp.toString(-1), "-1");
    a.checkEqual("37. toString", ivp.toString(-2147483648U), "-2147483648");
}
