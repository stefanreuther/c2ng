/**
  *  \file u/t_game_parser_messagetemplate.cpp
  *  \brief Test for game::parser::MessageTemplate
  */

#include "game/parser/messagetemplate.hpp"

#include "t_game_parser.hpp"
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
     private:
        int m_player;
    };
}

/** Test regular unparsed assignments. */
void
TestGameParserMessageTemplate::testValues()
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
    TS_ASSERT(result);
    TS_ASSERT_EQUALS(values.size(), 1U);
    TS_ASSERT_EQUALS(values[0], "1");
    TS_ASSERT_EQUALS(game::parser::parseIntegerValue(values[0]), 1);

    // Test
    lines[0] = "value = -42";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    TS_ASSERT(result);
    TS_ASSERT_EQUALS(values.size(), 1U);
    TS_ASSERT_EQUALS(values[0], "-42");
    TS_ASSERT_EQUALS(game::parser::parseIntegerValue(values[0]), -42);

    // Test
    lines[0] = "value = 15%";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    TS_ASSERT(result);
    TS_ASSERT_EQUALS(values.size(), 1U);
    TS_ASSERT_EQUALS(values[0], "15%");
    TS_ASSERT_EQUALS(game::parser::parseIntegerValue(values[0]), 15);

    // Test
    lines[0] = "value =";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    TS_ASSERT(!result);
}

/** Test regular assignments of type "X100". */
void
TestGameParserMessageTemplate::testValuesX100()
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
    TS_ASSERT(result);
    TS_ASSERT_EQUALS(values.size(), 1U);
    TS_ASSERT_EQUALS(values[0], "100");
    TS_ASSERT_EQUALS(game::parser::parseIntegerValue(values[0]), 100);

    // Test
    lines[0] = "value = -42";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    TS_ASSERT(result);
    TS_ASSERT_EQUALS(values.size(), 1U);
    TS_ASSERT_EQUALS(values[0], "-4200");
    TS_ASSERT_EQUALS(game::parser::parseIntegerValue(values[0]), -4200);

    // Test
    lines[0] = "value = 15%";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    TS_ASSERT(result);
    TS_ASSERT_EQUALS(values.size(), 1U);
    TS_ASSERT_EQUALS(values[0], "1500");
    TS_ASSERT_EQUALS(game::parser::parseIntegerValue(values[0]), 1500);

    // Test
    lines[0] = "value = .5";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    TS_ASSERT(result);
    TS_ASSERT_EQUALS(values.size(), 1U);
    TS_ASSERT_EQUALS(values[0], "50");
    TS_ASSERT_EQUALS(game::parser::parseIntegerValue(values[0]), 50);

    // Test
    lines[0] = "value = .15";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    TS_ASSERT(result);
    TS_ASSERT_EQUALS(values.size(), 1U);
    TS_ASSERT_EQUALS(values[0], "15");
    TS_ASSERT_EQUALS(game::parser::parseIntegerValue(values[0]), 15);

    // Test
    lines[0] = "value = .1234";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    TS_ASSERT(result);
    TS_ASSERT_EQUALS(values.size(), 1U);
    TS_ASSERT_EQUALS(values[0], "12");
    TS_ASSERT_EQUALS(game::parser::parseIntegerValue(values[0]), 12);

    // Test
    lines[0] = "value = 123.456";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    TS_ASSERT(result);
    TS_ASSERT_EQUALS(values.size(), 1U);
    TS_ASSERT_EQUALS(values[0], "12345");
    TS_ASSERT_EQUALS(game::parser::parseIntegerValue(values[0]), 12345);

    // Test
    lines[0] = "value = -123.456%";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    TS_ASSERT(result);
    TS_ASSERT_EQUALS(values.size(), 1U);
    TS_ASSERT_EQUALS(values[0], "-12345");
    TS_ASSERT_EQUALS(game::parser::parseIntegerValue(values[0]), -12345);
}

/** Test regular assignments of enumerated types. */
void
TestGameParserMessageTemplate::testValuesEnum()
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
    TS_ASSERT(result);
    TS_ASSERT_EQUALS(values.size(), 1U);
    TS_ASSERT_EQUALS(values[0], "");

    // Test
    lines[0] = "value = aa";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    TS_ASSERT(result);
    TS_ASSERT_EQUALS(values.size(), 1U);
    TS_ASSERT_EQUALS(values[0], "0");
    TS_ASSERT_EQUALS(game::parser::parseIntegerValue(values[0]), 0);

    // Test
    lines[0] = "value = bb";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    TS_ASSERT(result);
    TS_ASSERT_EQUALS(values.size(), 1U);
    TS_ASSERT_EQUALS(values[0], "1");
    TS_ASSERT_EQUALS(game::parser::parseIntegerValue(values[0]), 1);

    // Test
    lines[0] = "value = dd";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    TS_ASSERT(result);
    TS_ASSERT_EQUALS(values.size(), 1U);
    TS_ASSERT_EQUALS(values[0], "3");
    TS_ASSERT_EQUALS(game::parser::parseIntegerValue(values[0]), 3);

    // Test
    lines[0] = "value = ddd";
    values.clear();
    result = tpl.match(lines, NullDataInterface(), values);
    TS_ASSERT(result);
    TS_ASSERT_EQUALS(values.size(), 1U);
    TS_ASSERT_EQUALS(values[0], "");
}

/** Test assignment of values of other types. */
void
TestGameParserMessageTemplate::testValuesFormat()
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
        TS_ASSERT(tpl.match(lines, MockDataInterface(), values));
        TS_ASSERT_EQUALS(values.size(), 1U);
        TS_ASSERT_EQUALS(values[0], "9");

        // Test (failure to interpret the value produces "" value and does not fail the parse!)
        lines[0] = "value = 77";
        values.clear();
        TS_ASSERT(tpl.match(lines, MockDataInterface(), values));
        TS_ASSERT_EQUALS(values.size(), 1U);
        TS_ASSERT_EQUALS(values[0], "");
    }

    // "RACE.ADJ"
    {
        game::parser::MessageTemplate tpl(game::parser::MessageInformation::Configuration, "foo");
        tpl.addCheckInstruction(tpl.iParse + tpl.sAny, 0, "value = $");
        tpl.addVariable("VALUE:RACE.ADJ");

        // Test
        lines[0] = "value = a5";
        values.clear();
        TS_ASSERT(tpl.match(lines, MockDataInterface(), values));
        TS_ASSERT_EQUALS(values.size(), 1U);
        TS_ASSERT_EQUALS(values[0], "5");

        // Test (failure to interpret the value produces "" value and does not fail the parse!)
        lines[0] = "value = 77";
        values.clear();
        TS_ASSERT(tpl.match(lines, MockDataInterface(), values));
        TS_ASSERT_EQUALS(values.size(), 1U);
        TS_ASSERT_EQUALS(values[0], "");

        // Test (failure to interpret the value produces "" value and does not fail the parse!)
        lines[0] = "value = a8+!";
        values.clear();
        TS_ASSERT(tpl.match(lines, MockDataInterface(), values));
        TS_ASSERT_EQUALS(values.size(), 1U);
        TS_ASSERT_EQUALS(values[0], "");
    }

    // "RACE.SHORT"
    {
        game::parser::MessageTemplate tpl(game::parser::MessageInformation::Configuration, "foo");
        tpl.addCheckInstruction(tpl.iParse + tpl.sAny, 0, "value = $");
        tpl.addVariable("VALUE:RACE.SHORT");

        // Test
        lines[0] = "value = s14";
        values.clear();
        TS_ASSERT(tpl.match(lines, MockDataInterface(), values));
        TS_ASSERT_EQUALS(values.size(), 1U);
        TS_ASSERT_EQUALS(values[0], "14");

        // Test (failure to interpret the value produces "" value and does not fail the parse!)
        lines[0] = "value = 77";
        values.clear();
        TS_ASSERT(tpl.match(lines, MockDataInterface(), values));
        TS_ASSERT_EQUALS(values.size(), 1U);
        TS_ASSERT_EQUALS(values[0], "");
    }

    // "HULL"
    {
        game::parser::MessageTemplate tpl(game::parser::MessageInformation::Configuration, "foo");
        tpl.addCheckInstruction(tpl.iParse + tpl.sAny, 0, "value = $");
        tpl.addVariable("VALUE:HULL");

        // Test
        lines[0] = "value = h104";
        values.clear();
        TS_ASSERT(tpl.match(lines, MockDataInterface(), values));
        TS_ASSERT_EQUALS(values.size(), 1U);
        TS_ASSERT_EQUALS(values[0], "104");

        // Test (failure to interpret the value produces "" value and does not fail the parse!)
        lines[0] = "value = 77";
        values.clear();
        TS_ASSERT(tpl.match(lines, MockDataInterface(), values));
        TS_ASSERT_EQUALS(values.size(), 1U);
        TS_ASSERT_EQUALS(values[0], "");
    }

    // "RACE.ADJ+ALLIES"
    {
        game::parser::MessageTemplate tpl(game::parser::MessageInformation::Configuration, "foo");
        tpl.addCheckInstruction(tpl.iParse + tpl.sAny, 0, "value = $");
        tpl.addVariable("VALUE:RACE.ADJ+ALLIES");

        // Test
        lines[0] = "value = a8!+";
        values.clear();
        TS_ASSERT(tpl.match(lines, MockDataInterface(), values));
        TS_ASSERT_EQUALS(values.size(), 1U);
        TS_ASSERT_EQUALS(values[0], "8");
    }
}

/** Test getMessageHeaderInformation(). */
void
TestGameParserMessageTemplate::testGetMessageHeaderInformation()
{
    namespace p = game::parser;

    // Standard case
    {
        p::MessageLines_t msg;
        msg.push_back("(-m1234)<<< Hi Mom>>>");
        msg.push_back("whatever");
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrKind),  int('m'));
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrSubId), int('1'));
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrId),    234);
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrBigId), 1234);
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrAge),   0);
    }

    // Race reference
    {
        p::MessageLines_t msg;
        msg.push_back("(-9c035)<<< Hi Mom>>>");
        msg.push_back("whatever");
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrKind),  int('9'));
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrSubId), int('c'));
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrId),    35);
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrBigId), 35);
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrAge),   0);
    }

    // Old one
    {
        p::MessageLines_t msg;
        msg.push_back("(or3000)<<< Hi Mom>>>");
        msg.push_back("whatever");
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrKind),  int('r'));
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrSubId), int('3'));
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrId),    0);
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrBigId), 3000);
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrAge),   1);
    }

    // Long one
    {
        p::MessageLines_t msg;
        msg.push_back("(-m17000)<<< Hi Mom>>>");
        msg.push_back("whatever");
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrKind),  int('m'));
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrSubId), int('1'));
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrId),    7000);
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrBigId), 17000);
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrAge),   0);
    }

    // Out-of-range case
    {
        p::MessageLines_t msg;
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrKind),  0);
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrSubId), 0);
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrId),    0);
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrBigId), 0);
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrAge),   0);
    }

    // Old host case
    {
        p::MessageLines_t msg;
        msg.push_back("<<< Sub-Space Message >>>");
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrKind),  0);
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrSubId), 0);
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrId),    0);
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrBigId), 0);
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrAge),   0);
    }

    // Broken case 1
    {
        p::MessageLines_t msg;
        msg.push_back("");
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrKind),  0);
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrSubId), 0);
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrId),    0);
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrBigId), 0);
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrAge),   0);
    }

    // Broken case 1
    {
        p::MessageLines_t msg;
        msg.push_back("12345");
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrKind),  0);
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrSubId), 0);
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrId),    0);
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrBigId), 0);
        TS_ASSERT_EQUALS(p::getMessageHeaderInformation(msg, p::MsgHdrAge),   0);
    }
}

/** Test splitMessage(). */
void
TestGameParserMessageTemplate::testSplitMessage()
{
    namespace p = game::parser;
    {
        p::MessageLines_t m;
        p::splitMessage(m, "");
        TS_ASSERT_EQUALS(m.size(), 1U);
        TS_ASSERT_EQUALS(m[0], "");
    }
    {
        p::MessageLines_t m;
        p::splitMessage(m, "xyz");
        TS_ASSERT_EQUALS(m.size(), 1U);
        TS_ASSERT_EQUALS(m[0], "xyz");
    }
    {
        p::MessageLines_t m;
        p::splitMessage(m, "\nxyz\n");
        TS_ASSERT_EQUALS(m.size(), 3U);
        TS_ASSERT_EQUALS(m[0], "");
        TS_ASSERT_EQUALS(m[1], "xyz");
        TS_ASSERT_EQUALS(m[2], "");
    }
}

/** Test parseIntegerValue(). */
void
TestGameParserMessageTemplate::testParseInteger()
{
    TS_ASSERT_EQUALS(game::parser::parseIntegerValue("0"), 0);
    TS_ASSERT_EQUALS(game::parser::parseIntegerValue("99 kt"), 99);
    TS_ASSERT_EQUALS(game::parser::parseIntegerValue("77$"), 77);
    TS_ASSERT_EQUALS(game::parser::parseIntegerValue("0x99"), 0);
    TS_ASSERT_EQUALS(game::parser::parseIntegerValue("-100"), -100);
    TS_ASSERT_EQUALS(game::parser::parseIntegerValue("3.5"), 3);

    TS_ASSERT_EQUALS(game::parser::parseIntegerValue("$"), -1);
    TS_ASSERT_EQUALS(game::parser::parseIntegerValue(""), -1);
}

/** Test match() with metadata information: Kind.
    Also checks extraction of "id". */
void
TestGameParserMessageTemplate::testMatchMeta()
{
    // Build a template
    game::parser::MessageTemplate testee(game::parser::MessageInformation::Planet, "pm");
    testee.addMatchInstruction(game::parser::MessageTemplate::iMatchKind, 'p');
    testee.addValueInstruction(game::parser::MessageTemplate::iValue,     "id");
    testee.addVariable("Id");

    // Verify template
    TS_ASSERT_EQUALS(testee.getContinueFlag(), false);
    TS_ASSERT_EQUALS(testee.getTemplateName(), "pm");
    TS_ASSERT_EQUALS(testee.getMessageType(), game::parser::MessageInformation::Planet);

    TS_ASSERT_EQUALS(testee.getNumVariables(), 1U);
    TS_ASSERT_EQUALS(testee.getVariableName(0), "ID");
    TS_ASSERT_EQUALS(testee.getVariableName(1), "");      // out-of-range

    size_t n = 99;
    TS_ASSERT(testee.getVariableSlotByName("ID", n));     // variables are internally upcased...
    TS_ASSERT_EQUALS(n, 0u);
    TS_ASSERT(!testee.getVariableSlotByName("id", n));    // ...but matched case-sensitively

    TS_ASSERT_EQUALS(testee.getNumRestrictions(), 1U);
    TS_ASSERT_EQUALS(testee.getNumWildcards(), 1U);

    // Match successfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-p0363)<<< Report >>>");
        m.push_back("etc...");

        std::vector<String_t> result;
        TS_ASSERT(testee.match(m, NullDataInterface(), result));
        TS_ASSERT_EQUALS(result.size(), 1U);
        TS_ASSERT_EQUALS(result[0], "363");
    }

    // Match unsuccessfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-s0015)<<< Report >>>");
        m.push_back("etc...");

        std::vector<String_t> result;
        TS_ASSERT(!testee.match(m, NullDataInterface(), result));
    }
}

/** Test match() with metadata information: SubId.
    Also test extraction of player. */
void
TestGameParserMessageTemplate::testMatchMetaSubId()
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
        TS_ASSERT(testee.match(m, NullDataInterface(7), result));
        TS_ASSERT_EQUALS(result.size(), 1U);
        TS_ASSERT_EQUALS(result[0], "7");
    }

    // Match unsuccessfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-90111)<<< Report >>>");
        m.push_back("etc...");

        std::vector<String_t> result;
        TS_ASSERT(!testee.match(m, NullDataInterface(7), result));
    }
}

/** Test match() with metadata information: BigId.
    Also test production of fixed values. */
void
TestGameParserMessageTemplate::testMatchMetaBigId()
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
        TS_ASSERT(testee.match(m, NullDataInterface(), result));
        TS_ASSERT_EQUALS(result.size(), 1U);
        TS_ASSERT_EQUALS(result[0], "49");
    }

    // Match unsuccessfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-9c1234)<<< Report >>>");
        m.push_back("etc...");

        std::vector<String_t> result;
        TS_ASSERT(!testee.match(m, NullDataInterface(), result));
    }
}

/** Test match() with check/fail/find.
    Also test extraction of id/bigid/subid. */
void
TestGameParserMessageTemplate::testMatchCheck()
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
    TS_ASSERT_EQUALS(testee.getNumVariables(), 4U);
    TS_ASSERT_EQUALS(testee.getVariableName(0), "FIND");
    TS_ASSERT_EQUALS(testee.getVariableName(1), "ID");
    TS_ASSERT_EQUALS(testee.getVariableName(2), "BIGID");
    TS_ASSERT_EQUALS(testee.getVariableName(3), "SUBID");
    TS_ASSERT_EQUALS(testee.getNumRestrictions(), 3U);
    TS_ASSERT_EQUALS(testee.getNumWildcards(), 4U);

    // Match successfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-s0599)<<< Report >>>");
        m.push_back("check this");

        std::vector<String_t> result;
        TS_ASSERT(testee.match(m, NullDataInterface(), result));
        TS_ASSERT_EQUALS(result.size(), 4U);
        TS_ASSERT_EQUALS(result[0], "0");
        TS_ASSERT_EQUALS(result[1], "599");
        TS_ASSERT_EQUALS(result[2], "599");
        TS_ASSERT_EQUALS(result[3], "0");
    }

    // Match successfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-sc456)<<< find this >>>");
        m.push_back("check this");

        std::vector<String_t> result;
        TS_ASSERT(testee.match(m, NullDataInterface(), result));
        TS_ASSERT_EQUALS(result.size(), 4U);
        TS_ASSERT_EQUALS(result[0], "1");
        TS_ASSERT_EQUALS(result[1], "456");
        TS_ASSERT_EQUALS(result[2], "456");
        TS_ASSERT_EQUALS(result[3], "12");
    }

    // Match successfully, bad sub Id
    {
        game::parser::MessageLines_t m;
        m.push_back("(-s*456)<<< check this >>>");
        m.push_back("find this");

        std::vector<String_t> result;
        TS_ASSERT(testee.match(m, NullDataInterface(), result));
        TS_ASSERT_EQUALS(result.size(), 4U);
        TS_ASSERT_EQUALS(result[0], "1");
        TS_ASSERT_EQUALS(result[1], "456");
        TS_ASSERT_EQUALS(result[2], "456");
        TS_ASSERT_EQUALS(result[3], "0");
    }

    // Match unsuccessfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-sc456)<<< blank >>>");
        m.push_back("also blank");

        std::vector<String_t> result;
        TS_ASSERT(!testee.match(m, NullDataInterface(), result));
    }

    // Match unsuccessfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-sc456)<<< blank >>>");
        m.push_back("fail this");
        m.push_back("check this");

        std::vector<String_t> result;
        TS_ASSERT(!testee.match(m, NullDataInterface(), result));
    }
}

/** Test match() with parse/fail and value extration. */
void
TestGameParserMessageTemplate::testMatchParseValues()
{
    // Build a template
    game::parser::MessageTemplate testee(game::parser::MessageInformation::Ship, "ff");
    testee.addCheckInstruction(game::parser::MessageTemplate::iParse, 0, "parse this $ : $");
    testee.addVariables("a:X100,b");
    testee.addCheckInstruction(game::parser::MessageTemplate::iFail + game::parser::MessageTemplate::sRelative, 1, "fail this");

    // Verify template
    TS_ASSERT_EQUALS(testee.getNumVariables(), 2U);
    TS_ASSERT_EQUALS(testee.getVariableName(0), "A");
    TS_ASSERT_EQUALS(testee.getVariableName(1), "B");
    TS_ASSERT_EQUALS(testee.getNumRestrictions(), 2U);
    TS_ASSERT_EQUALS(testee.getNumWildcards(), 2U);

    // Match successfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-s0100)<<< Title >>>");
        m.push_back("parse this 100 : 1");

        std::vector<String_t> result;
        TS_ASSERT(testee.match(m, NullDataInterface(), result));
        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT_EQUALS(result[0], "10000");
        TS_ASSERT_EQUALS(result[1], "1");
    }

    // Match successfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-s0100)<<< Title >>>");
        m.push_back("fail this");                 // not hit due to sRelative, 1
        m.push_back("parse this 3.14 : 1");

        std::vector<String_t> result;
        TS_ASSERT(testee.match(m, NullDataInterface(), result));
        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT_EQUALS(result[0], "314");
        TS_ASSERT_EQUALS(result[1], "1");
    }

    // Match unsuccessfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-s0100)<<< Title >>>");
        m.push_back("parse this 2.71 : 1");
        m.push_back("fail this");                 // hits

        std::vector<String_t> result;
        TS_ASSERT(!testee.match(m, NullDataInterface(), result));
    }
}

/** Test parsing an array. */
void
TestGameParserMessageTemplate::testMatchArray()
{
    // Build a template
    game::parser::MessageTemplate testee(game::parser::MessageInformation::Ship, "aa");
    testee.addCheckInstruction(game::parser::MessageTemplate::iArray, 0, "$=$:$");
    testee.addVariables("Index:Race.Adj,Attack,Defend");

    // Verify template
    TS_ASSERT_EQUALS(testee.getNumVariables(), 3U);
    TS_ASSERT_EQUALS(testee.getVariableName(0), "INDEX");
    TS_ASSERT_EQUALS(testee.getVariableName(1), "ATTACK");
    TS_ASSERT_EQUALS(testee.getVariableName(2), "DEFEND");
    TS_ASSERT_EQUALS(testee.getNumRestrictions(), 1U);
    TS_ASSERT_EQUALS(testee.getNumWildcards(), 3U);

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
        TS_ASSERT(testee.match(m, MockDataInterface(), result));
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT_EQUALS(result[0], "");             // Index is not returned
        TS_ASSERT_EQUALS(result[1], "1,10,3,,,,,,,,");
        TS_ASSERT_EQUALS(result[2], "7,1,1,,,,,,,,");
    }

    // Match sparsely
    {
        game::parser::MessageLines_t m;
        m.push_back("(-s0100)<<< Title >>>");
        m.push_back("a1=1: 7");
        m.push_back("a4 = 3 : 2");

        std::vector<String_t> result;
        TS_ASSERT(testee.match(m, MockDataInterface(), result));
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT_EQUALS(result[0], "");             // Index is not returned
        TS_ASSERT_EQUALS(result[1], "1,,,3,,,,,,,");
        TS_ASSERT_EQUALS(result[2], "7,,,2,,,,,,,");
    }

    // Match unsuccessfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-s0100)<<< Title >>>");
        m.push_back("whatever");

        std::vector<String_t> result;
        TS_ASSERT(!testee.match(m, MockDataInterface(), result));
    }
}

/** Test parsing an array at a fixed place. */
void
TestGameParserMessageTemplate::testMatchArrayFixed()
{
    // Build a template
    game::parser::MessageTemplate testee(game::parser::MessageInformation::Ship, "aa");
    testee.addCheckInstruction(game::parser::MessageTemplate::iCheck, 0, "check this");
    testee.addCheckInstruction(game::parser::MessageTemplate::iArray + game::parser::MessageTemplate::sRelative, 1, "$=$:$");
    testee.addVariables("Index:Race.Adj,Attack,Defend");

    // Verify template
    TS_ASSERT_EQUALS(testee.getNumVariables(), 3U);
    TS_ASSERT_EQUALS(testee.getVariableName(0), "INDEX");
    TS_ASSERT_EQUALS(testee.getVariableName(1), "ATTACK");
    TS_ASSERT_EQUALS(testee.getVariableName(2), "DEFEND");
    TS_ASSERT_EQUALS(testee.getNumRestrictions(), 2U);
    TS_ASSERT_EQUALS(testee.getNumWildcards(), 3U);

    // Match sparsely
    {
        game::parser::MessageLines_t m;
        m.push_back("(-s0100)<<< Title >>>");
        m.push_back("a2=1: 7");
        m.push_back("check this");
        m.push_back("a1=1: 7");
        m.push_back("a4 = 3 : 2");

        std::vector<String_t> result;
        TS_ASSERT(testee.match(m, MockDataInterface(), result));
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT_EQUALS(result[0], "");             // Index is not returned
        TS_ASSERT_EQUALS(result[1], "1,,,3,,,,,,,");
        TS_ASSERT_EQUALS(result[2], "7,,,2,,,,,,,");
    }

    // Match unsuccessfully
    {
        game::parser::MessageLines_t m;
        m.push_back("(-s0100)<<< Title >>>");
        m.push_back("a2=1: 7");
        m.push_back("check this");

        std::vector<String_t> result;
        TS_ASSERT(!testee.match(m, MockDataInterface(), result));
    }
}

