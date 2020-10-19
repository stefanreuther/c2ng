/**
  *  \file game/msg/outbox.hpp
  *  \brief Class game::msg::Outbox
  */
#ifndef C2NG_GAME_MSG_OUTBOX_HPP
#define C2NG_GAME_MSG_OUTBOX_HPP

#include "afl/container/ptrvector.hpp"
#include "game/msg/mailbox.hpp"
#include "game/types.hpp"

namespace game { namespace msg {

    /** Outgoing message store.

        This stores player-to-player messages.
        With PCC, players can edit these messages after "sending" before they are actually sent.

        As of 20200528, this interface is preliminary.

        The primary means of addressing a message is to use an index (same as in PCC2).
        In addition, we store a "stable" identifier of a message, the message Id.
        This is used for two reasons:
        - in general, this allows an observer to watch a particular message,
          even if deletions of unrelated messages happen.
        - in particular, in c2play-server, it allows sensible change management.
          Instead of an index-based approach where deleting "message 4"
          invalidates the meaning of "message 4", "message 5", and everything beyond,
          an Id-based approach just invalidates the id-to-index mapping.
        This Id is not a user-visible feature.

        Message receivers are specified as a PlayerSet_t.
        In those sets, player 0 means the Host.
        (In physical v3 files, the host is Player 12; this is handled by the TurnLoader.)
        Messages to multiple receivers are broken up to single messages by the TurnLoader;
        Outbox offers a method to recombine them upon load (addMessageFromFile()).

        Messages are stored including a sender, to allow a possible host or multi-RST view. */
    class Outbox : public Mailbox {
     public:
        /** Constructor.
            Makes a blank outbox. */
        Outbox();

        /** Destructor. */
        ~Outbox();

        // Mailbox:
        virtual size_t getNumMessages() const;
        virtual String_t getMessageText(size_t index, afl::string::Translator& tx, const PlayerList& players) const;
        virtual String_t getMessageHeading(size_t index, afl::string::Translator& tx, const PlayerList& players) const;
        virtual int getMessageTurnNumber(size_t index) const;


        /** Get prefix for message when sent.
            For use by the TurnLoader to break up a message-to-many into individual messages.
            The prefix can be concatenated to the message text.
            \param index    message number [0,getNumMessages())
            \param receiver addressee of this incarnation of the message
            \param tx       translator
            \param players  players
            \return prefix (multi-line string, ending in "\n") */
        String_t getMessageSendPrefix(size_t index, int receiver, afl::string::Translator& tx, const PlayerList& players) const;

        /** Get raw message text.
            This is the part the user can edit.
            \param index message number [0,getNumMessages())
            \return text */
        String_t getMessageRawText(size_t index) const;

        /** Get set of message receivers.
            \param index message number [0,getNumMessages())
            \return receivers; 0 means host */
        PlayerSet_t getMessageReceivers(size_t index) const;

        /** Get message sender number.
            \param index message number [0,getNumMessages())
            \return sender */
        int getMessageSender(size_t index) const;

        /** Get message Id.
            \param index message number [0,getNumMessages())
            \return Id (nonzero) */
        Id_t getMessageId(size_t index) const;

        /** Set receivers.
            \param index message number [0,getNumMessages())
            \param receivers receivers (see getMessageReceivers()) */
        void setMessageReceivers(size_t index, PlayerSet_t receivers);

        /** Set message text.
            \param index message number [0,getNumMessages())
            \param text text (see getMessageRawText()) */
        void setMessageText(size_t index, String_t text);

        /** Delete messages starting at an index.
            \param index First index to delete (=number of messages to keep) */
        void deleteMessagesAfter(size_t index);

        /** Delete message by index.
            \param index message number [0,getNumMessages()) */
        void deleteMessage(size_t index);

        /** Find message, given a Id.
            \param [in] id Message Id
            \param [out] index Index
            \retval true message found; index has been set
            \retval false message not found */
        bool findMessageById(Id_t id, size_t& index) const;

        /** Add a new message (send).
            \param sender sender
            \param text message text (see getMessageRawText())
            \param receivers receivers (see getMessageReceivers())
            \return assigned message Id */
        Id_t addMessage(int sender, String_t text, PlayerSet_t receivers);

        /** Add a new message coming from a message file.
            Like addMessage(), but attempts to recombine messages to multiple receivers
            that have been broken up into single messages.
            \param sender sender
            \param text message text (see getMessageRawText())
            \param receivers receivers (see getMessageReceivers()) */
        void addMessageFromFile(int sender, String_t text, PlayerSet_t receivers);

        /** Clear this mailbox.
            \post getNumMessages() == 0 */
        void clear();

        /** Get message headers for display.
            Produces headers that should be used to display a message in the outbox.
            \param sender sender
            \param receivers receivers
            \param tx Translator
            \param players player list
            \return headers (multi-line string, ending in "\n") */
        static String_t getHeadersForDisplay(int sender, PlayerSet_t receivers, afl::string::Translator& tx, const PlayerList& players);
     private:
        struct Message;
        afl::container::PtrVector<Message> m_messages;
        Id_t m_idCounter;

        Id_t allocateId();
    };

} }

#endif
