/**
  *  \file u/t_game_config_valueparser.cpp
  *  \brief Test for game::config::ValueParser
  */

#include "game/config/valueparser.hpp"

#include "t_game_config.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"

void
TestGameConfigValueParser::testArray()
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
    TS_ASSERT_EQUALS(p.parse("42"), 42);
    TS_ASSERT_EQUALS(p.toString(42), "42");

    // Array
    static const int32_t a[3] = { 42, 0, 99 };
    TS_ASSERT_EQUALS(p.toStringArray(a), "42,0,99");

    int32_t b[5];
    p.parseArray("1,2,3", b);
    TS_ASSERT_EQUALS(b[0], 1);
    TS_ASSERT_EQUALS(b[1], 2);
    TS_ASSERT_EQUALS(b[2], 3);
    TS_ASSERT_EQUALS(b[3], 3);
    TS_ASSERT_EQUALS(b[4], 3);

    p.parseArray("4,5,6,7,8,9,10", b);
    TS_ASSERT_EQUALS(b[0], 4);
    TS_ASSERT_EQUALS(b[1], 5);
    TS_ASSERT_EQUALS(b[2], 6);
    TS_ASSERT_EQUALS(b[3], 7);
    TS_ASSERT_EQUALS(b[4], 8);
}

