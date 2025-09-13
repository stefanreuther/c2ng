/**
  *  \file server/talk/commandhandler.cpp
  *  \brief Class server::talk::CommandHandler
  */

#include <stdexcept>
#include "server/talk/commandhandler.hpp"
#include "afl/string/char.hpp"
#include "afl/string/string.hpp"
#include "afl/sys/mutexguard.hpp"
#include "interpreter/arguments.hpp"
#include "server/errors.hpp"
#include "server/interface/talkaddressserver.hpp"
#include "server/interface/talkfolderserver.hpp"
#include "server/interface/talkforumserver.hpp"
#include "server/interface/talkgroupserver.hpp"
#include "server/interface/talknntpserver.hpp"
#include "server/interface/talkpmserver.hpp"
#include "server/interface/talkpostserver.hpp"
#include "server/interface/talkrenderserver.hpp"
#include "server/interface/talksyntaxserver.hpp"
#include "server/interface/talkthreadserver.hpp"
#include "server/interface/talkuserserver.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"
#include "server/talk/talkaddress.hpp"
#include "server/talk/talkfolder.hpp"
#include "server/talk/talkforum.hpp"
#include "server/talk/talkgroup.hpp"
#include "server/talk/talknntp.hpp"
#include "server/talk/talkpm.hpp"
#include "server/talk/talkpost.hpp"
#include "server/talk/talkrender.hpp"
#include "server/talk/talksyntax.hpp"
#include "server/talk/talkthread.hpp"
#include "server/talk/talkuser.hpp"
#include "server/types.hpp"


server::talk::CommandHandler::CommandHandler(Root& root, Session& session)
    : m_root(root),
      m_session(session)
{ }

bool
server::talk::CommandHandler::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    // Mutex to protect against parallel access (from notifier)
    afl::sys::MutexGuard g(m_root.mutex());

    // Log it
    logCommand(upcasedCommand, args);

    // Command dispatcher
    bool ok = false;
    if (!ok && upcasedCommand == "PING") {
        /* @q PING (Talk Command)
           Alive test.
           @retval Str "PONG". */
        result.reset(makeStringValue("PONG"));
        ok = true;
    }
    if (!ok && upcasedCommand == "HELP") {
        /* @q HELP [page:Str] (Talk Command)
           @retval Str Help page. */
        result.reset(makeStringValue(getHelp(afl::string::strUCase(toString(args.getNext())))));
        ok = true;
    }
    if (!ok && upcasedCommand == "USER") {
        /* @q USER user:UID (Talk Command)
           Set context (caller) for following commands on this connection. */
        // FIXME: can we do checkArgumentCount(1) here?
        m_session.setUser(toString(args.getNext()));
        result.reset(makeStringValue("OK"));
        ok = true;
    }
    if (!ok) {
        // SYNTAX
        TalkSyntax impl(m_root.keywordTable());
        ok = server::interface::TalkSyntaxServer(impl).handleCommand(upcasedCommand, args, result);
    }
    if (!ok) {
        // RENDER
        TalkRender impl(m_session, m_root);
        ok = server::interface::TalkRenderServer(impl).handleCommand(upcasedCommand, args, result);
    }
    if (!ok) {
        // GROUP
        TalkGroup impl(m_session, m_root);
        ok = server::interface::TalkGroupServer(impl).handleCommand(upcasedCommand, args, result);
    }
    if (!ok) {
        // POST
        TalkPost impl(m_session, m_root);
        ok = server::interface::TalkPostServer(impl).handleCommand(upcasedCommand, args, result);
    }
    if (!ok) {
        // FORUM
        TalkForum impl(m_session, m_root);
        ok =  server::interface::TalkForumServer(impl).handleCommand(upcasedCommand, args, result);
    }
    if (!ok) {
        // THREAD
        TalkThread impl(m_session, m_root);
        ok = server::interface::TalkThreadServer(impl).handleCommand(upcasedCommand, args, result);
    }
    if (!ok) {
        // USER
        TalkUser impl(m_session, m_root);
        ok = server::interface::TalkUserServer(impl).handleCommand(upcasedCommand, args, result);
    }
    if (!ok) {
        // FOLDER
        TalkFolder impl(m_session, m_root);
        ok = server::interface::TalkFolderServer(impl).handleCommand(upcasedCommand, args, result);
    }
    if (!ok) {
        // PM
        TalkPM impl(m_session, m_root);
        ok = server::interface::TalkPMServer(impl).handleCommand(upcasedCommand, args, result);
    }
    if (!ok) {
        // ADDR
        TalkAddress impl(m_session, m_root);
        ok = server::interface::TalkAddressServer(impl).handleCommand(upcasedCommand, args, result);
    }
    if (!ok) {
        // NNTP
        TalkNNTP impl(m_session, m_root);
        ok = server::interface::TalkNNTPServer(impl).handleCommand(upcasedCommand, args, result);
    }
    return ok;
}

String_t
server::talk::CommandHandler::getHelp(String_t topic)
{
    // ex planetscentral/talk/talkconnection.cc:getHelp
    if (topic == "GROUP") {
        return "Group commands:\n"
            "GROUPADD <grid> [<key> <value>...]\n"
            "GROUPGET <grid> <key>\n"
            "GROUPLS <grid>\n"
            "GROUPMSTAT <grid>...\n"
            "GROUPSET <grid> [<key> <value>...]\n"
            "GROUPSTAT <grid>\n";
    } else if (topic == "FORUM") {
        return "Forum commands:\n"
            "FORUMADD [<key> <value>...]\n"
            "FORUMBYNAME <name>\n"
            "FORUMGET <fid> <key>\n"
            "FORUMLSPOST <fid> <listoptions>\n"
            "FORUMLSSTICKY <fid> <listoptions>\n"
            "FORUMLSTHREAD <fid> <listoptions>\n"
            "FORUMMSTAT <fid>...\n"
            "FORUMPERMS <fid> <perm>...\n"
            "FORUMSET <fid> <key> <value>...\n"
            "FORUMSIZE <fid>\n"
            "FORUMSTAT <fid>\n";
    } else if (topic == "RENDER") {
        return "Render commands:\n"
            "RENDEROPTION <renderoptions>\n"
            "RENDER <text> <renderoptions>\n"
            "RENDERCHECK <text>\n";
    } else if (topic == "SYNTAX") {
        return "Syntax commands:\n"
            "SYNTAXGET <key>\n"
            "SYNTAXMGET <keys...>\n";
    } else if (topic == "POST") {
        return "Posting commands:\n"
            "POSTEDIT <mid> <subj> <text>\n"
            "POSTGET <mid> <field>\n"
            "POSTLSNEW <count>\n"
            "POSTMRENDER <mid>...\n"
            "POSTMSTAT <mid>...\n"
            "POSTNEW <fid> <subj> <text> [USER|READPERM|ANSWERPERM <arg>] [ALSO <fid>]\n"
            "POSTRENDER <mid> <renderoptions>\n"
            "POSTREPLY <mid> <subj> <text> [USER <arg>]\n"
            "POSTRM <mid>\n"
            "POSTSTAT <mid>\n";
    } else if (topic == "THREAD") {
        return "Thread commands:\n"
            "THREADLSPOST <tid> <listoptions>\n"
            "THREADMSTAT <tid>...\n"
            "THREADMV <tid> <fid>\n"
            "THREADPERMS <tid> <perm>...\n"
            "THREADRM <tid>\n"
            "THREADSTAT <tid>\n"
            "THREADSTICKY <tid> <value>\n";
    } else if (topic == "USER") {
        return "User commands:\n"
            "USERLSCROSS <listoptions>\n"
            "USERLSPOSTED <uid> []\n"
            "USERLSWATCHEDFORUMS <listoptions>\n"
            "USERLSWATCHEDTHREADS <listoptions>\n"
            "USERMARKSEEN [THREAD n|FORUM n]...\n"
            "USERNEWSRC [GET|SET|CLEAR|ANY|ALL|FIRSTSET|FIRSTCLEAR...]\n"
            "   [POST n n...|RANGE a b|THREAD n|FORUM n]...\n"
            "USERUNWATCH [THREAD n|FORUM n]...\n"
            "USERWATCH [THREAD n|FORUM n]...\n";
    } else if (topic == "FOLDER") {
        return "Folder commands:\n"
            "FOLDERLS\n"
            "FOLDERLSPM <ufid> <listoptions>\n"
            "FOLDERMSTAT <ufid>...\n"
            "FOLDERNEW <name>\n"
            "FOLDERRM <ufid>...\n"
            "FOLDERSET <ufid> <key> <value>...\n"
            "FOLDERSTAT <ufid>\n";
    } else if (topic == "PM") {
        return "PM commands:\n"
            "PMCP <src-ufid> <dst-ufid> <pmid>...\n"
            "PMFLAG <ufid> <clear> <set> <pmid>...\n"
            "PMMRENDER <ufid> <pmid>...\n"
            "PMMSTAT <ufid> <pmid>...\n"
            "PMMV <src-ufid> <dst-ufid> <pmid>...\n"
            "PMNEW <to> <subj> <text> [PARENT <pmid>]\n"
            "PMRENDER <ufid> <pmid> <renderoptions>\n"
            "PMRM <ufid> <pmid>...\n"
            "PMSTAT <ufid> <pmid>\n";
    } else if (topic == "ADDR") {
        return "ADDR commands:\n"
            "ADDRMPARSE <addr>...\n"
            "ADDRMRENDER <to>...\n";
    } else if (topic == "NNTP") {
        return "NNTP commands:\n"
            "NNTPFORUMLS <fid>\n"
            "NNTPGROUPLS <grid>\n"
            "NNTPLIST\n"
            "NNTPFINDNG <newsgroup>\n"
            "NNTPFINDMID <rfcmsgid>\n"
            "NNTPPOSTHEAD <mid>\n"
            "NNTPUSER <user> <pass>\n";
    } else if (topic == "OPTIONS") {
        return "List options (one per command):\n"
            "  CONTAINS <n>\n"
            "  LIMIT <start> <count>\n"
            "  SIZE\n"
            "  SORT <sortkey>\n"
            "Sort keys:\n"
            "  AUTHOR (post, PM)\n"
            "  EDITTIME (post)\n"
            "  FIRSTPOST (thread)\n"
            "  FORUM (thread)\n"
            "  KEY (forum)\n"
            "  LASTPOST (forum, thread)\n"
            "  LASTTIME (forum, thread)\n"
            "  NAME (forum)\n"
            "  SUBJECT (post, PM, thread)\n"
            "  THREAD (post)\n"
            "  TIME (post, PM)\n"
            "Render options (any number):\n"
            "  BASEURL <url>\n"
            "  FORMAT <fmt>\n"
            "Formats:\n"
            "  abstract:FORMAT (out)\n"
            "  break:FORMAT (out)\n"
            "  code:SYNTAX (in)\n"
            "  force:FORMAT (out)\n"
            "  forum<opts> (in, out)\n"
            "    <opts>: L (links), S (smileys)\n"
            "  html (out)\n"
            "  mail (out)\n"
            "  noquote:FORMAT (out)\n"
            "  quote:FORMAT (out)\n"
            "  raw (out)\n"
            "  text (in, out)\n";
    } else if (topic == "UID") {
        return "User Id specifications (ACL/PM addressee):\n"
            "-<spec>         (ACL) deny these\n"
            "all             (ACL) everyone\n"
            "p:<key>         (ACL) profile key\n"
            "u:<uid>         (ACL/PM) user\n"
            "g:<gid>         (ACL/PM) everyone in game\n"
            "g:<gid>:<slot>  (PM) slot in game\n";
    } else {
        return "Commands:\n"
            "HELP [<topic>]\n"
            "PING\n"
            "USER <uid>\n"
            "FOLDER->\n"
            "FORUM->\n"
            "GROUP->\n"
            "NNTP->\n"
            "OPTIONS->\n"
            "ADDR->\n"
            "PM->\n"
            "POST->\n"
            "RENDER->\n"
            "THREAD->\n"
            "UID->\n"
            "USER->\n"
            "This is c2talk-server (c2ng).\n";
    }
}

inline void
server::talk::CommandHandler::logCommand(const String_t& verb, interpreter::Arguments args)
{
    m_session.logCommand(m_root.log(), "talk.command", verb, args, (verb=="NNTPUSER" ? 2 : 0));
}
