/**
  *  \file server/interface/sessionroutersingleserver.cpp
  */

#include <stdexcept>
#include "server/interface/sessionroutersingleserver.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/net/line/linesink.hpp"
#include "afl/string/char.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/string/string.hpp"
#include "server/errors.hpp"
#include "server/interface/sessionrouter.hpp"

namespace {
    const char*const DEFAULT_SUCCESS = "200 OK";
}


server::interface::SessionRouterSingleServer::SessionRouterSingleServer(SessionRouter& impl)
    : LineHandler(),
      m_impl(impl),
      m_state(ReadCommand),
      m_talkCommand(),
      m_talkSession()
{ }

bool
server::interface::SessionRouterSingleServer::handleOpening(afl::net::line::LineSink& /*response*/)
{
    // No greeting
    return false;
}

bool
server::interface::SessionRouterSingleServer::handleLine(const String_t& line, afl::net::line::LineSink& response)
{
    try {
        switch (m_state) {
         case ReadCommand:
            return handleCommand(line, response);

         case ReadTalkCommand:
         case ReadTalkBody:
            // RouterSession::executeRequest (part)
            if ((m_state == ReadTalkCommand && isPOST(line))
                || (m_state == ReadTalkBody && line != "."))
            {
                m_talkCommand += line;
                m_talkCommand += '\n';
                m_state = ReadTalkBody;
                return false;
            } else {
                if (m_state == ReadTalkCommand) {
                    m_talkCommand = line;
                }
                response.handleLine(m_impl.talk(m_talkSession, m_talkCommand));
                return finish();
            }

         case Finished:
            // No more data expected in this state
            return true;
        }
        return true;
    }
    catch (std::exception& e) {
        char firstChar = e.what()[0];
        if (firstChar >= '4' && firstChar <= '6') {
            // Looks like a well-formatted error, give to client as is
            response.handleLine(e.what());
        } else {
            // Make a server response
            response.handleLine(INTERNAL_ERROR);
        }
        m_state = Finished;
        return true;
    }
}

void
server::interface::SessionRouterSingleServer::handleConnectionClose()
{ }

bool
server::interface::SessionRouterSingleServer::isPOST(const String_t& cmd)
{
    return cmd.size() >= 4
        && afl::string::charToUpper(cmd[0]) == 'P'
        && afl::string::charToUpper(cmd[1]) == 'O'
        && afl::string::charToUpper(cmd[2]) == 'S'
        && afl::string::charToUpper(cmd[3]) == 'T'
        && (cmd.size() == 4 || cmd[4] == ' ');
}

bool
server::interface::SessionRouterSingleServer::isSAVE(const String_t& cmd)
{
    return cmd.size() >= 4
        && afl::string::charToUpper(cmd[0]) == 'S'
        && afl::string::charToUpper(cmd[1]) == 'A'
        && afl::string::charToUpper(cmd[2]) == 'V'
        && afl::string::charToUpper(cmd[3]) == 'E'
        && (cmd.size() == 4 || cmd[4] == ' ');
}

bool
server::interface::SessionRouterSingleServer::handleCommand(const String_t& line, afl::net::line::LineSink& response)
{
    // ex router.cc:handleRequest
    const char*const WHITESPACE = " \t";
    const String_t::size_type verbEnd  = std::min(line.find_first_of(WHITESPACE), line.size());
    const String_t::size_type argStart = std::min(line.find_first_not_of(WHITESPACE, verbEnd), line.size());
    const String_t verb = afl::string::strUCase(line.substr(0, verbEnd));
    const String_t arg = line.substr(argStart);

    // FIXME: log the line

    // Process request
    if (verb == "LIST") {
        /* @q LIST (Router Command)
           List active sessions.
           Active sessions will be reported as a table:
           - session Id
           - PID
           - idle time in seconds
           - "," if the session was used, "V" if it is still virgin (indicates a browser without JavaScript)
           - "." if the session was modified, "S" if it was saved
           - command line parameters of the session */
        response.handleLine(m_impl.getStatus());
        return finish();
    } else if (verb == "INFO") {
        /* @q INFO id:RouterSession (Router Command)
           Get information about a session.
           Reports all command line parameters, one per line.
           @err 452 Session timed out (no such session) */
        response.handleLine(m_impl.getInfo(arg));
        return finish();
    } else if (verb == "S") {
        /* @q S id:RouterSession (Router Command)
           Talk to a session.
           The actual request (e.g. "GET ...") follows on a line after the command.
           For POST requests, the POST body follows after the request.
           The response is equal to the response reported by the server.
           @err 452 Session timed out (no such session)
           @err 400 Bad request (%id invalid, or request cannot be read)
           @err 500 Invalid answer from server */
        m_talkSession = arg;
        m_state = ReadTalkCommand;
        return false;
    } else if (verb == "CLOSE") {
        /* @q CLOSE {id:RouterSession | -flag} (Router Command)
           When given an %id, closes that session.

           When given a %flag, closes all sessions that conflict with that flag.
           The session Ids of all closed sessions are reported with the response. */
        return handleAction(arg, SessionRouter::Close, response);
    } else if (verb == "RESTART") {
        /* @q RESTART {id:RouterSession | -flag} (Router Command)
           When given an %id, restarts that session.

           When given a %flag, restarts all sessions that conflict with that flag.
           The session Ids of all restarted sessions are reported with the response.

           @err 500 Restart failed (some sessions could not be restarted)
           @err 452 Session timed out */
        return handleAction(arg, SessionRouter::Restart, response);
    } else if (verb == "SAVE") {
        /* @q SAVE {id:RouterSession | -flag} (Router Command)
           When given an %id, saves that session.

           When given a %flag, saves all sessions that conflict with that flag.
           The session Ids of all saved sessions are reported with the response.

           @uses FORGET (File Command) */
        return handleAction(arg, SessionRouter::Save, response);
    } else if (verb == "SAVENN") {
        /* @q SAVENN {id:RouterSession | -flag} (Router Command)
           When given an %id, saves that session.

           When given a %flag, saves all sessions that conflict with that flag.
           The session Ids of all saved sessions are reported with the response.

           Unlike {SAVE (Router Command)}, this command does not notify the {File (Service)|File Service}. */
        return handleAction(arg, SessionRouter::SaveNN, response);
    } else if (verb == "NEW") {
        /* @q NEW args:Str... (Router Command)
           Start new session.
           The new session identifier will be reported as NN:{@type RouterSession} in a "201 NN Created" response.

           @err 403 Forbidden (generail failure)
           @err 451 Too many sessions
           @err 453 Conflicting session already active
           @rettype RouterSession */

        // Split into words
        afl::data::StringList_t args;
        String_t::size_type pos = 0;
        while (pos < arg.size()) {
            const String_t::size_type a = std::min(arg.find_first_not_of(WHITESPACE, pos), arg.size());
            const String_t::size_type b = std::min(arg.find_first_of    (WHITESPACE, a),   arg.size());
            if (a == b) {
                break;
            }
            args.push_back(arg.substr(a, b-a));
            pos = b;
        }

        // Do it
        String_t sessionId = m_impl.create(args);
        response.handleLine(afl::string::Format("201 %s Created", sessionId));
        return finish();
    } else if (verb == "CONFIG") {
        /* @q CONFIG (Router Command)
           Report the configuration. */
        response.handleLine(m_impl.getConfiguration());
        return finish();
    } else {
        response.handleLine(UNKNOWN_COMMAND);
        return finish();
    }
}

bool
server::interface::SessionRouterSingleServer::handleAction(const String_t& arg, SessionRouter::Action action, afl::net::line::LineSink& response)
{
    // ex GroupCommand::run (part)
    if (arg.empty()) {
        // Missing argument
        response.handleLine(INVALID_NUMBER_OF_ARGUMENTS);
    } else if (arg[0] == '-') {
        // Group action
        afl::data::StringList_t result;
        m_impl.groupAction(arg.substr(1), action, result);
        response.handleLine(DEFAULT_SUCCESS);
        for (size_t i = 0, n = result.size(); i < n; ++i) {
            response.handleLine(result[i]);
        }
    } else if (!arg.empty()) {
        // Single action
        m_impl.sessionAction(arg, action);
        response.handleLine(DEFAULT_SUCCESS);
    } else {
        // Bad argument
        response.handleLine(INVALID_VALUE);
    }

    return finish();
}

bool
server::interface::SessionRouterSingleServer::finish()
{
    m_state = Finished;
    return true;
}
