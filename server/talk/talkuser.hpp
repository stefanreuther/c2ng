/**
  *  \file server/talk/talkuser.hpp
  *  \brief Class server::talk::TalkUser
  */
#ifndef C2NG_SERVER_TALK_TALKUSER_HPP
#define C2NG_SERVER_TALK_TALKUSER_HPP

#include "server/interface/talkuser.hpp"

namespace server { namespace talk {

    class Session;
    class Root;

    /** Implementation of USER commands. */
    class TalkUser : public server::interface::TalkUser {
     public:
        /** Constructor.
            @param session Session
            @param root Service root */
        TalkUser(Session& session, Root& root);

        // TalkUser:
        virtual afl::data::Value* accessNewsrc(Modification modif, Result res, afl::base::Memory<const Selection> selections, afl::base::Memory<const int32_t> posts);
        virtual void watch(afl::base::Memory<const Selection> selections);
        virtual void unwatch(afl::base::Memory<const Selection> selections);
        virtual void markSeen(afl::base::Memory<const Selection> selections);
        virtual afl::data::Value* getWatchedThreads(const ListParameters& params);
        virtual afl::data::Value* getWatchedForums(const ListParameters& params);
        virtual afl::data::Value* getPostedMessages(String_t user, const ListParameters& params);
        virtual afl::data::Value* getCrosspostToGameCandidates(const ListParameters& params);

     private:
        Session& m_session;
        Root& m_root;

        enum WatchAction {
            Watch,
            Unwatch,
            MarkSeen
        };
        void processWatch(WatchAction action, afl::base::Memory<const TalkUser::Selection> selections);
    };

} }

#endif
