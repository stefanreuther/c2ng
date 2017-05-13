/**
  *  \file u/t_game_parser_messageinformation.cpp
  *  \brief Test for game::parser::MessageInformation
  */

#include "game/parser/messageinformation.hpp"

#include "t_game_parser.hpp"

using game::parser::MessageInformation;

/** Test general behaviour with an object. */
void
TestGameParserMessageInformation::testIt()
{
    // Verify initial state
    MessageInformation testee(MessageInformation::Ship, 77, 12);
    TS_ASSERT_EQUALS(testee.getObjectType(), MessageInformation::Ship);
    TS_ASSERT_EQUALS(testee.getObjectId(), 77);
    TS_ASSERT_EQUALS(testee.getTurnNumber(), 12);
    TS_ASSERT_EQUALS(testee.begin(), testee.end());

    // Add information
    testee.addValue(game::parser::mi_ShipHull, 15);
    testee.addValue(game::parser::ms_Name, "NN");
    testee.addValue(game::parser::mi_ShipRemoteFlag, 1);

    // Verify
    MessageInformation::Iterator_t it = testee.begin();
    TS_ASSERT(it != testee.end());
    TS_ASSERT(dynamic_cast<game::parser::MessageIntegerValue_t*>(*it) != 0);
    TS_ASSERT_EQUALS(dynamic_cast<game::parser::MessageIntegerValue_t*>(*it)->getIndex(), game::parser::mi_ShipHull);
    TS_ASSERT_EQUALS(dynamic_cast<game::parser::MessageIntegerValue_t*>(*it)->getValue(), 15);

    ++it;
    TS_ASSERT(it != testee.end());
    TS_ASSERT(dynamic_cast<game::parser::MessageStringValue_t*>(*it) != 0);
    TS_ASSERT_EQUALS(dynamic_cast<game::parser::MessageStringValue_t*>(*it)->getIndex(), game::parser::ms_Name);
    TS_ASSERT_EQUALS(dynamic_cast<game::parser::MessageStringValue_t*>(*it)->getValue(), "NN");

    ++it;
    TS_ASSERT(it != testee.end());
    TS_ASSERT(dynamic_cast<game::parser::MessageIntegerValue_t*>(*it) != 0);
    TS_ASSERT_EQUALS(dynamic_cast<game::parser::MessageIntegerValue_t*>(*it)->getIndex(), game::parser::mi_ShipRemoteFlag);
    TS_ASSERT_EQUALS(dynamic_cast<game::parser::MessageIntegerValue_t*>(*it)->getValue(), 1);

    ++it;
    TS_ASSERT_EQUALS(it, testee.end());
}

/** Test behaviour with a PlayerScore. */
void
TestGameParserMessageInformation::testPlayerScore()
{
    // Verify initial state
    MessageInformation testee(MessageInformation::PlayerScore, 1000, 3);
    TS_ASSERT_EQUALS(testee.getObjectType(), MessageInformation::PlayerScore);
    TS_ASSERT_EQUALS(testee.getObjectId(), 1000);
    TS_ASSERT_EQUALS(testee.getTurnNumber(), 3);
    TS_ASSERT_EQUALS(testee.begin(), testee.end());

    // Add
    testee.addScoreValue(3, 105);
    testee.addScoreValue(4, 291);

    // Verify
    MessageInformation::Iterator_t it = testee.begin();
    TS_ASSERT(it != testee.end());
    TS_ASSERT(dynamic_cast<game::parser::MessageScoreValue_t*>(*it) != 0);
    TS_ASSERT_EQUALS(dynamic_cast<game::parser::MessageScoreValue_t*>(*it)->getIndex(), 3);
    TS_ASSERT_EQUALS(dynamic_cast<game::parser::MessageScoreValue_t*>(*it)->getValue(), 105);

    ++it;
    TS_ASSERT(it != testee.end());
    TS_ASSERT(dynamic_cast<game::parser::MessageScoreValue_t*>(*it) != 0);
    TS_ASSERT_EQUALS(dynamic_cast<game::parser::MessageScoreValue_t*>(*it)->getIndex(), 4);
    TS_ASSERT_EQUALS(dynamic_cast<game::parser::MessageScoreValue_t*>(*it)->getValue(), 291);

    ++it;
    TS_ASSERT_EQUALS(it, testee.end());
}

/** Test behaviour with a configuration data. */
void
TestGameParserMessageInformation::testConfiguration()
{
    // Verify initial state
    MessageInformation testee(MessageInformation::Configuration, 0, 5);
    TS_ASSERT_EQUALS(testee.getObjectType(), MessageInformation::Configuration);
    TS_ASSERT_EQUALS(testee.getObjectId(), 0);
    TS_ASSERT_EQUALS(testee.getTurnNumber(), 5);
    TS_ASSERT_EQUALS(testee.begin(), testee.end());

    // Add
    testee.addConfigurationValue("GameName", "The Game");

    // Verify
    MessageInformation::Iterator_t it = testee.begin();
    TS_ASSERT(it != testee.end());
    TS_ASSERT(dynamic_cast<game::parser::MessageConfigurationValue_t*>(*it) != 0);
    TS_ASSERT_EQUALS(dynamic_cast<game::parser::MessageConfigurationValue_t*>(*it)->getIndex(), "GameName");
    TS_ASSERT_EQUALS(dynamic_cast<game::parser::MessageConfigurationValue_t*>(*it)->getValue(), "The Game");

    ++it;
    TS_ASSERT_EQUALS(it, testee.end());
}

