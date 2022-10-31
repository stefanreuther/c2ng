/**
  *  \file game/interface/notificationstore.cpp
  *  \brief Class game::interface::NotificationStore
  *
  *  FIXME: for consideration in c2ng: like in PCC2, NotificationStore implements Mailbox.
  *  This necessitates that it permanently knows a ProcessList, and enlargens the interface.
  *  It could make sense to separate the Mailbox implementation.
  *
  *  FIXME: for consideration in c2ng: 'confirmed' is a bool that is set when a message is confirmed.
  *  Every future call to resumeConfirmedProcesses() will resume that process,
  *  even if it has long proceeded, until the process generates a new message.
  *  This is normally harmless, but unnecessary and unexpected.
  *  PCC1 tracks that more precisely (ms_Stopped, ms_Continue, ms_Continued, ms_Terminate).
  *
  *  FIXME: it seems we don't need the distinction between header and body;
  *  it could be just a single long string.
  *
  *  Change to PCC2: our notifications live completely outside the interpreter function.
  *  In PCC2, processes have a pointer to their associated message.
  *  We perform the mapping using process Ids, and explicitly clean up when processes are removed.
  */

#include "game/interface/notificationstore.hpp"
#include "game/interface/processlisteditor.hpp"
#include "interpreter/process.hpp"
#include "util/unicodechars.hpp"

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

// Constructor.
game::interface::NotificationStore::NotificationStore(interpreter::ProcessList& processList)
    : Mailbox(), m_messages(), m_processList(processList)
{ }

// Destructor.
game::interface::NotificationStore::~NotificationStore()
{ }

// Find message associated with a process.
game::interface::NotificationStore::Message*
game::interface::NotificationStore::findMessageByProcessId(uint32_t processId) const
{
    size_t index;
    if (findMessage(ProcessAssociation_t(processId), index)) {
        return m_messages[index];
    } else {
        return 0;
    }
}

// Find message index by process Id.
bool
game::interface::NotificationStore::findIndexByProcessId(uint32_t processId, size_t& index) const
{
    return findMessage(ProcessAssociation_t(processId), index);
}

// Get message by index.
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

// Add new message.
game::interface::NotificationStore::Message*
game::interface::NotificationStore::addMessage(ProcessAssociation_t assoc, String_t header, String_t body)
{
    // ex IntNotificationMessageStore::addNewMessage (sort-of)
    // Remove previous message
    size_t index;
    if (findMessage(assoc, index)) {
        m_messages.erase(m_messages.begin() + index);
    }

    // Add new one
    return m_messages.pushBackNew(new Message(assoc, header, body));
}

// Check whether message is confirmed.
bool
game::interface::NotificationStore::isMessageConfirmed(const Message* msg) const
{
    // ex IntNotificationMessage::isConfirmed
    return msg != 0
        && msg->confirmed;
}

// Get message body text.
String_t
game::interface::NotificationStore::getMessageBody(const Message* msg) const
{
    return msg != 0
        ? msg->body
        : String_t();
}

// Confirm a message.
void
game::interface::NotificationStore::confirmMessage(Message* msg, bool flag)
{
    // ex IntNotificationMessage::setConfirmed
    if (msg != 0) {
        msg->confirmed = flag;
    }
}

// Remove orphaned messages.
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

// Resume processes associated with confirmed messages.
void
game::interface::NotificationStore::resumeConfirmedProcesses(ProcessListEditor& editor)
{
    // ex IntNotificationMessageStore::makeConfirmedProcessesRunnable
    // ex ccexec.pas:ProcessNotifyMessages (sort-of)
    // FIXME: PCC1 also allows a notification message to terminate the process (ms_Terminate)
    for (size_t i = 0, n = m_messages.size(); i < n; ++i) {
        uint32_t pid;
        if (m_messages[i]->confirmed) {
            if (m_messages[i]->assoc.get(pid)) {
                editor.setProcessState(pid, ProcessListEditor::Runnable);
            }
        }
    }
}

size_t
game::interface::NotificationStore::getNumMessages() const
{
    // ex IntNotificationMessageStore::getNumMessages, IntNotificationMessageStore::getCount
    return m_messages.size();
}

String_t
game::interface::NotificationStore::getMessageHeaderText(size_t index, afl::string::Translator& /*tx*/, const PlayerList& /*players*/) const
{
    // ex IntNotificationMessageStore::getText, IntNotificationMessage::getText
    if (const Message* msg = getMessageByIndex(index)) {
        return msg->header;
    } else {
        return String_t();
    }
}

String_t
game::interface::NotificationStore::getMessageBodyText(size_t index, afl::string::Translator& /*tx*/, const PlayerList& /*players*/) const
{
    // ex IntNotificationMessageStore::getText, IntNotificationMessage::getText
    if (const Message* msg = getMessageByIndex(index)) {
        return msg->body;
    } else {
        return String_t();
    }
}

String_t
game::interface::NotificationStore::getMessageForwardText(size_t index, afl::string::Translator& tx, const PlayerList& players) const
{
    return defaultGetMessageForwardText(index, tx, players);
}

String_t
game::interface::NotificationStore::getMessageReplyText(size_t index, afl::string::Translator& tx, const PlayerList& players) const
{
    return defaultGetMessageReplyText(index, tx, players);
}

util::rich::Text
game::interface::NotificationStore::getMessageDisplayText(size_t index, afl::string::Translator& tx, const PlayerList& players) const
{
    if (const Message* msg = getMessageByIndex(index)) {
        // Body
        util::rich::Text result = defaultGetMessageDisplayText(msg->header + msg->body, NoData, tx, players);

        // Extra info
        uint32_t pid;
        if (msg->assoc.get(pid)) {
            if (interpreter::Process* proc = m_processList.getProcessById(pid)) {
                if (msg->confirmed) {
                    addStatus(result, UTF_CHECK_MARK, util::SkinColor::Green, tx("This message has been confirmed."));
                } else if (proc->getProcessKind() != interpreter::Process::pkDefault) {
                    addStatus(result, UTF_STOPWATCH, util::SkinColor::Yellow, tx("The auto task has been stopped; it will continue when you confirm this message."));
                } else {
                    addStatus(result, UTF_STOPWATCH, util::SkinColor::Yellow, tx("The script has been stopped; it will continue when you confirm this message."));
                }
            }
        }
        return result;
    } else {
        return util::rich::Text();
    }
}

String_t
game::interface::NotificationStore::getMessageHeading(size_t index, afl::string::Translator& /*tx*/, const PlayerList& /*players*/) const
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

        // Remove whitespace
        result = afl::string::strTrim(result);
    }
    return result;
}

game::msg::Mailbox::Metadata
game::interface::NotificationStore::getMessageMetadata(size_t index, afl::string::Translator& /*tx*/, const PlayerList& /*players*/) const
{
    Metadata md;
    if (const Message* msg = getMessageByIndex(index)) {
        if (isMessageConfirmed(msg)) {
            md.flags += Confirmed;
        }
    }
    return md;
}

game::msg::Mailbox::Actions_t
game::interface::NotificationStore::getMessageActions(size_t index) const
{
    Actions_t result;
    const Message* msg = getMessageByIndex(index);
    if (!isMessageConfirmed(msg)) {
        result += ToggleConfirmed;
    }
    return result;
}

void
game::interface::NotificationStore::performMessageAction(size_t index, Action a)
{
    Message* msg = getMessageByIndex(index);
    switch (a) {
     case ToggleConfirmed:
        confirmMessage(msg, !isMessageConfirmed(msg));
        break;
    }
}

void
game::interface::NotificationStore::receiveMessageData(size_t /*index*/, game::parser::InformationConsumer& /*consumer*/, const TeamSettings& /*teamSettings*/, bool /*onRequest*/, afl::charset::Charset& /*cs*/)
{ }

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
