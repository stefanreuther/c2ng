/**
  *  \file u/t_game_msg_subsetmailbox.cpp
  *  \brief Test for game::msg::SubsetMailbox
  */

#include "game/msg/subsetmailbox.hpp"

#include "t_game_msg.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/playerlist.hpp"

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
        virtual String_t getMessageText(size_t index, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            { return afl::string::Format("t%d", index); }
        virtual String_t getMessageHeading(size_t index, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            { return afl::string::Format("h%d", index); }
        virtual int getMessageTurnNumber(size_t index) const
            { return 10 + (int(index) % 20); }
        virtual bool isMessageFiltered(size_t /*index*/, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/, const game::msg::Configuration& /*config*/) const
            { return false; }
        virtual Flags_t getMessageFlags(size_t /*index*/) const
            { return Flags_t(); }
        virtual Actions_t getMessageActions(size_t /*index*/) const
            { return Actions_t(); }
        virtual void performMessageAction(size_t /*index*/, Action /*a*/)
            { }
    };
    UnderlyingMailbox under;

    std::vector<size_t> indexes;
    indexes.push_back(33);
    indexes.push_back(5);
    indexes.push_back(99);

    game::msg::SubsetMailbox testee(under, indexes);

    TS_ASSERT_EQUALS(testee.getNumMessages(), 3U);
    TS_ASSERT_EQUALS(testee.getMessageText(0, tx, list), "t33");
    TS_ASSERT_EQUALS(testee.getMessageText(1, tx, list), "t5");
    TS_ASSERT_EQUALS(testee.getMessageText(2, tx, list), "t99");
    TS_ASSERT_EQUALS(testee.getMessageText(3, tx, list), "");
    TS_ASSERT_EQUALS(testee.getMessageHeading(0, tx, list), "h33");
    TS_ASSERT_EQUALS(testee.getMessageHeading(1, tx, list), "h5");
    TS_ASSERT_EQUALS(testee.getMessageHeading(2, tx, list), "h99");
    TS_ASSERT_EQUALS(testee.getMessageHeading(3, tx, list), "");
    TS_ASSERT_EQUALS(testee.getMessageTurnNumber(0), 23);
    TS_ASSERT_EQUALS(testee.getMessageTurnNumber(1), 15);
    TS_ASSERT_EQUALS(testee.getMessageTurnNumber(2), 29);
    TS_ASSERT_EQUALS(testee.getMessageTurnNumber(3), 0);
}

