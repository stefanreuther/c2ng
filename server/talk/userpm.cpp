/**
  *  \file server/talk/userpm.cpp
  *  \brief Class server::talk::UserPM
  */

#include <stdexcept>
#include "server/talk/userpm.hpp"
#include "server/errors.hpp"
#include "server/talk/root.hpp"
#include "server/talk/user.hpp"
#include "server/talk/userfolder.hpp"

// Constructor.
server::talk::UserPM::UserPM(Root& root, int32_t pmId)
    : m_root(root),
      m_pmTree(root.pmRoot().subtree(pmId)),
      m_pmId(pmId)
{ }

// Wildcard constructor.
server::talk::UserPM::UserPM(Root& root, Wildcard /*wild*/)
    : m_root(root),
      m_pmTree(root.pmRoot().subtree("*")),
      m_pmId(0)
{ }

// Access message header.
afl::net::redis::HashKey
server::talk::UserPM::header()
{
    // ex UserPM::header
    return m_pmTree.hashKey("header");
}

// Access message author.
afl::net::redis::StringField
server::talk::UserPM::author()
{
    // ex UserPM::author
    return header().stringField("author");
}

// Access message receivers.
afl::net::redis::StringField
server::talk::UserPM::receivers()
{
    // ex UserPM::receivers
    return header().stringField("to");
}

// Access message submission time.
afl::net::redis::IntegerField
server::talk::UserPM::time()
{
    // ex UserPM::time
    return header().intField("time");
}

// Access message subject.
afl::net::redis::StringField
server::talk::UserPM::subject()
{
    // ex UserPM::subject
    return header().stringField("subject");
}

// Access message reference counter.
afl::net::redis::IntegerField
server::talk::UserPM::referenceCounter()
{
    // ex UserPM::referenceCounter
    return header().intField("ref");
}

// Access parent message Id.
afl::net::redis::IntegerField
server::talk::UserPM::parentMessageId()
{
    // ex UserPM::parentMessageId
    return header().intField("parent");
}

// Access message flags.
afl::net::redis::IntegerField
server::talk::UserPM::flags(String_t forUser)
{
    // ex UserPM::flags
    return header().intField("flags/" + forUser);
}

// Access message text.
afl::net::redis::StringKey
server::talk::UserPM::text()
{
    // ex UserPM::text
    return m_pmTree.stringKey("text");
}

// Describe this message.
server::interface::TalkPM::Info
server::talk::UserPM::describe(String_t forUser, int32_t folderId)
{
    // ex UserPM::describe
    server::interface::TalkPM::Info result;
    result.author    = author().get();
    result.receivers = receivers().get();
    result.time      = time().get();
    result.subject   = subject().get();
    result.flags     = flags(forUser).get();

    // Parent
    User u(m_root, forUser);
    if (int32_t parent = parentMessageId().get()) {
        // Return parent Id if known, even if it cannot be mapped to a folder
        result.parent = parent;

        // Try to map to a folder; only if we have that, produce further metadata
        int32_t parentFolder = UserFolder::findFolder(u, m_root, parent, folderId);
        if (parentFolder != 0) {
            result.parentSubject = UserPM(m_root, parent).subject().get();
            result.parentFolder = parentFolder;
            result.parentFolderName = UserFolder(u, parentFolder).getHeader("name", m_root);
        }
    }

    // Suggested folder
    if (int32_t suggestedFolder = UserFolder::findSuggestedFolder(u, m_root, m_pmId, folderId)) {
        result.suggestedFolder = suggestedFolder;
        result.suggestedFolderName = UserFolder(u, suggestedFolder).getHeader("name", m_root);
    }
    return result;
}

// Get message Id.
int32_t
server::talk::UserPM::getId() const
{
    // ex UserPM::getId
    return m_pmId;
}

// Add a reference.
void
server::talk::UserPM::addReference()
{
    // ex UserPM::addReference
    ++referenceCounter();
}

// Remove a reference.
void
server::talk::UserPM::removeReference()
{
    // ex UserPM::removeReference
    if (--referenceCounter() == 0) {
        header().remove();
        text().remove();
    }
}

// Allocate a PM.
int32_t
server::talk::UserPM::allocatePM(Root& root)
{
    // ex UserPM::allocatePM
    return ++root.pmRoot().intKey("id");
}


server::talk::UserPM::PMSorter::PMSorter(Root& root)
    : m_root(root)
{ }

void
server::talk::UserPM::PMSorter::applySortKey(afl::net::redis::SortOperation& op, const String_t& keyName) const
{
    // ex ListParams::pmSortKeys
    if (keyName == "AUTHOR") {
        op.by(m_root.pmRoot().subtree("*").hashKey("header").field("author")).sortLexicographical();
    } else if (keyName == "SUBJECT") {
        op.by(m_root.pmRoot().subtree("*").hashKey("header").field("subject")).sortLexicographical();
    } else if (keyName == "TIME") {
        op.by(m_root.pmRoot().subtree("*").hashKey("header").field("time"));
    } else {
        throw std::runtime_error(INVALID_SORT_KEY);
    }
}
