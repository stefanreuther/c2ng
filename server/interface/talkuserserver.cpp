/**
  *  \file server/interface/talkuserserver.cpp
  */

#include <vector>
#include <stdexcept>
#include "server/interface/talkuserserver.hpp"
#include "afl/data/integerlist.hpp"
#include "server/types.hpp"
#include "interpreter/arguments.hpp"
#include "server/interface/talkforumserver.hpp"
#include "server/errors.hpp"

server::interface::TalkUserServer::TalkUserServer(TalkUser& implementation)
    : m_implementation(implementation)
{ }

server::interface::TalkUserServer::~TalkUserServer()
{ }

bool
server::interface::TalkUserServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    if (upcasedCommand == "USERNEWSRC") {
        /* @q USERNEWSRC action:Str [range...] (Talk Command)
           Access user's newsrc file.
           The newsrc file contains read/unread bits for each posting, indexed by posting Id ({@type MID})
           (bit set: post was read; bit clear: post was not read).

           The %action parameter is a keyword that specifies the desired operation:
           - "GET" (get all affected items. Returns a {@type Str|string} of 0/1. In RESP, this doubles as a boolean result.)
           - "SET" (set all affected items (=mark read)).
           - "CLEAR" (clear all affected items (=mark unread)).
           - "ANY" (return 1 if any of the affected items is 1 (=return 1 if any item was read)).
           - "ALL" (return 1 if all of the affected items are 1 (=return 0 if any item was unread)).
           - "FIRSTSET" (return {@type MID} of first set (=read) item, 0 if none).
           - "FIRSTCLEAR" (return {@type MID} of first clear (=unread) item, 0 if none).

           The %range is one or more keyword parameters that specify the items (postings) to check:
           - POST n:{@type MID}... (followed by any number of posting Ids until the end of the command; checks these postings)
           - RANGE a:{@type MID} b:{@type MID} (checks the postings from a (inclusive) to b (inclusive))
           - THREAD n:{@type TID} (checks all postings in the specified thread)
           - FORUM n:{@type FID} (checks all postings in the specified forum)

           Note that if an error happens, the operation may complete partially.

           Permissions: user context required, accesses user's newsrc

           @err 413 Range error (MID parameter in POST/RANGE is not a valid posting Id)
           @argtype MID
           @argtype TID
           @argtype FID
           @rettype Any
           @rettype MID
           @rettype Str
           @rettype Int
           @uses user:$UID:forum:newsrc:data, user:$UID:forum:newsrc:index */
        /* @change PCC2 would accept actions and ranges in any order, and apply them on the go.
           This would yield combinations such as FIRSTSET + ALL becoming FIRSTCLEAR,
           or FIRSTSET <range> FIRSTCLEAR <range> looking for a read post in the first range, and an unread in the second.
           It would also stop parsing the command upon encountering a match in FIRSTSET/FIRSTCLEAR.
           This implementation no longer supports this: we completely parse the command,
           and then decide on one action/result used for all of them. */
        std::vector<TalkUser::Selection> selections;
        afl::data::IntegerList_t posts;
        TalkUser::Modification modif = TalkUser::NoModification;
        TalkUser::Result desiredResult = TalkUser::NoResult;

        while (args.getNumArgs() > 0) {
            String_t keyword = afl::string::strUCase(toString(args.getNext()));
            if (keyword == "GET") {
                desiredResult = TalkUser::GetAll;
            } else if (keyword == "SET") {
                modif = TalkUser::MarkRead;
            } else if (keyword == "CLEAR") {
                modif = TalkUser::MarkUnread;
            } else if (keyword == "ANY") {
                desiredResult = TalkUser::CheckIfAnyRead;
            } else if (keyword == "ALL") {
                desiredResult = TalkUser::CheckIfAllRead;
            } else if (keyword == "FIRSTSET") {
                desiredResult = TalkUser::GetFirstRead;
            } else if (keyword == "FIRSTCLEAR") {
                desiredResult = TalkUser::GetFirstUnread;
            } else if (keyword == "POST") {
                while (args.getNumArgs() > 0) {
                    posts.push_back(toInteger(args.getNext()));
                }
            } else if (keyword == "RANGE") {
                TalkUser::Selection sel;
                args.checkArgumentCountAtLeast(2);
                sel.scope  = TalkUser::RangeScope;
                sel.id     = toInteger(args.getNext());
                sel.lastId = toInteger(args.getNext());
                selections.push_back(sel);
            } else if (keyword == "FORUM") {
                TalkUser::Selection sel;
                args.checkArgumentCountAtLeast(1);
                sel.scope  = TalkUser::ForumScope;
                sel.id     = toInteger(args.getNext());
                sel.lastId = 0;
                selections.push_back(sel);
            } else if (keyword == "THREAD") {
                TalkUser::Selection sel;
                args.checkArgumentCountAtLeast(1);
                sel.scope  = TalkUser::ThreadScope;
                sel.id     = toInteger(args.getNext());
                sel.lastId = 0;
                selections.push_back(sel);
            } else {
                throw std::runtime_error(INVALID_OPTION);
            }
        }

        result.reset(m_implementation.accessNewsrc(modif, desiredResult, selections, posts));
        return true;
    } else if (upcasedCommand == "USERWATCH") {
        /* @q USERWATCH [THREAD n:TID] [FORUM n:FID]... (Talk Command)
           Watch thread or forum.
           Any number of threads or forums can be watched in a single command
           by specifying multiple THREAD or FORUM parameters.

           Permissions: user context required, accesses user's profile

           @err 404 Not found
           @uses user:$UID:forum:watchedThreads, user:$UID:forum:watchedForums
           @uses user:$UID:forum:notifiedThreads, user:$UID:forum:notifiedForums
           @uses forum:$FID:watchers, thread:$TID:watchers
           @see USERLSWATCHEDTHREADS, USERLSWATCHEDFORUMS */
        std::vector<TalkUser::Selection> selections;
        parseSelection(args, selections);
        m_implementation.watch(selections);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "USERUNWATCH") {
        /* @q USERUNWATCH [THREAD n:TID] [FORUM n:FID]... (Talk Command)
           Stop watching thread or forum.
           Any number of threads or forums can be unwatched in a single command
           by specifying multiple THREAD or FORUM parameters.

           Permissions: user context required, accesses user's profile

           @err 404 Not found
           @uses user:$UID:forum:watchedThreads, user:$UID:forum:watchedForums
           @uses user:$UID:forum:notifiedThreads, user:$UID:forum:notifiedForums
           @uses forum:$FID:watchers, thread:$TID:watchers
           @see USERLSWATCHEDTHREADS, USERLSWATCHEDFORUMS */
        std::vector<TalkUser::Selection> selections;
        parseSelection(args, selections);
        m_implementation.unwatch(selections);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "USERMARKSEEN") {
        /* @q USERMARKSEEN [THREAD n:TID] [FORUM n:FID]... (Talk Command)
           Reset notification status for a thread or forum.
           If a user has set their notifications to "one per/thread forum"
           ({user:$UID:profile}->talkwatchindividual),
           they only get a notification for the first change.
           This command resets the status so a following change will again send mail.

           Any number of threads or forums can be marked seen in a single command
           by specifying multiple THREAD or FORUM parameters.

           Permissions: user context required, accesses user's profile

           @err 404 Not found
           @uses forum:$FID:watchers, thread:$TID:watchers */
        std::vector<TalkUser::Selection> selections;
        parseSelection(args, selections);
        m_implementation.markSeen(selections);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "USERLSWATCHEDTHREADS") {
        /* @q USERLSWATCHEDTHREADS [listParameters...] (Talk Command)
           List threads watched by user.

           The list can be accessed in different ways, see {pcc:talk:listparams|listParameters}.
           Valid sort keys for threads are:
           - firstpost
           - forum
           - lastpost
           - lasttime
           - subject

           Permissions: user context required, accesses user's profile

           @rettype Any
           @rettype TID
           @uses user:$UID:forum:watchedThreads
           @see USERWATCH, USERUNWATCH */
        TalkUser::ListParameters params;
        TalkForumServer::parseListParameters(params, args);
        result.reset(m_implementation.getWatchedThreads(params));
        return true;
    } else if (upcasedCommand == "USERLSWATCHEDFORUMS") {
        /* @q USERLSWATCHEDFORUMS [listParameters...] (Talk Command)
           List forums watched by user.

           The list can be accessed in different ways, see {pcc:talk:listparams|listParameters}.
           Valid sort keys for forums are:
           - key
           - lastpost
           - lasttime
           - name

           Permissions: user context required, accesses user's profile

           @rettype Any
           @rettype FID
           @uses user:$UID:forum:watchedForums
           @see USERWATCH, USERUNWATCH */
        TalkUser::ListParameters params;
        TalkForumServer::parseListParameters(params, args);
        result.reset(m_implementation.getWatchedForums(params));
        return true;
    } else if (upcasedCommand == "USERLSPOSTED") {
        /* @q USERLSPOSTED user:UID [listParameters...] (Talk Command)
           List user's postings.

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
           @uses user:$UID:forum:posted */
        args.checkArgumentCountAtLeast(1);
        String_t user = toString(args.getNext());
        TalkUser::ListParameters params;
        TalkForumServer::parseListParameters(params, args);
        result.reset(m_implementation.getPostedMessages(user, params));
        return true;
    } else if (upcasedCommand == "USERLSCROSS") {
        /* @q USERLSCROSS [listParameters...] (Talk Command)
           List forums that a user can cross-post to using "allowgpost" permission.

           The list can be accessed in different ways, see {pcc:talk:listparams|listParameters}.
           Valid sort keys for forums are:
           - key
           - lastpost
           - lasttime
           - name

           Permissions: user context required, accesses user's profile

           @rettype Any
           @rettype FID */
        TalkUser::ListParameters params;
        TalkForumServer::parseListParameters(params, args);
        result.reset(m_implementation.getCrosspostToGameCandidates(params));
        return true;
    } else {
        return false;
    }
}

void
server::interface::TalkUserServer::parseSelection(interpreter::Arguments& args, std::vector<TalkUser::Selection>& selections)
{
    // ex WatchAction::process (part)
    while (args.getNumArgs() > 0) {
        String_t keyword = afl::string::strUCase(toString(args.getNext()));
        if (keyword == "THREAD") {
            TalkUser::Selection sel;
            args.checkArgumentCountAtLeast(1);
            sel.scope  = TalkUser::ThreadScope;
            sel.id     = toInteger(args.getNext());
            sel.lastId = 0;
            selections.push_back(sel);
        } else if (keyword == "FORUM") {
            TalkUser::Selection sel;
            args.checkArgumentCountAtLeast(1);
            sel.scope  = TalkUser::ForumScope;
            sel.id     = toInteger(args.getNext());
            sel.lastId = 0;
            selections.push_back(sel);
        } else {
            throw std::runtime_error(INVALID_OPTION);
        }
    }
}
