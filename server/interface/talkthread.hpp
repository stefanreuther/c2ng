/**
  *  \file server/interface/talkthread.hpp
  *  \brief Interface server::interface::TalkThread
  */
#ifndef C2NG_SERVER_INTERFACE_TALKTHREAD_HPP
#define C2NG_SERVER_INTERFACE_TALKTHREAD_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/memory.hpp"
#include "afl/base/types.hpp"
#include "afl/data/integerlist.hpp"
#include "afl/data/value.hpp"
#include "afl/string/string.hpp"
#include "server/interface/talkforum.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    /** Talk Thread Interface.
        This interface allows access and manipulation of forum threads.

        Forum threads are created by using TalkPost::create().
        Threads contain a hierarchy of postings.
        A thread can be sticky to allow the user interface to keep it on top even if there are other newer threads. */
    class TalkThread : public afl::base::Deletable {
     public:
        typedef TalkForum::ListParameters ListParameters;

        /** Information about a thread. */
        struct Info {
            String_t subject;       ///< Thread subject (plain text).
            int32_t forumId;        ///< Containing forum's Id.
            int32_t firstPostId;    ///< First (oldest) posting in this thread.
            int32_t lastPostId;     ///< Last (newest) posting in this thread.
            Time_t lastTime;        ///< Time of last posting in this thread.
            bool isSticky;          ///< True if thread is sticky.
            afl::data::IntegerList_t alsoPostedTo;  ///< List of forums this thread is cross-posted to.
        };

        /** Get information about a forum thread (THREADSTAT).
            \param threadId thread Id
            \return information */
        virtual Info getInfo(int32_t threadId) = 0;

        /** Get information about multiple forum threads (THREADMSTAT).
            \param [in] threadIds List of thread Ids
            \param [out] result Receives information */
        virtual void getInfo(afl::base::Memory<const int32_t> threadIds, afl::container::PtrVector<Info>& result) = 0;

        /** List postings in a thread (THREADLSPOST).
            \param threadId Thread Id
            \param params List parameters
            \return Newly-allocated value, as determined by parameters (can be single value or list) */
        virtual afl::data::Value* getPosts(int32_t threadId, const ListParameters& params) = 0;

        /** Set thread stickyness (THREADSTICKY).
            \param threadId thread Id
            \param flag Stickiness flag */
        virtual void setSticky(int32_t threadId, bool flag) = 0;

        /** Get thread permissions (THREADPERMS).
            \param threadId thread Id
            \param permissionList List of permission names to query
            \return bitfield */
        virtual int getPermissions(int32_t threadId, afl::base::Memory<const String_t> permissionList) = 0;

        /** Move thread to another forum (THREADMV).
            \param threadId thread Id
            \param forumId new forum Id */
        virtual void moveToForum(int32_t threadId, int32_t forumId) = 0;

        /** Remove a thread (THREADRM).
            \param threadId thread Id
            \return true if thread was removed, false if it did not exist */
        virtual bool remove(int32_t threadId) = 0;
    };

} }

#endif
