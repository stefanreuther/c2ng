/**
  *  \file test/game/msg/mailboxtest.cpp
  *  \brief Test for game::msg::Mailbox
  */

#include "game/msg/mailbox.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.msg.Mailbox")
{
    class Tester : public game::msg::Mailbox {
     public:
        virtual size_t getNumMessages() const
            { return 0; }
        virtual String_t getMessageHeaderText(size_t /*index*/, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            { return String_t(); }
        virtual String_t getMessageBodyText(size_t /*index*/, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            { return String_t(); }
        virtual String_t getMessageForwardText(size_t index, afl::string::Translator& tx, const game::PlayerList& players) const
            { return defaultGetMessageForwardText(index, tx, players); }
        virtual String_t getMessageReplyText(size_t index, afl::string::Translator& tx, const game::PlayerList& players) const
            { return defaultGetMessageReplyText(index, tx, players); }
        virtual util::rich::Text getMessageDisplayText(size_t index, afl::string::Translator& tx, const game::PlayerList& players) const
            { return getMessageText(index, tx, players); }
        virtual String_t getMessageHeading(size_t /*index*/, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            { return String_t(); }
        virtual Metadata getMessageMetadata(size_t /*index*/, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            { return Metadata(); }
        virtual Actions_t getMessageActions(size_t /*index*/) const
            { return Actions_t(); }
        virtual void performMessageAction(size_t /*index*/, Action /*a*/)
            { }
        virtual void receiveMessageData(size_t /*index*/, game::parser::InformationConsumer& /*consumer*/, const game::TeamSettings& /*teamSettings*/, bool /*onRequest*/, afl::charset::Charset& /*cs*/)
            { }
    };
    Tester t;
}
