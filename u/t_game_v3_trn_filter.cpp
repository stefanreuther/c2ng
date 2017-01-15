/**
  *  \file u/t_game_v3_trn_filter.cpp
  *  \brief Test for game::v3::trn::Filter
  */

#include "game/v3/trn/filter.hpp"

#include "t_game_v3_trn.hpp"
#include "afl/charset/utf8charset.hpp"
#include "game/timestamp.hpp"
#include "game/v3/turnfile.hpp"
#include "game/v3/trn/parseexception.hpp"

/** Interface test. */
void
TestGameV3TrnFilter::testInterface()
{
    class Tester : public game::v3::trn::Filter {
     public:
        virtual bool accept(const game::v3::TurnFile& /*trn*/, size_t /*index*/) const
            { return false; }
    };
    Tester t;
}

/** Test the parser. */
void
TestGameV3TrnFilter::testParser()
{
    // Create a dummy turn
    afl::charset::Utf8Charset cs;
    game::v3::TurnFile trn(cs, 1, game::Timestamp());

    const uint8_t DUMMY[] = { 'a', 'b', 'c', };
    trn.addCommand(game::v3::tcm_ShipChangeSpeed,    9, DUMMY);   // 0
    trn.addCommand(game::v3::tcm_ShipChangeSpeed,   10, DUMMY);   // 1
    trn.addCommand(game::v3::tcm_PlanetChangeFc,    10, DUMMY);   // 1
    trn.addCommand(game::v3::tcm_PlanetChangeMines, 10, DUMMY);   // 1

    // Test operands (parseSingle)
    afl::base::Deleter del;
    TS_ASSERT( game::v3::trn::Filter::parse("true", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse("false", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse("  true  ", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse("TRUE", del).accept(trn, 0));

    TS_ASSERT(!game::v3::trn::Filter::parse("'abc'", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse("'abc'", del).accept(trn, 2));
    TS_ASSERT(!game::v3::trn::Filter::parse("\"abc\"", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse("\"abc\"", del).accept(trn, 2));
    TS_ASSERT( game::v3::trn::Filter::parse("  \"abc\"  ", del).accept(trn, 2));

    TS_ASSERT( game::v3::trn::Filter::parse("9", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse("9", del).accept(trn, 1));
    TS_ASSERT( game::v3::trn::Filter::parse(" 9 ", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse("7-9", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse("7-9", del).accept(trn, 1));
    TS_ASSERT( game::v3::trn::Filter::parse(" 7-9 ", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse(" 7 - 9 ", del).accept(trn, 0));

    TS_ASSERT( game::v3::trn::Filter::parse("#1", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse("#1", del).accept(trn, 1));
    TS_ASSERT( game::v3::trn::Filter::parse(" #1 ", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse(" # 1 ", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse("#1-2", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse("#1-2", del).accept(trn, 1));
    TS_ASSERT(!game::v3::trn::Filter::parse("#1-2", del).accept(trn, 2));
    TS_ASSERT( game::v3::trn::Filter::parse(" #1-2 ", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse(" # 1 - 2 ", del).accept(trn, 0));

    TS_ASSERT( game::v3::trn::Filter::parse("ship*", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse("ship*", del).accept(trn, 1));
    TS_ASSERT(!game::v3::trn::Filter::parse("ship*", del).accept(trn, 2));
    TS_ASSERT(!game::v3::trn::Filter::parse("ship*", del).accept(trn, 3));

    TS_ASSERT(!game::v3::trn::Filter::parse("planetchangefc", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse("planetchangefc", del).accept(trn, 1));
    TS_ASSERT( game::v3::trn::Filter::parse("planetchangefc", del).accept(trn, 2));
    TS_ASSERT(!game::v3::trn::Filter::parse("planetchangefc", del).accept(trn, 3));

    // Test implicit and (parseEx)
    TS_ASSERT(!game::v3::trn::Filter::parse("ship*10", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse("ship*10", del).accept(trn, 1));
    TS_ASSERT(!game::v3::trn::Filter::parse("ship*10", del).accept(trn, 2));
    TS_ASSERT(!game::v3::trn::Filter::parse("ship*10", del).accept(trn, 3));

    TS_ASSERT( game::v3::trn::Filter::parse(" true true ", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse(" true false ", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse(" false true ", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse(" false false ", del).accept(trn, 0));

    // Test negation (parseAnd)
    TS_ASSERT(!game::v3::trn::Filter::parse(" ! true true ", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse(" ! true false ", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse(" ! false true ", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse(" ! false false ", del).accept(trn, 0));

    // Test explicit and (parseOr)
    TS_ASSERT( game::v3::trn::Filter::parse("true&true", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse(" true & true ", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse(" true & false ", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse(" false & true ", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse(" false & false ", del).accept(trn, 0));

    // Test explicit or (parseFilter1)
    TS_ASSERT( game::v3::trn::Filter::parse(" true | true ", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse(" true | false ", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse(" false | true ", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse(" false | false ", del).accept(trn, 0));

    // Test explicit or of explicit and
    TS_ASSERT( game::v3::trn::Filter::parse(" true | true & true ", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse(" true | false & true ", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse(" false | true & true ", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse(" false | false & true ", del).accept(trn, 0));

    TS_ASSERT( game::v3::trn::Filter::parse(" true | true & false ", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse(" true | false & false ", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse(" false | true & false ", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse(" false | false & false ", del).accept(trn, 0));

    // Test parenisation
    TS_ASSERT( game::v3::trn::Filter::parse(" (true | true) & true ", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse(" (true | false) & true ", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse(" (false | true) & true ", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse(" (false | false) & true ", del).accept(trn, 0));

    TS_ASSERT(!game::v3::trn::Filter::parse(" (true | true) & false ", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse(" (true | false) & false ", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse(" (false | true) & false ", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse(" (false | false) & false ", del).accept(trn, 0));

    // Parenisation vs. implicit and
    TS_ASSERT( game::v3::trn::Filter::parse(" (true | true)true ", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse(" (true | false) true ", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse(" (false | true)true ", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse(" (false | false) true ", del).accept(trn, 0));

    TS_ASSERT(!game::v3::trn::Filter::parse(" (true | true) false ", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse(" (true | false)false ", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse(" (false | true) false ", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse(" (false | false)false ", del).accept(trn, 0));

    TS_ASSERT( game::v3::trn::Filter::parse(" true(true | true) ", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse(" true (true | false) ", del).accept(trn, 0));
    TS_ASSERT( game::v3::trn::Filter::parse(" true (false | true) ", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse(" true(false | false) ", del).accept(trn, 0));

    TS_ASSERT(!game::v3::trn::Filter::parse(" false(true | true) ", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse(" false (true | false) ", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse(" false(false | true) ", del).accept(trn, 0));
    TS_ASSERT(!game::v3::trn::Filter::parse(" false (false | false) ", del).accept(trn, 0));
}

/** Test parser failures. */
void
TestGameV3TrnFilter::testParserFailure()
{
    afl::base::Deleter del;

    // Too short
    TS_ASSERT_THROWS(game::v3::trn::Filter::parse("", del), game::v3::trn::ParseException);
    TS_ASSERT_THROWS(game::v3::trn::Filter::parse("a|", del), game::v3::trn::ParseException);
    TS_ASSERT_THROWS(game::v3::trn::Filter::parse("a&", del), game::v3::trn::ParseException);
    TS_ASSERT_THROWS(game::v3::trn::Filter::parse("(", del), game::v3::trn::ParseException);
    TS_ASSERT_THROWS(game::v3::trn::Filter::parse("1-", del), game::v3::trn::ParseException);
    TS_ASSERT_THROWS(game::v3::trn::Filter::parse("#1-", del), game::v3::trn::ParseException);
    TS_ASSERT_THROWS(game::v3::trn::Filter::parse("'foo", del), game::v3::trn::ParseException);
    TS_ASSERT_THROWS(game::v3::trn::Filter::parse("'", del), game::v3::trn::ParseException);
    TS_ASSERT_THROWS(game::v3::trn::Filter::parse("\"foo", del), game::v3::trn::ParseException);
    TS_ASSERT_THROWS(game::v3::trn::Filter::parse("\"", del), game::v3::trn::ParseException);
    TS_ASSERT_THROWS(game::v3::trn::Filter::parse("#", del), game::v3::trn::ParseException);

    // Bad syntax
    TS_ASSERT_THROWS(game::v3::trn::Filter::parse("a+b", del), game::v3::trn::ParseException);
    TS_ASSERT_THROWS(game::v3::trn::Filter::parse("a-b", del), game::v3::trn::ParseException);
    TS_ASSERT_THROWS(game::v3::trn::Filter::parse("a()", del), game::v3::trn::ParseException);
    TS_ASSERT_THROWS(game::v3::trn::Filter::parse("a)", del), game::v3::trn::ParseException);
    TS_ASSERT_THROWS(game::v3::trn::Filter::parse("(a", del), game::v3::trn::ParseException);
    TS_ASSERT_THROWS(game::v3::trn::Filter::parse("#1-#2", del), game::v3::trn::ParseException);
    TS_ASSERT_THROWS(game::v3::trn::Filter::parse("#a", del), game::v3::trn::ParseException);
    TS_ASSERT_THROWS(game::v3::trn::Filter::parse("#-9", del), game::v3::trn::ParseException);

    TS_ASSERT_THROWS(game::v3::trn::Filter::parse("10-5", del), game::v3::trn::ParseException);
    TS_ASSERT_THROWS(game::v3::trn::Filter::parse("#10-5", del), game::v3::trn::ParseException);
}
