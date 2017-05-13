/**
  *  \file server/interface/talkuser.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKUSER_HPP
#define C2NG_SERVER_INTERFACE_TALKUSER_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/types.hpp"
#include "afl/base/memory.hpp"
#include "server/interface/talkforum.hpp"

namespace server { namespace interface {

    class TalkUser : public afl::base::Deletable {
     public:
        enum Modification {
            NoModification,     // no modification (default)
            MarkRead,           // mark read (SET)
            MarkUnread          // mark unread (CLEAR)
        };
        enum Result {
            NoResult,           // no result ("OK")
            GetAll,             // get all "read" bits (GET)
            CheckIfAnyRead,     // produce "1" if anything was read, "0" if everything was unread (ANY)
            CheckIfAllRead,     // produce "1" if everything was read, "0" if anything was unread (ALL)
            GetFirstRead,       // return Id of first read item, 0 if none (FIRSTSET)
            GetFirstUnread      // return Id of first unread item, 0 if none (FIRSTCLEAR)
        };

        enum Scope {
            ForumScope,
            ThreadScope,
            RangeScope
        };
        struct Selection {
            Scope scope;
            int32_t id;
            int32_t lastId;
        };

        typedef TalkForum::ListParameters ListParameters;

        // USERNEWSRC action:Str [range...] (Talk Command)
        virtual afl::data::Value* accessNewsrc(Modification modif, Result res, afl::base::Memory<const Selection> selections, afl::base::Memory<const int32_t> posts) = 0;

        // USERWATCH [THREAD n:TID] [FORUM n:FID]... (Talk Command)
        // using the 'Selection' type allows to send an illegal "USERWATCH RANGE ..." command
        virtual void watch(afl::base::Memory<const Selection> selections) = 0;

        // USERUNWATCH [THREAD n:TID] [FORUM n:FID]... (Talk Command)
        virtual void unwatch(afl::base::Memory<const Selection> selections) = 0;

        // USERMARKSEEN [THREAD n:TID] [FORUM n:FID]... (Talk Command)
        virtual void markSeen(afl::base::Memory<const Selection> selections) = 0;

        // USERLSWATCHEDTHREADS [listParameters...] (Talk Command)
        virtual afl::data::Value* getWatchedThreads(const ListParameters& params) = 0;

        // USERLSWATCHEDFORUMS [listParameters...] (Talk Command)
        virtual afl::data::Value* getWatchedForums(const ListParameters& params) = 0;

        // USERLSPOSTED user:UID [listParameters...] (Talk Command)
        virtual afl::data::Value* getPostedMessages(String_t user, const ListParameters& params) = 0;
    };

} }

#endif
