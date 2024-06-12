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
          Otherwise, this number is not interpreted in any way by this class. */
    class Inbox : public Mailbox {
     public:
        Inbox();
        ~Inbox();

        // Mailbox:
        virtual size_t getNumMessages() const;
        virtual String_t getMessageHeaderText(size_t index, afl::string::Translator& tx, const PlayerList& players) const;
        virtual String_t getMessageBodyText(size_t index, afl::string::Translator& tx, const PlayerList& players) const;
        virtual String_t getMessageForwardText(size_t index, afl::string::Translator& tx, const PlayerList& players) const;
        virtual String_t getMessageReplyText(size_t index, afl::string::Translator& tx, const PlayerList& players) const;
        virtual util::rich::Text getMessageDisplayText(size_t index, afl::string::Translator& tx, const PlayerList& players) const;
        virtual String_t getMessageHeading(size_t index, afl::string::Translator& tx, const PlayerList& players) const;
        virtual Metadata getMessageMetadata(size_t index, afl::string::Translator& tx, const PlayerList& players) const;
        virtual Actions_t getMessageActions(size_t index) const;
        virtual void performMessageAction(size_t index, Action a);
        virtual void receiveMessageData(size_t index, game::parser::InformationConsumer& consumer, const TeamSettings& teamSettings, bool onRequest, afl::charset::Charset& cs);

        /*
         *  Manipulation
         */

        /** Add a single message.
            \param text       Complete text of message
            \param turnNumber Turn number
            \return Index of messag */
        size_t addMessage(String_t text, int turnNumber);

        /** Set message's primary link.
            \param index Message index
            \param ref Reference to use as primary link (unset to use default) */
        void setMessagePrimaryLink(size_t index, Reference ref);

        /** Sort messages.
            This will group messages of equal subjects together, but preserves the overall order
            (i.e. messages from other races will remain first).
            \param tx Translator
            \param players Player list */
        void sort(afl::string::Translator& tx, const PlayerList& players);

     private:
        struct Message;
        afl::container::PtrVector<Message> m_messages;

        Message* getMessage(size_t index);
        const Message* getMessage(size_t index) const;
    };

} }

#endif
