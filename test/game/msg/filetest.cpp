/**
  *  \file test/game/msg/filetest.cpp
  *  \brief Test for game::msg::File
  */

#include "game/msg/file.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
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

    String_t loadMessageTextFromString(String_t str, const game::StringVerifier* pSV)
    {
        afl::io::ConstMemoryStream ms(afl::string::toBytes(str));
        afl::io::TextFile tf(ms);
        return game::msg::loadMessageText(tf, pSV);
    }
}

/** Test writing a single message. */
AFL_TEST("game.msg.File:writeMessages:single", a)
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

    a.checkEqual("file content", afl::string::fromBytes(out.getContent()),
                 "=== Turn 10 ===\n"
                 "--- Message 2 ---\n"
                 "second header\n"
                 "second body\n"
                 "\n");
}

/** Test writing multiple messages. */
AFL_TEST("game.msg.File:writeMessages:multiple", a)
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

    a.checkEqual("file content", afl::string::fromBytes(out.getContent()),
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
AFL_TEST("game.msg.File:writeMessages:different-turns", a)
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

    a.checkEqual("file content", afl::string::fromBytes(out.getContent()),
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
AFL_TEST("game.msg.File:loadMessages", a)
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

    a.checkEqual("01. getNumMessages", mbox.getNumMessages(), 3U);
    a.checkEqual("02. getMessageBodyText", mbox.getMessageBodyText(0, tx, players), "second header\nsecond body\n");
    a.checkEqual("03. getMessageBodyText", mbox.getMessageBodyText(1, tx, players), "third header\nthird body\n");
    a.checkEqual("04. getMessageBodyText", mbox.getMessageBodyText(2, tx, players), "fourth header\nfourth body\n");
    a.checkEqual("05. turnNumber", mbox.getMessageMetadata(0, tx, players).turnNumber, 10);
    a.checkEqual("06. turnNumber", mbox.getMessageMetadata(1, tx, players).turnNumber, 20);
    a.checkEqual("07. turnNumber", mbox.getMessageMetadata(2, tx, players).turnNumber, 20);
}

/** Test loading empty messages.
    Empty messages will be ignored. */
AFL_TEST("game.msg.File:loadMessages:empty", a)
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

    a.checkEqual("01. getNumMessages", mbox.getNumMessages(), 0U);
}

/** Test loading file with undelimited content.
    Undelimited content will be ignored. */
AFL_TEST("game.msg.File:loadMessages:undelimited", a)
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

    a.checkEqual("01. getNumMessages", mbox.getNumMessages(), 0U);
}

/** Test loading messages with TURN headers. */
AFL_TEST("game.msg.File:loadMessages:turn-header", a)
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

    a.checkEqual("01. getNumMessages", mbox.getNumMessages(), 3U);
    a.checkEqual("02. getMessageBodyText", mbox.getMessageBodyText(0, tx, players), "TURN: 30\nfirst\n");
    a.checkEqual("03. getMessageBodyText", mbox.getMessageBodyText(1, tx, players), "second\n");
    a.checkEqual("04. getMessageBodyText", mbox.getMessageBodyText(2, tx, players), "TURN:20\nthird\n");
    a.checkEqual("05. turnNumber", mbox.getMessageMetadata(0, tx, players).turnNumber, 30);
    a.checkEqual("06. turnNumber", mbox.getMessageMetadata(1, tx, players).turnNumber, 30);
    a.checkEqual("07. turnNumber", mbox.getMessageMetadata(2, tx, players).turnNumber, 20);
}

/** Test loadMessageText(). */
AFL_TEST("game.msg.File:loadMessageTextFromString", a)
{
    // Trivial case
    a.checkEqual("01. loadMessageTextFromString", loadMessageTextFromString("", 0), "");

    // Standard case
    a.checkEqual("11. loadMessageTextFromString", loadMessageTextFromString("first\nsecond\nthird\n\n", 0),
                 "first\nsecond\nthird");

    // Newline removal
    a.checkEqual("21. loadMessageTextFromString", loadMessageTextFromString("\n\n\nfoo\n\n\n", 0), "foo");

    // Lots of headers
    a.checkEqual("31. loadMessageTextFromString", loadMessageTextFromString("--- Message ---\n"
                                                                            "TURN: 30\n"
                                                                            "first\n"
                                                                            "--- Message ---\n"
                                                                            "second\n"
                                                                            "--- Message ---\n"
                                                                            "TURN:20\n"
                                                                            "third\n", 0),
                 "first\nsecond\nthird");

    // With StringVerifier that rejects capital letters
    class TestSV : public game::StringVerifier {
     public:
        TestSV(afl::test::Assert a)
            : m_assert(a)
            { }
        virtual bool isValidString(Context /*ctx*/, const String_t& /*text*/) const
            {
                m_assert.fail("41. isValidString unexpected");
                return false;
            }
        virtual bool isValidCharacter(Context ctx, afl::charset::Unichar_t ch) const
            {
                m_assert.checkEqual("42. isValidCharacter: context", ctx, Message);
                return (ch >= 'a' && ch <= 'z');
            }
        virtual size_t getMaxStringLength(Context /*ctx*/) const
            {
                m_assert.fail("43. getMaxStringLength unexpected");
                return 0;
            }
        virtual TestSV* clone() const
            { return new TestSV(m_assert); }
     private:
        afl::test::Assert m_assert;
    };
    TestSV sv(a);
    a.checkEqual("44. loadMessageTextFromString", loadMessageTextFromString("First\nSecond\nThird\n\n", &sv),
                 "irst\necond\nhird");
}
