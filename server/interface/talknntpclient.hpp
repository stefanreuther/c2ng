/**
  *  \file server/interface/talknntpclient.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKNNTPCLIENT_HPP
#define C2NG_SERVER_INTERFACE_TALKNNTPCLIENT_HPP

#include "server/interface/talknntp.hpp"
#include "afl/net/commandhandler.hpp"

namespace server { namespace interface {

    class TalkNNTPClient : public TalkNNTP {
     public:
        TalkNNTPClient(afl::net::CommandHandler& commandHandler);
        ~TalkNNTPClient();

        virtual void listNewsgroups(afl::container::PtrVector<Info>& result);
        virtual Info findNewsgroup(String_t newsgroupName);
        virtual int32_t findMessage(String_t rfcMsgId);
        virtual void listMessages(int32_t forumId, afl::data::IntegerList_t& result);
        virtual afl::data::Hash::Ref_t getMessageHeader(int32_t messageId);
        virtual void getMessageHeader(afl::base::Memory<const int32_t> messageIds, afl::data::Segment& results);
        virtual void listNewsgroupsByGroup(String_t groupId, afl::data::StringList_t& result);

        static Info unpackInfo(const afl::data::Value* p);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
