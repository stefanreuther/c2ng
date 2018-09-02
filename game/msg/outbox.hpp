/**
  *  \file game/msg/outbox.hpp
  */
#ifndef C2NG_GAME_MSG_OUTBOX_HPP
#define C2NG_GAME_MSG_OUTBOX_HPP

#include "game/msg/mailbox.hpp"
#include "afl/container/ptrvector.hpp"

namespace game { namespace msg {

    // //! Standard Outbox.
    class Outbox : public Mailbox {
     public:
        Outbox();
        ~Outbox();

        // Mailbox:
        virtual size_t getNumMessages();
        virtual String_t getMessageText(size_t index, afl::string::Translator& tx, PlayerList& players);
        virtual String_t getMessageHeading(size_t index, afl::string::Translator& tx, PlayerList& players);
        virtual int getMessageTurnNumber(size_t index);

        /*
         *  In receiver list, 0 means host
         */

        String_t getMessageSendPrefix(size_t index, int receiver,
                                      afl::string::Translator& tx,
                                      const PlayerList& players) const;
        String_t getMessageRawText(size_t index) const;
        PlayerSet_t getMessageReceiverMask(size_t index) const;
        int getMessageSender(size_t index) const;

    //     void setReceiverMask(int index, GPlayerSet receivers);
    //     void setMessage(int index, string_t text, GPlayerSet receivers);

        void deleteMessagesAfter(size_t index);
    //     void     deleteMessage(int index);

        //     /** Set mailbox storage format. */
        //     void setFormat(Format format)
        //         { this->format = format; }
        //     /** Get mailbox storage format. */
        //     Format getFormat() const
        //         { return format; }

        void addMessage(int sender, String_t text, PlayerSet_t receivers);
        void addMessageFromFile(int sender, String_t text, PlayerSet_t receivers);
        void clear();

        static String_t getHeadersForDisplay(int sender,
                                             PlayerSet_t receivers,
                                             afl::string::Translator& tx,
                                             const PlayerList& players);

        //  public:
        //     enum Format {
        //         fDos,                   ///< DOS-style mailbox (messX.dat).
        //         fWinplan                ///< Winplan-style mailbox (mess35X.dat).
        //     };
     private:
        struct Message;
        afl::container::PtrVector<Message> m_messages;
        //     Format format;
    };

} }

#endif
