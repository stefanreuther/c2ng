/**
  *  \file game/interface/notificationstore.hpp
  *  \brief Class game::interface::NotificationStore
  */
#ifndef C2NG_GAME_INTERFACE_NOTIFICATIONSTORE_HPP
#define C2NG_GAME_INTERFACE_NOTIFICATIONSTORE_HPP

#include "afl/base/optional.hpp"
#include "afl/base/types.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/string.hpp"
#include "game/msg/mailbox.hpp"
#include "game/reference.hpp"
#include "interpreter/processlist.hpp"

namespace game { namespace interface {

    class ProcessListEditor;

    /** Storage for notification messages.

        Notification messages provide a way for background scripts (in particular, auto tasks) to inform the user.
        The intent is to provide some kind of "push" interface, actively presenting these messages to the users,
        unlike the easy-to-oversee interface like the console.

        This implements the Mailbox interface which allows index-based access to message texts.
        It also implements an interface using opaque Message pointers for manipulating the messages. */
    class NotificationStore : public game::msg::Mailbox {
     public:
        struct Message;

        /** Association with a process.
            Messages can optionally be associated with a process. */
        typedef afl::base::Optional<uint32_t> ProcessAssociation_t;


        /** Constructor.
            \param processList ProcessList */
        explicit NotificationStore(interpreter::ProcessList& processList);

        /** Destructor. */
        ~NotificationStore();

        /** Find message associated with a process.
            \param processId Process ID
            \return Message if any, null if none */
        Message* findMessageByProcessId(uint32_t processId) const;

        /** Find message index by process Id.
            \param processId Process ID
            \return Index, if any */
        afl::base::Optional<size_t> findIndexByProcessId(uint32_t processId) const;

        /** Get message by index.
            \param index Index [0,getNumMessages())
            \return Message; null if index out of range */
        Message* getMessageByIndex(size_t index) const;

        /** Add new message.
            \param assoc Optional process Id
            \param header Message header (should contain \n separator)
            \param body Message body (concatenated to body for message text)
            \param ref Associated object Id */
        Message* addMessage(ProcessAssociation_t assoc, String_t header, String_t body, Reference ref);

        /** Check whether message is confirmed.
            \param msg Message to check
            \return true if confirmed */
        bool isMessageConfirmed(const Message* msg) const;

        /** Get message body text.
            The body text does not include explanatory text or headers.
            \param msg Message
            \return Text; empty if msg is null */
        String_t getMessageBody(const Message* msg) const;

        /** Confirm a message.
            \param msg Message
            \param flag true to confirm, false to un-confirm */
        void confirmMessage(Message* msg, bool flag);

        /** Remove orphaned messages.
            Orphaned messages are messages associated with a process that no longer exists. */
        void removeOrphanedMessages();

        /** Resume processes associated with confirmed messages.
            \param editor Mark processes resumed in this ProcessListEditor */
        void resumeConfirmedProcesses(ProcessListEditor& editor);

        // Mailbox interface:
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

     private:
        afl::base::Optional<size_t> findMessage(ProcessAssociation_t assoc) const;

        afl::container::PtrVector<Message> m_messages;
        interpreter::ProcessList& m_processList;
    };

} }

#endif
