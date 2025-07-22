/**
  *  \file server/interface/talkuser.hpp
  *  \brief Class server::interface::TalkUser
  */
#ifndef C2NG_SERVER_INTERFACE_TALKUSER_HPP
#define C2NG_SERVER_INTERFACE_TALKUSER_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/types.hpp"
#include "afl/base/memory.hpp"
#include "server/interface/talkforum.hpp"

namespace server { namespace interface {

    /** Talk User interface.
        This interface allows access of user-specific information from the forum.
        In particular, it contains the list of read postings (newsrc). */
    class TalkUser : public afl::base::Deletable {
     public:
        /** Newsrc modification operation. */
        enum Modification {
            NoModification,     ///< No modification (default).
            MarkRead,           ///< Mark read (SET).
            MarkUnread          ///< Mark unread (CLEAR).
        };

        /** Desired result of newsrc operation. */
        enum Result {
            NoResult,           ///< No result ("OK").
            GetAll,             ///< Get all "read" bits (GET).
            CheckIfAnyRead,     ///< Produce "1" if anything was read, "0" if everything was unread (ANY).
            CheckIfAllRead,     ///< Produce "1" if everything was read, "0" if anything was unread (ALL).
            GetFirstRead,       ///< Return Id of first read item, 0 if none (FIRSTSET).
            GetFirstUnread      ///< Return Id of first unread item, 0 if none (FIRSTCLEAR).
        };

        /** Scope of a Selection. */
        enum Scope {
            ForumScope,         ///< Process a forum given by Id.
            ThreadScope,        ///< Process a topic given by Id.
            RangeScope          ///< Process a message range.
        };

        /** Selection of messages for an operation. */
        struct Selection {
            Scope scope;        ///< Scope.
            int32_t id;         ///< Forum or topic Id, first message Id.
            int32_t lastId;     ///< Last message Id.
        };

        typedef TalkForum::ListParameters ListParameters;

        /** Access newsrc (USERNEWSRC).
            @param modif       Desired modification
            @param res         Desired result
            @param selections  Selection of messages to process
            @param posts       Individual messages to process
            @return Result; newly-allocated object, type depending on @c res. */
        virtual afl::data::Value* accessNewsrc(Modification modif, Result res, afl::base::Memory<const Selection> selections, afl::base::Memory<const int32_t> posts) = 0;

        /** Watch threads/forums for modifications (USERWATCH).
            @param selections  Selection of forums/threads to process. A selection scope of RangeScope is not permitted. */
        virtual void watch(afl::base::Memory<const Selection> selections) = 0;

        /** Stop watching threads/forums for modifications (USERUNWATCH).
            @param selections  Selection of forums/threads to process. A selection scope of RangeScope is not permitted. */
        virtual void unwatch(afl::base::Memory<const Selection> selections) = 0;

        /** Mark messages seen (USERMARKSEEN).
            @param selections  Selection of messages to process */
        virtual void markSeen(afl::base::Memory<const Selection> selections) = 0;

        /** Get list of watched threads (USERLSWATCHEDTHREADS).
            @param params  List operation parameters (e.g. sort order)
            @return Result; newly-allocated object, type depending on operation. */
        virtual afl::data::Value* getWatchedThreads(const ListParameters& params) = 0;

        /** Get list of watched forums (USERLSWATCHEDFORUMS).
            @param params  List operation parameters (e.g. sort order)
            @return Result; newly-allocated object, type depending on operation. */
        virtual afl::data::Value* getWatchedForums(const ListParameters& params) = 0;

        /** List posted messages (USERLSPOSTED).
            @param user    User whose messages to list
            @param params  List operation parameters (e.g. sort order)
            @return Result; newly-allocated object, type depending on operation. */
        virtual afl::data::Value* getPostedMessages(String_t user, const ListParameters& params) = 0;

        /** List forums user is allowed to cross-post to (USERLSCROSS).
            This checks the condition of the User::isAllowedToCrosspostToGames() permission.
            No specific command is provided for User::isAllowedToCrosspost(); a user with that permission can cross-post anywhere.
            @param params  List operation parameters (e.g. sort order)
            @return Result; newly-allocated object, type depending on operation. */
        virtual afl::data::Value* getCrosspostToGameCandidates(const ListParameters& params) = 0;
    };

} }

#endif
