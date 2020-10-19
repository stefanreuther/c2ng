/**
  *  \file server/router/sessionrouter.cpp
  */

#include <stdexcept>
#include "server/router/sessionrouter.hpp"
#include "afl/string/format.hpp"
#include "server/router/root.hpp"
#include "server/router/session.hpp"
#include "server/errors.hpp"
#include "util/string.hpp"

server::router::SessionRouter::SessionRouter(Root& root)
    : m_root(root)
{ }

String_t
server::router::SessionRouter::getStatus()
{
    // ex router.cc:handleListRequest
    m_root.removeExpiredSessions();

    const Root::Sessions_t& sessions = m_root.sessions();

    String_t result = afl::string::Format("200 OK, %d sessions\n", sessions.size());

    afl::sys::Time now = afl::sys::Time::getCurrentTime();
    for (size_t i = 0, n = sessions.size(); i < n; ++i) {
        if (Session* ps = sessions[i]) {
            // Command line
            String_t commandStr;
            afl::base::Memory<const String_t> commandLine = ps->getCommandLine();
            while (const String_t* p = commandLine.eat()) {
                commandStr += ' ';
                commandStr += *p;
            }

            // Produce output
            result += (afl::string::Format("%10s  %6d %6ds  %c%c%s\n")
                       << ps->getId().substr(0, 10)
                       << ps->getProcessId()
                       << ((now - ps->getLastAccessTime()).getMilliseconds() / 1000)
                       << (ps->isUsed() ? '.' : 'V')
                       << (ps->isModified() ? '.' : 'S')
                       << commandStr);
        }
    }
    util::removeTrailingCharacter(result, '\n');
    return result;
}

String_t
server::router::SessionRouter::getInfo(SessionId_t sessionId)
{
    // ex router.cc:handleInfoRequest, RouterSession::writeArgs
    m_root.removeExpiredSessions();

    // Fetch session
    Session* ps = m_root.getSessionById(sessionId);
    if (ps == 0) {
        throw std::runtime_error(SESSION_TIMED_OUT);
    }

    // Produce output
    String_t result = "200 OK\n";
    afl::base::Memory<const String_t> commandLine = ps->getCommandLine();
    while (const String_t* p = commandLine.eat()) {
        result += *p;
        result += '\n';
    }
    util::removeTrailingCharacter(result, '\n');
    return result;
}

String_t
server::router::SessionRouter::talk(SessionId_t sessionId, String_t command)
{
    // ex handleTalkRequest
    // Fetch session
    Session* ps = m_root.getSessionById(sessionId);
    if (ps == 0) {
        throw std::runtime_error(SESSION_TIMED_OUT);
    }

    // Do it
    String_t result = ps->talk(command);
    util::removeTrailingCharacter(result, '\n');

    // Expire other sessions (and this one if the command closes it)
    m_root.removeExpiredSessions();

    return result;
}

void
server::router::SessionRouter::sessionAction(SessionId_t sessionId, Action action)
{
    // ex GroupCommand::run (part)
    // c2router-classic answers with
    // - "200 OK, n sessions closed/saved" for CLOSE/SAVE/SAVENN
    // - "200 OK", "500 Restart failed", "452 Session timed out" on RESTART
    // Let's keep it simple: if it does not exist, fail it; otherwise, let the action decide.
    Session* ps = m_root.getSessionById(sessionId);
    if (ps == 0) {
        throw std::runtime_error(SESSION_TIMED_OUT);
    }

    doAction(*ps, action);

    m_root.removeExpiredSessions();
}

void
server::router::SessionRouter::groupAction(String_t key, Action action, afl::data::StringList_t& result)
{
    const Root::Sessions_t& sessions = m_root.sessions();
    key.insert(0, "-");
    for (size_t i = 0, n = sessions.size(); i < n; ++i) {
        if (Session* ps = sessions[i]) {
            if (ps->checkConflict(key, true)) {
                result.push_back(ps->getId());
                doAction(*ps, action);
            }
        }
    }

    m_root.removeExpiredSessions();
}

server::interface::SessionRouter::SessionId_t
server::router::SessionRouter::create(afl::base::Memory<const String_t> args)
{
    return m_root.createSession(args).getId();
}

String_t
server::router::SessionRouter::getConfiguration()
{
    // This command mainly serves as a litmus test that c2router came up ok and with the right config.
    // c2router-classic also logged its bind IP/port, but I don't emulate that today.
    const Configuration& config = m_root.config();
    return afl::string::Format("200 OK\n"
                               "Router.Timeout=%d\n"
                               "Router.VirginTimeout=%d\n"
                               "Router.MaxSessions=%d\n"
                               "Router.NewSessionsWin=%d\n")
        << config.normalTimeout
        << config.virginTimeout
        << config.maxSessions
        << int(config.newSessionsWin);
}

void
server::router::SessionRouter::doAction(Session& s, Action action)
{
    switch (action) {
     case Close:
        s.stop();
        break;

     case Restart:
        m_root.restartSession(s);
        break;

     case Save:
        s.save(true);
        break;

     case SaveNN:
        s.save(false);
        break;
    }
}
