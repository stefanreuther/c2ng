/**
  *  \file u/t_game_msg_subsetmailbox.cpp
  *  \brief Test for game::msg::SubsetMailbox
  */

#include "game/msg/subsetmailbox.hpp"

#include "t_game_msg.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/parser/messageinformation.hpp"
#include "game/playerlist.hpp"
#include "game/teamsettings.hpp"

/** Simple function test. */
void
TestGameMsgSubsetMailbox::testIt()
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
    TS_ASSERT_EQUALS(testee.getNumMessages(), 3U);
    TS_ASSERT_EQUALS(testee.getMessageHeaderText(0, tx, list), "a33");
    TS_ASSERT_EQUALS(testee.getMessageHeaderText(1, tx, list), "a5");
    TS_ASSERT_EQUALS(testee.getMessageHeaderText(2, tx, list), "a99");
    TS_ASSERT_EQUALS(testee.getMessageHeaderText(3, tx, list), "");
    TS_ASSERT_EQUALS(testee.getMessageBodyText(0, tx, list), "t33");
    TS_ASSERT_EQUALS(testee.getMessageBodyText(1, tx, list), "t5");
    TS_ASSERT_EQUALS(testee.getMessageBodyText(2, tx, list), "t99");
    TS_ASSERT_EQUALS(testee.getMessageBodyText(3, tx, list), "");
    TS_ASSERT_EQUALS(testee.getMessageText(0, tx, list), "a33t33");
    TS_ASSERT_EQUALS(testee.getMessageText(1, tx, list), "a5t5");
    TS_ASSERT_EQUALS(testee.getMessageText(2, tx, list), "a99t99");
    TS_ASSERT_EQUALS(testee.getMessageText(3, tx, list), "");
    TS_ASSERT_EQUALS(testee.getMessageForwardText(0, tx, list), "f33");
    TS_ASSERT_EQUALS(testee.getMessageForwardText(1, tx, list), "f5");
    TS_ASSERT_EQUALS(testee.getMessageForwardText(2, tx, list), "f99");
    TS_ASSERT_EQUALS(testee.getMessageForwardText(3, tx, list), "");
    TS_ASSERT_EQUALS(testee.getMessageReplyText(0, tx, list), "r33");
    TS_ASSERT_EQUALS(testee.getMessageReplyText(1, tx, list), "r5");
    TS_ASSERT_EQUALS(testee.getMessageReplyText(2, tx, list), "r99");
    TS_ASSERT_EQUALS(testee.getMessageReplyText(3, tx, list), "");
    TS_ASSERT_EQUALS(testee.getMessageHeading(0, tx, list), "h33");
    TS_ASSERT_EQUALS(testee.getMessageHeading(1, tx, list), "h5");
    TS_ASSERT_EQUALS(testee.getMessageHeading(2, tx, list), "h99");
    TS_ASSERT_EQUALS(testee.getMessageHeading(3, tx, list), "");
    TS_ASSERT_EQUALS(testee.getMessageMetadata(0, tx, list).turnNumber, 23);
    TS_ASSERT_EQUALS(testee.getMessageMetadata(1, tx, list).turnNumber, 15);
    TS_ASSERT_EQUALS(testee.getMessageMetadata(2, tx, list).turnNumber, 29);
    TS_ASSERT_EQUALS(testee.getMessageMetadata(3, tx, list).turnNumber, 0);

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
        TS_ASSERT_EQUALS(c.get(), "#5");
    }
    {
        Consumer c;
        testee.receiveMessageData(3, c, teams, false, cs);
        TS_ASSERT_EQUALS(c.get(), "");
    }

    // getOuterIndex
    TS_ASSERT_EQUALS(testee.getOuterIndex(0), 33U);
    TS_ASSERT_EQUALS(testee.getOuterIndex(1), 5U);
    TS_ASSERT_EQUALS(testee.getOuterIndex(2), 99U);
    TS_ASSERT_EQUALS(testee.getOuterIndex(3), 0U);

    // find
    TS_ASSERT_EQUALS(testee.find(33).orElse(777), 0U);
    TS_ASSERT_EQUALS(testee.find(5).orElse(777), 1U);
    TS_ASSERT_EQUALS(testee.find(99).orElse(777), 2U);
    TS_ASSERT_EQUALS(testee.find(77).orElse(777), 777U);
}

