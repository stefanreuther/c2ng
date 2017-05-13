/**
  *  \file server/talk/userfolder.hpp
  */
#ifndef C2NG_SERVER_TALK_USERFOLDER_HPP
#define C2NG_SERVER_TALK_USERFOLDER_HPP

#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/subtree.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
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
        UserFolder(User& user, int32_t userFolderId);

        // FIXME: better name...
        bool checkExistance(Root& root);

        afl::net::redis::IntegerSetKey messages();
        afl::net::redis::HashKey header();
        afl::net::redis::IntegerField unreadMessages();

        String_t getHeader(String_t key, Root& root);

        server::interface::TalkFolder::Info describe(bool isUser, Root& root);

        void remove();

        static int32_t allocateFolder(User& user);
        static afl::net::redis::IntegerSetKey defaultFolders(Root& root);

     private:
        afl::net::redis::HashKey defaultHeader(Root& root);

        afl::net::redis::IntegerSetKey m_userFolderSet;
        afl::net::redis::Subtree m_userFolder;
        int32_t m_userFolderId;
    };

} }

#endif
