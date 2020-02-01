/**
  *  \file game/interface/notificationstore.cpp
  *
  *  FIXME: for consideration in c2ng: like in PCC2, NotificationStore implements Mailbox.
  *  This necessitates that it permanently knows a ProcessList, and enlargens the interface.
  *  It could make sense to separate the Mailbox implementation.
  *
  *  FIXME: for consideration in c2ng: 'confirmed' is a bool that is set when a message is confirmed.
  *  Every future call to resumeConfirmedProcesses() will resume that process,
  *  even if it has long proceeded, until the process generates a new message.
  *  This is normally harmless, but unnecessary and unexpected.
  *
  *  Change to PCC2: our notifications live completely outside the interpreter function.
  *  In PCC2, processes have a pointer to their associated message.
  *  We perform the mapping using process Ids, and explicitly clean up when processes are removed.
  */

#include "game/interface/notificationstore.hpp"
#include "interpreter/process.hpp"

struct game::interface::NotificationStore::Message {
    ProcessAssociation_t assoc;
    bool confirmed;
    String_t header;
    String_t body;

    Message(ProcessAssociation_t assoc, String_t header, String_t body)
        : assoc(assoc),
          confirmed(false),
          header(header),
          body(body)
        { }
};



game::interface::NotificationStore::NotificationStore(interpreter::ProcessList& processList)
    : Mailbox(), m_messages(), m_processList(processList)
{ }

game::interface::NotificationStore::~NotificationStore()
{ }

game::interface::NotificationStore::Message*
game::interface::NotificationStore::findMessageByProcessId(uint32_t pid) const
{
    size_t index;
    if (findMessage(ProcessAssociation_t(pid), index)) {
        return m_messages[index];
    } else {
        return 0;
    }
}

game::interface::NotificationStore::Message*
game::interface::NotificationStore::getMessageByIndex(size_t index) const
{
    // ex IntNotificationMessageStore::getMessageByIndex
    if (index < m_messages.size()) {
        return m_messages[index];
    } else {
        return 0;
    }
}

void
game::interface::NotificationStore::addMessage(ProcessAssociation_t assoc, String_t header, String_t body)
{
    // ex IntNotificationMessageStore::addNewMessage (sort-of)
    // Remove previous message
    size_t index;
    if (findMessage(assoc, index)) {
        m_messages.erase(m_messages.begin() + index);
    }

    // Add new one
    m_messages.pushBackNew(new Message(assoc, header, body));
}

bool
game::interface::NotificationStore::isMessageConfirmed(Message* msg) const
{
    // ex IntNotificationMessage::isConfirmed
    return msg != 0
        && msg->confirmed;
}

void
game::interface::NotificationStore::confirmMessage(Message* msg, bool flag)
{
    // ex IntNotificationMessage::setConfirmed
    if (msg != 0) {
        msg->confirmed = flag;
    }
}

void
game::interface::NotificationStore::removeOrphanedMessages()
{
    size_t out = 0;
    for (size_t i = 0, n = m_messages.size(); i < n; ++i) {
        uint32_t pid;
        if (m_messages[i]->assoc.get(pid) && m_processList.getProcessById(pid) == 0) {
            // Message linked with a pid, but that pid does not exist - skip it
        } else {
            // Message shall be kept
            m_messages.swapElements(i, out);
            ++out;
        }
    }
    m_messages.resize(out);
}

// /** Make all confirmed processes runnable. Only affects processes that are
//     actually suspended. In particular, frozen processes are not changed. */
void
game::interface::NotificationStore::resumeConfirmedProcesses(uint32_t pgid)
{
    // ex IntNotificationMessageStore::makeConfirmedProcessesRunnable
    for (size_t i = 0, n = m_messages.size(); i < n; ++i) {
        uint32_t pid;
        if (m_messages[i]->assoc.get(pid)) {
            if (interpreter::Process* proc = m_processList.getProcessById(pid)) {
                if (proc->getState() == interpreter::Process::Suspended) {
                    m_processList.resumeProcess(*proc, pgid);
                }
            }
        }
    }
}

size_t
game::interface::NotificationStore::getNumMessages()
{
    // ex IntNotificationMessageStore::getNumMessages, IntNotificationMessageStore::getCount
    return m_messages.size();
}

String_t
game::interface::NotificationStore::getMessageText(size_t index, afl::string::Translator& tx, const PlayerList& /*players*/)
{
    // ex IntNotificationMessageStore::getText, IntNotificationMessage::getText
    String_t result;
    if (const Message* msg = getMessageByIndex(index)) {
        // Body
        result = msg->header + msg->body;

        // Extra info
        uint32_t pid;
        if (msg->assoc.get(pid)) {
            if (interpreter::Process* proc = m_processList.getProcessById(pid)) {
                if (msg->confirmed) {
                    result += tx("\n\nThis message has been confirmed.");
                } else if (proc->getProcessKind() != interpreter::Process::pkDefault) {
                    result += tx("\n\nThe auto task has been stopped; it will continue when you confirm this message.");
                } else {
                    result += tx("\n\nThe script has been stopped; it will continue when you confirm this message.");
                }
            }
        }
    }
    return result;
}

String_t
game::interface::NotificationStore::getMessageHeading(size_t index, afl::string::Translator& /*tx*/, const PlayerList& /*players*/)
{
    // ex IntNotificationMessageStore::getHeading, IntNotificationMessage::getHeader
    String_t result;
    if (const Message* msg = getMessageByIndex(index)) {
        result = msg->header;

        // Remove everything after first line
        String_t::size_type n = result.find('\n');
        if (n != String_t::npos) {
            result.erase(n);
        }

        // If message starts with '(-sXXX)', remove XXX
        if (result.size() > 3 && result[0] == '(') {
            result.erase(3, result.find(')', 3) - 3);
        }

        // Remove all angle brackets
         while ((n = result.find_first_of("<>")) != String_t::npos) {
             result.erase(n, 1);
         }
    }
    return result;
}

int
game::interface::NotificationStore::getMessageTurnNumber(size_t /*index*/)
{
    return 0;
}

bool
game::interface::NotificationStore::findMessage(ProcessAssociation_t assoc, size_t& index) const
{
    bool found = false;
    uint32_t pid;
    if (assoc.get(pid)) {
        for (size_t i = 0, n = m_messages.size(); i < n; ++i) {
            uint32_t theirPid;
            if (m_messages[i]->assoc.get(theirPid)) {
                if (theirPid == pid) {
                    index = i;
                    found = true;
                    break;
                }
            }
        }
    }
    return found;
}

// FIXME: needed?
// /** Get index of a message, given the IntNotificationMessage object.
//     \return index, nil if message not contained in this store. */
// IntNotificationMessageStore::index_t
// IntNotificationMessageStore::getIndexOfMessage(IntNotificationMessage* msg)
// {
//     for (index_t i = 0; i < messages.size(); ++i)
//         if (messages[i] == msg)
//             return i;
//     return nil;
// }

// FIXME: needed?
// const string_t&
// IntNotificationMessage::getBody() const
// {
//     return body;
// }

// FIXME: needed?
// /** Remove this message. This deletes (invalidates) the
//     IntNotificationMessage object. */
// void
// IntNotificationMessage::remove()
// {
//     store.removeMessageByIndex(store.getIndexOfMessage(this));
// }

// FIXME: needed?
// /** Remove message, given an index. */
// void
// IntNotificationMessageStore::removeMessageByIndex(index_t idx)
// {
//     messages.erase(messages.begin() + idx);
//     if (idx < getCurrent()) {
//         setCurrent(getCurrent()-1);
//     }
//     sig_changed.raise();
// }

// FIXME: needed?
// inline void
// IntNotificationMessage::setAssociatedProcess(IntExecutionContext* process)
// {
//     associated_process = process;
// }

// FIXME: needed?
// inline IntExecutionContext*
// IntNotificationMessage::getAssociatedProcess() const
// {
//     return associated_process;
// }
