/**
  *  \file u/t_game_msg_file.cpp
  *  \brief Test for game::msg::File
  */

#include "game/msg/file.hpp"

#include "t_game_msg.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/msg/inbox.hpp"
#include "game/msg/mailbox.hpp"

namespace {
    class TestMailbox : public game::msg::Mailbox {
     public:
        void add(String_t header, String_t body, int turnNumber)
            { m_data.push_back(Data(header, body, turnNumber)); }
        virtual size_t getNumMessages() const
            { return m_data.size(); }
        virtual String_t getMessageHeaderText(size_t index, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            { return m_data.at(index).header; }
        virtual String_t getMessageBodyText(size_t index, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            { return m_data.at(index).body; }
        virtual String_t getMessageForwardText(size_t index, afl::string::Translator& tx, const game::PlayerList& players) const
            { return defaultGetMessageForwardText(index, tx, players); }
        virtual String_t getMessageReplyText(size_t index, afl::string::Translator& tx, const game::PlayerList& players) const
            { return defaultGetMessageReplyText(index, tx, players); }
        virtual util::rich::Text getMessageDisplayText(size_t index, afl::string::Translator& tx, const game::PlayerList& players) const
            { return getMessageText(index, tx, players); }
        virtual String_t getMessageHeading(size_t /*index*/, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            { return "HEADING"; }
        virtual Metadata getMessageMetadata(size_t index, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            {
                Metadata md;
                md.turnNumber = m_data.at(index).turnNumber;
                return md;
            }
        virtual Actions_t getMessageActions(size_t /*index*/) const
            { return Actions_t(); }
        virtual void performMessageAction(size_t /*index*/, Action /*a*/)
            { }
        virtual void receiveMessageData(size_t /*index*/, game::parser::InformationConsumer& /*consumer*/, const game::TeamSettings& /*teamSettings*/, bool /*onRequest*/, afl::charset::Charset& /*cs*/)
            { }
     private:
        struct Data {
            String_t header;
            String_t body;
            int turnNumber;
            Data(const String_t& header, const String_t& body, int turnNumber)
                : header(header), body(body), turnNumber(turnNumber)
                { }
        };
        std::vector<Data> m_data;
    };
}

/** Test writing a single message. */
void
TestGameMsgFile::testWriteSingle()
{
    TestMailbox mbox;
    mbox.add("first header\n",  "first body\n",  10);
    mbox.add("second header\n", "second body\n", 10);
    mbox.add("third header\n",  "third body\n",  10);
    mbox.add("fourth header\n", "fourth body\n", 10);

    afl::io::InternalStream out;
    afl::io::TextFile textOut(out);
    textOut.setSystemNewline(false);

    game::PlayerList players;
    afl::string::NullTranslator tx;

    game::msg::writeMessages(textOut, mbox, 1, 2, players, tx);
    textOut.flush();

    TS_ASSERT_EQUALS(afl::string::fromBytes(out.getContent()),
                     "=== Turn 10 ===\n"
                     "--- Message 2 ---\n"
                     "second header\n"
                     "second body\n"
                     "\n");
}

/** Test writing multiple messages. */
void
TestGameMsgFile::testWriteMulti()
{
    TestMailbox mbox;
    mbox.add("first header\n",  "first body\n",  10);
    mbox.add("second header\n", "second body\n", 10);
    mbox.add("third header\n",  "third body\n",  10);
    mbox.add("fourth header\n", "fourth body\n", 10);

    afl::io::InternalStream out;
    afl::io::TextFile textOut(out);
    textOut.setSystemNewline(false);

    game::PlayerList players;
    afl::string::NullTranslator tx;

    game::msg::writeMessages(textOut, mbox, 1, 3, players, tx);
    textOut.flush();

    TS_ASSERT_EQUALS(afl::string::fromBytes(out.getContent()),
                     "=== Turn 10 ===\n"
                     "   2 message(s)\n"
                     "--- Message 2 ---\n"
                     "second header\n"
                     "second body\n"
                     "\n"
                     "--- Message 3 ---\n"
                     "third header\n"
                     "third body\n"
                     "\n");
}

/** Test writing multiple messages from different turns. */
void
TestGameMsgFile::testWriteDifferentTurns()
{
    TestMailbox mbox;
    mbox.add("first header\n",  "first body\n",  10);
    mbox.add("second header\n", "second body\n", 10);
    mbox.add("third header\n",  "third body\n",  20);
    mbox.add("fourth header\n", "fourth body\n", 20);

    afl::io::InternalStream out;
    afl::io::TextFile textOut(out);
    textOut.setSystemNewline(false);

    game::PlayerList players;
    afl::string::NullTranslator tx;

    game::msg::writeMessages(textOut, mbox, 1, 4, players, tx);
    textOut.flush();

    TS_ASSERT_EQUALS(afl::string::fromBytes(out.getContent()),
                     "=== Turn 10 ===\n"
                     "--- Message 2 ---\n"
                     "second header\n"
                     "second body\n"
                     "\n"
                     "=== Turn 20 ===\n"
                     "   2 message(s)\n"
                     "--- Message 3 ---\n"
                     "third header\n"
                     "third body\n"
                     "\n"
                     "--- Message 4 ---\n"
                     "fourth header\n"
                     "fourth body\n"
                     "\n");
}

/** Test loading, normal case. */
void
TestGameMsgFile::testLoad()
{
    // Test text (same as testWriteDifferentTurns)
    const char*const TEXT =
        "=== Turn 10 ===\n"
        "--- Message 2 ---\n"
        "second header\n"
        "second body\n"
        "\n"
        "=== Turn 20 ===\n"
        "   2 message(s)\n"
        "--- Message 3 ---\n"
        "third header\n"
        "third body\n"
        "\n"
        "--- Message 4 ---\n"
        "fourth header\n"
        "fourth body\n"
        "\n";
    afl::io::ConstMemoryStream ms(afl::string::toBytes(TEXT));
    afl::io::TextFile text(ms);

    game::msg::Inbox mbox;
    game::msg::loadMessages(text, mbox);

    game::PlayerList players;
    afl::string::NullTranslator tx;

    TS_ASSERT_EQUALS(mbox.getNumMessages(), 3U);
    TS_ASSERT_EQUALS(mbox.getMessageBodyText(0, tx, players), "second header\nsecond body\n");
    TS_ASSERT_EQUALS(mbox.getMessageBodyText(1, tx, players), "third header\nthird body\n");
    TS_ASSERT_EQUALS(mbox.getMessageBodyText(2, tx, players), "fourth header\nfourth body\n");
    TS_ASSERT_EQUALS(mbox.getMessageMetadata(0, tx, players).turnNumber, 10);
    TS_ASSERT_EQUALS(mbox.getMessageMetadata(1, tx, players).turnNumber, 20);
    TS_ASSERT_EQUALS(mbox.getMessageMetadata(2, tx, players).turnNumber, 20);
}

/** Test loading empty messages.
    Empty messages will be ignored. */
void
TestGameMsgFile::testLoadEmpty()
{
    const char*const TEXT =
        "=== Turn 10 ===\n"
        "--- Message 2 ---\n"
        "=== Turn 20 ===\n"
        "--- Message ---\n"
        "\n\n\n"
        "--- Message ---\n"
        "--- Message ---\n"
        "\n";
    afl::io::ConstMemoryStream ms(afl::string::toBytes(TEXT));
    afl::io::TextFile text(ms);

    game::msg::Inbox mbox;
    game::msg::loadMessages(text, mbox);

    game::PlayerList players;
    afl::string::NullTranslator tx;

    TS_ASSERT_EQUALS(mbox.getNumMessages(), 0U);
}

/** Test loading file with undelimited content.
    Undelimited content will be ignored. */
void
TestGameMsgFile::testLoadUndelimited()
{
    const char*const TEXT =
        "just some text\n"
        "some more text\n";
    afl::io::ConstMemoryStream ms(afl::string::toBytes(TEXT));
    afl::io::TextFile text(ms);

    game::msg::Inbox mbox;
    game::msg::loadMessages(text, mbox);

    game::PlayerList players;
    afl::string::NullTranslator tx;

    TS_ASSERT_EQUALS(mbox.getNumMessages(), 0U);
}

/** Test loading messages with TURN headers. */
void
TestGameMsgFile::testLoadTurn()
{
    const char*const TEXT =
        "--- Message ---\n"
        "TURN: 30\n"
        "first\n"
        "--- Message ---\n"
        "second\n"
        "--- Message ---\n"
        "TURN:20\n"              // space is optional
        "third\n";
    afl::io::ConstMemoryStream ms(afl::string::toBytes(TEXT));
    afl::io::TextFile text(ms);

    game::msg::Inbox mbox;
    game::msg::loadMessages(text, mbox);

    game::PlayerList players;
    afl::string::NullTranslator tx;

    TS_ASSERT_EQUALS(mbox.getNumMessages(), 3U);
    TS_ASSERT_EQUALS(mbox.getMessageBodyText(0, tx, players), "TURN: 30\nfirst\n");
    TS_ASSERT_EQUALS(mbox.getMessageBodyText(1, tx, players), "second\n");
    TS_ASSERT_EQUALS(mbox.getMessageBodyText(2, tx, players), "TURN:20\nthird\n");
    TS_ASSERT_EQUALS(mbox.getMessageMetadata(0, tx, players).turnNumber, 30);
    TS_ASSERT_EQUALS(mbox.getMessageMetadata(1, tx, players).turnNumber, 30);
    TS_ASSERT_EQUALS(mbox.getMessageMetadata(2, tx, players).turnNumber, 20);
}

