/**
  *  \file test/game/parser/messageinformationtest.cpp
  *  \brief Test for game::parser::MessageInformation
  */

#include "game/parser/messageinformation.hpp"
#include "afl/test/testrunner.hpp"

using game::parser::MessageInformation;

/** Test general behaviour with an object. */
AFL_TEST("game.parser.MessageInformation:basics", a)
{
    // Verify initial state
    MessageInformation testee(MessageInformation::Ship, 77, 12);
    a.checkEqual("01. getObjectType", testee.getObjectType(), MessageInformation::Ship);
    a.checkEqual("02. getObjectId", testee.getObjectId(), 77);
    a.checkEqual("03. getTurnNumber", testee.getTurnNumber(), 12);
    a.check("04. empty", testee.begin() == testee.end());
    a.checkEqual("05. getObjectReference", testee.getObjectReference(), game::Reference(game::Reference::Ship, 77));

    // Add information
    testee.addValue(game::parser::mi_ShipHull, 15);
    testee.addValue(game::parser::ms_Name, "NN");
    testee.addValue(game::parser::mi_ShipRemoteFlag, 1);

    // Verify
    MessageInformation::Iterator_t it = testee.begin();
    a.check("11. not end", it != testee.end());
    a.checkNonNull("12. int", dynamic_cast<game::parser::MessageIntegerValue_t*>(*it));
    a.checkEqual("13. int", dynamic_cast<game::parser::MessageIntegerValue_t*>(*it)->getIndex(), game::parser::mi_ShipHull);
    a.checkEqual("14. int", dynamic_cast<game::parser::MessageIntegerValue_t*>(*it)->getValue(), 15);

    ++it;
    a.check("21. not end", it != testee.end());
    a.checkNonNull("22. string", dynamic_cast<game::parser::MessageStringValue_t*>(*it));
    a.checkEqual("23. string", dynamic_cast<game::parser::MessageStringValue_t*>(*it)->getIndex(), game::parser::ms_Name);
    a.checkEqual("24. string", dynamic_cast<game::parser::MessageStringValue_t*>(*it)->getValue(), "NN");

    ++it;
    a.check("31. not end", it != testee.end());
    a.checkNonNull("32. int", dynamic_cast<game::parser::MessageIntegerValue_t*>(*it));
    a.checkEqual("33. int", dynamic_cast<game::parser::MessageIntegerValue_t*>(*it)->getIndex(), game::parser::mi_ShipRemoteFlag);
    a.checkEqual("34. int", dynamic_cast<game::parser::MessageIntegerValue_t*>(*it)->getValue(), 1);

    ++it;
    a.check("41. end", it == testee.end());
}

/** Test behaviour with a PlayerScore. */
AFL_TEST("game.parser.MessageInformation:PlayerScore", a)
{
    // Verify initial state
    MessageInformation testee(MessageInformation::PlayerScore, 1000, 3);
    a.checkEqual("01. getObjectType", testee.getObjectType(), MessageInformation::PlayerScore);
    a.checkEqual("02. getObjectId", testee.getObjectId(), 1000);
    a.checkEqual("03. getTurnNumber", testee.getTurnNumber(), 3);
    a.check("04. empty", testee.begin() == testee.end());
    a.checkEqual("05. getObjectReference", testee.getObjectReference(), game::Reference());

    // Add
    testee.addScoreValue(3, 105);
    testee.addScoreValue(4, 291);

    // Verify
    MessageInformation::Iterator_t it = testee.begin();
    a.check("11. not end", it != testee.end());
    a.checkNonNull("12. score type", dynamic_cast<game::parser::MessageScoreValue_t*>(*it));
    a.checkEqual("13. score index", dynamic_cast<game::parser::MessageScoreValue_t*>(*it)->getIndex(), 3);
    a.checkEqual("14. score value", dynamic_cast<game::parser::MessageScoreValue_t*>(*it)->getValue(), 105);

    ++it;
    a.check("21. not end", it != testee.end());
    a.checkNonNull("22. score type", dynamic_cast<game::parser::MessageScoreValue_t*>(*it));
    a.checkEqual("23. score index", dynamic_cast<game::parser::MessageScoreValue_t*>(*it)->getIndex(), 4);
    a.checkEqual("24. score value", dynamic_cast<game::parser::MessageScoreValue_t*>(*it)->getValue(), 291);

    ++it;
    a.check("31. end", it == testee.end());
}

/** Test behaviour with a configuration data. */
AFL_TEST("game.parser.MessageInformation:Configuration", a)
{
    // Verify initial state
    MessageInformation testee(MessageInformation::Configuration, 0, 5);
    a.checkEqual("01. getObjectType", testee.getObjectType(), MessageInformation::Configuration);
    a.checkEqual("02. getObjectId", testee.getObjectId(), 0);
    a.checkEqual("03. getTurnNumber", testee.getTurnNumber(), 5);
    a.check("04. empty", testee.begin() == testee.end());
    a.checkEqual("05. getObjectReference", testee.getObjectReference(), game::Reference());

    // Add
    testee.addConfigurationValue("GameName", "The Game");

    // Verify
    MessageInformation::Iterator_t it = testee.begin();
    a.check("11. not end", it != testee.end());
    a.checkNonNull("12. config type", dynamic_cast<game::parser::MessageConfigurationValue_t*>(*it));
    a.checkEqual("13. config index", dynamic_cast<game::parser::MessageConfigurationValue_t*>(*it)->getIndex(), "GameName");
    a.checkEqual("14. config value", dynamic_cast<game::parser::MessageConfigurationValue_t*>(*it)->getValue(), "The Game");

    ++it;
    a.check("21. end", it == testee.end());
}

/** Test getValue(). */
AFL_TEST("game.parser.MessageInformation:getValue", a)
{
    MessageInformation testee(MessageInformation::Ship, 77, 12);
    testee.addValue(game::parser::mi_ShipHull, 15);
    testee.addValue(game::parser::ms_Name, "NN");
    testee.addValue(game::parser::mi_ShipRemoteFlag, 1);
    a.checkEqual("01. getObjectReference", testee.getObjectReference(), game::Reference(game::Reference::Ship, 77));

    // Normal
    a.checkEqual("11. normal", testee.getValue(game::parser::mi_ShipHull).orElse(0), 15);

    // Range check, success
    a.checkEqual("21. range check success", testee.getValue(game::parser::mi_ShipHull, 0, 100).orElse(0), 15);

    // Range check, failure
    a.check("31. range check failure", !testee.getValue(game::parser::mi_ShipHull, 0, 10).isValid());

    // String
    a.checkEqual("41. string", testee.getValue(game::parser::ms_Name).orElse(""), "NN");

    // Missing index: integer
    a.check("51. missing integer", !testee.getValue(game::parser::mi_Owner).isValid());

    // Missing index: string
    a.check("61. missing string", !testee.getValue(game::parser::ms_DrawingComment).isValid());
}
