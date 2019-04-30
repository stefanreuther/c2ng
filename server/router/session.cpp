/**
  *  \file server/router/session.cpp
  *  \brief Class server::router::Session
  */

#include "server/router/session.hpp"
#include "afl/net/line/linehandler.hpp"
#include "afl/net/line/linesink.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/loglistener.hpp"
#include "server/errors.hpp"
#include "server/interface/sessionroutersingleserver.hpp"
#include "util/process/subprocess.hpp"
#include "util/string.hpp"

namespace {
    const char*const LOG_NAME = "router.session";

    bool isConflictMarker(const String_t& s)
    {
        return s.size() >= 2
            && s[0] == '-'
            && (s[1] == 'R' || s[1] == 'W');
    }

    bool isSame(const String_t& a, const String_t& b, bool bIsWild)
    {
        std::size_t an = a.size();
        std::size_t bn = b.size();
        if (bIsWild && bn > 0 && b[bn-1] == '*') {
            return an >= bn-1
                && a.compare(0, bn-1, b, 0, bn-1) == 0
                && (an == bn-1
                    || a[bn-1] == '/');
        } else {
            return a == b;
        }
    }

    /** Check for conflict. Conflict resolution uses "reader/writer lock" terminology.
        Each session can be associated with a set of keywords, starting with "-R" or "-W"
        (they need not have a real-world meaning, i.e. they need not imply that someone
        is reading or writing).

        Any number with an identical "-R" keyword can coexist, but if there is a "-W"
        session, it must be the only one.

        @change The original implementation had a aWinsTie parameter
        and allowed this function to decide the direction of conflict resolution.
        This is wrong: we don't win ties in a strenght contest, it's older-vs-newer session.
        This does not change the site behaviour; as of 20190127 the only conflict we trigger is '-WDIR' for closing sessions.

        \param a,b Keywords
        \param bIsWild true if b is a wildcard ("FOO*").
        \return true on conflict */
    bool checkConflict(const String_t& a, const String_t& b, bool bIsWild)
    {
        return isConflictMarker(a)
            && isConflictMarker(b)
            && (a[1] == 'W' || b[1] == 'W')
            && isSame(a.substr(2), b.substr(2), bIsWild);
    }
}


server::router::Session::Session(util::process::Factory& factory, afl::base::Memory<const String_t> args, String_t id, afl::sys::LogListener& log, server::interface::FileBase* pFileBase)
    : m_id(id),
      m_log(log),
      m_pFileBase(pFileBase),
      m_args(),
      m_lastAccessTime(afl::sys::Time::getCurrentTime()),
      m_isModified(false),
      m_isUsed(false),
      m_process(factory.createNewProcess())
{
    // ex RouterSession::RouterSession, parseArgs
    while (const String_t* p = args.eat()) {
        m_args.push_back(*p);
    }
}

server::router::Session::~Session()
{
    // RouterSession::~RouterSession
    stop();
}

String_t
server::router::Session::getId() const
{
    // ex RouterSession::getSessionId
    return m_id;
}

uint32_t
server::router::Session::getProcessId() const
{
    // ex RouterSession::getPid
    return m_process->getProcessId();
}

bool
server::router::Session::isModified() const
{
    // ex RouterSession::wasModified
    return m_isModified;
}

bool
server::router::Session::isUsed() const
{
    // ex RouterSession::wasUsed
    return m_isUsed;
}

bool
server::router::Session::isActive() const
{
    return m_process->isActive();
}

afl::sys::Time
server::router::Session::getLastAccessTime() const
{
    // ex RouterSession::getLastAccessTime
    return m_lastAccessTime;
}

afl::base::Memory<const String_t>
server::router::Session::getCommandLine() const
{
    // ex RouterSession::getName (but totally different)
    return m_args;
}

bool
server::router::Session::checkConflict(const Session& other) const
{
    // RouterSession::checkConflict
    for (size_t i = 0, n = m_args.size(); i < n; ++i) {
        if (other.checkConflict(m_args[i], false)) {
            return true;
        }
    }
    return false;
}

// /** Check for conflict.
//     \param query Keyword ("-Rfoo", "-Wfoo")
//     \return true iff this session conflicts with this keyword */
bool
server::router::Session::checkConflict(const String_t& query, bool queryIsWild) const
{
    // RouterSession::checkConflict
    if (!isConflictMarker(query)) {
        return false;
    }
    for (size_t i = 0, n = m_args.size(); i < n; ++i) {
        if (::checkConflict(m_args[i], query, queryIsWild)) {
            return true;
        }
    }
    return false;
}

bool
server::router::Session::start(const String_t& serverPath)
{
    logCommandLine();
    bool ok = m_process->start(serverPath, m_args);
    if (ok) {
        // Wait for child to start up. It will write a "hello" message with a "100" code, or some error messages.
        String_t greeting;
        if (readLine(greeting) && greeting.compare(0, 3, "100", 3) == 0) {
            // Looks like a success message
            logProcess(afl::sys::LogListener::Info, "started");
        } else {
            // Looks like a failure message
            logProcess(afl::sys::LogListener::Warn, "failed to start");
            do {
                util::removeTrailingCharacter(greeting, '\n');
                m_log.write(afl::sys::LogListener::Trace, LOG_NAME, greeting);
            } while (readLine(greeting));
            stop();
            ok = false;
        }
    } else {
        m_log.write(afl::sys::LogListener::Warn, LOG_NAME, afl::string::Format("[%s] failed to start: %s", m_id, m_process->getStatus()));
    }
    return ok;
}

void
server::router::Session::stop()
{
    // ex RouterSession::stop
    if (m_process->isActive()) {
        uint32_t savedPID = m_process->getProcessId();
        logProcess(afl::sys::LogListener::Info, "stopping...", savedPID);
        bool ok = m_process->stop();
        logProcess(ok ? afl::sys::LogListener::Info : afl::sys::LogListener::Warn, m_process->getStatus(), savedPID);
        notifyFileServer();
    }
}

void
server::router::Session::save(bool notify)
{
    // ex RouterSession::save
    if (m_process->isActive()) {
        setLastAccessTime();
        m_isUsed = true;
        if (m_isModified) {
            // When we're here, the session was modified and can be saved
            logProcess(afl::sys::LogListener::Trace, "'SAVE' (from router)");

            // Save
            if (!m_process->writeLine("SAVE\n")) {
                handleError("write error (command)");
            }

            String_t h, b;
            readResponse(h, b);

            // Mark saved
            m_isModified = false;

            // Notify file server
            if (notify) {
                notifyFileServer();
            }
        }
    }
}

String_t
server::router::Session::talk(String_t command)
{
    // ex RouterSession::executeRequest
    if (m_process->isActive()) {
        util::addTrailingCharacter(command, '\n');
        if (server::interface::SessionRouterSingleServer::isPOST(command)) {
            command += ".\n";
        }

        String_t::size_type n = command.find('\n');
        logProcess(afl::sys::LogListener::Trace, afl::string::Format("'%s'", command.substr(0, n)));

        m_isUsed = true;
        m_isModified = !server::interface::SessionRouterSingleServer::isSAVE(command);
        setLastAccessTime();

        if (!m_process->writeLine(command)) {
            handleError("write error (command)");
        }

        String_t header, body;
        readResponse(header, body);

        // FIXME: what should be the proper response format?
        return header + body;
    } else {
        return String_t(SESSION_TIMED_OUT);
    }
}

void
server::router::Session::logCommandLine()
{
    String_t msg = afl::string::Format("[%s] starting:", m_id);
    for (size_t i = 0, n = m_args.size(); i < n; ++i) {
        msg += " ";
        msg += m_args[i];
    }
    m_log.write(afl::sys::LogListener::Info, LOG_NAME, msg);
}

void
server::router::Session::logProcess(afl::sys::LogListener::Level level, const String_t& msg)
{
    logProcess(level, msg, m_process.get() != 0 ? m_process->getProcessId() : 0);
}

void
server::router::Session::logProcess(afl::sys::LogListener::Level level, const String_t& msg, uint32_t pid)
{
    m_log.write(level, LOG_NAME, afl::string::Format("[%s:%d] %s", m_id.substr(0, 10), pid, msg));
}

void
server::router::Session::setLastAccessTime()
{
    m_lastAccessTime = afl::sys::Time::getCurrentTime();
}

bool
server::router::Session::readLine(String_t& line)
{
    return m_process->readLine(line);
}

void
server::router::Session::readResponse(String_t& header, String_t& body)
{
    if (!m_process->readLine(header)) {
        handleError("read error (header)");
    }
    if (header.empty()) {
        handleError("protocol error (empty header)");
    }
    if (header[0] == '2') {
        while (true) {
            String_t line;
            if (!m_process->readLine(line)) {
                handleError("read error (body)");
            }
            if (line == ".\n") {
                break;
            }
            body += line;
        }
    }
}

void
server::router::Session::notifyFileServer()
{
    // ex RouterSession::maybeNotifyFileServer
    if (m_pFileBase != 0) {
        for (afl::data::StringList_t::const_iterator i = m_args.begin(); i != m_args.end(); ++i) {
            if (i->compare(0, 6, "-WDIR=", 6) == 0) {
                // Normally, FORGET does not report errors (if it's not there, it's forgotten, right?).
                // This still may throw if we have a network hiccup or similar,
                // and we don't want that to hinder us *at this point*.
                // - if we're running on the filers filespace, and filer is not running, there's nothing to sync.
                // - if we're running on the filer itself, the error will be detected earlier
                try {
                    m_pFileBase->forgetDirectory(i->substr(6));
                }
                catch (std::exception&)
                { }
            }
        }
    }
}

void
server::router::Session::handleError(const char* reason)
{
    logProcess(afl::sys::LogListener::Warn, reason);
    stop();
    throw std::runtime_error(SESSION_TIMED_OUT);
}
