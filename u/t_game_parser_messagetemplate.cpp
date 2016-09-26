/**
  *  \file u/t_game_parser_messagetemplate.cpp
  *  \brief Test for game::parser::MessageTemplate
  */

#include "game/parser/messagetemplate.hpp"

#include "t_game_parser.hpp"
#include "game/parser/datainterface.hpp"

namespace {
    class NullDataInterface : public game::parser::DataInterface {
     public:
        virtual int getPlayerNumber() const
            { return 0; }
        virtual int parseName(Name /*which*/, const String_t& /*name*/) const
            { return 0; }
        virtual String_t expandRaceNames(String_t name) const
            { return name; }
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
