/**
  *  \file server/interface/talknntp.hpp
  *  \brief Interface server::interface::TalkNNTP
  */
#ifndef C2NG_SERVER_INTERFACE_TALKNNTP_HPP
#define C2NG_SERVER_INTERFACE_TALKNNTP_HPP

#include "afl/base/deletable.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/integerlist.hpp"
#include "afl/data/stringlist.hpp"

namespace server { namespace interface {

    /** Talk NNTP Interface.
        This interface contains assorted commands intended specifically for implementing a NNTP front-end (c2nntp-server, server::nntp).
        In addition to commands from this interface, the NNTP front-end will use other Talk interfaces.

        Originally, TalkNNTP included user authentication; this has now been moved to the UserManagement interface.

        Note that "Message Id" generally refers to the Talk message Id (an integer),
        and we use the term "RFC Message-ID" to specifically refer to the NNTP "Message-ID" field ("localpart@host"). */
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

        /** List forums as newsgroups (NNTPLIST).
            \param [out] result List of newsgroups */
        virtual void listNewsgroups(afl::container::PtrVector<Info>& result) = 0;

        /** Find forum by newsgroup name (NNTPFINDNG).
            \param newsgroupName Newsgroup name
            \return Newsgroup information, if found */
        virtual Info findNewsgroup(String_t newsgroupName) = 0;

        /** Find posting by RFC Message-ID (NNTPFINDMID).
            \param rfcMsgId RFC Message-ID
            \return Posting ID */
        virtual int32_t findMessage(String_t rfcMsgId) = 0;

        /** List forum (NNTPFORUMLS).
            \param [in] forumId Forum Id
            \param [out] result List of NNTP sequence numbers and posting Ids. */
        virtual void listMessages(int32_t forumId, afl::data::IntegerList_t& result) = 0;

        /** Get RFC message header for posting (NNTPPOSTHEAD).
            \param messageId Message ID
            \return Header as hash. A hash is returned (instead of an STL map) because it preserves the order of fields. */
        virtual afl::data::Hash::Ref_t getMessageHeader(int32_t messageId) = 0;

        /** Get RFC message header for multiple posting (NNTPPOSTMHEAD).
            \param [in] messageIds Message Ids
            \param [out] results Segment containing result hashes */
        virtual void getMessageHeader(afl::base::Memory<const int32_t> messageIds, afl::data::Segment& results) = 0;

        /** List forum group as newsgroup list (NNTPGROUPLS).
            \param [in] groupId Group Id
            \param [out] result List of newsgroup names */
        virtual void listNewsgroupsByGroup(String_t groupId, afl::data::StringList_t& result) = 0;
    };

} }

#endif
