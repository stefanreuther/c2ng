/**
  *  \file server/talk/userfolder.hpp
  *  \brief Class server::talk::UserFolder
  */
#ifndef C2NG_SERVER_TALK_USERFOLDER_HPP
#define C2NG_SERVER_TALK_USERFOLDER_HPP

#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/subtree.hpp"
#include "server/interface/talkfolder.hpp"

namespace server { namespace talk {

    class User;
    class Root;

    /** User folder.
        Stores personal messages (UserPM).
        The user's folders (headers and content) are managed in his user profile tree.
        As a special exception, there are default folders provided by the system.
        Default values for their headers are stored in the "default" tree.
        Users cannot delete those folders (although they can create their own headers for them).

        A folder is a set of messages.
        This means it cannot contain two copies of a message. */
    class UserFolder {
     public:
        /** Constructor.
            @param user          User to create this folder for
            @param userFolderId  User's folder Id */
        UserFolder(User& user, int32_t userFolderId);

        /** Check existance of this folder.
            @param root Service root
            @retval true this is a user folder
            @retval false this is a default folder */
        bool checkExistance(Root& root);

        /** Access set of messages in this folder.
            @return set key */
        afl::net::redis::IntegerSetKey messages();

        /** Access header of this folder.
            This returns the user-specific header, even if the folder does not have one.
            A write access would create it.
            @return header
            @see getHeader() */
        afl::net::redis::HashKey header();

        /** Access flag for unread messages in this folder.
            This is vaguely specified to allow an unread message count later,
            but so far it is implemented as a flag only.
            @return flag */
        afl::net::redis::IntegerField unreadMessages();

        /** Get header value.
            Folders can have user-specific and global default headers.
            The latter are used for folders every user has.
            This function looks into the user-specific header first, then into the global one.
            That is, if the user has renamed his outbox, this will return the new name;
            if he has not touched it, it will return the global name.
            @param key  Header field name
            @param root Service root
            @return value */
        String_t getHeader(String_t key, Root& root);

        /** Describe this folder.
            @param isUser true iff this is a user folder
            @param root Server root
            @return description */
        server::interface::TalkFolder::Info describe(bool isUser, Root& root);

        /** Remove this folder from the database.
            Assumes that it has already been unlinked from global lists (e.g. User::pmFolders()),
            and all messages have been unlinked. */
        void remove();

        /** Allocate a new folder Id for a user.
            @param user User
            @return new folder Id (ufid) */
        static int32_t allocateFolder(User& user);

        /** Access default folder set.
            @param root Root
            @return set key */
        static afl::net::redis::IntegerSetKey defaultFolders(Root& root);

        /** Find folder containing a PM.
            @param user          User
            @param root          Root
            @param pmId          Message to find
            @param preferFolder  If message exists in this folder, use that
            @return folder Id, 0 if none */
        static int32_t findFolder(User& user, Root& root, int32_t pmId, int32_t preferFolder);

        /** Suggest folder for filing a PM.
            @param user          User
            @param root          Root
            @param pmId          Message to file
            @param excludeFolder Do not suggest this folder
            @return folder Id, 0 if none */
        static int32_t findSuggestedFolder(User& user, Root& root, int32_t pmId, int32_t excludeFolder);

     private:
        afl::net::redis::HashKey defaultHeader(Root& root);

        afl::net::redis::IntegerSetKey m_userFolderSet;
        afl::net::redis::Subtree m_userFolder;
        int32_t m_userFolderId;
    };

} }

#endif
