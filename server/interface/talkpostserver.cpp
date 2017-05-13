/**
  *  \file server/interface/talkpostserver.cpp
  */

#include <stdexcept>
#include "server/interface/talkpostserver.hpp"
#include "interpreter/arguments.hpp"
#include "server/types.hpp"
#include "server/interface/talkpost.hpp"
#include "server/interface/talkrenderserver.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "server/errors.hpp"

server::interface::TalkPostServer::TalkPostServer(TalkPost& impl)
    : m_implementation(impl)
{ }

server::interface::TalkPostServer::~TalkPostServer()
{ }

bool
server::interface::TalkPostServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    if (upcasedCommand == "POSTNEW") {
        /* @q POSTNEW forum:FID subj:Str text:TalkText [USER user:UID] [READPERM rp:Str] [ANSWERPERM ap:Str] (Talk Command)
           Create a new thread and add a posting.

           New message attributes:
           - thread: newly allocated
           - time: current wall-clock time
           - author: given in {USER} command or %user parameter.
           - parent, edittime, msgid, rfcheader: blank
           - subject: as given
           - seq: newly allocated

           Thread attributes:
           - subject, forum, readperm, answerperm: as given
           - firstpost: MID of new posting

           Permissions: write-permission for forum.

           @err 400 Need USER (in admin context, USER must be specified)
           @err 403 USER not allowed (in user context, USER must be identical to current user or omitted)
           @err 404 No such forum

           @retval MID new message Id

           @uses thread:id, thread:$TID:header
           @uses msg:id, msg:$MID:header, msg:$MID:text
           @uses MAIL (Mailout Command) */
        args.checkArgumentCountAtLeast(3);
        int32_t fid      = toInteger(args.getNext());
        String_t subject = toString(args.getNext());
        String_t text    = toString(args.getNext());
        TalkPost::CreateOptions opts;
        while (args.getNumArgs() > 0) {
            String_t key = afl::string::strUCase(toString(args.getNext()));
            if (key == "USER") {
                args.checkArgumentCountAtLeast(1);
                opts.userId = toString(args.getNext());
            } else if (key == "READPERM") {
                args.checkArgumentCountAtLeast(1);
                opts.readPermissions = toString(args.getNext());
            } else if (key == "ANSWERPERM") {
                args.checkArgumentCountAtLeast(1);
                opts.answerPermissions = toString(args.getNext());
            } else {
                throw std::runtime_error(INVALID_OPTION);
            }
        }

        result.reset(makeIntegerValue(m_implementation.create(fid, subject, text, opts)));
        return true;
    } else if (upcasedCommand == "POSTREPLY") {
        /* @q POSTREPLY parent:MID subj:Str text:TalkText [USER user:UID] (Talk Command)
           Create reply to a message.

           New message attributes:
           - thread: taken from %parent
           - parent: as given
           - time: current wall-clock time
           - author: given in {USER} command or %user parameter.
           - edittime, msgid, rfcheader: blank
           - subject: as given
           - seq: newly allocated

           Permissions: answer-permission for thread.

           @err 400 Need USER (in admin context, USER must be specified)
           @err 403 USER not allowed (in user context, USER must be identical to current user or omitted)
           @err 404 Parent message does not exist

           @retval MID new message Id

           @uses thread:$TID:header
           @uses msg:id, msg:$MID:header, msg:$MID:text
           @uses MAIL (Mailout Command) */
        args.checkArgumentCountAtLeast(3);
        int32_t parentId = toInteger(args.getNext());
        String_t subject = toString(args.getNext());
        String_t text    = toString(args.getNext());
        server::interface::TalkPost::ReplyOptions opts;
        while (args.getNumArgs() > 0) {
            String_t key = afl::string::strUCase(toString(args.getNext()));
            if (key == "USER") {
                args.checkArgumentCountAtLeast(1);
                opts.userId = toString(args.getNext());
            } else {
                throw std::runtime_error(INVALID_OPTION);
            }
        }

        result.reset(makeIntegerValue(m_implementation.reply(parentId, subject, text, opts)));
        return true;
    } else if (upcasedCommand == "POSTEDIT") {
        /* @q POSTEDIT msg:MID subj:Str text:TalkText (Talk Command)
           Change an existing posting.

           Updated message attributes:
           - thread, parent, time, author: unchanged
           - subject, text: updated
           - edittime: current time
           - msgid, rfcheader: deleted (because this is not a RFC message)
           - seq: updated
           - prevseq, prevmsgid: copied from old seq, msgid (to generate Supersedes header)

           Permissions: admin or message author.

           @err 404 Message not found
           @err 403 Not author

           @uses msg:$MID:text, msg:$MID:header

           @change PCC2 returns "OK, unchanged" if there is no change */
        args.checkArgumentCount(3);
        int32_t messageId = toInteger(args.getNext());
        String_t subject  = toString(args.getNext());
        String_t text     = toString(args.getNext());
        m_implementation.edit(messageId, subject, text);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "POSTRENDER") {
        /* @q POSTRENDER msg:MID [renderOptions...] (Talk Command)
           Render a message.

           The message is rendered using the current render options, see {RENDEROPTION}.
           You can temporarily override rendering options by specifying the new settings within the command.

           Permissions: read-access to posting.

           @err 404 Message not found

           @retval Str rendered posting

           @uses msg:$MID:text */
        args.checkArgumentCountAtLeast(1);
        int32_t messageId = toInteger(args.getNext());

        TalkRender::Options opts;
        TalkRenderServer::parseOptions(args, opts);

        result.reset(makeStringValue(m_implementation.render(messageId, opts)));
        return true;
    } else if (upcasedCommand == "POSTMRENDER") {
        /* @q POSTMRENDER msg:MID... (Talk Command)
           Render messages.

           The messages are rendered using the current render options, see {RENDEROPTION}.

           If one of the requested messages cannot be accessed,
           null is returned instead of a rendered message; no error is generated.

           Permissions: none.

           @retval StrList rendered postings

           @uses msg:$MID:text */
        afl::data::IntegerList_t list;
        while (args.getNumArgs() > 0) {
            list.push_back(toInteger(args.getNext()));
        }

        afl::data::StringList_t strings;
        m_implementation.render(list, strings);

        result.reset(new afl::data::VectorValue(afl::data::Vector::create(afl::data::Segment().pushBackElements(strings))));
        return true;
    } else if (upcasedCommand == "POSTSTAT") {
        /* @q POSTSTAT msg:MID (Talk Command)
           Get information about one posting.

           Permissions: read-access to posting.

           @err 404 Message not found

           @retval TalkPostInfo information about posting
           @uses msg:$MID:header */
        // Fetch args
        args.checkArgumentCount(1);
        int32_t messageId = toInteger(args.getNext());

        result.reset(packInfo(m_implementation.getInfo(messageId)));
        return true;
    } else if (upcasedCommand == "POSTMSTAT") {
        /* @q POSTMSTAT msg:MID... (Talk Command)
           Get information about multiple postings.

           If one of the requested messages cannot be accessed,
           null is returned instead of the information; no error is generated.

           Permissions: none.

           @retval TalkPostInfo[] information about postings in an array
           @uses msg:$MID:header */
        afl::data::IntegerList_t list;
        while (args.getNumArgs() > 0) {
            list.push_back(toInteger(args.getNext()));
        }

        afl::container::PtrVector<TalkPost::Info> infos;
        m_implementation.getInfo(list, infos);

        afl::data::Vector::Ref_t v = afl::data::Vector::create();
        for (size_t i = 0, n = infos.size(); i < n; ++i) {
            if (infos[i] == 0) {
                v->pushBackNew(0);
            } else {
                v->pushBackNew(packInfo(*infos[i]));
            }
        }
        result.reset(new afl::data::VectorValue(v));
        return true;
    } else if (upcasedCommand == "POSTGET") {
        /* @q POSTGET msg:MID key:Str (Talk Command)
           Get information field of one posting.
           This fetches one field of the message header,
           see {@type TalkPostInfo} and {msg:$MID:header}.
           In addition to the fields defined there, this command also supports:
           - %rfcmsgid: Message-Id for posting on RFC (NNTP) side.

           Permissions: read-access to posting.

           @err 404 Message not found

           @retval Any result (string, Id, Time, etc.)
           @uses msg:$MID:header */
        args.checkArgumentCount(2);
        int32_t messageId = toInteger(args.getNext());
        String_t field    = toString(args.getNext());
        result.reset(makeStringValue(m_implementation.getHeaderField(messageId, field)));
        return true;
    } else if (upcasedCommand == "POSTRM") {
        /* @q POSTRM msg:MID (Talk Command)
           Remove a posting.
           If this is the last posting in a thread, removes the thread.

           Permissions: admin or author of posting or delete-access to thread.

           @err 403 Not author (insufficient permissions)
           @retval Int 1=removed, 0=not removed, posting did not exist
           @uses msg:$MID:header, thread:$TID:messages, forum:$FID:messages */
        args.checkArgumentCount(1);
        int32_t messageId = toInteger(args.getNext());
        result.reset(makeIntegerValue(m_implementation.remove(messageId)));
        return true;
    } else if (upcasedCommand == "POSTLSNEW") {
        /* @q POSTLSNEW n:Int (Talk Command)
           List newest postings.
           Produces a list of the %n most recent postings the user can see.

           Permissions: none (only accessible postings are returned).

           @retval IntList list of {@type MID}s, starting with the most recent one.
           @rettype MID
           @uses msg:id */
        args.checkArgumentCount(1);
        int count = toInteger(args.getNext());

        afl::data::IntegerList_t ids;
        m_implementation.getNewest(count, ids);

        result.reset(new afl::data::VectorValue(afl::data::Vector::create(afl::data::Segment().pushBackElements(ids))));
        return true;
    } else {
        return false;
    }
}

server::interface::TalkPostServer::Value_t*
server::interface::TalkPostServer::packInfo(const TalkPost::Info& info)
{
    /* @type TalkPostInfo
       Information about one message.
       This is an enriched version of {msg:$MID:header}.

       @key thread:TID                        Thread Id of message
       @key parent:MID                        Message Id of parent message
       @key time:Time                         Timestamp
       @key edittime:Time                     Timestamp of last edit
       @key author:UID                        User Id of author of message. In the API, this is a {@type UserName}.
       @key subject:Str                       Subject of message if different from thread subject
       @key msgid:Str                         RFC message Id (always valid) */
    afl::data::Hash::Ref_t result = afl::data::Hash::create();
    result->setNew("thread",   makeIntegerValue(info.threadId));
    result->setNew("parent",   makeIntegerValue(info.parentPostId));
    result->setNew("time",     makeIntegerValue(info.postTime));
    result->setNew("edittime", makeIntegerValue(info.editTime));
    result->setNew("author",   makeStringValue(info.author));
    result->setNew("subject",  makeStringValue(info.subject));
    result->setNew("msgid",    makeStringValue(info.rfcMessageId));
    return new afl::data::HashValue(result);
}
