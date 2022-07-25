/**
  *  \file server/talk/userpm.hpp
  *  \brief Class server::talk::UserPM
  */
#ifndef C2NG_SERVER_TALK_USERPM_HPP
#define C2NG_SERVER_TALK_USERPM_HPP

#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/stringkey.hpp"
#include "afl/net/redis/subtree.hpp"
#include "server/interface/talkpm.hpp"
#include "server/talk/sorter.hpp"

namespace server { namespace talk {

    class Root;

    /** A user's personal message.
        This uses a single-instance store for messages.
        Each message has a reference counter.
        A message to multiple users is stored only once.
        Also, the sender's outbox copy is another reference of the message.

        Individual messages have no access control.
        In the protocol, messages are always addressed using a user folder Id (ufid) and a PM Id.
        Since folder addresses are user-specific, this will always address the user's copy,
        and a message a user shall not be able to access will just not be addressable.

        As a drawback, it is not possible to make a stable URL for a message
        because that would always have to include the folder. */
    class UserPM {
     public:
        /** Constructor.
            \param root Service root
            \param pmId Identifier */
        UserPM(Root& root, int32_t pmId);

        /** Wildcard constructor.
            \param root Service root
            \param wild Marker */
        UserPM(Root& root, Wildcard wild);

        /*
         *  Database Fields
         */

        /** Access message header.
            \return header */
        afl::net::redis::HashKey header();

        /** Access message author.
            Format: a user Id
            \return author field */
        afl::net::redis::StringField author();

        /** Access message receivers.
            Format: comma-separated string containing
            - u:uid (users)
            - g:gid (all players in a game)
            - g:gid:slot (slot in a game)
            \return receivers field */
        afl::net::redis::StringField receivers();

        /** Access message submission time.
            Format: a Time_t.
            \return time field */
        afl::net::redis::IntegerField time();

        /** Access message subject.
            Format: string
            \return subject field */
        afl::net::redis::StringField subject();

        /** Access message reference counter.
            Note that you normally use addReference()/removeReference() instead.
            \return reference counter field */
        afl::net::redis::IntegerField referenceCounter();

        /** Access parent message Id.
            Format: a pmid if there is a parent message, 0 otherwise.
            \return parent field */
        afl::net::redis::IntegerField parentMessageId();

        /** Access message flags.
            Flags are user-specific (but not folder-specific).
            \param forUser User Id
            \return flags field */
        afl::net::redis::IntegerField flags(String_t forUser);

        /** Access message text.
            Format: a string suitable for rendering (i.e. type prefix + text).
            \return message text field */
        afl::net::redis::StringKey text();

        /*
         *  Higher-Level Operations
         */

        /** Describe this message.
            Produces an Info structure containing the message's metainformation.
            Since this contains the flags, it is user-specific.
            \param forUser User Id
            \param folderId Folder Id
            \return Info structure */
        server::interface::TalkPM::Info describe(String_t forUser, int32_t folderId);

        /** Get message Id.
            \return the Id used when constructing this object. */
        int32_t getId() const;

        /** Add a reference.
            Call whenever adding this message to a folder. */
        void addReference();

        /** Remove a reference.
            If this causes the reference count to drop to zero, removes the message from the database.
            Call whenever removing this message from a folder. */
        void removeReference();

        /** Allocate a PM.
            \param root Service root
            \return pmid for a new message. Does not correspond to an already-active message, and is never zero. */
        static int32_t allocatePM(Root& root);

        /** Message sorter.
            Pass this object to executeListOperation() if the list contains a list of PMs. */
        class PMSorter : public Sorter {
         public:
            PMSorter(Root& root);
            virtual void applySortKey(afl::net::redis::SortOperation& op, const String_t& keyName) const;
         private:
            Root& m_root;
        };

     private:
        /** Root. */
        Root& m_root;

        /** Database subtree. */
        afl::net::redis::Subtree m_pmTree;

        /** PM Id. */
        const int32_t m_pmId;
    };

} }

#endif
