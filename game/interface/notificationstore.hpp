/**
  *  \file game/interface/notificationstore.hpp
  *  \brief Class game::interface::NotificationStore
  */
#ifndef C2NG_GAME_INTERFACE_NOTIFICATIONSTORE_HPP
#define C2NG_GAME_INTERFACE_NOTIFICATIONSTORE_HPP

#include "afl/container/ptrvector.hpp"
#include "afl/string/string.hpp"
#include "afl/base/types.hpp"
#include "afl/base/optional.hpp"
#include "interpreter/processlist.hpp"
#include "game/msg/mailbox.hpp"

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
            \param [in]  processId Process ID
            \param [out] index Index, if any
            \return true on success */
        bool findIndexByProcessId(uint32_t processId, size_t& index) const;

        /** Get message by index.
            \param index Index [0,getNumMessages())
            \return Message; null if index out of range */
        Message* getMessageByIndex(size_t index) const;

        /** Add new message.
            \param assoc Optional process Id
            \param header Message header (should contain \n separator)
            \param body Message body (concatenated to body for message text) */
        Message* addMessage(ProcessAssociation_t assoc, String_t header, String_t body);

        /** Check whether message is confirmed.
            \param msg Message to check
            \return true if confirmed */
        bool isMessageConfirmed(const Message* msg) const;

        /** Confirm a message.
            \param msg Message
            \return flag */
        void confirmMessage(Message* msg, bool flag);

        /** Remove orphaned messages.
            Orphaned messages are messages associated with a process that no longer exists. */
        void removeOrphanedMessages();

        /** Resume processes associated with confirmed messages.
            \param editor Mark processes resumed in this ProcessListEditor */
        void resumeConfirmedProcesses(ProcessListEditor& editor);

        // Mailbox interface:
        virtual size_t getNumMessages() const;
        virtual String_t getMessageText(size_t index, afl::string::Translator& tx, const PlayerList& players) const;
        virtual String_t getMessageHeading(size_t index, afl::string::Translator& tx, const PlayerList& players) const;
        virtual int getMessageTurnNumber(size_t index) const;
        virtual bool isMessageFiltered(size_t index, afl::string::Translator& tx, const PlayerList& players, const game::msg::Configuration& config) const;
        virtual Flags_t getMessageFlags(size_t index) const;
        virtual Actions_t getMessageActions(size_t index) const;
        virtual void performMessageAction(size_t index, Action a);

     private:
        bool findMessage(ProcessAssociation_t assoc, size_t& index) const;

        afl::container::PtrVector<Message> m_messages;
        interpreter::ProcessList& m_processList;
    };

} }

#endif
