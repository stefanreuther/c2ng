/**
  *  \file game/msg/inbox.hpp
  */
#ifndef C2NG_GAME_MSG_INBOX_HPP
#define C2NG_GAME_MSG_INBOX_HPP

#include "game/msg/mailbox.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/translator.hpp"
#include "game/playerlist.hpp"
#include "game/config/userconfiguration.hpp"

namespace game { namespace msg {

    /** Standard inbox.

        Changes to PCC2:
        - remove ability to store message recipient (add that later when we need it, possibly in a generic way)
        - add ability to store turn numbers as per interface
        - processing of the "collapse old messages" option removed; should be done by GUI

        Original comment from PCC2 (no longer valid):
          The idea is to allow having a single inbox even when there are multiple results,
          where all messages are listed and probably duplicates removed.

          When looking at a message, we still need to know who originally got the message,
          to know whom "we are scanning our mines" refers to.
          We can therefore associate a player number with each message.
          Otherwise, this number is not interpreted in any way by this class.

        FIXME: this class interprets host-specific formats.
        Does it make sense to move it into the implementation namespace? */
    class Inbox : public Mailbox {
     public:
        Inbox();
        ~Inbox();

        // Mailbox:
        virtual size_t getNumMessages();
        virtual String_t getMessageText(size_t index, afl::string::Translator& tx, PlayerList& players);
        virtual String_t getMessageHeading(size_t index, afl::string::Translator& tx, PlayerList& players);
        virtual int getMessageTurnNumber(size_t index);

        // Manipulation
        void addMessage(String_t text, int turnNumber);
        void sort(afl::string::Translator& tx, PlayerList& players);

     private:
        struct Message;
        afl::container::PtrVector<Message> m_messages;
    };

} }

#endif
