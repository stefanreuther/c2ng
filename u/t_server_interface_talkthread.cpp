/**
  *  \file u/t_server_interface_talkthread.cpp
  *  \brief Test for server::interface::TalkThread
  */

#include "server/interface/talkthread.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceTalkThread::testInterface()
{
    class Tester : public server::interface::TalkThread {
     public:
        virtual Info getInfo(int32_t /*threadId*/)
            { return Info(); }
        virtual void getInfo(afl::base::Memory<const int32_t> /*threadIds*/, afl::container::PtrVector<Info>& /*result*/)
            { }
        virtual afl::data::Value* getPosts(int32_t /*threadId*/, const ListParameters& /*params*/)
            { return 0; }
        virtual void setSticky(int32_t /*threadId*/, bool /*flag*/)
            { }
        virtual int getPermissions(int32_t /*threadId*/, afl::base::Memory<const String_t> /*permissionList*/)
            { return 0; }
        virtual void moveToForum(int32_t /*threadId*/, int32_t /*forumId*/)
            { }
        virtual bool remove(int32_t /*threadId*/)
            { return false; }
    };
    Tester t;
}

