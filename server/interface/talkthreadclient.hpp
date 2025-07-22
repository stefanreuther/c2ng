/**
  *  \file server/interface/talkthreadclient.hpp
  *  \brief Class server::interface::TalkThreadClient
  */
#ifndef C2NG_SERVER_INTERFACE_TALKTHREADCLIENT_HPP
#define C2NG_SERVER_INTERFACE_TALKTHREADCLIENT_HPP

#include "server/interface/talkthread.hpp"
#include "afl/net/commandhandler.hpp"

namespace server { namespace interface {

    /** Client for TalkThread.
        Uses a CommandHandler to send commands to a server, and receives the results. */
    class TalkThreadClient : public TalkThread {
     public:
        /** Constructor.
            @param commandHandler Server connection. Lifetime must exceed that of the TalkThreadClient. */
        explicit TalkThreadClient(afl::net::CommandHandler& commandHandler);

        // TalkThread:
        virtual Info getInfo(int32_t threadId);
        virtual void getInfo(afl::base::Memory<const int32_t> threadIds, afl::container::PtrVector<Info>& result);
        virtual afl::data::Value* getPosts(int32_t threadId, const ListParameters& params);
        virtual void setSticky(int32_t threadId, bool flag);
        virtual int getPermissions(int32_t threadId, afl::base::Memory<const String_t> permissionList);
        virtual void moveToForum(int32_t threadId, int32_t forumId);
        virtual bool remove(int32_t threadId);

        /** Deserialize a TalkThread::Info.
            @param p Data received from server
            @return deserialized information */
        static Info unpackInfo(const afl::data::Value* p);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
