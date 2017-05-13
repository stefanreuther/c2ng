/**
  *  \file u/t_server_interface_talkpost.cpp
  *  \brief Test for server::interface::TalkPost
  */

#include "server/interface/talkpost.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceTalkPost::testInterface()
{
    class Tester : public server::interface::TalkPost {
     public:
        virtual int32_t create(int32_t /*forumId*/, String_t /*subject*/, String_t /*text*/, const CreateOptions& /*options*/)
            { return 0; }
        virtual int32_t reply(int32_t /*parentPostId*/, String_t /*subject*/, String_t /*text*/, const ReplyOptions& /*options*/)
            { return 0; }
        virtual void edit(int32_t /*postId*/, String_t /*subject*/, String_t /*text*/)
            { }
        virtual String_t render(int32_t /*postId*/, const server::interface::TalkRender::Options& /*options*/)
            { return String_t(); }
        virtual void render(afl::base::Memory<const int32_t> /*postIds*/, afl::data::StringList_t& /*result*/)
            { }
        virtual Info getInfo(int32_t /*postId*/)
            { return Info(); }
        virtual void getInfo(afl::base::Memory<const int32_t> /*postIds*/, afl::container::PtrVector<Info>& /*result*/)
            { }
        virtual String_t getHeaderField(int32_t /*postId*/, String_t /*fieldName*/)
            { return String_t(); }
        virtual bool remove(int32_t /*postId*/)
            { return false; }
        virtual void getNewest(int /*count*/, std::vector<int32_t>& /*postIds*/)
            { }
    };
    Tester t;
}

