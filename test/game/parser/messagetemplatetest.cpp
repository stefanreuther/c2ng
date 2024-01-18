/**
  *  \file test/game/parser/messagetemplatetest.cpp
  *  \brief Test for game::parser::MessageTemplate
  */

#include "game/parser/messagetemplate.hpp"

#include "afl/test/testrunner.hpp"
#include "game/parser/datainterface.hpp"
#include "util/stringparser.hpp"

namespace {
    class NullDataInterface : public game::parser::DataInterface {
     public:
        NullDataInterface(int player = 0)
            : m_player(player)
            { }
        virtual int getPlayerNumber() const
            { return m_player; }
        virtual int parseName(Name /*which*/, const String_t& /*name*/) const
            { return 0; }
        virtual String_t expandRaceNames(String_t name) const
            { return name; }
     private:
        int m_player;
    };

    class MockDataInterface : public game::parser::DataInterface {
     public:
        MockDataInterface()
            { }
        virtual int getPlayerNumber() const
            { return 0; }
        virtual int parseName(Name which, const String_t& name) const
            {
                util::StringParser sp(name);
                bool ok = false;
                switch (which) {
                 case ShortRaceName:     ok = sp.parseString("s"); break;
                 case LongRaceName:      ok = sp.parseString("f"); break;
                 case AdjectiveRaceName: ok = sp.parseString("a"); break;
                 case HullName:          ok = sp.parseString("h"); break;
                }
                int n;
                if (ok && sp.parseInt(n) && sp.parseEnd()) {
                    return n;
                } else {
                    return 0;
                }
            }
        virtual String_t expandRaceNames(String_t name) const
            { return name; }
    };
}

/** Test regular unparsed assignments. */
AFL_TEST("game.parser.MessageTemplate:value", a)
{
    // ex GameMsgTemplateTestSuite::testValues

    // Lines for testing
    game::parser::MessageLines_t lines;
    lines.push_back(String_t());

    std::vector<String_t> values;
    bool result;

    game::parser::MessageTemplate tpl(game::parser::MessageInformation::Configuration, "foo");
    tpl.addCheckInstruction(tpl.iParse + tpl.sAny, 0, "value = $");
    tpl.addVariable("VALUE");

    // Test
    lines[0] = "value = 1";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    a.check("01. result", result);
    a.checkEqual("02. size", values.size(), 1U);
    a.checkEqual("03. value", values[0], "1");
    a.checkEqual("04. int", game::parser::parseIntegerValue(values[0]), 1);

    // Test
    lines[0] = "value = -42";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    a.check("11. result", result);
    a.checkEqual("12. size", values.size(), 1U);
    a.checkEqual("13. value", values[0], "-42");
    a.checkEqual("14. int", game::parser::parseIntegerValue(values[0]), -42);

    // Test
    lines[0] = "value = 15%";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    a.check("21. result", result);
    a.checkEqual("22. size", values.size(), 1U);
    a.checkEqual("23. value", values[0], "15%");
    a.checkEqual("24. int", game::parser::parseIntegerValue(values[0]), 15);

    // Test
    lines[0] = "value =";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    a.check("31. result", !result);
}

/** Test regular assignments of type "X100". */
AFL_TEST("game.parser.MessageTemplate:value-x100", a)
{
    // ex GameMsgTemplateTestSuite::testValuesX100

    // Lines for testing
    game::parser::MessageLines_t lines;
    lines.push_back(String_t());

    std::vector<String_t> values;
    bool result;

    game::parser::MessageTemplate tpl(game::parser::MessageInformation::Configuration, "foo");
    tpl.addCheckInstruction(tpl.iParse + tpl.sAny, 0, "value = $");
    tpl.addVariable("VALUE:X100");

    // Test
    lines[0] = "value = 1";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    a.check("01. result", result);
    a.checkEqual("02. size", values.size(), 1U);
    a.checkEqual("03. value", values[0], "100");
    a.checkEqual("04. int", game::parser::parseIntegerValue(values[0]), 100);

    // Test
    lines[0] = "value = -42";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    a.check("11. result", result);
    a.checkEqual("12. size", values.size(), 1U);
    a.checkEqual("13. value", values[0], "-4200");
    a.checkEqual("14. int", game::parser::parseIntegerValue(values[0]), -4200);

    // Test
    lines[0] = "value = 15%";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    a.check("21. result", result);
    a.checkEqual("22. size", values.size(), 1U);
    a.checkEqual("23. value", values[0], "1500");
    a.checkEqual("24. int", game::parser::parseIntegerValue(values[0]), 1500);

    // Test
    lines[0] = "value = .5";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    a.check("31. result", result);
    a.checkEqual("32. size", values.size(), 1U);
    a.checkEqual("33. value", values[0], "50");
    a.checkEqual("34. int", game::parser::parseIntegerValue(values[0]), 50);

    // Test
    lines[0] = "value = .15";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    a.check("41. result", result);
    a.checkEqual("42. size", values.size(), 1U);
    a.checkEqual("43. value", values[0], "15");
    a.checkEqual("44. int", game::parser::parseIntegerValue(values[0]), 15);

    // Test
    lines[0] = "value = .1234";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    a.check("51. result", result);
    a.checkEqual("52. size", values.size(), 1U);
    a.checkEqual("53. value", values[0], "12");
    a.checkEqual("54. int", game::parser::parseIntegerValue(values[0]), 12);

    // Test
    lines[0] = "value = 123.456";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    a.check("61. result", result);
    a.checkEqual("62. size", values.size(), 1U);
    a.checkEqual("63. value", values[0], "12345");
    a.checkEqual("64. int", game::parser::parseIntegerValue(values[0]), 12345);

    // Test
    lines[0] = "value = -123.456%";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    a.check("71. result", result);
    a.checkEqual("72. size", values.size(), 1U);
    a.checkEqual("73. value", values[0], "-12345");
    a.checkEqual("74. int", game::parser::parseIntegerValue(values[0]), -12345);
}

/** Test regular assignments of enumerated types. */
AFL_TEST("game.parser.MessageTemplate:value-enum", a)
{
    // ex GameMsgTemplateTestSuite::testValuesEnum

    // Lines for testing
    game::parser::MessageLines_t lines;
    lines.push_back(String_t());

    std::vector<String_t> values;
    bool result;

    game::parser::MessageTemplate tpl(game::parser::MessageInformation::Configuration, "foo");
    tpl.addCheckInstruction(tpl.iParse + tpl.sAny, 0, "value = $");
    tpl.addVariable("VALUE:aa/bb/cc/dd");

    // Test
    lines[0] = "value = 1";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    a.check("01. result", result);
    a.checkEqual("02. size", values.size(), 1U);
    a.checkEqual("03. value", values[0], "");

    // Test
    lines[0] = "value = aa";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    a.check("11. result", result);
    a.checkEqual("12. size", values.size(), 1U);
    a.checkEqual("13. value", values[0], "0");
    a.checkEqual("14. int", game::parser::parseIntegerValue(values[0]), 0);

    // Test
    lines[0] = "value = bb";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    a.check("21. result", result);
    a.checkEqual("22. size", values.size(), 1U);
    a.checkEqual("23. value", values[0], "1");
    a.checkEqual("24. int", game::parser::parseIntegerValue(values[0]), 1);

    // Test
    lines[0] = "value = dd";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    a.check("31. result", result);
    a.checkEqual("32. size", values.size(), 1U);
    a.checkEqual("33. value", values[0], "3");
    a.checkEqual("34. int", game::parser::parseIntegerValue(values[0]), 3);

    // Test
    lines[0] = "value = ddd";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    a.check("41. result", result);
    a.checkEqual("42. size", values.size(), 1U);
    a.checkEqual("43. value", values[0], "");
}

/** Test assignment of values of other types. */
AFL_TEST("game.parser.MessageTemplate:value-formats", a)
{
    // Lines for testing
    game::parser::MessageLines_t lines;
    lines.push_back(String_t());

    std::vector<String_t> values;

    // "RACE"
    {
        game::parser::MessageTemplate tpl(game::parser::MessageInformation::Configuration, "foo");
        tpl.addCheckInstruction(tpl.iParse + tpl.sAny, 0, "value = $");
        tpl.addVariable("VALUE:RACE");

        // Test
        lines[0] = "value = f9";
        values.clear();
        a.check("01. match", tpl.match(lines, MockDataInterface(), values));
        a.checkEqual("02. size", values.size(), 1U);
        a.checkEqual("03. value", values[0], "9");

        // Test (failure to interpret the value produces "" value and does not fail the parse!)
        lines[0] = "value = 77";
        values.clear();
        a.check("11. match", tpl.match(lines, MockDataInterface(), values));
        a.checkEqual("12. size", values.size(), 1U);
        a.checkEqual("13. value", values[0], "");
    }

    // "RACE.ADJ"
    {
        game::parser::MessageTemplate tpl(game::parser::MessageInformation::Configuration, "foo");
        tpl.addCheckInstruction(tpl.iParse + tpl.sAny, 0, "value = $");
        tpl.addVariable("VALUE:RACE.ADJ");

        // Test
        lines[0] = "value = a5";
        values.clear();
        a.check("21. match", tpl.match(lines, MockDataInterface(), values));
        a.checkEqual("22. size", values.size(), 1U);
        a.checkEqual("23. value", values[0], "5");

        // Test (failure to interpret the value produces "" value and does not fail the parse!)
        lines[0] = "value = 77";
        values.clear();
        a.check("31. match", tpl.match(lines, MockDataInterface(), values));
        a.checkEqual("32. size", values.size(), 1U);
        a.checkEqual("33. value", values[0], "");

        // Test (failure to interpret the value produces "" value and does not fail the parse!)
        lines[0] = "value = a8+!";
        values.clear();
        a.check("41. match", tpl.match(lines, MockDataInterface(), values));
        a.checkEqual("42. size", values.size(), 1U);
        a.checkEqual("43. value", values[0], "");
    }

    // "RACE.SHORT"
    {
        game::parser::MessageTemplate tpl(game::parser::MessageInformation::Configuration, "foo");
        tpl.addCheckInstruction(tpl.iParse + tpl.sAny, 0, "value = $");
        tpl.addVariable("VALUE:RACE.SHORT");

        // Test
        lines[0] = "value = s14";
        values.clear();
        a.check("51. match", tpl.match(lines, MockDataInterface(), values));
        a.checkEqual("52. size", values.size(), 1U);
        a.checkEqual("53. value", values[0], "14");

        // Test (failure to interpret the value produces "" value and does not fail the parse!)
        lines[0] = "value = 77";
        values.clear();
        a.check("61. match", tpl.match(lines, MockDataInterface(), values));
        a.checkEqual("62. size", values.size(), 1U);
        a.checkEqual("63. value", values[0], "");
    }

    // "HULL"
    {
        game::parser::MessageTemplate tpl(game::parser::MessageInformation::Configuration, "foo");
        tpl.addCheckInstruction(tpl.iParse + tpl.sAny, 0, "value = $");
        tpl.addVariable("VALUE:HULL");

        // Test
        lines[0] = "value = h104";
        values.clear();
        a.check("71. match", tpl.match(lines, MockDataInterface(), values));
        a.checkEqual("72. size", values.size(), 1U);
        a.checkEqual("73. value", values[0], "104");

        // Test (failure to interpret the value produces "" value and does not fail the parse!)
        lines[0] = "value = 77";
        values.clear();
        a.check("81. match", tpl.match(lines, MockDataInterface(), values));
        a.checkEqual("82. size", values.size(), 1U);
        a.checkEqual("83. value", values[0], "");
    }

    // "RACE.ADJ+ALLIES"
    {
        game::parser::MessageTemplate tpl(game::parser::MessageInformation::Configuration, "foo");
        tpl.addCheckInstruction(tpl.iParse + tpl.sAny, 0, "value = $");
        tpl.addVariable("VALUE:RACE.ADJ+ALLIES");

        // Test
        lines[0] = "value = a8!+";
        values.clear();
        a.check("91. match", tpl.match(lines, MockDataInterface(), values));
        a.checkEqual("92. size", values.size(), 1U);
        a.checkEqual("93. value", values[0], "8");
    }
}

/** Test getMessageHeaderInformation(). */
AFL_TEST("game.parser.MessageTemplate:getMessageHeaderInformation", a)
{
    namespace p = game::parser;

    // Standard case
    {
        p::MessageLines_t msg;
        msg.push_back("(-m1234)<<< Hi Mom>>>");
        msg.push_back("whatever");
        a.checkEqual("01", p::getMessageHeaderInformation(msg, p::MsgHdrKind),  int('m'));
        a.checkEqual("02", p::getMessageHeaderInformation(msg, p::MsgHdrSubId), int('1'));
        a.checkEqual("03", p::getMessageHeaderInformation(msg, p::MsgHdrId),    234);
        a.checkEqual("04", p::getMessageHeaderInformation(msg, p::MsgHdrBigId), 1234);
        a.checkEqual("05", p::getMessageHeaderInformation(msg, p::MsgHdrAge),   0);
    }

    // Race reference
    {
        p::MessageLines_t msg;
        msg.push_back("(-9c035)<<< Hi Mom>>>");
        msg.push_back("whatever");
        a.checkEqual("11", p::getMessageHeaderInformation(msg, p::MsgHdrKind),  int('9'));
        a.checkEqual("12", p::getMessageHeaderInformation(msg, p::MsgHdrSubId), int('c'));
        a.checkEqual("13", p::getMessageHeaderInformation(msg, p::MsgHdrId),    35);
        a.checkEqual("14", p::getMessageHeaderInformation(msg, p::MsgHdrBigId), 35);
        a.checkEqual("15", p::getMessageHeaderInformation(msg, p::MsgHdrAge),   0);
    }

    // Old one
    {
        p::MessageLines_t msg;
        msg.push_back("(or3000)<<< Hi Mom>>>");
        msg.push_back("whatever");
        a.checkEqual("21", p::getMessageHeaderInformation(msg, p::MsgHdrKind),  int('r'));
        a.checkEqual("22", p::getMessageHeaderInformation(msg, p::MsgHdrSubId), int('3'));
        a.checkEqual("23", p::getMessageHeaderInformation(msg, p::MsgHdrId),    0);
        a.checkEqual("24", p::getMessageHeaderInformation(msg, p::MsgHdrBigId), 3000);
        a.checkEqual("25", p::getMessageHeaderInformation(msg, p::MsgHdrAge),   1);
    }

    // Long one
    {
        p::MessageLines_t msg;
        msg.push_back("(-m17000)<<< Hi Mom>>>");
        msg.push_back("whatever");
        a.checkEqual("31", p::getMessageHeaderInformation(msg, p::MsgHdrKind),  int('m'));
        a.checkEqual("32", p::getMessageHeaderInformation(msg, p::MsgHdrSubId), int('1'));
        a.checkEqual("33", p::getMessageHeaderInformation(msg, p::MsgHdrId),    7000);
        a.checkEqual("34", p::getMessageHeaderInformation(msg, p::MsgHdrBigId), 17000);
        a.checkEqual("35", p::getMessageHeaderInformation(msg, p::MsgHdrAge),   0);
    }

    // Out-of-range case
    {
        p::MessageLines_t msg;
        a.checkEqual("41", p::getMessageHeaderInformation(msg, p::MsgHdrKind),  0);
        a.checkEqual("42", p::getMessageHeaderInformation(msg, p::MsgHdrSubId), 0);
        a.checkEqual("43", p::getMessageHeaderInformation(msg, p::MsgHdrId),    0);
        a.checkEqual("44", p::getMessageHeaderInformation(msg, p::MsgHdrBigId), 0);
        a.checkEqual("45", p::getMessageHeaderInformation(msg, p::MsgHdrAge),   0);
    }

    // Old host case
    {
        p::MessageLines_t msg;
        msg.push_back("<<< Sub-Space Message >>>");
        a.checkEqual("51", p::getMessageHeaderInformation(msg, p::MsgHdrKind),  0);
        a.checkEqual("52", p::getMessageHeaderInformation(msg, p::MsgHdrSubId), 0);
        a.checkEqual("53", p::getMessageHeaderInformation(msg, p::MsgHdrId),    0);
        a.checkEqual("54", p::getMessageHeaderInformation(msg, p::MsgHdrBigId), 0);
        a.checkEqual("55", p::getMessageHeaderInformation(msg, p::MsgHdrAge),   0);
    }

    // Broken case 1
    {
        p::MessageLines_t msg;
        msg.push_back("");
        a.checkEqual("61", p::getMessageHeaderInformation(msg, p::MsgHdrKind),  0);
        a.checkEqual("62", p::getMessageHeaderInformation(msg, p::MsgHdrSubId), 0);
        a.checkEqual("63", p::getMessageHeaderInformation(msg, p::MsgHdrId),    0);
        a.checkEqual("64", p::getMessageHeaderInformation(msg, p::MsgHdrBigId), 0);
        a.checkEqual("65", p::getMessageHeaderInformation(msg, p::MsgHdrAge),   0);
    }

    // Broken case 1
    {
        p::MessageLines_t msg;
        msg.push_back("12345");
        a.checkEqual("71", p::getMessageHeaderInformation(msg, p::MsgHdrKind),  0);
        a.checkEqual("72", p::getMessageHeaderInformation(msg, p::MsgHdrSubId), 0);
        a.checkEqual("73", p::getMessageHeaderInformation(msg, p::MsgHdrId),    0);
        a.checkEqual("74", p::getMessageHeaderInformation(msg, p::MsgHdrBigId), 0);
        a.checkEqual("75", p::getMessageHeaderInformation(msg, p::MsgHdrAge),   0);
    }
}

/** Test splitMessage(). */
AFL_TEST("game.parser.MessageTemplate:splitMessage", a)
{
    namespace p = game::parser;
    {
        p::MessageLines_t m;
        p::splitMessage(m, "");
        a.checkEqual("01. size", m.size(), 1U);
        a.checkEqual("02. line", m[0], "");
    }
    {
        p::MessageLines_t m;
        p::splitMessage(m, "xyz");
        a.checkEqual("03. size", m.size(), 1U);
        a.checkEqual("04. line", m[0], "xyz");
    }
    {
        p::MessageLines_t m;
        p::splitMessage(m, "\nxyz\n");
        a.checkEqual("05. size", m.size(), 3U);
        a.checkEqual("06. line", m[0], "");
        a.checkEqual("07. line", m[1], "xyz");
        a.checkEqual("08. line", m[2], "");
    }
}

/** Test parseIntegerValue(). */
AFL_TEST("game.parser.MessageTemplate:parseIntegerValue", a)
{
    a.checkEqual("01", game::parser::parseIntegerValue("0"), 0);
    a.checkEqual("02", game::parser::parseIntegerValue("99 kt"), 99);
    a.checkEqual("03", game::parser::parseIntegerValue("77$"), 77);
    a.checkEqual("04", game::parser::parseIntegerValue("0x99"), 0);
    a.checkEqual("05", game::parser::parseIntegerValue("-100"), -100);
    a.checkEqual("06", game::parser::parseIntegerValue("3.5"), 3);

    a.checkEqual("11", game::parser::parseIntegerValue("$"), -1);
    a.checkEqual("12", game::parser::parseIntegerValue(""), -1);
}

/** Test match() with metadata information: Kind.
    Also checks extraction of "id". */
AFL_TEST("game.parser.MessageTemplate:match:iMatchKind", a)
{
    // Build a template
    game::parser::MessageTemplate testee(game::parser::MessageInformation::Planet, "pm");
    testee.addMatchInstruction(game::parser::MessageTemplate::iMatchKind, 'p');
    testee.addValueInstruction(game::parser::MessageTemplate::iValue,     "id");
    testee.addVariable("Id");

    // Verify template
    a.checkEqual("01. getContinueFlag", testee.getContinueFlag(), false);
    a.checkEqual("02. getTemplateName", testee.getTemplateName(), "pm");
    a.checkEqual("03. getMessageType", testee.getMessageType(), game::parser::MessageInformation::Planet);

    a.checkEqual("11. getNumVariables", testee.getNumVariables(), 1U);
    a.checkEqual("12. getVariableName", testee.getVariableName(0), "ID");
    a.checkEqual("13. getVariableName", testee.getVariableName(1), "");      // out-of-range

    a.checkEqual("21. getVariableSlotByName", testee.getVariableSlotByName("ID").orElse(99), 0u);     // variables are internally upcased...
    a.check("22. getVariableSlotByName", !testee.getVariableSlotByName("id").isValid());                // ...but matched case-sensitively

    a.checkEqual("31. getNumRestrictions", testee.getNumRestrictions(), 1U);
    a.checkEqual("32. getNumWildcards", testee.getNumWildcards(), 1U);

    // Match successfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-p0363)<<< Report >>>");
        m.push_back("etc...");

        std::vector<String_t> result;
        a.check("41. match", testee.match(m, NullDataInterface(), result));
        a.checkEqual("42. size", result.size(), 1U);
        a.checkEqual("43. result", result[0], "363");
    }

    // Match unsuccessfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-s0015)<<< Report >>>");
        m.push_back("etc...");

        std::vector<String_t> result;
        a.check("51. match", !testee.match(m, NullDataInterface(), result));
    }
}

/** Test match() with metadata information: SubId.
    Also test extraction of player. */
AFL_TEST("game.parser.MessageTemplate:match:iMatchSubId", a)
{
    // Build a template
    game::parser::MessageTemplate testee(game::parser::MessageInformation::Planet, "pm");
    testee.addMatchInstruction(game::parser::MessageTemplate::iMatchSubId, 'c');
    testee.addValueInstruction(game::parser::MessageTemplate::iValue,      "player");
    testee.addVariable("Player");

    // Match successfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-9c111)<<< Report >>>");
        m.push_back("etc...");

        std::vector<String_t> result;
        a.check("01. match", testee.match(m, NullDataInterface(7), result));
        a.checkEqual("02. size", result.size(), 1U);
        a.checkEqual("03. result", result[0], "7");
    }

    // Match unsuccessfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-90111)<<< Report >>>");
        m.push_back("etc...");

        std::vector<String_t> result;
        a.check("11. match", !testee.match(m, NullDataInterface(7), result));
    }
}

/** Test match() with metadata information: BigId.
    Also test production of fixed values. */
AFL_TEST("game.parser.MessageTemplate:match:iMatchBigId", a)
{
    // Build a template
    game::parser::MessageTemplate testee(game::parser::MessageInformation::Planet, "pm");
    testee.addMatchInstruction(game::parser::MessageTemplate::iMatchBigId, 12345);
    testee.addValueInstruction(game::parser::MessageTemplate::iValue,      "49");
    testee.addVariable("Answer");

    // Match successfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-912345)<<< Report >>>");
        m.push_back("etc...");

        std::vector<String_t> result;
        a.check("01. match", testee.match(m, NullDataInterface(), result));
        a.checkEqual("02. size", result.size(), 1U);
        a.checkEqual("03. result", result[0], "49");
    }

    // Match unsuccessfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-9c1234)<<< Report >>>");
        m.push_back("etc...");

        std::vector<String_t> result;
        a.check("11. match", !testee.match(m, NullDataInterface(), result));
    }
}

/** Test match() with check/fail/find.
    Also test extraction of id/bigid/subid. */
AFL_TEST("game.parser.MessageTemplate:match:iCheck+iFail+iFind", a)
{
    // Build a template
    game::parser::MessageTemplate testee(game::parser::MessageInformation::Ship, "ff");
    testee.addCheckInstruction(game::parser::MessageTemplate::iCheck, 0, "check this");
    testee.addCheckInstruction(game::parser::MessageTemplate::iFail, 0, "fail this");
    testee.addCheckInstruction(game::parser::MessageTemplate::iFind, 0, "find this");
    testee.addVariable("find");
    testee.addValueInstruction(game::parser::MessageTemplate::iValue, "id,bigid,subid");
    testee.addVariables("id,bigid,subid");

    // Verify template
    a.checkEqual("01. getNumVariables", testee.getNumVariables(), 4U);
    a.checkEqual("02. getVariableName", testee.getVariableName(0), "FIND");
    a.checkEqual("03. getVariableName", testee.getVariableName(1), "ID");
    a.checkEqual("04. getVariableName", testee.getVariableName(2), "BIGID");
    a.checkEqual("05. getVariableName", testee.getVariableName(3), "SUBID");
    a.checkEqual("06. getNumRestrictions", testee.getNumRestrictions(), 3U);
    a.checkEqual("07. getNumWildcards", testee.getNumWildcards(), 4U);

    // Match successfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-s0599)<<< Report >>>");
        m.push_back("check this");

        std::vector<String_t> result;
        a.check("11. match", testee.match(m, NullDataInterface(), result));
        a.checkEqual("12. size", result.size(), 4U);
        a.checkEqual("13. result", result[0], "0");
        a.checkEqual("14. result", result[1], "599");
        a.checkEqual("15. result", result[2], "599");
        a.checkEqual("16. result", result[3], "0");
    }

    // Match successfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-sc456)<<< find this >>>");
        m.push_back("check this");

        std::vector<String_t> result;
        a.check("21. match", testee.match(m, NullDataInterface(), result));
        a.checkEqual("22. size", result.size(), 4U);
        a.checkEqual("23. result", result[0], "1");
        a.checkEqual("24. result", result[1], "456");
        a.checkEqual("25. result", result[2], "456");
        a.checkEqual("26. result", result[3], "12");
    }

    // Match successfully, bad sub Id
    {
        game::parser::MessageLines_t m;
        m.push_back("(-s*456)<<< check this >>>");
        m.push_back("find this");

        std::vector<String_t> result;
        a.check("31. match", testee.match(m, NullDataInterface(), result));
        a.checkEqual("32. size", result.size(), 4U);
        a.checkEqual("33. result", result[0], "1");
        a.checkEqual("34. result", result[1], "456");
        a.checkEqual("35. result", result[2], "456");
        a.checkEqual("36. result", result[3], "0");
    }

    // Match unsuccessfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-sc456)<<< blank >>>");
        m.push_back("also blank");

        std::vector<String_t> result;
        a.check("41. match", !testee.match(m, NullDataInterface(), result));
    }

    // Match unsuccessfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-sc456)<<< blank >>>");
        m.push_back("fail this");
        m.push_back("check this");

        std::vector<String_t> result;
        a.check("51. match", !testee.match(m, NullDataInterface(), result));
    }
}

/** Test match() with parse/fail and value extration. */
AFL_TEST("game.parser.MessageTemplate:match:iParse+iFail", a)
{
    // Build a template
    game::parser::MessageTemplate testee(game::parser::MessageInformation::Ship, "ff");
    testee.addCheckInstruction(game::parser::MessageTemplate::iParse, 0, "parse this $ : $");
    testee.addVariables("a:X100,b");
    testee.addCheckInstruction(game::parser::MessageTemplate::iFail + game::parser::MessageTemplate::sRelative, 1, "fail this");

    // Verify template
    a.checkEqual("01. getNumVariables", testee.getNumVariables(), 2U);
    a.checkEqual("02. getVariableName", testee.getVariableName(0), "A");
    a.checkEqual("03. getVariableName", testee.getVariableName(1), "B");
    a.checkEqual("04. getNumRestrictions", testee.getNumRestrictions(), 2U);
    a.checkEqual("05. getNumWildcards", testee.getNumWildcards(), 2U);

    // Match successfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-s0100)<<< Title >>>");
        m.push_back("parse this 100 : 1");

        std::vector<String_t> result;
        a.check("11. match", testee.match(m, NullDataInterface(), result));
        a.checkEqual("12. size", result.size(), 2U);
        a.checkEqual("13. result", result[0], "10000");
        a.checkEqual("14. result", result[1], "1");
    }

    // Match successfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-s0100)<<< Title >>>");
        m.push_back("fail this");                 // not hit due to sRelative, 1
        m.push_back("parse this 3.14 : 1");

        std::vector<String_t> result;
        a.check("21. match", testee.match(m, NullDataInterface(), result));
        a.checkEqual("22. size", result.size(), 2U);
        a.checkEqual("23. result", result[0], "314");
        a.checkEqual("24. result", result[1], "1");
    }

    // Match unsuccessfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-s0100)<<< Title >>>");
        m.push_back("parse this 2.71 : 1");
        m.push_back("fail this");                 // hits

        std::vector<String_t> result;
        a.check("31. match", !testee.match(m, NullDataInterface(), result));
    }
}

/** Test parsing an array. */
AFL_TEST("game.parser.MessageTemplate:match:iArray", a)
{
    // Build a template
    game::parser::MessageTemplate testee(game::parser::MessageInformation::Ship, "aa");
    testee.addCheckInstruction(game::parser::MessageTemplate::iArray, 0, "$=$:$");
    testee.addVariables("Index:Race.Adj,Attack,Defend");

    // Verify template
    a.checkEqual("01. getNumVariables", testee.getNumVariables(), 3U);
    a.checkEqual("02. getVariableName", testee.getVariableName(0), "INDEX");
    a.checkEqual("03. getVariableName", testee.getVariableName(1), "ATTACK");
    a.checkEqual("04. getVariableName", testee.getVariableName(2), "DEFEND");
    a.checkEqual("05. getNumRestrictions", testee.getNumRestrictions(), 1U);
    a.checkEqual("06. getNumWildcards", testee.getNumWildcards(), 3U);

    // Match successfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-s0100)<<< Title >>>");
        m.push_back("a2 = 10 : 1");
        m.push_back("a3 = 3:1");
        m.push_back("a1=1: 7");
        m.push_back("whatever");
        m.push_back("a4 = 3 : 2");

        std::vector<String_t> result;
        a.check("11. match", testee.match(m, MockDataInterface(), result));
        a.checkEqual("12. size", result.size(), 3U);
        a.checkEqual("13. result", result[0], "");             // Index is not returned
        a.checkEqual("14. result", result[1], "1,10,3,,,,,,,,");
        a.checkEqual("15. result", result[2], "7,1,1,,,,,,,,");
    }

    // Match sparsely
    {
        game::parser::MessageLines_t m;
        m.push_back("(-s0100)<<< Title >>>");
        m.push_back("a1=1: 7");
        m.push_back("a4 = 3 : 2");

        std::vector<String_t> result;
        a.check("21. match", testee.match(m, MockDataInterface(), result));
        a.checkEqual("22. size", result.size(), 3U);
        a.checkEqual("23. result", result[0], "");             // Index is not returned
        a.checkEqual("24. result", result[1], "1,,,3,,,,,,,");
        a.checkEqual("25. result", result[2], "7,,,2,,,,,,,");
    }

    // Match unsuccessfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-s0100)<<< Title >>>");
        m.push_back("whatever");

        std::vector<String_t> result;
        a.check("31. match", !testee.match(m, MockDataInterface(), result));
    }
}

/** Test parsing an array at a fixed place. */
AFL_TEST("game.parser.MessageTemplate:match:iArray:fixed-position", a)
{
    // Build a template
    game::parser::MessageTemplate testee(game::parser::MessageInformation::Ship, "aa");
    testee.addCheckInstruction(game::parser::MessageTemplate::iCheck, 0, "check this");
    testee.addCheckInstruction(game::parser::MessageTemplate::iArray + game::parser::MessageTemplate::sRelative, 1, "$=$:$");
    testee.addVariables("Index:Race.Adj,Attack,Defend");

    // Verify template
    a.checkEqual("01. getNumVariables", testee.getNumVariables(), 3U);
    a.checkEqual("02. getVariableName", testee.getVariableName(0), "INDEX");
    a.checkEqual("03. getVariableName", testee.getVariableName(1), "ATTACK");
    a.checkEqual("04. getVariableName", testee.getVariableName(2), "DEFEND");
    a.checkEqual("05. getNumRestrictions", testee.getNumRestrictions(), 2U);
    a.checkEqual("06. getNumWildcards", testee.getNumWildcards(), 3U);

    // Match sparsely
    {
        game::parser::MessageLines_t m;
        m.push_back("(-s0100)<<< Title >>>");
        m.push_back("a2=1: 7");
        m.push_back("check this");
        m.push_back("a1=1: 7");
        m.push_back("a4 = 3 : 2");

        std::vector<String_t> result;
        a.check("11. match", testee.match(m, MockDataInterface(), result));
        a.checkEqual("12. size", result.size(), 3U);
        a.checkEqual("13. result", result[0], "");             // Index is not returned
        a.checkEqual("14. result", result[1], "1,,,3,,,,,,,");
        a.checkEqual("15. result", result[2], "7,,,2,,,,,,,");
    }

    // Match unsuccessfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-s0100)<<< Title >>>");
        m.push_back("a2=1: 7");
        m.push_back("check this");

        std::vector<String_t> result;
        a.check("21. match", !testee.match(m, MockDataInterface(), result));
    }
}
