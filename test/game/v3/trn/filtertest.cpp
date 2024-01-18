/**
  *  \file test/game/v3/trn/filtertest.cpp
  *  \brief Test for game::v3::trn::Filter
  */

#include "game/v3/trn/filter.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/timestamp.hpp"
#include "game/v3/trn/parseexception.hpp"
#include "game/v3/turnfile.hpp"

using game::v3::trn::Filter;
using game::v3::trn::ParseException;

/** Interface test. */
AFL_TEST_NOARG("game.v3.trn.Filter:interface")
{
    class Tester : public Filter {
     public:
        virtual bool accept(const game::v3::TurnFile& /*trn*/, size_t /*index*/) const
            { return false; }
    };
    Tester t;
}

/** Test the parser. */
AFL_TEST("game.v3.trn.Filter:parse:success", a)
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
    afl::string::NullTranslator tx;
    a.check("01",  Filter::parse("true", del, tx).accept(trn, 0));
    a.check("02", !Filter::parse("false", del, tx).accept(trn, 0));
    a.check("03",  Filter::parse("  true  ", del, tx).accept(trn, 0));
    a.check("04",  Filter::parse("TRUE", del, tx).accept(trn, 0));

    a.check("11", !Filter::parse("'abc'", del, tx).accept(trn, 0));
    a.check("12",  Filter::parse("'abc'", del, tx).accept(trn, 2));
    a.check("13", !Filter::parse("\"abc\"", del, tx).accept(trn, 0));
    a.check("14",  Filter::parse("\"abc\"", del, tx).accept(trn, 2));
    a.check("15",  Filter::parse("  \"abc\"  ", del, tx).accept(trn, 2));

    a.check("21",  Filter::parse("9", del, tx).accept(trn, 0));
    a.check("22", !Filter::parse("9", del, tx).accept(trn, 1));
    a.check("23",  Filter::parse(" 9 ", del, tx).accept(trn, 0));
    a.check("24",  Filter::parse("7-9", del, tx).accept(trn, 0));
    a.check("25", !Filter::parse("7-9", del, tx).accept(trn, 1));
    a.check("26",  Filter::parse(" 7-9 ", del, tx).accept(trn, 0));
    a.check("27",  Filter::parse(" 7 - 9 ", del, tx).accept(trn, 0));

    a.check("31",  Filter::parse("#1", del, tx).accept(trn, 0));
    a.check("32", !Filter::parse("#1", del, tx).accept(trn, 1));
    a.check("33",  Filter::parse(" #1 ", del, tx).accept(trn, 0));
    a.check("34",  Filter::parse(" # 1 ", del, tx).accept(trn, 0));
    a.check("35",  Filter::parse("#1-2", del, tx).accept(trn, 0));
    a.check("36",  Filter::parse("#1-2", del, tx).accept(trn, 1));
    a.check("37", !Filter::parse("#1-2", del, tx).accept(trn, 2));
    a.check("38",  Filter::parse(" #1-2 ", del, tx).accept(trn, 0));
    a.check("39",  Filter::parse(" # 1 - 2 ", del, tx).accept(trn, 0));

    a.check("41",  Filter::parse("ship*", del, tx).accept(trn, 0));
    a.check("42",  Filter::parse("ship*", del, tx).accept(trn, 1));
    a.check("43", !Filter::parse("ship*", del, tx).accept(trn, 2));
    a.check("44", !Filter::parse("ship*", del, tx).accept(trn, 3));

    a.check("51", !Filter::parse("planetchangefc", del, tx).accept(trn, 0));
    a.check("52", !Filter::parse("planetchangefc", del, tx).accept(trn, 1));
    a.check("53",  Filter::parse("planetchangefc", del, tx).accept(trn, 2));
    a.check("54", !Filter::parse("planetchangefc", del, tx).accept(trn, 3));

    // Test implicit and (parseEx)
    a.check("61", !Filter::parse("ship*10", del, tx).accept(trn, 0));
    a.check("62",  Filter::parse("ship*10", del, tx).accept(trn, 1));
    a.check("63", !Filter::parse("ship*10", del, tx).accept(trn, 2));
    a.check("64", !Filter::parse("ship*10", del, tx).accept(trn, 3));

    a.check("71",  Filter::parse(" true true ", del, tx).accept(trn, 0));
    a.check("72", !Filter::parse(" true false ", del, tx).accept(trn, 0));
    a.check("73", !Filter::parse(" false true ", del, tx).accept(trn, 0));
    a.check("74", !Filter::parse(" false false ", del, tx).accept(trn, 0));

    // Test negation (parseAnd)
    a.check("81", !Filter::parse(" ! true true ", del, tx).accept(trn, 0));
    a.check("82",  Filter::parse(" ! true false ", del, tx).accept(trn, 0));
    a.check("83",  Filter::parse(" ! false true ", del, tx).accept(trn, 0));
    a.check("84",  Filter::parse(" ! false false ", del, tx).accept(trn, 0));

    // Test explicit and (parseOr)
    a.check("91",  Filter::parse("true&true", del, tx).accept(trn, 0));
    a.check("92",  Filter::parse(" true & true ", del, tx).accept(trn, 0));
    a.check("93", !Filter::parse(" true & false ", del, tx).accept(trn, 0));
    a.check("94", !Filter::parse(" false & true ", del, tx).accept(trn, 0));
    a.check("95", !Filter::parse(" false & false ", del, tx).accept(trn, 0));

    // Test explicit or (parseFilter1)
    a.check("101",  Filter::parse(" true | true ", del, tx).accept(trn, 0));
    a.check("102",  Filter::parse(" true | false ", del, tx).accept(trn, 0));
    a.check("103",  Filter::parse(" false | true ", del, tx).accept(trn, 0));
    a.check("104", !Filter::parse(" false | false ", del, tx).accept(trn, 0));

    // Test explicit or of explicit and
    a.check("111",  Filter::parse(" true | true & true ", del, tx).accept(trn, 0));
    a.check("112",  Filter::parse(" true | false & true ", del, tx).accept(trn, 0));
    a.check("113",  Filter::parse(" false | true & true ", del, tx).accept(trn, 0));
    a.check("114", !Filter::parse(" false | false & true ", del, tx).accept(trn, 0));

    a.check("121",  Filter::parse(" true | true & false ", del, tx).accept(trn, 0));
    a.check("122",  Filter::parse(" true | false & false ", del, tx).accept(trn, 0));
    a.check("123", !Filter::parse(" false | true & false ", del, tx).accept(trn, 0));
    a.check("124", !Filter::parse(" false | false & false ", del, tx).accept(trn, 0));

    // Test parenisation
    a.check("131",  Filter::parse(" (true | true) & true ", del, tx).accept(trn, 0));
    a.check("132",  Filter::parse(" (true | false) & true ", del, tx).accept(trn, 0));
    a.check("133",  Filter::parse(" (false | true) & true ", del, tx).accept(trn, 0));
    a.check("134", !Filter::parse(" (false | false) & true ", del, tx).accept(trn, 0));

    a.check("141", !Filter::parse(" (true | true) & false ", del, tx).accept(trn, 0));
    a.check("142", !Filter::parse(" (true | false) & false ", del, tx).accept(trn, 0));
    a.check("143", !Filter::parse(" (false | true) & false ", del, tx).accept(trn, 0));
    a.check("144", !Filter::parse(" (false | false) & false ", del, tx).accept(trn, 0));

    // Parenisation vs. implicit and
    a.check("151",  Filter::parse(" (true | true)true ", del, tx).accept(trn, 0));
    a.check("152",  Filter::parse(" (true | false) true ", del, tx).accept(trn, 0));
    a.check("153",  Filter::parse(" (false | true)true ", del, tx).accept(trn, 0));
    a.check("154", !Filter::parse(" (false | false) true ", del, tx).accept(trn, 0));

    a.check("161", !Filter::parse(" (true | true) false ", del, tx).accept(trn, 0));
    a.check("162", !Filter::parse(" (true | false)false ", del, tx).accept(trn, 0));
    a.check("163", !Filter::parse(" (false | true) false ", del, tx).accept(trn, 0));
    a.check("164", !Filter::parse(" (false | false)false ", del, tx).accept(trn, 0));

    a.check("171",  Filter::parse(" true(true | true) ", del, tx).accept(trn, 0));
    a.check("172",  Filter::parse(" true (true | false) ", del, tx).accept(trn, 0));
    a.check("173",  Filter::parse(" true (false | true) ", del, tx).accept(trn, 0));
    a.check("174", !Filter::parse(" true(false | false) ", del, tx).accept(trn, 0));

    a.check("181", !Filter::parse(" false(true | true) ", del, tx).accept(trn, 0));
    a.check("182", !Filter::parse(" false (true | false) ", del, tx).accept(trn, 0));
    a.check("183", !Filter::parse(" false(false | true) ", del, tx).accept(trn, 0));
    a.check("184", !Filter::parse(" false (false | false) ", del, tx).accept(trn, 0));
}

/** Test parser failures. */
AFL_TEST("game.v3.trn.Filter:parse:error", a)
{
    afl::base::Deleter del;
    afl::string::NullTranslator tx;

    // Too short
    AFL_CHECK_THROWS(a("01. too short"), Filter::parse("", del, tx), ParseException);
    AFL_CHECK_THROWS(a("02. too short"), Filter::parse("a|", del, tx), ParseException);
    AFL_CHECK_THROWS(a("03. too short"), Filter::parse("a&", del, tx), ParseException);
    AFL_CHECK_THROWS(a("04. too short"), Filter::parse("(", del, tx), ParseException);
    AFL_CHECK_THROWS(a("05. too short"), Filter::parse("1-", del, tx), ParseException);
    AFL_CHECK_THROWS(a("06. too short"), Filter::parse("#1-", del, tx), ParseException);
    AFL_CHECK_THROWS(a("07. too short"), Filter::parse("'foo", del, tx), ParseException);
    AFL_CHECK_THROWS(a("08. too short"), Filter::parse("'", del, tx), ParseException);
    AFL_CHECK_THROWS(a("09. too short"), Filter::parse("\"foo", del, tx), ParseException);
    AFL_CHECK_THROWS(a("10. too short"), Filter::parse("\"", del, tx), ParseException);
    AFL_CHECK_THROWS(a("11. too short"), Filter::parse("#", del, tx), ParseException);

    // Bad syntax
    AFL_CHECK_THROWS(a("21. bad syntax"), Filter::parse("a+b", del, tx), ParseException);
    AFL_CHECK_THROWS(a("22. bad syntax"), Filter::parse("a-b", del, tx), ParseException);
    AFL_CHECK_THROWS(a("23. bad syntax"), Filter::parse("a()", del, tx), ParseException);
    AFL_CHECK_THROWS(a("24. bad syntax"), Filter::parse("a)", del, tx), ParseException);
    AFL_CHECK_THROWS(a("25. bad syntax"), Filter::parse("(a", del, tx), ParseException);
    AFL_CHECK_THROWS(a("26. bad syntax"), Filter::parse("#1-#2", del, tx), ParseException);
    AFL_CHECK_THROWS(a("27. bad syntax"), Filter::parse("#a", del, tx), ParseException);
    AFL_CHECK_THROWS(a("28. bad syntax"), Filter::parse("#-9", del, tx), ParseException);

    AFL_CHECK_THROWS(a("31. bad syntax"), Filter::parse("10-5", del, tx), ParseException);
    AFL_CHECK_THROWS(a("32. bad syntax"), Filter::parse("#10-5", del, tx), ParseException);
}
