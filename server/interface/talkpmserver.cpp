/**
  *  \file server/interface/talkpmserver.cpp
  *  \brief Class server::interface::TalkPMServer
  */

#include <stdexcept>
#include "server/interface/talkpmserver.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/integerlist.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "interpreter/arguments.hpp"
#include "server/errors.hpp"
#include "server/interface/talkrenderserver.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;

server::interface::TalkPMServer::TalkPMServer(TalkPM& impl)
    : m_implementation(impl)
{ }

server::interface::TalkPMServer::~TalkPMServer()
{ }

bool
server::interface::TalkPMServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    if (upcasedCommand == "PMNEW") {
        /* @q PMNEW to:TalkAddr subject:Str text:TalkText [PARENT parent:PMID] (Talk Command)
           Send a PM.

           New message attributes:
           - author: current user
           - to, subject, text, parent: as given
           - time: current time
           The new message is stored in the sender's outbox (folder 2),
           and in all receivers' inboxes (folder 1).

           Permissions: user context required

           @err 400 Receiver syntax error
           @err 400 Receiver not found
           @err 412 No receivers (receiver list expands to no users at all, e.g. when sending to an empty game)

           @retval PMID Id of new message
           @uses pm:id, pm:$PMID:header, pm:$PMID:text, user:$UID:pm:folder:$UFID:messages
           @uses MAIL (Mailout Command) */
        args.checkArgumentCountAtLeast(3);
        String_t to      = toString(args.getNext());
        String_t subject = toString(args.getNext());
        String_t text    = toString(args.getNext());
        afl::base::Optional<int32_t> parent;
        while (args.getNumArgs() > 0) {
            String_t key = afl::string::strUCase(toString(args.getNext()));
            if (key == "PARENT") {
                args.checkArgumentCountAtLeast(1);
                parent = toInteger(args.getNext());
            } else {
                throw std::runtime_error(INVALID_OPTION);
            }
        }

        result.reset(makeIntegerValue(m_implementation.create(to, subject, text, parent)));
        return true;
    } else if (upcasedCommand == "PMSTAT") {
        /* @q PMSTAT folder:UFID msg:PMID (Talk Command)
           Get information about one message.

           Permissions: user context required, accesses user's PMs

           @err 404 Not found (PM does not exist)
           @retval TalkMailInfo information
           @uses pm:$PMID:header, user:$UID:pm:folder:$UFID:messages
           @see PMMSTAT */
        args.checkArgumentCount(2);
        int32_t ufid = toInteger(args.getNext());
        int32_t pmid = toInteger(args.getNext());
        result.reset(packInfo(m_implementation.getInfo(ufid, pmid)));
        return true;
    } else if (upcasedCommand == "PMMSTAT") {
        /* @q PMMSTAT folder:UFID msg:PMID... (Talk Command)
           Get information about multiple messages.

           Permissions: user context required, accesses user's PMs

           @retval TalkMailInfo[] information for each message; null if the message cannot be accessed
           @uses pm:$PMID:header, user:$UID:pm:folder:$UFID:messages
           @see PMSTAT */
        args.checkArgumentCountAtLeast(1);
        int32_t ufid = toInteger(args.getNext());

        afl::data::IntegerList_t pmids;
        while (args.getNumArgs() > 0) {
            pmids.push_back(toInteger(args.getNext()));
        }

        afl::container::PtrVector<TalkPM::Info> infos;
        m_implementation.getInfo(ufid, pmids, infos);

        Vector::Ref_t vec = Vector::create();
        for (size_t i = 0, n = infos.size(); i < n; ++i) {
            if (const TalkPM::Info* p = infos[i]) {
                vec->pushBackNew(packInfo(*p));
            } else {
                vec->pushBackNew(0);
            }
        }
        result.reset(new VectorValue(vec));
        return true;
    } else if (upcasedCommand == "PMCP") {
        /* @q PMCP src:UFID dest:UFID msg:PMID... (Talk Command)
           Move messages between folders.
           This implements the semantic "make sure it's here AND there",
           i.e. does not fail if the target already exists.
           It is not an error if the message does not exist in the source folder.

           Permissions: user context required; accesses user's PMs.

           @retval Int number of messages successfully copied
           @uses user:$UID:pm:folder:$UFID:messages */
        args.checkArgumentCountAtLeast(2);
        int32_t srcufid = toInteger(args.getNext());
        int32_t dstufid = toInteger(args.getNext());

        afl::data::IntegerList_t pmids;
        while (args.getNumArgs() > 0) {
            pmids.push_back(toInteger(args.getNext()));
        }

        result.reset(makeIntegerValue(m_implementation.copy(srcufid, dstufid, pmids)));
        return true;
    } else if (upcasedCommand == "PMMV") {
        /* @q PMMV src:UFID dest:UFID msg:PMID... (Talk Command)
           Move messages between folders.
           This implements the semantic "make sure it's not here but there",
           i.e. does not fail if the target already exists.
           It is not an error if the message does not exist in the source folder.

           Permissions: user context required; accesses user's PMs.

           @retval Int number of messages successfully moved
           @uses user:$UID:pm:folder:$UFID:messages */
        args.checkArgumentCountAtLeast(2);
        int32_t srcufid = toInteger(args.getNext());
        int32_t dstufid = toInteger(args.getNext());

        afl::data::IntegerList_t pmids;
        while (args.getNumArgs() > 0) {
            pmids.push_back(toInteger(args.getNext()));
        }

        result.reset(makeIntegerValue(m_implementation.move(srcufid, dstufid, pmids)));
        return true;
    } else if (upcasedCommand == "PMRM") {
        /* @q PMRM folder:UFID msg:PMID... (Talk Command)
           Delete messages.
           The specified messages are removed from the given folder.
           It is not an error to attempt to delete a message that does not exist.

           Permissions: user context required; accesses user's PMs.

           @retval Int number of messages successfully deleted
           @uses user:$UID:pm:folder:$UFID:messages */
        args.checkArgumentCountAtLeast(1);
        int32_t ufid = toInteger(args.getNext());

        afl::data::IntegerList_t pmids;
        while (args.getNumArgs() > 0) {
            pmids.push_back(toInteger(args.getNext()));
        }

        result.reset(makeIntegerValue(m_implementation.remove(ufid, pmids)));
        return true;
    } else if (upcasedCommand == "PMRENDER") {
        /* @q PMRENDER folder:UFID msg:PMID [renderOptions...] (Talk Command)
           Render a PM.
           The message is rendered using the current render options, see {RENDEROPTION}.
           You can temporarily override rendering options by specifying the new settings within the command.

           Permissions: user context required; accesses user's PMs.

           @err 404 Not found (Message does not exist in the specified folder)

           @retval Str rendered message
           @uses pm:$PMID:text
           @see PMMRENDER */
        // Get parameters
        args.checkArgumentCountAtLeast(2);
        int32_t ufid = toInteger(args.getNext());
        int32_t pmid = toInteger(args.getNext());

        TalkRender::Options opts;
        TalkRenderServer::parseOptions(args, opts);

        result.reset(makeStringValue(m_implementation.render(ufid, pmid, opts)));
        return true;
    } else if (upcasedCommand == "PMMRENDER") {
        /* @q PMMRENDER folder:UFID msg:PMID... (Talk Command)
           Render PMs.
           Any number of messages can be specified,
           but all must be from the same folder because only one folder Id is given.

           The messages are rendered using the current render options, see {RENDEROPTION}.

           If one of the requested messages cannot be accessed,
           null is returned instead of a rendered message; no error is generated.

           Permissions: user context required; accesses user's PMs.

           @retval StrList rendered PMs
           @uses pm:$PMID:text
           @see PMRENDER */
        args.checkArgumentCountAtLeast(1);
        int32_t ufid = toInteger(args.getNext());

        afl::data::IntegerList_t pmids;
        while (args.getNumArgs() > 0) {
            pmids.push_back(toInteger(args.getNext()));
        }

        afl::container::PtrVector<String_t> out;
        m_implementation.render(ufid, pmids, out);

        Vector::Ref_t vec = Vector::create();
        for (size_t i = 0, n = pmids.size(); i < n; ++i) {
            if (const String_t* p = out[i]) {
                vec->pushBackString(*p);
            } else {
                vec->pushBackNew(0);
            }
        }
        result.reset(new VectorValue(vec));
        return true;
    } else if (upcasedCommand == "PMFLAG") {
        /* @q PMFLAG folder:UFID clear:TalkFlag set:TalkFlag msg:PMID... (Talk Command)
           Change flags of a message.
           The %clear and %set parameters specify the bits to clear and set on all listed messages.
           %clear and %set should be mutually disjoint.

           Permissions: user context required; accesses user's PMs.

           @retval Int number of messages processed
           @uses pm:$PMID:header, user:$UID:pm:folder:$UFID:messages */
        // Get parameters
        args.checkArgumentCountAtLeast(3);
        int32_t ufid = toInteger(args.getNext());
        int32_t clear = toInteger(args.getNext());
        int32_t set = toInteger(args.getNext());

        afl::data::IntegerList_t pmids;
        while (args.getNumArgs() > 0) {
            pmids.push_back(toInteger(args.getNext()));
        }

        result.reset(makeIntegerValue(m_implementation.changeFlags(ufid, clear, set, pmids)));
        return true;
    } else {
        return false;
    }
}

server::interface::TalkPMServer::Value_t*
server::interface::TalkPMServer::packInfo(const TalkPM::Info& info)
{
    // ex UserPM::describe (part)
    /* @type TalkMailInfo
       Information about one message.
       This is a variant of {pm:$PMID:header}.

       @key author:UID               (user who sent it)
       @key to:TalkAddr              (users who receive it)
       @key time:Time                (time when sent)
       @key subject:Str              (subject)
       @key flags:TalkFlag           (user's flags)
       @key parent:PMID              (parent message, 0 if none)
       @key parentSubject:Str        (parent message subject, missing if none)
       @key parentFolder:UFID        (parent folder Id if known, missing if none)
       @key parentFolderName:Str     (parent folder name if known, missing if none)
       @key suggestedFolder:UFID     (suggested folder to move into, missing if none known)
       @key suggestedFolderName:UFID (name of suggested folder, missing if none known) */
    Hash::Ref_t result = Hash::create();
    result->setNew("author",  makeStringValue(info.author));
    result->setNew("to",      makeStringValue(info.receivers));
    result->setNew("time",    makeIntegerValue(info.time));
    result->setNew("subject", makeStringValue(info.subject));
    result->setNew("flags",   makeIntegerValue(info.flags));
    result->setNew("parent",  makeIntegerValue(info.parent.orElse(0)));
    addOptionalStringKey(*result,  "parentSubject",       info.parentSubject);
    addOptionalIntegerKey(*result, "parentFolder",        info.parentFolder);
    addOptionalStringKey(*result,  "parentFolderName",    info.parentFolderName);
    addOptionalIntegerKey(*result, "suggestedFolder",     info.suggestedFolder);
    addOptionalStringKey(*result,  "suggestedFolderName", info.suggestedFolderName);

    return new HashValue(result);
}
