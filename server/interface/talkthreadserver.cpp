/**
  *  \file server/interface/talkthreadserver.cpp
  */

#include <stdexcept>
#include "server/interface/talkthreadserver.hpp"
#include "interpreter/arguments.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "server/types.hpp"
#include "afl/data/integerlist.hpp"
#include "afl/data/stringlist.hpp"
#include "server/interface/talkforumserver.hpp"
#include "server/errors.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;

server::interface::TalkThreadServer::TalkThreadServer(TalkThread& implementation)
    : m_implementation(implementation)
{ }

server::interface::TalkThreadServer::~TalkThreadServer()
{ }

bool
server::interface::TalkThreadServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    if (upcasedCommand == "THREADSTAT") {
        /* @q THREADSTAT thread:TID (Talk Command)
           Get information about a forum thread.

           Permissions: read-access to thread.

           @retval TalkThreadInfo information about thread
           @err 404 Not found
           @uses thread:$TID:header */
        args.checkArgumentCount(1);
        int32_t tid = toInteger(args.getNext());

        result.reset(packInfo(m_implementation.getInfo(tid)));
        return true;
    } else if (upcasedCommand == "THREADMSTAT") {
        /* @q THREADMSTAT thread:TID... (Talk Command)
           Get information about multiple forum threads.

           If one of the requested threads cannot be accessed,
           null is returned instead of the information; no error is generated.

           Permissions: none.

           @retval TalkThreadInfo[] information
           @uses thread:$TID:header */
        afl::data::IntegerList_t threadIds;
        while (args.getNumArgs() > 0) {
            threadIds.push_back(toInteger(args.getNext()));
        }

        afl::container::PtrVector<TalkThread::Info> infos;
        m_implementation.getInfo(threadIds, infos);

        Vector::Ref_t vec = Vector::create();
        for (size_t i = 0, n = infos.size(); i < n; ++i) {
            if (TalkThread::Info* p = infos[i]) {
                vec->pushBackNew(packInfo(*p));
            } else {
                vec->pushBackNew(0);
            }
        }
        result.reset(new VectorValue(vec));
        return true;
    } else if (upcasedCommand == "THREADLSPOST") {
        /* @q THREADLSPOST thread:TID [listParameters...] (Talk Command)
           List postings in a thread.

           The list can be accessed in different ways, see {pcc:talk:listparams|listParameters}.
           Valid sort keys for postings are:
           - author
           - edittime
           - subject
           - thread
           - time

           Permissions: none (everyone can execute this command).

           @rettype Any
           @rettype MID
           @uses thread:$TID:messages */
        args.checkArgumentCountAtLeast(1);
        int32_t threadId = toInteger(args.getNext());

        TalkThread::ListParameters p;
        TalkForumServer::parseListParameters(p, args);

        result.reset(m_implementation.getPosts(threadId, p));
        return true;
    } else if (upcasedCommand == "THREADSTICKY") {
        /* @q THREADSTICKY thread:TID flag:Int (Talk Command)
           Set thread stickyness.
           We distinguish between sticky and normal (non-sticky) threads;
           sticky threads can be rendered separately.

           Permissions: delete-access to forum (treated as admin access).

           @err 404 Not found
           @uses thread:$TID:header, forum:$FID:threads, forum:$FID:stickythreads */
        args.checkArgumentCount(2);
        int32_t topicId = toInteger(args.getNext());
        int32_t value   = toInteger(args.getNext());

        m_implementation.setSticky(topicId, value != 0);

        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "THREADPERMS") {
        /* @q THREADPERMS thread:TID [perm:Str ...] (Talk Command)
           Get thread permissions.
           For each given permission name, checks whether the user has the respective privilege.
           - %read (read postings)
           - %write (create new threads)
           - %answer (answer to a posting)
           - %delete (delete postings)
           The returned value is an integer with each bit corresponding to a privilege.
           For example, "THREADPERMS 1 answer write" returns the "answer" permission in bit 0, the "write" permission in bit 1.

           If a permission is not set separately on the thread, the containing forum is consulted.

           Permissions: none (everyone can execute this command).

           @err 404 Not found
           @retval Int permissions
           @uses forum:$FID:header, thread:$TID:header */
        args.checkArgumentCountAtLeast(1);
        int32_t topicId = toInteger(args.getNext());

        afl::data::StringList_t a;
        while (args.getNumArgs() > 0) {
            a.push_back(toString(args.getNext()));
        }
        result.reset(makeIntegerValue(m_implementation.getPermissions(topicId, a)));
        return true;
    } else if (upcasedCommand == "THREADMV") {
        /* @q THREADMV thread:TID forum:FID (Talk Command)
           Move thread to another forum.
           This moves all postings within the thread.

           Permissions: delete-access to old forum, write-access to new forum.

           @err 404 Not found
           @uses thread:$TID:header, msg:$MID:header
           @uses forum:$FID:threads, forum:$FID:stickythreads, forum:$FID:messages */
        args.checkArgumentCount(2);
        int32_t topicId = toInteger(args.getNext());
        int32_t forumId = toInteger(args.getNext());
        m_implementation.moveToForum(topicId, forumId);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "THREADRM") {
        /* @q THREADRM thread:TID (Talk Command)
           Remove a thread.
           Removes all postings it contains.

           Permissions: delete-access to forum.

           @retval Int 0=thread did not exist, 1=thread removed
           @uses thread:$TID:header, msg:$MID:header
           @uses forum:$FID:threads, forum:$FID:stickythreads, forum:$FID:messages */
        args.checkArgumentCount(1);
        int32_t topicId = toInteger(args.getNext());
        result.reset(makeIntegerValue(m_implementation.remove(topicId)));
        return true;
    } else {
        return false;
    }
}

server::interface::TalkThreadServer::Value_t*
server::interface::TalkThreadServer::packInfo(const TalkThread::Info& info)
{
    /* @type TalkThreadInfo
       Information about a forum thread (topic).
       This is an excerpt of {thread:$TID:header}.

       @key subject:Str (subject)
       @key forum:FID (forum Id)
       @key firstpost:MID (MID of first posting)
       @key lastpost:MID (MID of last posting)
       @key lasttime:Time (time of last posting)
       @key sticky:Int (1 if thread is sticky) */
    Hash::Ref_t result = Hash::create();
    result->setNew("subject",   makeStringValue(info.subject));
    result->setNew("forum",     makeIntegerValue(info.forumId));
    result->setNew("firstpost", makeIntegerValue(info.firstPostId));
    result->setNew("lastpost",  makeIntegerValue(info.lastPostId));
    result->setNew("lasttime",  makeIntegerValue(info.lastTime));
    result->setNew("sticky",    makeIntegerValue(info.isSticky));
    return new HashValue(result);
}
