/**
  *  \file server/talk/talkforum.hpp
  *  \brief Class server::talk::TalkForum
  */
#ifndef C2NG_SERVER_TALK_TALKFORUM_HPP
#define C2NG_SERVER_TALK_TALKFORUM_HPP

#include "server/interface/talkforum.hpp"
#include "afl/net/redis/integersetkey.hpp"

namespace server { namespace talk {

    class Session;
    class Root;
    class Sorter;

    /** Implementation of FORUM commands. */
    class TalkForum : public server::interface::TalkForum {
     public:
        /** Constructor.
            \param session Session
            \param root Service root */
        TalkForum(Session& session, Root& root);

        // TalkForum interface:
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

        static afl::data::Value* executeListOperation(const ListParameters& params, afl::net::redis::IntegerSetKey key, const Sorter& sorter);

     private:
        Session& m_session;
        Root& m_root;
    };

} }

#endif
