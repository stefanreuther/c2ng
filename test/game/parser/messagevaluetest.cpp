/**
  *  \file test/game/parser/messagevaluetest.cpp
  *  \brief Test for game::parser::MessageValue
  */

#include "game/parser/messagevalue.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

/** Test values. */
AFL_TEST("game.parser.MessageValue:value-types", a)
{
    // General
    game::parser::MessageValue<int,int> genValue(99,33);
    a.checkEqual("01", genValue.getIndex(), 99);
    a.checkEqual("02", genValue.getValue(), 33);
    genValue.setValue(22);
    a.checkEqual("03", genValue.getValue(), 22);

    // String Value
    game::parser::MessageStringValue_t stringValue(game::parser::ms_Name, "N");
    a.checkEqual("11", stringValue.getIndex(), game::parser::ms_Name);
    a.checkEqual("12", stringValue.getValue(), "N");
    stringValue.setValue("M");
    a.checkEqual("13", stringValue.getValue(), "M");

    // Integer Value
    game::parser::MessageIntegerValue_t integerValue(game::parser::mi_X, 2000);
    a.checkEqual("21", integerValue.getIndex(), game::parser::mi_X);
    a.checkEqual("22", integerValue.getValue(), 2000);
    integerValue.setValue(2350);
    a.checkEqual("23", integerValue.getValue(), 2350);

    // Config Value
    game::parser::MessageConfigurationValue_t configValue("Foo", "Bar");
    a.checkEqual("31", configValue.getIndex(), "Foo");
    a.checkEqual("32", configValue.getValue(), "Bar");
    configValue.setValue("Baz");
    a.checkEqual("33", configValue.getValue(), "Baz");

    // Score Value
    game::parser::MessageScoreValue_t scoreValue(11, 12);
    a.checkEqual("41", scoreValue.getIndex(), 11);
    a.checkEqual("42", scoreValue.getValue(), 12);
    scoreValue.setValue(13);
    a.checkEqual("43", scoreValue.getValue(), 13);
}

/** Test names.
    Verifies that all names are disjoint and different from "?". */
AFL_TEST("game.parser.MessageValue:getNameFromIndex", a)
{
    afl::string::NullTranslator tx;

    // String names
    {
        typedef game::parser::MessageStringIndex Index_t;
        for (Index_t i = Index_t(0); i != game::parser::ms_Max; i = Index_t(i+1)) {
            String_t thisName = game::parser::getNameFromIndex(i, tx);
            a.checkDifferent("01", thisName, "?");
            a.checkDifferent("02", thisName, "");
            for (Index_t j = Index_t(0); j != i; j = Index_t(j+1)) {
                a.checkDifferent("03", thisName, game::parser::getNameFromIndex(j, tx));
            }
        }
    }

    // Integer names
    {
        typedef game::parser::MessageIntegerIndex Index_t;
        for (Index_t i = Index_t(0); i != game::parser::mi_Max; i = Index_t(i+1)) {
            String_t thisName = game::parser::getNameFromIndex(i, tx);
            a.checkDifferent("11", thisName, "?");
            a.checkDifferent("12", thisName, "");
            for (Index_t j = Index_t(0); j != i; j = Index_t(j+1)) {
                a.checkDifferent("13", thisName, game::parser::getNameFromIndex(j, tx));
            }
        }
    }

    // Max values resolve to "?"
    a.checkEqual("21", game::parser::getNameFromIndex(game::parser::mi_Max, tx), "?");
    a.checkEqual("22", game::parser::getNameFromIndex(game::parser::ms_Max, tx), "?");
}

/** Test keywords. */
AFL_TEST("game.parser.MessageValue:getStringIndexFromKeyword", a)
{
    a.checkEqual("01", game::parser::getStringIndexFromKeyword("FCODE"), game::parser::ms_FriendlyCode);
    a.checkEqual("02", game::parser::getStringIndexFromKeyword("NAME"),  game::parser::ms_Name);
    a.checkEqual("03", game::parser::getStringIndexFromKeyword("fcode"), game::parser::ms_Max);
    a.checkEqual("04", game::parser::getStringIndexFromKeyword(""),      game::parser::ms_Max);
    a.checkEqual("05", game::parser::getStringIndexFromKeyword("BASE"),  game::parser::ms_Max);
}

AFL_TEST("game.parser.MessageValue:getIntegerIndexFromKeyword", a)
{
    a.checkEqual("01", game::parser::getIntegerIndexFromKeyword("HULL"),    game::parser::mi_ShipHull);
    a.checkEqual("02", game::parser::getIntegerIndexFromKeyword("MINES"),   game::parser::mi_PlanetMines);
    a.checkEqual("03", game::parser::getIntegerIndexFromKeyword("ADDED.D"), game::parser::mi_PlanetAddedD);
    a.checkEqual("04", game::parser::getIntegerIndexFromKeyword("Added.D"), game::parser::mi_Max);
    a.checkEqual("05", game::parser::getIntegerIndexFromKeyword(""),        game::parser::mi_Max);
    a.checkEqual("06", game::parser::getIntegerIndexFromKeyword("FCODE"),   game::parser::mi_Max);
}
