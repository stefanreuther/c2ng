/**
  *  \file server/talk/talknntp.hpp
  *  \brief Class server::talk::TalkNNTP
  */
#ifndef C2NG_SERVER_TALK_TALKNNTP_HPP
#define C2NG_SERVER_TALK_TALKNNTP_HPP

#include "server/interface/talknntp.hpp"

namespace server { namespace talk {

    class Session;
    class Root;

    /** Implementation of NNTP commands. */
    class TalkNNTP : public server::interface::TalkNNTP {
     public:
        /** Constructor.
            @param session Session
            @param root Service root */
        TalkNNTP(Session& session, Root& root);

        virtual void listNewsgroups(afl::container::PtrVector<Info>& result);
        virtual Info findNewsgroup(String_t newsgroupName);
        virtual int32_t findMessage(String_t rfcMsgId);
        virtual void listMessages(int32_t forumId, afl::data::IntegerList_t& result);
        virtual afl::data::Hash::Ref_t getMessageHeader(int32_t messageId);
        virtual void getMessageHeader(afl::base::Memory<const int32_t> messageIds, afl::data::Segment& results);
        virtual void listNewsgroupsByGroup(String_t groupId, afl::data::StringList_t& result);

     private:
        Session& m_session;
        Root& m_root;
    };

} }

#endif
