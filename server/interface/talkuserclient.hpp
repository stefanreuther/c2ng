/**
  *  \file server/interface/talkuserclient.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKUSERCLIENT_HPP
#define C2NG_SERVER_INTERFACE_TALKUSERCLIENT_HPP

#include "afl/net/commandhandler.hpp"
#include "server/interface/talkuser.hpp"

namespace server { namespace interface {

    class TalkUserClient : public TalkUser {
     public:
        explicit TalkUserClient(afl::net::CommandHandler& commandHandler);
        virtual ~TalkUserClient();

        virtual afl::data::Value* accessNewsrc(Modification modif, Result res, afl::base::Memory<const Selection> selections, afl::base::Memory<const int32_t> posts);
        virtual void watch(afl::base::Memory<const Selection> selections);
        virtual void unwatch(afl::base::Memory<const Selection> selections);
        virtual void markSeen(afl::base::Memory<const Selection> selections);
        virtual afl::data::Value* getWatchedThreads(const ListParameters& params);
        virtual afl::data::Value* getWatchedForums(const ListParameters& params);
        virtual afl::data::Value* getPostedMessages(String_t user, const ListParameters& params);
        virtual afl::data::Value* getCrosspostToGameCandidates(const ListParameters& params);

        static void packSelections(afl::data::Segment& cmd, afl::base::Memory<const Selection> selection);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
