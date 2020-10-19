/**
  *  \file u/t_game_msg_mailbox.cpp
  *  \brief Test for game::msg::Mailbox
  */

#include "game/msg/mailbox.hpp"

#include "t_game_msg.hpp"

/** Interface test. */
void
TestGameMsgMailbox::testIt()
{
    class Tester : public game::msg::Mailbox {
     public:
        virtual size_t getNumMessages() const
            { return 0; }
        virtual String_t getMessageText(size_t /*index*/, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            { return String_t(); }
        virtual String_t getMessageHeading(size_t /*index*/, afl::string::Translator& /*tx*/, const game::PlayerList& /*players*/) const
            { return String_t(); }
        virtual int getMessageTurnNumber(size_t /*index*/) const
            { return 0; }
    };
    Tester t;
}

