/**
  *  \file u/t_game_msg_outbox.cpp
  *  \brief Test for game::msg::Outbox
  */

#include "game/msg/outbox.hpp"

#include "t_game_msg.hpp"
#include "game/playerlist.hpp"
#include "afl/string/nulltranslator.hpp"

/** Simple test. */
void
TestGameMsgOutbox::testIt()
{
    // Environment
    afl::string::NullTranslator tx;
    game::PlayerList players;
    for (int i = 1; i <= 11; ++i) {
        players.create(i);
    }
    players.create(3)->setName(game::Player::LongName, "Long Three");

    // Create an outbox with two messages
    game::msg::Outbox testee;
    testee.addMessage(3, "text", game::PlayerSet_t(4));
    testee.addMessage(9, "text2", game::PlayerSet_t(5)+6);

    // Verify properties
    TS_ASSERT_EQUALS(testee.getNumMessages(), 2U);
    TS_ASSERT_EQUALS(testee.getMessageText(0, tx, players),
                     "<<< Sub Space Message >>>\n"
                     "FROM: Long Three\n"
                     "TO: Player 4\n"
                     "text");
    TS_ASSERT_EQUALS(testee.getMessageText(1, tx, players),
                     "<<< Sub Space Message >>>\n"
                     "FROM: Player 9\n"
                     "TO: 5 6\n"
                     "CC: 5 6\n"
                     "text2");
    TS_ASSERT_EQUALS(testee.getMessageHeading(0, tx, players), "To: Player 4");
    TS_ASSERT_EQUALS(testee.getMessageHeading(1, tx, players), "To: 5 6");

    // Border cases/unimplemented
    TS_ASSERT_EQUALS(testee.getMessageText(99, tx, players), "");
    TS_ASSERT_EQUALS(testee.getMessageHeading(99, tx, players), "");
    TS_ASSERT_EQUALS(testee.getMessageTurnNumber(0), 0);
}

/** Test merging. */
void
TestGameMsgOutbox::testMerge()
{
    // Environment
    afl::string::NullTranslator tx;
    game::PlayerList players;
    for (int i = 1; i <= 11; ++i) {
        players.create(i);
    }
    players.create(3)->setName(game::Player::LongName, "Long Three");

    // Add. These messages will all be merged
    game::msg::Outbox testee;
    testee.addMessageFromFile(4, "<CC: The Frogs\ntext", game::PlayerSet_t(4));
    testee.addMessageFromFile(4, "text", game::PlayerSet_t(5));
    testee.addMessageFromFile(4, "CC: 4\ntext", game::PlayerSet_t(6));
    testee.addMessageFromFile(4, "  <<< Universal Message >>>\ntext", game::PlayerSet_t(7));

    // Verify
    TS_ASSERT_EQUALS(testee.getNumMessages(), 1U);
    TS_ASSERT_EQUALS(testee.getMessageHeading(0, tx, players), "To: 4 5 6 7");
    TS_ASSERT_EQUALS(testee.getMessageSender(0), 4);
    TS_ASSERT_EQUALS(testee.getMessageReceiverMask(0).toInteger(), 0xF0U);
    TS_ASSERT_EQUALS(testee.getMessageRawText(0), "text");
    TS_ASSERT_EQUALS(testee.getMessageSendPrefix(0, 4, tx, players), "<CC: 5 6 7\n");
    TS_ASSERT_EQUALS(testee.getMessageSendPrefix(0, 5, tx, players), "CC: 4 6 7\n");
}

