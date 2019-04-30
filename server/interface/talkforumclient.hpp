/**
  *  \file server/interface/talkforumclient.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKFORUMCLIENT_HPP
#define C2NG_SERVER_INTERFACE_TALKFORUMCLIENT_HPP

#include "afl/net/commandhandler.hpp"
#include "server/interface/talkforum.hpp"

namespace server { namespace interface {

    class TalkForumClient : public TalkForum {
     public:
        TalkForumClient(afl::net::CommandHandler& commandHandler);
        ~TalkForumClient();

        virtual int32_t add(afl::base::Memory<const String_t> config);
        virtual void configure(int32_t fid, afl::base::Memory<const String_t> config);
        virtual afl::data::Value* getValue(int32_t fid, String_t keyName);
        virtual Info getInfo(int32_t fid);
        virtual void getInfo(afl::base::Memory<const int32_t> fids, afl::container::PtrVector<Info>& result);
        virtual int32_t getPermissions(int32_t fid, afl::base::Memory<const String_t> permissionList);
        virtual Size getSize(int32_t fid);
        virtual afl::data::Value* getThreads(int32_t fid, const ListParameters& params);
        virtual afl::data::Value* getStickyThreads(int32_t fid, const ListParameters& params);
        virtual afl::data::Value* getPosts(int32_t fid, const ListParameters& params);
        virtual int32_t findForum(String_t key);

        static Info unpackInfo(const afl::data::Value* value);
        static void packListParameters(afl::data::Segment& cmd, const ListParameters& params);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
