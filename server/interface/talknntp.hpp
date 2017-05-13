/**
  *  \file server/interface/talknntp.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKNNTP_HPP
#define C2NG_SERVER_INTERFACE_TALKNNTP_HPP

#include "afl/base/deletable.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/data/integerlist.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/stringlist.hpp"

namespace server { namespace interface {

    class TalkNNTP : public afl::base::Deletable {
     public:
        struct Info {
            int32_t forumId;
            String_t newsgroupName;
            int32_t firstSequenceNumber;
            int32_t lastSequenceNumber;
            bool writeAllowed;
            String_t description;
        };

        // @q NNTPUSER user:UserName pass:Str (Talk Command)
        // @retval UID user Id
        virtual String_t checkUser(String_t loginName, String_t password) = 0;

        // @q NNTPLIST (Talk Command)
        // @retval TalkNewsgroupInfo[] list of newsgroup information
        virtual void listNewsgroups(afl::container::PtrVector<Info>& result) = 0;

        // @q NNTPFINDNG name:Str (Talk Command)
        // @retval TalkNewsgroupInfo information
        virtual Info findNewsgroup(String_t newsgroupName) = 0;

        // @q NNTPFINDMID mid:Str (Talk Command)
        // @retval MID posting Id
        virtual int32_t findMessage(String_t rfcMsgId) = 0;

        // @q NNTPFORUMLS forum:FID (Talk Command)
        // @retval IntList list of message sequence numbers and posting Ids.
        virtual void listMessages(int32_t forumId, afl::data::IntegerList_t& result) = 0;

        // @q NNTPPOSTHEAD msg:MID (Talk Command)
        virtual afl::data::Hash::Ref_t getMessageHeader(int32_t messageId) = 0;

        // @q NNTPPOSTMHEAD msg:MID... (Talk Command)
        virtual void getMessageHeader(afl::base::Memory<const int32_t> messageIds, afl::data::Segment& results) = 0;

        // @q NNTPGROUPLS group:GRID (Talk Command)
        // @retval StrList list of newsgroup names
        virtual void listNewsgroupsByGroup(String_t groupId, afl::data::StringList_t& result) = 0;
    };

} }

#endif
