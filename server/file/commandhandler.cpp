/**
  *  \file server/file/commandhandler.cpp
  */

#include <stdexcept>
#include "server/file/commandhandler.hpp"
#include "server/types.hpp"
#include "server/errors.hpp"
#include "afl/string/char.hpp"
#include "interpreter/arguments.hpp"
#include "server/file/root.hpp"
#include "server/file/session.hpp"
#include "server/file/filebase.hpp"
#include "server/interface/filebaseserver.hpp"
#include "server/interface/filegameserver.hpp"
#include "server/file/filegame.hpp"

namespace {
    // FIXME: code duplication
    bool isPrintable(const String_t& s)
    {
        for (String_t::size_type i = 0; i < s.size(); ++i) {
            if (!afl::string::charIsAlphanumeric(s[i]) && s[i] != '/' && s[i] != '.' && s[i] != '_' && s[i] != '-' && s[i] != '*' && s[i] != ':' && s[i] != ',') {
                return false;
            }
        }
        return true;
    }

    String_t formatWord(const String_t& word)
    {
        if (word.empty()) {
            return "''";
        } else if (word.size() < 100 && isPrintable(word)) {
            return word;
        } else {
            return "...";
        }
    }
}

server::file::CommandHandler::CommandHandler(Root& root, Session& session)
    : m_root(root),
      m_session(session)
{ }

bool
server::file::CommandHandler::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    // Log it
    logCommand(upcasedCommand, args);

    // Command dispatcher
    // @change PCC2 has a QUIT command, we do not
    bool ok = false;
    if (!ok && upcasedCommand == "PING") {
        /* @q PING (File Command)
           Alive test.
           @retval Str "PONG". */
        result.reset(makeStringValue("PONG"));
        ok = true;
    }
    if (!ok && upcasedCommand == "HELP") {
        /* @q HELP (File Command)
           @retval Str Help page. */
        result.reset(makeStringValue(getHelp()));
        ok = true;
    }
    if (!ok && upcasedCommand == "USER") {
        /* @q USER user:UID (File Command)
           Set context (caller) for following commands on this connection. */
        args.checkArgumentCount(1);
        m_session.setUser(toString(args.getNext()));
        result.reset(makeStringValue("OK"));
        ok = true;
    }
    if (!ok) {
        // Most commands
        FileBase base(m_session, m_root);
        ok = server::interface::FileBaseServer(base).handleCommand(upcasedCommand, args, result);
    }
    if (!ok) {
        // GAME/REG commands
        FileGame game(m_session, m_root);
        ok = server::interface::FileGameServer(game).handleCommand(upcasedCommand, args, result);
    }
    // @change PCC2 returns 405 here
    return ok;
}

String_t
server::file::CommandHandler::getHelp()
{
    return "List of commands:\n"
        "QUIT\n"
        "HELP\n"
        "PING\n"
        "STAT file\n"
        "LS dir\n"
        "USER user\n"
        "MKDIR dir\n"
        "MKDIRAS dir user\n"
        "GET file\n"
        "PUT file data\n"
        "CP from to\n"
        "RM file-or-dir\n"
        "RMDIR dir\n"
        "FORGET dir\n"
        "USAGE dir\n"
        "STATREG dir\n"
        "LSREG dir\n"
        "STATGAME dir\n"
        "LSGAME dir\n"
        "PROPGET dir prop\n"
        "PROPSET dir prop value\n"
        "SETPERM dir user perms\n"
        "LSPERM dir\n"
        "FTEST file...\n"
        "This is c2file-ng\n";
}

// FIXME: code duplication...
void
server::file::CommandHandler::logCommand(const String_t& verb, interpreter::Arguments args)
{
    // Log channel name
    String_t channel = "file.command";
    if (!m_session.isAdmin()) {
        channel += ".";
        channel += m_session.getUser();
    }

    // Command
    String_t text = formatWord(verb);
    while (args.getNumArgs() != 0) {
        text += " ";
        text += formatWord(toString(args.getNext()));
    }

    // Log it
    m_root.log().write(afl::sys::Log::Info, channel, text);
}
