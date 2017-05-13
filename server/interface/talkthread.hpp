/**
  *  \file server/interface/talkthread.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKTHREAD_HPP
#define C2NG_SERVER_INTERFACE_TALKTHREAD_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/memory.hpp"
#include "afl/base/types.hpp"
#include "afl/string/string.hpp"
#include "afl/data/value.hpp"
#include "server/interface/talkforum.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    class TalkThread : public afl::base::Deletable {
     public:
        typedef TalkForum::ListParameters ListParameters;

        struct Info {
            String_t subject;
            int32_t forumId;
            int32_t firstPostId;
            int32_t lastPostId;
            Time_t lastTime;
            bool isSticky;
        };

        // @q THREADSTAT thread:TID (Talk Command)
        // @retval TalkThreadInfo information about thread
        virtual Info getInfo(int32_t threadId) = 0;

        // @q THREADMSTAT thread:TID... (Talk Command)
        // @retval TalkThreadInfo[] information
        virtual void getInfo(afl::base::Memory<const int32_t> threadIds, afl::container::PtrVector<Info>& result) = 0;

        // @q THREADLSPOST thread:TID [listParameters...] (Talk Command)
        virtual afl::data::Value* getPosts(int32_t threadId, const ListParameters& params) = 0;

        // @q THREADSTICKY thread:TID flag:Int (Talk Command)
        virtual void setSticky(int32_t threadId, bool flag) = 0;

        // @q THREADPERMS thread:TID [perm:Str ...] (Talk Command)
        // @retval Int permissions
        virtual int getPermissions(int32_t threadId, afl::base::Memory<const String_t> permissionList) = 0;

        // @q THREADMV thread:TID forum:FID (Talk Command)
        virtual void moveToForum(int32_t threadId, int32_t forumId) = 0;

        // @q THREADRM thread:TID (Talk Command)
        // @retval Int 0=thread did not exist, 1=thread removed
        virtual bool remove(int32_t threadId) = 0;
    };

} }

#endif
