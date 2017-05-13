/**
  *  \file server/interface/talkthreadclient.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKTHREADCLIENT_HPP
#define C2NG_SERVER_INTERFACE_TALKTHREADCLIENT_HPP

#include "server/interface/talkthread.hpp"
#include "afl/net/commandhandler.hpp"

namespace server { namespace interface {

    class TalkThreadClient : public TalkThread {
     public:
        TalkThreadClient(afl::net::CommandHandler& commandHandler);
        ~TalkThreadClient();

        virtual Info getInfo(int32_t threadId);
        virtual void getInfo(afl::base::Memory<const int32_t> threadIds, afl::container::PtrVector<Info>& result);
        virtual afl::data::Value* getPosts(int32_t threadId, const ListParameters& params);
        virtual void setSticky(int32_t threadId, bool flag);
        virtual int getPermissions(int32_t threadId, afl::base::Memory<const String_t> permissionList);
        virtual void moveToForum(int32_t threadId, int32_t forumId);
        virtual bool remove(int32_t threadId);

        static Info unpackInfo(const afl::data::Value* p);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
