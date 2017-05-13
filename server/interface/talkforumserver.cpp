/**
  *  \file server/interface/talkforumserver.cpp
  */

#include <stdexcept>
#include "server/interface/talkforumserver.hpp"
#include "server/types.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/integerlist.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "server/errors.hpp"

using afl::data::Vector;
using afl::data::VectorValue;
using afl::data::Hash;
using afl::data::HashValue;
using afl::data::StringList_t;
using afl::data::IntegerList_t;

server::interface::TalkForumServer::TalkForumServer(TalkForum& impl)
    : m_implementation(impl)
{ }

server::interface::TalkForumServer::~TalkForumServer()
{ }

bool
server::interface::TalkForumServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    if (upcasedCommand == "FORUMADD") {
        /* @q FORUMADD [key:Str value:Str ...] (Talk Command)
           Create forum.
           The command is followed by %key/%value pairs that configure the forum, see {FORUMSET}.

           Permissions: admin.

           @retval FID new forum Id
           @uses forum:$FID:header, forum:all */
        StringList_t a;
        while (args.getNumArgs() > 0) {
            a.push_back(toString(args.getNext()));
        }
        if ((a.size() & 1) != 0) {
            throw std::runtime_error(INVALID_NUMBER_OF_ARGUMENTS);
        }
        result.reset(makeIntegerValue(m_implementation.add(a)));
        return true;
    } else if (upcasedCommand == "FORUMSET") {
        /* @q FORUMSET forum:FID [key:Str value:Str ...] (Talk Command)
           Configure forum.
           Valid keys are the same as keys in {forum:$FID:header}; sensible values listed here:
           - %name ({@type Str|string}, forum name)
           - %parent ({@type GRID}, containing group)
           - %description ({@type TalkText}, description/subtitle)
           - %newsgroup ({@type Str|string}, NNTP newsgroup name)
           - %readperm, %writeperm, %answerperm, %deleteperm ({@type TalkPerm}, configure permissions)
           - %key ({@type Str|string}, sort key)

           Permissions: none.

           @uses forum:$FID:header, group:$GRID:forums, forum:newsgroups
           @err 404 Not found
           @argtype Str
           @argtype GRID
           @argtype TalkText
           @argtype TalkPerm */
        args.checkArgumentCountAtLeast(1);
        int32_t fid = toInteger(args.getNext());
        afl::data::StringList_t a;
        while (args.getNumArgs() > 0) {
            a.push_back(toString(args.getNext()));
        }
        if ((a.size() & 1) != 0) {
            throw std::runtime_error(INVALID_NUMBER_OF_ARGUMENTS);
        }
        m_implementation.configure(fid, a);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "FORUMGET") {
        /* @q FORUMGET forum:FID key:Str (Talk Command)
           Get forum property.
           Valid keys are the same as keys in {forum:$FID:header}.

           Permissions: none.

           The returned value is taken directly from the forum header.

           @uses forum:$FID:header
           @err 404 Not found
           @rettype Str
           @rettype GRID
           @rettype TalkText
           @rettype TalkPerm
           @rettype Time
           @rettype Int */
        args.checkArgumentCount(2);
        int32_t fid = toInteger(args.getNext());
        String_t key = toString(args.getNext());
        result.reset(m_implementation.getValue(fid, key));
        return true;
    } else if (upcasedCommand == "FORUMSTAT") {
        /* @q FORUMSTAT forum:FID (Talk Command)
           Get forum information.

           Permissions: none.

           Note that this command uses the rendering configuration set with {RENDEROPTION}
           to render the forum description.

           @retval TalkForumInfo information about forum
           @see FORUMMSTAT
           @uses forum:$FID:header */
        args.checkArgumentCount(1);
        int32_t fid = toInteger(args.getNext());
        result.reset(packInfo(m_implementation.getInfo(fid)));
        return true;
    } else if (upcasedCommand == "FORUMMSTAT") {
        /* @q FORUMMSTAT forum:FID... (Talk Command)
           Get multiple forums information.

           Permissions: none.

           Note that this command uses the rendering configuration set with {RENDEROPTION}
           to render the forum descriptions.

           @retval TalkForumInfo[] information about forum
           @see FORUMSTAT
           @uses forum:$FID:header */
        IntegerList_t fids;
        while (args.getNumArgs() > 0) {
            fids.push_back(toInteger(args.getNext()));
        }

        afl::container::PtrVector<TalkForum::Info> infos;
        m_implementation.getInfo(fids, infos);

        Vector::Ref_t vec = Vector::create();
        for (size_t i = 0, n = infos.size(); i < n; ++i) {
            if (infos[i] != 0) {
                vec->pushBackNew(packInfo(*infos[i]));
            } else {
                vec->pushBackNew(0);
            }
        }
        result.reset(new VectorValue(vec));
        return true;
    } else if (upcasedCommand == "FORUMPERMS") {
        /* @q FORUMPERMS forum:FID [perm:Str ...] (Talk Command)
           Get forum permissions.
           For each given permission name, checks whether the user has the respective privilege.
           - %read (read postings)
           - %write (create new threads)
           - %answer (answer to a posting)
           - %delete (delete postings)
           The returned value is an integer with each bit corresponding to a privilege.
           For example, "FORUMPERMS 1 answer write" returns the "answer" permission in bit 0, the "write" permission in bit 1.

           Permissions: none (everyone can execute this command).

           @err 404 Not found
           @retval Int permissions
           @uses forum:$FID:header */
        args.checkArgumentCountAtLeast(1);
        int32_t fid = toInteger(args.getNext());

        StringList_t a;
        while (args.getNumArgs() > 0) {
            a.push_back(toString(args.getNext()));
        }
        result.reset(makeIntegerValue(m_implementation.getPermissions(fid, a)));
        return true;
    } else if (upcasedCommand == "FORUMSIZE") {
        /* @q FORUMSIZE forum:FID (Talk Command)
           Get forum size.

           Permissions: none.

           @retkey threads:Int (Number of threads)
           @retkey stickythreads:Int (Number of sticky threads)
           @retkey messages:Int (Number of messages)

           @err 404 Not found
           @uses forum:$FID:messages, forum:$FID:threads, forum:$FID:stickythreads */
        args.checkArgumentCount(1);
        int32_t fid = toInteger(args.getNext());

        TalkForum::Size size = m_implementation.getSize(fid);

        Hash::Ref_t h = Hash::create();
        h->setNew("threads",       makeIntegerValue(size.numThreads));
        h->setNew("stickythreads", makeIntegerValue(size.numStickyThreads));
        h->setNew("messages",      makeIntegerValue(size.numMessages));
        result.reset(new HashValue(h));
        return true;
    } else if (upcasedCommand == "FORUMLSTHREAD") {
        /* @q FORUMLSTHREAD forum:FID [listParameters...] (Talk Command)
           Query list of threads in a forum.

           The list can be accessed in different ways, see {pcc:talk:listparams|listParameters}.
           Valid sort keys for threads are:
           - firstpost
           - forum
           - lastpost
           - lasttime
           - subject

           Permissions: none (everyone can execute this command).
           @c FIXME: is this a good idea?

           If Ids are returned, these are {@type TID|thread Ids}.

           @err 404 Not found
           @rettype Any
           @rettype TID
           @uses forum:$FID:threads */
        args.checkArgumentCountAtLeast(1);
        int32_t fid = toInteger(args.getNext());

        TalkForum::ListParameters p;
        parseListParameters(p, args);

        result.reset(m_implementation.getThreads(fid, p));
        return true;
    } else if (upcasedCommand == "FORUMLSSTICKY") {
        /* @q FORUMLSSTICKY forum:FID [listParameters...] (Talk Command)
           Query list of sticky threads in a forum.

           The list can be accessed in different ways, see {pcc:talk:listparams|listParameters}.
           Valid sort keys for threads are:
           - firstpost
           - forum
           - lastpost
           - lasttime
           - subject

           Permissions: none (everyone can execute this command).
           @c FIXME: is this a good idea?

           If Ids are returned, these are {@type TID|thread Ids}.

           @err 404 Not found
           @rettype Any
           @rettype TID
           @uses forum:$FID:stickythreads */
        args.checkArgumentCountAtLeast(1);
        int32_t fid = toInteger(args.getNext());

        TalkForum::ListParameters p;
        parseListParameters(p, args);

        result.reset(m_implementation.getStickyThreads(fid, p));
        return true;
    } else if (upcasedCommand == "FORUMLSPOST") {
        /* @q FORUMLSPOST forum:FID [listParameters...] (Talk Command)
           Query list of postings in a forum.

           The list can be accessed in different ways, see {pcc:talk:listparams|listParameters}.
           Valid sort keys for postings are:
           - author
           - edittime
           - subject
           - thread
           - time

           Permissions: none (everyone can execute this command).
           @c FIXME: is this a good idea?

           If Ids are returned, these are {@type MID|message Ids}.

           @err 404 Not found
           @rettype Any
           @rettype MID
           @uses forum:$FID:messages */
        args.checkArgumentCountAtLeast(1);
        int32_t fid = toInteger(args.getNext());

        TalkForum::ListParameters p;
        parseListParameters(p, args);

        result.reset(m_implementation.getPosts(fid, p));
        return true;
    } else {
        return false;
    }
}

server::interface::TalkForumServer::Value_t*
server::interface::TalkForumServer::packInfo(const TalkForum::Info& info)
{
    Hash::Ref_t result = Hash::create();
    result->setNew("name", makeStringValue(info.name));
    result->setNew("parent", makeStringValue(info.parentGroup));
    result->setNew("description", makeStringValue(info.description));
    result->setNew("newsgroup", makeStringValue(info.newsgroupName));
    return new HashValue(result);
}

void
server::interface::TalkForumServer::parseListParameters(TalkForum::ListParameters& p, interpreter::Arguments& args)
{
    // ex ListParams::parse
    while (args.getNumArgs() > 0) {
        String_t key = afl::string::strUCase(toString(args.getNext()));
        if (key == "LIMIT") {
            args.checkArgumentCountAtLeast(2);
            p.start = toInteger(args.getNext());
            p.count = toInteger(args.getNext());
            p.mode = TalkForum::ListParameters::WantRange;
        } else if (key == "SIZE") {
            p.mode = TalkForum::ListParameters::WantSize;
        } else if (key == "CONTAINS") {
            args.checkArgumentCountAtLeast(1);
            p.item = toInteger(args.getNext());
            p.mode = TalkForum::ListParameters::WantMemberCheck;
        } else if (key == "SORT") {
            args.checkArgumentCountAtLeast(1);
            p.sortKey = afl::string::strUCase(toString(args.getNext()));
        } else {
            throw std::runtime_error(INVALID_OPTION);
        }
    }
}
