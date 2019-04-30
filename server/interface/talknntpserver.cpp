/**
  *  \file server/interface/talknntpserver.cpp
  */

#include <stdexcept>
#include "server/interface/talknntpserver.hpp"
#include "server/errors.hpp"
#include "interpreter/arguments.hpp"
#include "server/types.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"

using afl::data::Vector;
using afl::data::VectorValue;
using afl::data::Hash;
using afl::data::HashValue;

server::interface::TalkNNTPServer::TalkNNTPServer(TalkNNTP& impl)
    : m_implementation(impl)
{ }

server::interface::TalkNNTPServer::~TalkNNTPServer()
{ }

bool
server::interface::TalkNNTPServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    /* @q NNTPUSER user:UserName pass:Str (Talk Command)
       Authenticate user.
       This checks the user's credentials against the database.
       It does not change the user context.

       Permissions: none.

       This command was removed in c2ng 2.40.6. Use {LOGIN (User Command)} instead.

       @err 401 Failed (invalid user name)
       @err 401 Bad user Id (user does not exist)
       @err 401 Bad hash (invalid user profile)
       @err 401 Bad password (invalid password)

       @retval UID user Id

       @uses uid:$USERNAME, user:$UID:password */
    if (upcasedCommand == "NNTPLIST") {
        /* @q NNTPLIST (Talk Command)
           List forums as newsgroups.
           Lists all forums that can be read by the current user.

           Permissions: user context required.

           @retval TalkNewsgroupInfo[] list of newsgroup information
           @uses forum:newsgroups, forum:$FID:header
           @see GROUPLS */
        args.checkArgumentCount(0);

        afl::container::PtrVector<TalkNNTP::Info> info;
        m_implementation.listNewsgroups(info);

        Vector::Ref_t vec = Vector::create();
        for (size_t i = 0, n = info.size(); i < n; ++i) {
            if (TalkNNTP::Info* p = info[i]) {
                vec->pushBackNew(packInfo(*p));
            } else {
                vec->pushBackNew(0);
            }
        }
        result.reset(new VectorValue(vec));
        return true;
    } else if (upcasedCommand == "NNTPFINDNG") {
        /* @q NNTPFINDNG name:Str (Talk Command)
           Find forum by newsgroup name.

           Permissions: read-access to forum.

           @err 404 Not found (name does not correspond to a forum)
           @err 403 Permission denied (user is not allowed to read this forum)
           @retval TalkNewsgroupInfo information
           @uses forum:newsgroups, forum:$FID:header */
        args.checkArgumentCount(1);
        String_t groupName = toString(args.getNext());
        result.reset(packInfo(m_implementation.findNewsgroup(groupName)));
        return true;
    } else if (upcasedCommand == "NNTPFINDMID") {
        /* @q NNTPFINDMID mid:Str (Talk Command)
           Find posting by RFC Message-ID.
           The Message-ID is given without the angle brackets.

           Permissions: none.

           @err 404 Not found (Message-ID does not correspond to a posting)
           @retval MID posting Id */
        args.checkArgumentCount(1);
        String_t rfcMsgId = toString(args.getNext());
        result.reset(makeIntegerValue(m_implementation.findMessage(rfcMsgId)));
        return true;
    } else if (upcasedCommand == "NNTPFORUMLS") {
        /* @q NNTPFORUMLS forum:FID (Talk Command)
           List forum.

           Permissions: none.

           @retval IntList list of message sequence numbers and posting Ids.
           The sequence numbers (even indexes) are in ascending order.
           @err 404 Not found */
        args.checkArgumentCount(1);
        int32_t forumId = toInteger(args.getNext());

        afl::data::IntegerList_t messageIds;
        m_implementation.listMessages(forumId, messageIds);

        Vector::Ref_t vec = Vector::create();
        vec->pushBackElements(messageIds);
        result.reset(new VectorValue(vec));
        return true;
    } else if (upcasedCommand == "NNTPPOSTHEAD") {
        /* @q NNTPPOSTHEAD msg:MID (Talk Command)
           Get RFC message header for posting.
           The header contains regular RFC fields as well as some pseudo-headers.
           Field values are already preprocessed if necessary;
           The actual posting header can be produced by concatenating each field name, ": ", field value, "\r\n".

           Fields are returned in a useful order for generating the posting header
           and can thus be treated as a list (not a hash).
           The actual field selection is subject to change as needed
           and can possibly depend on the message.

           Permissions: user context required, read-access to message.

           @retkey :Id:MID (internal posting Id)
           @retkey :Seq:Int (sequence number)
           @retkey :Bytes:Int (estimate only)
           @retkey :Lines:Int (estimate only)
           @retkey Content-Transfer-Encoding:Str (synthetic)
           @retkey Content-Type:Str (synthetic)
           @retkey Date:Str (from message header)
           @retkey From:Str (depends on access checks)
           @retkey MIME-Version:Str (synthetic)
           @retkey Message-Id:Str (from message header or synthetic)
           @retkey Newsgroups:Str (synthetic)
           @retkey Path:Str (synthetic)
           @retkey Subject:Str (from message header)
           @retkey Supersedes:Str (from message header or synthetic)
           @retkey X-PCC-Posting-Id:MID (internal posting Id, for export)
           @retkey X-PCC-User:UserName (for export)
           @retkey Xref:Str (synthetic)

           @err 404 Message not found
           @see NNTPPOSTMHEAD
           @uses msg:$MID:header */
        args.checkArgumentCount(1);
        int32_t messageId = toInteger(args.getNext());

        result.reset(new HashValue(m_implementation.getMessageHeader(messageId)));
        return true;
    } else if (upcasedCommand == "NNTPPOSTMHEAD") {
        /* @q NNTPPOSTMHEAD msg:MID... (Talk Command)
           Get RFC message header for multiple posting.
           For details, see {NNTPPOSTHEAD}.

           If one of the requested messages cannot be accessed,
           null is returned instead of the information; no error is generated.

           Permissions: none.

           @rettype List
           @uses msg:$MID:header
           @see NNTPPOSTHEAD */
        afl::data::IntegerList_t mids;
        while (args.getNumArgs() > 0) {
            mids.push_back(toInteger(args.getNext()));
        }

        afl::data::Segment seg;
        m_implementation.getMessageHeader(mids, seg);

        result.reset(new VectorValue(Vector::create(seg)));
        return true;
    } else if (upcasedCommand == "NNTPGROUPLS") {
        /* @q NNTPGROUPLS group:GRID (Talk Command)
           List forum group as newsgroup list.
           This is a variant of {GROUPLS}.

           Permissions: none.

           Everyone may list every group, but unlistable groups will appear empty in user contexts.
           Nonexistant groups will appear empty.

           The results will be sorted by their sort key.

           @retval StrList list of newsgroup names
           @see GROUPLS
           @uses group:$GRID:forums, forum:$FID:header */
        args.checkArgumentCount(1);
        String_t groupId = toString(args.getNext());

        afl::data::StringList_t newsgroupNames;
        m_implementation.listNewsgroupsByGroup(groupId, newsgroupNames);

        Vector::Ref_t vec = Vector::create();
        vec->pushBackElements(newsgroupNames);
        result.reset(new VectorValue(vec));
        return true;
    } else {
        return false;
    }
}

server::interface::TalkNNTPServer::Value_t*
server::interface::TalkNNTPServer::packInfo(const TalkNNTP::Info& info)
{
    /* @type TalkNewsgroupInfo
       Description of a forum, as newsgroup

       @key id:FID (forum Id)
       @key newsgroup:Str (name of forum as newsgroup)
       @key firstSeq:Int (first sequence number)
       @key lastSeq:Int (last sequence number)
       @key writeAllowed:Int (1 if user can post in this newsgroup)
       @key description:Str (newsgroup tagline (description)) */
    Hash::Ref_t result = Hash::create();
    result->setNew("id",           makeIntegerValue(info.forumId));
    result->setNew("newsgroup",    makeStringValue(info.newsgroupName));
    result->setNew("firstSeq",     makeIntegerValue(info.firstSequenceNumber));
    result->setNew("lastSeq",      makeIntegerValue(info.lastSequenceNumber));
    result->setNew("writeAllowed", makeIntegerValue(info.writeAllowed));
    result->setNew("description",  makeStringValue(info.description));
    return new HashValue(result);
}
