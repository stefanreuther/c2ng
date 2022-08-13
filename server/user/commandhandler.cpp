/**
  *  \file server/user/commandhandler.cpp
  *  \brief Class server::user::CommandHandler
  */

#include "server/user/commandhandler.hpp"
#include "server/interface/userdataserver.hpp"
#include "server/interface/usermanagementserver.hpp"
#include "server/interface/usertokenserver.hpp"
#include "server/types.hpp"
#include "server/user/userdata.hpp"
#include "server/user/usermanagement.hpp"
#include "server/user/usertoken.hpp"

server::user::CommandHandler::CommandHandler(Root& root)
    : m_root(root)
{ }

bool
server::user::CommandHandler::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    // We do not log commands because they are very frequent and contain sensitive data.

    bool ok = false;
    if (!ok && upcasedCommand == "PING") {
        /* @q PING (User Command)
           Responds with PONG. */
        result.reset(makeStringValue("PONG"));
        ok = true;
    }
    if (!ok && upcasedCommand == "HELP") {
        /* @q HELP [page:Str] (User Command)
           Returns a help page.
           @rettype Str */
        result.reset(makeStringValue(getHelp(afl::string::strUCase(toString(args.getNext())))));
        ok = true;
    }
    if (!ok) {
        UserManagement impl(m_root);
        ok = server::interface::UserManagementServer(impl).handleCommand(upcasedCommand, args, result);
    }
    if (!ok) {
        UserToken impl(m_root);
        ok = server::interface::UserTokenServer(impl).handleCommand(upcasedCommand, args, result);
    }
    if (!ok) {
        UserData impl(m_root);
        ok = server::interface::UserDataServer(impl).handleCommand(upcasedCommand, args, result);
    }

    return ok;
}

String_t
server::user::CommandHandler::getHelp(String_t topic) const
{
    if (topic == "TOKEN") {
        return "Token commands:\n"
            "MAKETOKEN uid type\n"
            "CHECKTOKEN token [TYPE type] [RENEW]\n"
            "RESETTOKEN uid [type...]\n";
    } else if (topic == "USER") {
        return "User command:\n"
            "ADDUSER user pass [k v...]\n"
            "LOGIN user pass\n"
            "LOOKUP user\n"
            "NAME uid\n"
            "MNAME uid...\n"
            "GET uid k\n"
            "MGET uid k...\n"
            "SET uid [k v...]\n"
            "PASSWD uid pass\n";
    } else if (topic == "DATA") {
        return "Data command:\n"
            "UGET uid key\n"
            "USET uid key value\n";
    } else {
        return "Commands:\n"
            "PING\n"
            "HELP [topic]\n"
            "TOKEN->\n"
            "USER->\n"
            "DATA->\n"
            "This is c2user-ng.\n";
    }
}
