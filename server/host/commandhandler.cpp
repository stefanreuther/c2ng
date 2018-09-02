/**
  *  \file server/host/commandhandler.cpp
  *  \brief Class server::host::CommandHandler
  */

#include "server/host/commandhandler.hpp"
#include "afl/string/char.hpp"
#include "afl/sys/mutexguard.hpp"
#include "server/host/exporter.hpp"
#include "server/host/file/rootitem.hpp"
#include "server/host/hostcron.hpp"
#include "server/host/hostfile.hpp"
#include "server/host/hostgame.hpp"
#include "server/host/hostplayer.hpp"
#include "server/host/hostschedule.hpp"
#include "server/host/hosttool.hpp"
#include "server/host/hostturn.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"
#include "server/interface/hostcronserver.hpp"
#include "server/interface/hostfileserver.hpp"
#include "server/interface/hostgameserver.hpp"
#include "server/interface/hostplayerserver.hpp"
#include "server/interface/hostscheduleserver.hpp"
#include "server/interface/hosttoolserver.hpp"
#include "server/interface/hostturnserver.hpp"
#include "server/types.hpp"


server::host::CommandHandler::CommandHandler(Root& root, Session& session)
    : m_root(root),
      m_session(session)
{ }

bool
server::host::CommandHandler::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    // ex HostConnection::handleCommand
    // Obtain a mutex. Commands are automatically serialized,
    // but we must guard against the scheduler running in a different thread.
    // In the future, we might have multiple threads processing commands.
    afl::sys::MutexGuard g(m_root.mutex());

    // Log the command
    logCommand(upcasedCommand, args);

    // Configure child connections.
    m_root.configureReconnect();

    // Command dispatcher
    bool ok = false;
    if (!ok && upcasedCommand == "PING") {
        /* @q PING (Host Command)
           Responds with PONG. */
        result.reset(makeStringValue("PONG"));
        ok = true;
    }
    if (!ok && upcasedCommand == "HELP") {
        /* @q HELP [page:Str] (Host Command)
           Returns a help page.
           @rettype Str */
        result.reset(makeStringValue(getHelp(afl::string::strUCase(toString(args.getNext())))));
        ok = true;
    }
    if (!ok && upcasedCommand == "USER") {
        /* @q USER user:UID (Host Command)
           Set context (caller) for following commands on this connection. */
        // FIXME: can we do checkArgumentCount(1) here?
        m_session.setUser(toString(args.getNext()));
        result.reset(makeStringValue("OK"));
        ok = true;
    }
    if (!ok) {
        // HOSTxxx
        HostTool impl(m_session, m_root, m_root.hostRoot());
        ok = server::interface::HostToolServer(impl, HostTool::Host).handleCommand(upcasedCommand, args, result);
    }
    if (!ok) {
        // MASTERxxx
        HostTool impl(m_session, m_root, m_root.masterRoot());
        ok = server::interface::HostToolServer(impl, HostTool::Master).handleCommand(upcasedCommand, args, result);
    }
    if (!ok) {
        // SHIPLISTxxx
        HostTool impl(m_session, m_root, m_root.shipListRoot());
        ok = server::interface::HostToolServer(impl, HostTool::ShipList).handleCommand(upcasedCommand, args, result);
    }
    if (!ok) {
        // TOOLxxx
        HostTool impl(m_session, m_root, m_root.toolRoot());
        ok = server::interface::HostToolServer(impl, HostTool::Tool).handleCommand(upcasedCommand, args, result);
    }
    if (!ok) {
        // GAMExxx
        HostGame impl(m_session, m_root);
        ok = server::interface::HostGameServer(impl).handleCommand(upcasedCommand, args, result);
    }
    if (!ok) {
        // TRNxxx
        HostTurn impl(m_session, m_root);
        ok = server::interface::HostTurnServer(impl).handleCommand(upcasedCommand, args, result);
    }
    if (!ok) {
        // PLAYERxxx
        HostPlayer impl(m_session, m_root);
        ok = server::interface::HostPlayerServer(impl).handleCommand(upcasedCommand, args, result);
    }
    if (!ok) {
        // SCHEDULExxx
        HostSchedule impl(m_session, m_root);
        ok = server::interface::HostScheduleServer(impl).handleCommand(upcasedCommand, args, result);
    }
    if (!ok) {
        // GET, LS, ...
        server::host::file::RootItem item(m_session, m_root);
        HostFile impl(item);
        ok = server::interface::HostFileServer(impl).handleCommand(upcasedCommand, args, result);
    }
    if (!ok) {
        // CRONxxx
        HostCron impl(m_session, m_root);
        ok = server::interface::HostCronServer(impl).handleCommand(upcasedCommand, args, result);
    }
    return ok;
}

String_t
server::host::CommandHandler::getHelp(String_t topic) const
{
    // ex host/host.cc:getHelp
    if (topic == "HOST" || topic == "MASTER" || topic == "TOOL" || topic == "SHIPLIST") {
        return "HOST/MASTER/TOOL/SHIPLIST Commands:\n"
            " HOSTADD id path exe kind\n"
            " HOSTCP oldId newId\n"
            " HOSTDEFAULT id\n"
            " HOSTGET id field\n"
            " HOSTLS\n"
            " HOSTRATING id {{SET n|AUTO} {USE|SHOW}|NONE|GET}\n"
            " HOSTRM id\n"
            " HOSTSET id field value\n"
            "All commands also with prefix MASTER, TOOL, SHIPLIST instead of HOST.\n";
    } else if (topic == "CRON") {
        return "CRON Commands:\n"
            " CRONGET gid\n"
            " CRONKICK gid\n"
            " CRONLIST [LIMIT n]\n";
    } else if (topic == "GAME") {
        return "GAME Commands:\n"
            " NEWGAME\n"
            " CLONEGAME gid [state]\n"
            " GAMEADDTOOL gid toolid\n"
            " GAMECHECKPERM gid uid\n"
            " GAMEGET gid key\n"
            " GAMEGETCC gid key\n"
            " GAMEGETDIR gid\n"
            " GAMEGETNAME gid\n"
            " GAMEGETOWNER gid\n"
            " GAMEGETSTATE gid\n"
            " GAMEGETTYPE gid\n"
            " GAMEGETVC gid\n"
            " GAMELIST [STATE state] [TYPE type] [VERBOSE] [USER uid]\n"
            " GAMELSTOOLS gid\n"
            " GAMERMTOOL gid toolid\n"
            " GAMESET gid key value\n"
            " GAMESETNAME gid owner\n"
            " GAMESETOWNER gid ownerUid\n"
            " GAMESETSTATE gid state\n"
            " GAMESETTYPE gid type\n"
            " GAMESTAT gid\n"
            " GAMETOTALS\n"
            " GAMEUPDATE gid...\n";
    } else if (topic == "PLAYER") {
        return "PLAYER commands:\n"
            " PLAYERADD gid uid\n"
            " PLAYERCHECKFILE gid pid name [DIR dir]\n"
            " PLAYERGETDIR gid pid\n"
            " PLAYERJOIN gid slot uid\n"
            " PLAYERLS gid\n"
            " PLAYERRESIGN gid slot uid\n"
            " PLAYERSETDIR gid pid dir\n"
            " PLAYERSTAT gid slot\n"
            " PLAYERSUBST gid slot uid\n";
    } else if (topic == "SCHEDULE") {
        return "SCHEDULE Commands:\n"
            " SCHEDULEADD gid schedulespec\n"
            " SCHEDULEDROP gid\n"
            " SCHEDULELIST gid\n"
            " SCHEDULEMOD gid [schedulespec]\n"
            " SCHEDULESET gid schedulespec\n"
            " SCHEDULESHOW gid [TURNLIMIT n|TIMELIMIT n]\n"
            "schedulespec is:\n"
            " STOP|WEEKLY bits|DAILY interval|ASAP\n"
            " DAYTIME n\n"
            " EARLY|NOEARLY\n"
            " DELAY n\n"
            " LIMIT n\n"
            " UNTILTURN n|UNTILTIME n|FOREVER\n";
    } else {
        return "Commands:\n"
            " PING\n"
            " HELP [<topic>]\n"
            " USER <uid>\n"
            " CRON->\n"
            " GAME->\n"
            " HOST->\n"
            " MASTER->\n"
            " PLAYER->\n"
            " SCHEDULE->\n"
            " SHIPLIST->\n"
            " TOOL->\n"
            " TRN <data> [GAME <gid> [SLOT <slot>]] [MAIL <mail>]\n"
            " TRNMARKTEMP <gid> <slot> <state>\n"
            "This is c2host-ng.\n";
    }
}

inline void
server::host::CommandHandler::logCommand(const String_t& verb, interpreter::Arguments args)
{
    m_session.logCommand(m_root.log(), "host.command", verb, args, 0);
}
