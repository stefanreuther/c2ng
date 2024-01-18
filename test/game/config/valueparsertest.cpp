/**
  *  \file test/game/config/valueparsertest.cpp
  *  \brief Test for game::config::ValueParser
  */

#include "game/config/valueparser.hpp"

#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("game.config.ValueParser", a)
{
    class MyParser : public game::config::ValueParser {
     public:
        virtual int32_t parse(String_t value) const
            {
                int32_t result = 0;
                afl::string::strToInteger(value, result);
                return result;
            }
        virtual String_t toString(int32_t value) const
            {
                return afl::string::Format("%d", value);
            }

    };
    MyParser p;

    // Sanity check
    a.checkEqual("01. parse", p.parse("42"), 42);
    a.checkEqual("02. toString", p.toString(42), "42");

    // Array
    static const int32_t aa[3] = { 42, 0, 99 };
    a.checkEqual("11. toStringArray", p.toStringArray(aa), "42,0,99");

    int32_t b[5];
    p.parseArray("1,2,3", b);
    a.checkEqual("21. result", b[0], 1);
    a.checkEqual("22. result", b[1], 2);
    a.checkEqual("23. result", b[2], 3);
    a.checkEqual("24. result", b[3], 3);
    a.checkEqual("25. result", b[4], 3);

    p.parseArray("4,5,6,7,8,9,10", b);
    a.checkEqual("31. result", b[0], 4);
    a.checkEqual("32. result", b[1], 5);
    a.checkEqual("33. result", b[2], 6);
    a.checkEqual("34. result", b[3], 7);
    a.checkEqual("35. result", b[4], 8);
}
