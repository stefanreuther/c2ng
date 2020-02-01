/**
  *  \file game/interface/notificationstore.hpp
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

    /** Storage for notification messages.

        Notification messages provide a way for background scripts (in particular, auto tasks) to inform the user.
        The intent is to provide some kind of "push" interface, actively presenting these messages to the users,
        unlike the easy-to-oversee interface like the console. */
    class NotificationStore : public game::msg::Mailbox {
     public:
        struct Message;

        typedef afl::base::Optional<uint32_t> ProcessAssociation_t;

        NotificationStore(interpreter::ProcessList& processList);

        ~NotificationStore();

        Message* findMessageByProcessId(uint32_t pid) const;
        Message* getMessageByIndex(size_t index) const;

        void addMessage(ProcessAssociation_t assoc, String_t header, String_t body);

        // Message access
        bool isMessageConfirmed(Message* msg) const;
        void confirmMessage(Message* msg, bool flag);

        // Process access
        void removeOrphanedMessages();
        void resumeConfirmedProcesses(uint32_t pgid);

        // Mailbox interface:
        virtual size_t getNumMessages();
        virtual String_t getMessageText(size_t index, afl::string::Translator& tx, const PlayerList& players);
        virtual String_t getMessageHeading(size_t index, afl::string::Translator& tx, const PlayerList& players);
        virtual int getMessageTurnNumber(size_t index);

     private:
        bool findMessage(ProcessAssociation_t assoc, size_t& index) const;

        afl::container::PtrVector<Message> m_messages;
        interpreter::ProcessList& m_processList;
    };

} }

#endif
