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
    TS_ASSERT_EQUALS(testee.getMessageReceivers(0).toInteger(), 0xF0U);
    TS_ASSERT_EQUALS(testee.getMessageRawText(0), "text");
    TS_ASSERT_EQUALS(testee.getMessageSendPrefix(0, 4, tx, players), "<CC: 5 6 7\n");
    TS_ASSERT_EQUALS(testee.getMessageSendPrefix(0, 5, tx, players), "CC: 4 6 7\n");
}

/** Test add/delete. */
void
TestGameMsgOutbox::testAddDelete()
{
    game::msg::Outbox testee;
    size_t tmp = 0;

    // Add 3 messages. Ids must be distinct, consistent, order as expected.
    game::Id_t a = testee.addMessage(1, "a", game::PlayerSet_t(4));
    game::Id_t b = testee.addMessage(1, "b", game::PlayerSet_t(4));
    game::Id_t c = testee.addMessage(1, "c", game::PlayerSet_t(4));
    TS_ASSERT_DIFFERS(a, b);
    TS_ASSERT_DIFFERS(a, c);
    TS_ASSERT_DIFFERS(b, c);
    TS_ASSERT_DIFFERS(a, 0);
    TS_ASSERT_DIFFERS(b, 0);
    TS_ASSERT_DIFFERS(c, 0);

    TS_ASSERT(testee.findMessageById(a, tmp));
    TS_ASSERT_EQUALS(tmp, 0U);
    TS_ASSERT(testee.findMessageById(b, tmp));
    TS_ASSERT_EQUALS(tmp, 1U);
    TS_ASSERT(testee.findMessageById(c, tmp));
    TS_ASSERT_EQUALS(tmp, 2U);

    TS_ASSERT_EQUALS(testee.getMessageId(0), a);
    TS_ASSERT_EQUALS(testee.getMessageId(1), b);
    TS_ASSERT_EQUALS(testee.getMessageId(2), c);

    // Delete b, and add a new one. Verify consistency of new Ids.
    testee.deleteMessage(1);
    game::Id_t d = testee.addMessage(1, "d", game::PlayerSet_t(4));
    TS_ASSERT_DIFFERS(d, 0);
    TS_ASSERT_DIFFERS(d, a);
    TS_ASSERT_DIFFERS(d, b);
    TS_ASSERT_DIFFERS(d, c);

    TS_ASSERT(testee.findMessageById(a, tmp));
    TS_ASSERT_EQUALS(tmp, 0U);
    TS_ASSERT(!testee.findMessageById(b, tmp));
    TS_ASSERT(testee.findMessageById(c, tmp));
    TS_ASSERT_EQUALS(tmp, 1U);
    TS_ASSERT(testee.findMessageById(d, tmp));
    TS_ASSERT_EQUALS(tmp, 2U);

    TS_ASSERT_EQUALS(testee.getMessageId(0), a);
    TS_ASSERT_EQUALS(testee.getMessageId(1), c);
    TS_ASSERT_EQUALS(testee.getMessageId(2), d);
}

/** Test message modification. */
void
TestGameMsgOutbox::testModify()
{
    game::msg::Outbox testee;
    testee.addMessage(1, "a", game::PlayerSet_t(4));
    TS_ASSERT_EQUALS(testee.getMessageRawText(0), "a");
    TS_ASSERT_EQUALS(testee.getMessageReceivers(0), game::PlayerSet_t(4));
    TS_ASSERT_EQUALS(testee.getMessageSender(0), 1);

    testee.setMessageText(0, "b");
    TS_ASSERT_EQUALS(testee.getMessageRawText(0), "b");
    TS_ASSERT_EQUALS(testee.getMessageSender(0), 1);

    testee.setMessageReceivers(0, game::PlayerSet_t(6));
    TS_ASSERT_EQUALS(testee.getMessageRawText(0), "b");
    TS_ASSERT_EQUALS(testee.getMessageReceivers(0), game::PlayerSet_t(6));
    TS_ASSERT_EQUALS(testee.getMessageSender(0), 1);
}

/** Test out-of-range access. */
void
TestGameMsgOutbox::testOutOfRange()
{
    game::msg::Outbox testee;
    TS_ASSERT_EQUALS(testee.getMessageRawText(999), "");
    TS_ASSERT_EQUALS(testee.getMessageId(999), 0);
    TS_ASSERT_EQUALS(testee.getMessageReceivers(999), game::PlayerSet_t());
    TS_ASSERT_EQUALS(testee.getMessageSender(999), 0);
}

