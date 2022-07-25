/**
  *  \file server/talk/userfolder.cpp
  *  \brief Class server::talk::UserFolder
  */

#include <memory>
#include <stdexcept>
#include "server/talk/userfolder.hpp"
#include "afl/net/redis/integersetoperation.hpp"
#include "server/errors.hpp"
#include "server/talk/root.hpp"
#include "server/talk/user.hpp"
#include "server/talk/userpm.hpp"
#include "server/types.hpp"

namespace {
    /** Maximum number of loops for findSuggestedFolder().
        Go up only so many levels looking for a folder to store the message. */
    const int SUGGEST_LIMIT = 10;
}

// Constructor.
server::talk::UserFolder::UserFolder(User& user, int32_t userFolderId)
    : m_userFolderSet(user.pmFolders()),
      m_userFolder(user.pmFolderData().subtree(userFolderId)),
      m_userFolderId(userFolderId)
{ }

// Check existance of this folder.
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

// Access set of messages in this folder.
afl::net::redis::IntegerSetKey
server::talk::UserFolder::messages()
{
    // ex UserFolder::messages
    return m_userFolder.intSetKey("messages");
}

// Access header of this folder.
afl::net::redis::HashKey
server::talk::UserFolder::header()
{
    // ex UserFolder::header
    return m_userFolder.hashKey("header");
}

// Access flag for unread messages in this folder.
afl::net::redis::IntegerField
server::talk::UserFolder::unreadMessages()
{
    // ex UserFolder::unreadMessages
    return header().intField("unread");
}

// Get header value.
String_t
server::talk::UserFolder::getHeader(String_t key, Root& root)
{
    std::auto_ptr<afl::data::Value> result(header().field(key).getRawValue());
    if (result.get() == 0) {
        result.reset(defaultHeader(root).field(key).getRawValue());
    }
    return toString(result.get());
}

// Describe this folder.
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

// Remove this folder from the database.
void
server::talk::UserFolder::remove()
{
    // ex UserFolder::remove
    messages().remove();
    header().remove();
}

// Allocate a new folder Id for a user.
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

// Access default folder set.
afl::net::redis::IntegerSetKey
server::talk::UserFolder::defaultFolders(Root& root)
{
    // ex UserFolder::defaultFolders
    return root.defaultFolderRoot().intSetKey("all");
}

// Find folder containing a PM.
int32_t
server::talk::UserFolder::findFolder(User& user, Root& root, int32_t pmId, int32_t preferFolder)
{
    // Check preferred folder
    if (preferFolder != 0 && UserFolder(user, preferFolder).messages().contains(pmId)) {
        return preferFolder;
    }

    // Check other folders
    afl::data::IntegerList_t list;
    defaultFolders(root).merge(user.pmFolders()).getAll(list);
    std::sort(list.begin(), list.end());
    for (size_t i = 0, n = list.size(); i < n; ++i) {
        const int32_t folderId = list[i];
        if (preferFolder != folderId && UserFolder(user, folderId).messages().contains(pmId)) {
            return folderId;
        }
    }
    return 0;
}

// Suggest folder for filing a PM.
int32_t
server::talk::UserFolder::findSuggestedFolder(User& user, Root& root, int32_t pmId, int32_t excludeFolder)
{
    // Check user folders only (do not suggest filing in a system folder)
    // Required usecases:
    // - if you reply to a message and move that to folder X, suggest moving to X for further replies
    // - if you visit a message in folder X, but have moved its parent to Y, suggest moving to Y
    afl::data::IntegerList_t list;
    user.pmFolders().getAll(list);
    std::sort(list.begin(), list.end());

    for (int loop = 0; loop < SUGGEST_LIMIT; ++loop) {
        // Get message parent.
        // Refuse when reaching 0, or when Ids go backwards (=database inconsistency, cannot normally happen)
        const int32_t parentMessageId = UserPM(root, pmId).parentMessageId().get();

        if (parentMessageId == 0 || parentMessageId >= pmId) {
            break;
        }
        pmId = parentMessageId;

        for (size_t i = 0, n = list.size(); i < n; ++i) {
            const int32_t folderId = list[i];
            if (excludeFolder != folderId && UserFolder(user, folderId).messages().contains(pmId)) {
                return folderId;
            }
        }
    }

    return 0;
}

// Access default header. (Not exposed through service interface, thus, private here.)
afl::net::redis::HashKey
server::talk::UserFolder::defaultHeader(Root& root)
{
    // ex UserFolder::defaultHeader
    return root.defaultFolderRoot().subtree(m_userFolderId).hashKey("header");
}
