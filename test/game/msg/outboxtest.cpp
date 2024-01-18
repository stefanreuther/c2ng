/**
  *  \file test/game/msg/outboxtest.cpp
  *  \brief Test for game::msg::Outbox
  */

#include "game/msg/outbox.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/playerlist.hpp"

/** Simple test. */
AFL_TEST("game.msg.Outbox:basics", a)
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
    a.checkEqual("01. getNumMessages", testee.getNumMessages(), 2U);
    a.checkEqual("02. getMessageText", testee.getMessageText(0, tx, players),
                 "<<< Sub Space Message >>>\n"
                 "FROM: Long Three\n"
                 "TO: Player 4\n"
                 "text");
    a.checkEqual("03. getMessageText", testee.getMessageText(1, tx, players),
                 "<<< Sub Space Message >>>\n"
                 "FROM: Player 9\n"
                 "TO: 5 6\n"
                 "CC: 5 6\n"
                 "text2");
    a.checkEqual("04. getMessageHeading", testee.getMessageHeading(0, tx, players), "To: Player 4");
    a.checkEqual("05. getMessageHeading", testee.getMessageHeading(1, tx, players), "To: 5 6");

    // Border cases/unimplemented
    a.checkEqual("11. getMessageText", testee.getMessageText(99, tx, players), "");
    a.checkEqual("12. getMessageHeading", testee.getMessageHeading(99, tx, players), "");
    a.checkEqual("13. getMessageMetadata", testee.getMessageMetadata(0, tx, players).turnNumber, 0);

    a.checkEqual("21. getMessageForwardText", testee.getMessageForwardText(0, tx, players),
                 "--- Forwarded Message ---\n"
                 "<<< Sub Space Message >>>\n"
                 "FROM: Long Three\n"
                 "TO: Player 4\n"
                 "text\n"
                 "--- End Forwarded Message ---");
    a.checkEqual("22. getMessageReplyText", testee.getMessageReplyText(0, tx, players), "> text\n");
}

/** Test merging. */
AFL_TEST("game.msg.Outbox:addMessageFromFile", a)
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
    a.checkEqual("01. getNumMessages",       testee.getNumMessages(), 1U);
    a.checkEqual("02. getMessageHeading",    testee.getMessageHeading(0, tx, players), "To: 4 5 6 7");
    a.checkEqual("03. getMessageSender",     testee.getMessageSender(0), 4);
    a.checkEqual("04. getMessageReceivers",  testee.getMessageReceivers(0).toInteger(), 0xF0U);
    a.checkEqual("05. getMessageRawText",    testee.getMessageRawText(0), "text");
    a.checkEqual("06. getMessageSendPrefix", testee.getMessageSendPrefix(0, 4, tx, players), "<CC: 5 6 7\n");
    a.checkEqual("07. getMessageSendPrefix", testee.getMessageSendPrefix(0, 5, tx, players), "CC: 4 6 7\n");
}

/** Test add/delete. */
AFL_TEST("game.msg.Outbox:add+delete", a)
{
    game::msg::Outbox testee;
    size_t tmp = 0;

    // Add 3 messages. Ids must be distinct, consistent, order as expected.
    game::Id_t ia = testee.addMessage(1, "a", game::PlayerSet_t(4));
    game::Id_t ib = testee.addMessage(1, "b", game::PlayerSet_t(4));
    game::Id_t ic = testee.addMessage(1, "c", game::PlayerSet_t(4));
    a.checkDifferent("01. distinct Id", ia, ib);
    a.checkDifferent("02. distinct Id", ia, ic);
    a.checkDifferent("03. distinct Id", ib, ic);
    a.checkDifferent("04. distinct Id", ia, 0);
    a.checkDifferent("05. distinct Id", ib, 0);
    a.checkDifferent("06. distinct Id", ic, 0);

    a.check("11. findMessageById", testee.findMessageById(ia).get(tmp));
    a.checkEqual("12. index", tmp, 0U);
    a.check("13. findMessageById", testee.findMessageById(ib).get(tmp));
    a.checkEqual("14. index", tmp, 1U);
    a.check("15. findMessageById", testee.findMessageById(ic).get(tmp));
    a.checkEqual("16. index", tmp, 2U);

    a.checkEqual("21. getMessageId", testee.getMessageId(0), ia);
    a.checkEqual("22. getMessageId", testee.getMessageId(1), ib);
    a.checkEqual("23. getMessageId", testee.getMessageId(2), ic);

    // Delete ib, and add a new one. Verify consistency of new Ids.
    testee.deleteMessage(1);
    game::Id_t id = testee.addMessage(1, "d", game::PlayerSet_t(4));
    a.checkDifferent("31. distinct Id", id, 0);
    a.checkDifferent("32. distinct Id", id, ia);
    a.checkDifferent("33. distinct Id", id, ib);
    a.checkDifferent("34. distinct Id", id, ic);

    a.check("41. findMessageById", testee.findMessageById(ia).get(tmp));
    a.checkEqual("42. index", tmp, 0U);
    a.check("43. findMessageById", !testee.findMessageById(ib).get(tmp));
    a.check("44. findMessageById", testee.findMessageById(ic).get(tmp));
    a.checkEqual("45. index", tmp, 1U);
    a.check("46. findMessageById", testee.findMessageById(id).get(tmp));
    a.checkEqual("47. index", tmp, 2U);

    a.checkEqual("51. getMessageId", testee.getMessageId(0), ia);
    a.checkEqual("52. getMessageId", testee.getMessageId(1), ic);
    a.checkEqual("53. getMessageId", testee.getMessageId(2), id);
}

/** Test message modification. */
AFL_TEST("game.msg.Outbox:modify", a)
{
    game::msg::Outbox testee;
    testee.addMessage(1, "a", game::PlayerSet_t(4));
    a.checkEqual("01. getMessageRawText",   testee.getMessageRawText(0), "a");
    a.checkEqual("02. getMessageReceivers", testee.getMessageReceivers(0), game::PlayerSet_t(4));
    a.checkEqual("03. getMessageSender",    testee.getMessageSender(0), 1);

    testee.setMessageText(0, "b");
    a.checkEqual("11. getMessageRawText", testee.getMessageRawText(0), "b");
    a.checkEqual("12. getMessageSender",  testee.getMessageSender(0), 1);

    testee.setMessageReceivers(0, game::PlayerSet_t(6));
    a.checkEqual("21. getMessageRawText",   testee.getMessageRawText(0), "b");
    a.checkEqual("22. getMessageReceivers", testee.getMessageReceivers(0), game::PlayerSet_t(6));
    a.checkEqual("23. getMessageSender",    testee.getMessageSender(0), 1);
}

/** Test out-of-range access. */
AFL_TEST("game.msg.Outbox:range-error", a)
{
    game::msg::Outbox testee;
    a.checkEqual("01. getMessageRawText",   testee.getMessageRawText(999), "");
    a.checkEqual("02. getMessageId",        testee.getMessageId(999), 0);
    a.checkEqual("03. getMessageReceivers", testee.getMessageReceivers(999), game::PlayerSet_t());
    a.checkEqual("04. getMessageSender",    testee.getMessageSender(999), 0);
}

