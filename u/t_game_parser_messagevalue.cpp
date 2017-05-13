/**
  *  \file u/t_game_parser_messagevalue.cpp
  *  \brief Test for game::parser::MessageValue
  */

#include "game/parser/messagevalue.hpp"

#include "t_game_parser.hpp"
#include "afl/string/nulltranslator.hpp"

/** Test values. */
void
TestGameParserMessageValue::testValues()
{
    // General
    game::parser::MessageValue<int,int> genValue(99,33);
    TS_ASSERT_EQUALS(genValue.getIndex(), 99);
    TS_ASSERT_EQUALS(genValue.getValue(), 33);
    genValue.setValue(22);
    TS_ASSERT_EQUALS(genValue.getValue(), 22);

    // String Value
    game::parser::MessageStringValue_t stringValue(game::parser::ms_Name, "N");
    TS_ASSERT_EQUALS(stringValue.getIndex(), game::parser::ms_Name);
    TS_ASSERT_EQUALS(stringValue.getValue(), "N");
    stringValue.setValue("M");
    TS_ASSERT_EQUALS(stringValue.getValue(), "M");

    // Integer Value
    game::parser::MessageIntegerValue_t integerValue(game::parser::mi_X, 2000);
    TS_ASSERT_EQUALS(integerValue.getIndex(), game::parser::mi_X);
    TS_ASSERT_EQUALS(integerValue.getValue(), 2000);
    integerValue.setValue(2350);
    TS_ASSERT_EQUALS(integerValue.getValue(), 2350);

    // Config Value
    game::parser::MessageConfigurationValue_t configValue("Foo", "Bar");
    TS_ASSERT_EQUALS(configValue.getIndex(), "Foo");
    TS_ASSERT_EQUALS(configValue.getValue(), "Bar");
    configValue.setValue("Baz");
    TS_ASSERT_EQUALS(configValue.getValue(), "Baz");

    // Score Value
    game::parser::MessageScoreValue_t scoreValue(11, 12);
    TS_ASSERT_EQUALS(scoreValue.getIndex(), 11);
    TS_ASSERT_EQUALS(scoreValue.getValue(), 12);
    scoreValue.setValue(13);
    TS_ASSERT_EQUALS(scoreValue.getValue(), 13);
}

/** Test names.
    Verifies that all names are disjoint and different from "?". */
void
TestGameParserMessageValue::testNames()
{
    afl::string::NullTranslator tx;

    // String names
    {
        typedef game::parser::MessageStringIndex Index_t;
        for (Index_t i = Index_t(0); i != game::parser::ms_Max; i = Index_t(i+1)) {
            String_t thisName = game::parser::getNameFromIndex(i, tx);
            TS_ASSERT_DIFFERS(thisName, "?");
            TS_ASSERT_DIFFERS(thisName, "");
            for (Index_t j = Index_t(0); j != i; j = Index_t(j+1)) {
                TS_ASSERT_DIFFERS(thisName, game::parser::getNameFromIndex(j, tx));
            }
        }
    }

    // Integer names
    {
        typedef game::parser::MessageIntegerIndex Index_t;
        for (Index_t i = Index_t(0); i != game::parser::mi_Max; i = Index_t(i+1)) {
            String_t thisName = game::parser::getNameFromIndex(i, tx);
            TS_ASSERT_DIFFERS(thisName, "?");
            TS_ASSERT_DIFFERS(thisName, "");
            for (Index_t j = Index_t(0); j != i; j = Index_t(j+1)) {
                TS_ASSERT_DIFFERS(thisName, game::parser::getNameFromIndex(j, tx));
            }
        }
    }

    // Max values resolve to "?"
    TS_ASSERT_EQUALS(game::parser::getNameFromIndex(game::parser::mi_Max, tx), "?");
    TS_ASSERT_EQUALS(game::parser::getNameFromIndex(game::parser::ms_Max, tx), "?");
}

/** Test keywords. */
void
TestGameParserMessageValue::testKeywords()
{
    // Strings
    TS_ASSERT_EQUALS(game::parser::getStringIndexFromKeyword("FCODE"), game::parser::ms_FriendlyCode);
    TS_ASSERT_EQUALS(game::parser::getStringIndexFromKeyword("NAME"),  game::parser::ms_Name);
    TS_ASSERT_EQUALS(game::parser::getStringIndexFromKeyword("fcode"), game::parser::ms_Max);
    TS_ASSERT_EQUALS(game::parser::getStringIndexFromKeyword(""),      game::parser::ms_Max);
    TS_ASSERT_EQUALS(game::parser::getStringIndexFromKeyword("BASE"),  game::parser::ms_Max);

    // Integers (random sample)
    TS_ASSERT_EQUALS(game::parser::getIntegerIndexFromKeyword("HULL"),    game::parser::mi_ShipHull);
    TS_ASSERT_EQUALS(game::parser::getIntegerIndexFromKeyword("MINES"),   game::parser::mi_PlanetMines);
    TS_ASSERT_EQUALS(game::parser::getIntegerIndexFromKeyword("ADDED.D"), game::parser::mi_PlanetAddedD);
    TS_ASSERT_EQUALS(game::parser::getIntegerIndexFromKeyword("Added.D"), game::parser::mi_Max);
    TS_ASSERT_EQUALS(game::parser::getIntegerIndexFromKeyword(""),        game::parser::mi_Max);
    TS_ASSERT_EQUALS(game::parser::getIntegerIndexFromKeyword("FCODE"),   game::parser::mi_Max);
}

