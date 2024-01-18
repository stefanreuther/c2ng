/**
  *  \file test/game/msg/subsetmailboxtest.cpp
  *  \brief Test for game::msg::SubsetMailbox
  */

#include "game/msg/subsetmailbox.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/parser/messageinformation.hpp"
#include "game/playerlist.hpp"
#include "game/teamsettings.hpp"

/** Simple function test. */
AFL_TEST("game.msg.SubsetMailbox", a)
{
    afl::string::NullTranslator tx;
    game::PlayerList list;

    class UnderlyingMailbox : public game::msg::Mailbox {
     public:
        virtual size_t getNumMessages() const
            { return 100; }
        virtual String_t getMessageHeaderText(size_t index, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            { return afl::string::Format("a%d", index); }
        virtual String_t getMessageBodyText(size_t index, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            { return afl::string::Format("t%d", index); }
        virtual String_t getMessageForwardText(size_t index, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            { return afl::string::Format("f%d", index); }
        virtual String_t getMessageReplyText(size_t index, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            { return afl::string::Format("r%d", index); }
        virtual util::rich::Text getMessageDisplayText(size_t index, afl::string::Translator& tx, const game::PlayerList& players) const
            { return getMessageText(index, tx, players); }
        virtual String_t getMessageHeading(size_t index, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            { return afl::string::Format("h%d", index); }
        virtual Metadata getMessageMetadata(size_t index, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            {
                Metadata md;
                md.turnNumber = 10 + (int(index) % 20);
                return md;
            }
        virtual Actions_t getMessageActions(size_t /*index*/) const
            { return Actions_t(); }
        virtual void performMessageAction(size_t /*index*/, Action /*a*/)
            { }
        virtual void receiveMessageData(size_t index, game::parser::InformationConsumer& consumer, const game::TeamSettings& /*teamSettings*/, bool /*onRequest*/, afl::charset::Charset& /*cs*/)
            {
                game::parser::MessageInformation info(game::parser::MessageInformation::Ship, int(index), 99);
                consumer.addMessageInformation(info);
            }
    };
    UnderlyingMailbox under;

    std::vector<size_t> indexes;
    indexes.push_back(33);
    indexes.push_back(5);
    indexes.push_back(99);

    game::msg::SubsetMailbox testee(under, indexes);

    // Getters
    a.checkEqual("01. getNumMessages",        testee.getNumMessages(), 3U);
    a.checkEqual("02. getMessageHeaderText",  testee.getMessageHeaderText(0, tx, list), "a33");
    a.checkEqual("03. getMessageHeaderText",  testee.getMessageHeaderText(1, tx, list), "a5");
    a.checkEqual("04. getMessageHeaderText",  testee.getMessageHeaderText(2, tx, list), "a99");
    a.checkEqual("05. getMessageHeaderText",  testee.getMessageHeaderText(3, tx, list), "");
    a.checkEqual("06. getMessageBodyText",    testee.getMessageBodyText(0, tx, list), "t33");
    a.checkEqual("07. getMessageBodyText",    testee.getMessageBodyText(1, tx, list), "t5");
    a.checkEqual("08. getMessageBodyText",    testee.getMessageBodyText(2, tx, list), "t99");
    a.checkEqual("09. getMessageBodyText",    testee.getMessageBodyText(3, tx, list), "");
    a.checkEqual("10. getMessageText",        testee.getMessageText(0, tx, list), "a33t33");
    a.checkEqual("11. getMessageText",        testee.getMessageText(1, tx, list), "a5t5");
    a.checkEqual("12. getMessageText",        testee.getMessageText(2, tx, list), "a99t99");
    a.checkEqual("13. getMessageText",        testee.getMessageText(3, tx, list), "");
    a.checkEqual("14. getMessageForwardText", testee.getMessageForwardText(0, tx, list), "f33");
    a.checkEqual("15. getMessageForwardText", testee.getMessageForwardText(1, tx, list), "f5");
    a.checkEqual("16. getMessageForwardText", testee.getMessageForwardText(2, tx, list), "f99");
    a.checkEqual("17. getMessageForwardText", testee.getMessageForwardText(3, tx, list), "");
    a.checkEqual("18. getMessageReplyText",   testee.getMessageReplyText(0, tx, list), "r33");
    a.checkEqual("19. getMessageReplyText",   testee.getMessageReplyText(1, tx, list), "r5");
    a.checkEqual("20. getMessageReplyText",   testee.getMessageReplyText(2, tx, list), "r99");
    a.checkEqual("21. getMessageReplyText",   testee.getMessageReplyText(3, tx, list), "");
    a.checkEqual("22. getMessageHeading",     testee.getMessageHeading(0, tx, list), "h33");
    a.checkEqual("23. getMessageHeading",     testee.getMessageHeading(1, tx, list), "h5");
    a.checkEqual("24. getMessageHeading",     testee.getMessageHeading(2, tx, list), "h99");
    a.checkEqual("25. getMessageHeading",     testee.getMessageHeading(3, tx, list), "");
    a.checkEqual("26. getMessageMetadata",    testee.getMessageMetadata(0, tx, list).turnNumber, 23);
    a.checkEqual("27. getMessageMetadata",    testee.getMessageMetadata(1, tx, list).turnNumber, 15);
    a.checkEqual("28. getMessageMetadata",    testee.getMessageMetadata(2, tx, list).turnNumber, 29);
    a.checkEqual("29. getMessageMetadata",    testee.getMessageMetadata(3, tx, list).turnNumber, 0);

    // receiveMessageData
    class Consumer : public game::parser::InformationConsumer {
     public:
        virtual void addMessageInformation(const game::parser::MessageInformation& info)
            { m_acc += afl::string::Format("#%d", info.getObjectId()); }
        String_t get() const
            { return m_acc; }
     private:
        String_t m_acc;
    };

    game::TeamSettings teams;
    afl::charset::Utf8Charset cs;
    {
        Consumer c;
        testee.receiveMessageData(1, c, teams, false, cs);
        a.checkEqual("31. receiveMessageData", c.get(), "#5");
    }
    {
        Consumer c;
        testee.receiveMessageData(3, c, teams, false, cs);
        a.checkEqual("32. receiveMessageData", c.get(), "");
    }

    // getOuterIndex
    a.checkEqual("41. getOuterIndex", testee.getOuterIndex(0), 33U);
    a.checkEqual("42. getOuterIndex", testee.getOuterIndex(1), 5U);
    a.checkEqual("43. getOuterIndex", testee.getOuterIndex(2), 99U);
    a.checkEqual("44. getOuterIndex", testee.getOuterIndex(3), 0U);

    // find
    a.checkEqual("51. find", testee.find(33).orElse(777), 0U);
    a.checkEqual("52. find", testee.find(5).orElse(777), 1U);
    a.checkEqual("53. find", testee.find(99).orElse(777), 2U);
    a.checkEqual("54. find", testee.find(77).orElse(777), 777U);
}
