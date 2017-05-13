/**
  *  \file server/talk/message.cpp
  *  \brief Class server::talk::Message
  */

#include "server/talk/message.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/sys/parsedtime.hpp"
#include "afl/sys/time.hpp"
#include "server/errors.hpp"
#include "server/talk/forum.hpp"
#include "server/talk/root.hpp"
#include "server/talk/topic.hpp"
#include "server/talk/user.hpp"
#include "util/string.hpp"

using afl::string::Format;

// Constructor.
server::talk::Message::Message(Root& root, int32_t messageId)
    : m_message(root.messageRoot().subtree(messageId)),
      m_messageId(messageId)
{
    // ex Message::Message
}

// Access message header.
afl::net::redis::HashKey
server::talk::Message::header()
{
    // ex Message::header
    return m_message.hashKey("header");
}

// Access topic Id.
afl::net::redis::IntegerField
server::talk::Message::topicId()
{
    // ex Message::topicId
    return header().intField("thread");
}

// Access parent message.
afl::net::redis::IntegerField
server::talk::Message::parentMessageId()
{
    // ex Message::parentMessageId
    return header().intField("parent");
}

// Access post time.
afl::net::redis::IntegerField
server::talk::Message::postTime()
{
    // ex Message::postTime
    return header().intField("time");
}

// Access edit time.
afl::net::redis::IntegerField
server::talk::Message::editTime()
{
    // ex Message::editTime
    return header().intField("edittime");
}

// Access author of message.
afl::net::redis::StringField
server::talk::Message::author()
{
    // ex Message::author
    return header().stringField("author");
}

// Access subject of message.
afl::net::redis::StringField
server::talk::Message::subject()
{
    // ex Message::subject
    return header().stringField("subject");
}

// Access RfC Message Id of message, if present.
afl::net::redis::StringField
server::talk::Message::rfcMessageId()
{
    // ex Message::rfcMessageId
    return header().stringField("msgid");
}

// Access RfC headers.
afl::net::redis::StringField
server::talk::Message::rfcHeaders()
{
    // ex Message::rfcHeaders
    return header().stringField("rfcheader");
}

// Access sequence number.
afl::net::redis::IntegerField
server::talk::Message::sequenceNumber()
{
    // ex Message::sequenceNumber
    return header().intField("seq");
}

// Access previous sequence number.
afl::net::redis::IntegerField
server::talk::Message::previousSequenceNumber()
{
    // ex Message::previousSequenceNumber
    return header().intField("prevseq");
}

// Access previous RfC Message Id, if present.
afl::net::redis::StringField
server::talk::Message::previousRfcMessageId()
{
    // ex Message::previousRfcMessageId
    return header().stringField("prevmsgid");
}

// Access message text.
afl::net::redis::StringKey
server::talk::Message::text()
{
    // ex Message::text
    return m_message.stringKey("text");
}

// Check existance.
bool
server::talk::Message::exists()
{
    // ex Message::exists
    // A message exists if it has any header information.
    // Mandatory header inforamation is an author and a topic link, so a message cannot sensibly exist without a header.
    return header().exists();
}

// Remove this message.
void
server::talk::Message::remove(Root& root)
{
    // ex Message::remove
    // Remove from sets, so it becomes invisible
    Topic t(topic(root));
    Forum f(t.forum(root));
    t.messages().remove(m_messageId);
    f.messages().remove(m_messageId);
    User(root, author().get()).postedMessages().remove(m_messageId);

    // Remove from NNTP side
    removeRfcMessageId(root, rfcMessageId().get());

    // If the topic is now empty, remove it completely
    if (t.messages().empty()) {
        t.removeEmpty(root);
    }

    // Remove post
    text().remove();
    header().remove();
}

// Access message topic.
server::talk::Topic
server::talk::Message::topic(Root& root)
{
    // ex Message::topic
    return Topic(root, topicId().get());
}

// Get message Id.
int32_t
server::talk::Message::getId() const
{
    // ex Message::getId
    return m_messageId;
}

// Describe message.
server::interface::TalkPost::Info
server::talk::Message::describe(const Root& root)
{
    // ex Message::describe
    // FIXME: can we use HMGET?
    server::interface::TalkPost::Info info;
    info.threadId     = topicId().get();
    info.parentPostId = parentMessageId().get();
    info.postTime     = postTime().get();
    info.editTime     = editTime().get();
    info.author       = author().get();
    info.subject      = subject().get();
    info.rfcMessageId = getRfcMessageId(root);
    return info;
}

// Get RfC Message Id.
String_t
server::talk::Message::getRfcMessageId(const Root& root)
{
    // ex Message::getRfcMessageId
    String_t msgid = rfcMessageId().get();
    if (msgid.empty()) {
        msgid = Format("%d.%d%s", m_messageId, sequenceNumber().get(), root.config().messageIdSuffix);
    }
    return msgid;
}

// Get Previous RfC Message Id.
String_t
server::talk::Message::getPreviousRfcMessageId(const Root& root)
{
    // ex Message::getPreviousRfcMessageId
    String_t msgid = previousRfcMessageId().get();
    if (msgid.empty()) {
        // @change verify that we actually have a previous sequence number!
        int32_t prevSeq = previousSequenceNumber().get();
        if (prevSeq != 0) {
            msgid = Format("%d.%d%s", m_messageId, prevSeq, root.config().messageIdSuffix);
        }
    }
    return msgid;
}

// Get RfC header.
afl::data::Hash::Ref_t
server::talk::Message::getRfcHeader(Root& root)
{
    using util::encodeMimeHeader;

    // ex Message::getRfcHeader
    Topic t(topic(root));
    Forum f(t.forum(root));
    const String_t userId(author().get());
    User u(root, userId);

    afl::data::Hash::Ref_t head = afl::data::Hash::create();

    // Id pseudo-header
    head->setNew(":Id", makeIntegerValue(m_messageId));

    // Sequence pseudo-header
    int32_t seq = sequenceNumber().get();
    head->setNew(":Seq", makeIntegerValue(seq));

    // Xref
    head->setNew("Xref", makeStringValue(Format("%s %s:%d", root.config().pathHost, f.getNewsgroup(), seq)));

    // Path
    head->setNew("Path", makeStringValue(Format("%s!not-for-mail", root.config().pathHost)));

    // Message Ids
    head->setNew("Message-Id", makeStringValue(Format("<%s>", getRfcMessageId(root))));

    String_t prevMsgId = getPreviousRfcMessageId(root);
    if (!prevMsgId.empty()) {
        head->setNew("Supersedes", makeStringValue(Format("<%s>", prevMsgId)));
    }

    // From
    String_t userName(u.getLoginName());
    String_t email;
    if (u.profile().intField("infoemailflag").get()) { // FIXME: regular profile access?
        email = u.profile().stringField("email").get();
        if (!email.empty()) {
            if (root.emailRoot().subtree(email).hashKey("status").stringField(Format("status/%s", userId)).get() != "c") {
                email.clear();
            }
        }
    }
    if (email.empty()) {
        email = userName + "@invalid.invalid";
    }

    String_t realName = u.getRealName();
    if (realName.empty()) {
        realName = u.getScreenName();
    }
    head->setNew("From", makeStringValue(Format("%s <%s>", encodeMimeHeader(realName, "UTF-8"), encodeMimeHeader(email, "UTF-8"))));

    // Newsgroups
    head->setNew("Newsgroups", makeStringValue(f.getNewsgroup()));

    // Subject
    head->setNew("Subject", makeStringValue(encodeMimeHeader(subject().get(), "UTF-8")));

    // Date
    int32_t pt = postTime().get();
    int32_t et = editTime().get();
    afl::sys::ParsedTime effTime;
    unpackTime(et ? et : pt).unpack(effTime, afl::sys::Time::UniversalTime);
    head->setNew("Date", makeStringValue(effTime.format("%a, %d %b %Y %H:%M:%S +0000")));

    // References
    int32_t parent = parentMessageId().get();
    if (parent != 0) {
        String_t refs;

        // Fetch 5 message Ids
        for (int i = 0; i < 5 && parent != 0; ++i) {
            Message m(root, parent);
            if (!refs.empty()) {
                refs.insert(0, "\r\n ");
            }
            refs.insert(0, Format("<%s>", m.getRfcMessageId(root)));
            parent = m.parentMessageId().get();
        }

        // Still more to do? Get thread starter
        if (parent != 0) {
            Message m(root, t.firstPostingId().get());
            if (!refs.empty()) {
                refs.insert(0, "\r\n ");
            }
            refs.insert(0, Format("<%s>", m.getRfcMessageId(root)));
        }
        head->setNew("References", makeStringValue(refs));
    }

    // Fake a bytes/lines size. For a precise value, we'd have to render the posting.
    // But let's keep it simple, it's used for a preview whether this is a long posting
    // or not, anyway, and I hope nobody uses it for allocating buffers.
    int32_t bytes = int32_t(text().size());
    head->setNew(":Bytes", makeIntegerValue(bytes));
    head->setNew(":Lines", makeIntegerValue(bytes/40 + 1));

    // MIME
    head->setNew("MIME-Version", makeStringValue("1.0"));
    head->setNew("Content-Type", makeStringValue("text/plain; charset=UTF-8"));
    head->setNew("Content-Transfer-Encoding", makeStringValue("quoted-printable"));

    // Extras
    head->setNew("X-PCC-User", makeStringValue(userName));
    head->setNew("X-PCC-Posting-Id", makeIntegerValue(m_messageId));

    return head;    
}


// Remove RfC Message Id.
// FIXME: here?
void
server::talk::Message::removeRfcMessageId(Root& root, String_t id)
{
    // ex Message::removeRfcMessageId
    if (!id.empty()) {
        root.rfcMessageIdRoot().intKey(id).remove();
    }
}

// Add RfC Message Id.
// FIXME: here?
void
server::talk::Message::addRfcMessageId(Root& root, String_t id, int32_t messageId)
{
    // ex Message::addRfcMessageId
    if (!id.empty() && messageId != 0) {
        root.rfcMessageIdRoot().intKey(id).set(messageId);
    }
}

// Look up a RfC Message Id.
// FIXME: here?
int32_t
server::talk::Message::lookupRfcMessageId(Root& root, String_t rfcMsgId)
{
    // ex Message::lookupRfcMessageId
    const String_t& msgid_suffix = root.config().messageIdSuffix;
    String_t::size_type dot = rfcMsgId.find('.');
    int32_t result;

    if (rfcMsgId.empty()) {
        // error
        result = 0;
    } else if (dot != String_t::npos
               && rfcMsgId.size() - dot > msgid_suffix.size()
               && rfcMsgId.compare(rfcMsgId.size() - msgid_suffix.size(), msgid_suffix.size(), msgid_suffix) == 0
               && afl::string::strToInteger(rfcMsgId.substr(0, dot), result)
               && result > 0)
    {
        // a message Id generated by us
    } else {
        // external Id
        result = root.rfcMessageIdRoot().intKey(rfcMsgId).get();
    }

    // Cross-check
    if (result > 0 && Message(root, result).getRfcMessageId(root) == rfcMsgId) {
        return result;
    } else {
        return 0;
    }
}

// Apply sort-by-sequence.
void
server::talk::Message::applySortBySequence(Root& root, afl::net::redis::SortOperation& op)
{
    op.by(root.messageRoot().subtree("*").hashKey("header").field("seq"));
}

// Apply sort-by-sequence and return sequence numbers.
void
server::talk::Message::applySortBySequenceMap(Root& root, afl::net::redis::SortOperation& op)
{
    op.by(root.messageRoot().subtree("*").hashKey("header").field("seq"));
    op.get(root.messageRoot().subtree("*").hashKey("header").field("seq"));
    op.get();
}

server::talk::Message::MessageSorter::MessageSorter(Root& root)
    : Sorter(),
      m_root(root)
{ }

void
server::talk::Message::MessageSorter::applySortKey(afl::net::redis::SortOperation& op, const String_t& keyName) const
{
    // ex ListParams::postSortKeys
    if (keyName == "AUTHOR") {
        op.by(m_root.messageRoot().subtree("*").hashKey("header").field("author")).sortLexicographical();
    } else if (keyName == "EDITTIME") {
        op.by(m_root.messageRoot().subtree("*").hashKey("header").field("edittime"));
    } else if (keyName == "SUBJECT") {
        op.by(m_root.messageRoot().subtree("*").hashKey("header").field("subject")).sortLexicographical();
    } else if (keyName == "THREAD") {
        op.by(m_root.messageRoot().subtree("*").hashKey("header").field("thread"));
    } else if (keyName == "TIME") {
        op.by(m_root.messageRoot().subtree("*").hashKey("header").field("time"));
    } else {
        throw std::runtime_error(INVALID_SORT_KEY);
    }    
}

