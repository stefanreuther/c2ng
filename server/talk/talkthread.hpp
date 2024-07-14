/**
  *  \file server/talk/talkthread.hpp
  *  \brief Class server::talk::TalkThread
  */
#ifndef C2NG_SERVER_TALK_TALKTHREAD_HPP
#define C2NG_SERVER_TALK_TALKTHREAD_HPP

#include "server/interface/talkthread.hpp"

namespace server { namespace talk {

    class Session;
    class Root;

    /** Implementation of THREAD commands. */
    class TalkThread : public server::interface::TalkThread {
     public:
        /** Constructor.
            @param session Session
            @param root Service root */
        TalkThread(Session& session, Root& root);

        virtual Info getInfo(int32_t threadId);
        virtual void getInfo(afl::base::Memory<const int32_t> threadIds, afl::container::PtrVector<Info>& result);
        virtual afl::data::Value* getPosts(int32_t threadId, const ListParameters& params);
        virtual void setSticky(int32_t threadId, bool flag);
        virtual int getPermissions(int32_t threadId, afl::base::Memory<const String_t> permissionList);
        virtual void moveToForum(int32_t threadId, int32_t forumId);
        virtual bool remove(int32_t threadId);

     private:
        Session& m_session;
        Root& m_root;
    };

} }

#endif
