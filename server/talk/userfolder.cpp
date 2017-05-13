/**
  *  \file server/talk/userfolder.cpp
  */

#include <memory>
#include <stdexcept>
#include "server/talk/userfolder.hpp"
#include "server/talk/user.hpp"
#include "server/errors.hpp"
#include "server/types.hpp"
#include "server/talk/root.hpp"

// /** Constructor.
//     \param user User to create this folder for
//     \param userFolderId User's folder Id */
server::talk::UserFolder::UserFolder(User& user, int32_t userFolderId)
    : m_userFolderSet(user.pmFolders()),
      m_userFolder(user.pmFolderData().subtree(userFolderId)),
      m_userFolderId(userFolderId)
{ }

// /** Check existance of this folder.
//     \retval true this is a user folder
//     \retval false this is a default folder */
bool
server::talk::UserFolder::checkExistance(Root& root)
{
    // ex UserFolder::checkExistance
    if (m_userFolderSet.contains(m_userFolderId)) {
        return true;
    } else if (defaultFolders(root).contains(m_userFolderId)) {
        return false;
    } else {
        throw std::runtime_error(FOLDER_NOT_FOUND);
    }
}

// /** Set of messages in this folder. */
afl::net::redis::IntegerSetKey
server::talk::UserFolder::messages()
{
    // ex UserFolder::messages
    return m_userFolder.intSetKey("messages");
}

// /** Header of this folder. This returns the user-specific header,
//     even if the folder does not have one. A write access would create it.
//     See getHeader() for details. */
afl::net::redis::HashKey
server::talk::UserFolder::header()
{
    // ex UserFolder::header
    return m_userFolder.hashKey("header");
}

// /** Flag for unread messages in this folder.
//     This is vaguely specified to allow an unread message count later, but
//     so far it is implemented as a flag only. */
afl::net::redis::IntegerField
server::talk::UserFolder::unreadMessages()
{
    // ex UserFolder::unreadMessages
    return header().intField("unread");
}

// /** Get header element. Folders can have user-specific and global
//     default headers. The latter are used for folders every user has.
//     This looks into the user-specific header first, then into the
//     global one. That is, if the user has renamed his outbox, this will
//     return the new name; if he has not touched it, it will return the
//     global name. */
String_t
server::talk::UserFolder::getHeader(String_t key, Root& root)
{
    std::auto_ptr<afl::data::Value> result(header().field(key).getRawValue());
    if (result.get() == 0) {
        result.reset(defaultHeader(root).field(key).getRawValue());
    }
    return toString(result.get());
}

// /** Describe this folder.
//     \param isUser true iff this is a user folder */
server::interface::TalkFolder::Info
server::talk::UserFolder::describe(bool isUser, Root& root)
{
    // ex UserFolder::describe (part)
    server::interface::TalkFolder::Info result;
    result.name              = getHeader("name", root);
    result.description       = getHeader("description", root);
    result.numMessages       = messages().size();
    result.hasUnreadMessages = unreadMessages().get();
    result.isFixedFolder     = !isUser;
    return result;
}

// /** Remove this folder.
//     Assumes that it has already been unlinked, as well as all the messages. */
void
server::talk::UserFolder::remove()
{
    // ex UserFolder::remove
    messages().remove();
    header().remove();
}

// /** Allocate a new folder Id. */
int32_t
server::talk::UserFolder::allocateFolder(User& user)
{
    /* We want the user's first folder to be 100. Writing it this way
       makes sure that there will not be a folder below 100, even with
       concurrent accesses, although if there actually are concurrent
       accesses, initial folders might be a little bigger than expected.
       But this is not a problem. */
    int32_t result = ++user.pmFolderCount();
    if (result < 100) {
        result = user.pmFolderCount() += (100 - result);
    }
    return result;
}

afl::net::redis::IntegerSetKey
server::talk::UserFolder::defaultFolders(Root& root)
{
    // ex UserFolder::defaultFolders
    return root.defaultFolderRoot().intSetKey("all");
}

afl::net::redis::HashKey
server::talk::UserFolder::defaultHeader(Root& root)
{
    // ex UserFolder::defaultHeader
    return root.defaultFolderRoot().subtree(m_userFolderId).hashKey("header");
}
