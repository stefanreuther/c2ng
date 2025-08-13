/**
  *  \file server/common/session.cpp
  *  \brief Class server::common::Session
  */

#include <stdexcept>
#include "server/common/session.hpp"
#include "server/errors.hpp"
#include "server/types.hpp"
#include "afl/string/char.hpp"

namespace {
    bool isPrintable(const String_t& s)
    {
        for (String_t::size_type i = 0; i < s.size(); ++i) {
            if (!afl::string::charIsAlphanumeric(s[i]) && s[i] != '/' && s[i] != '.' && s[i] != '_' && s[i] != '-' && s[i] != '*' && s[i] != ':' && s[i] != ',') {
                return false;
            }
        }
        return true;
    }
}

// Constructor.
server::common::Session::Session()
    : m_user()
{ }

// Destructor.
server::common::Session::~Session()
{ }

// Set the user.
void
server::common::Session::setUser(String_t user)
{
    m_user = user;
}

// Get current user.
String_t
server::common::Session::getUser() const
{
    // ex TalkConnection::getUser
    return m_user;
}

// Check for admin permissions.
bool
server::common::Session::isAdmin() const
{
    // ex TalkConnection::isAdmin
    return m_user.empty();
}

// Check for admin permissions.
void
server::common::Session::checkAdmin() const
{
    // ex TalkConnection::checkAdmin
    if (!isAdmin()) {
        throw std::runtime_error(PERMISSION_DENIED);
    }
}

// Check for user context.
void
server::common::Session::checkUser() const
{
    // ex TalkConnection::checkUser
    if (isAdmin()) {
        throw std::runtime_error(MUST_HAVE_USER_CONTEXT);
    }
}

// Check for user context, provided by user or implicitly.
String_t
server::common::Session::checkUserOption(const afl::base::Optional<String_t>& opt) const
{
    if (isAdmin()) {
        const String_t* p = opt.get();
        if (p == 0 || p->empty()) {
            throw std::runtime_error(MUST_HAVE_USER_CONTEXT);
        }
        return *p;
    } else {
        const String_t* p = opt.get();
        if (p != 0 && *p != m_user) {
            throw std::runtime_error(USER_NOT_ALLOWED);
        }
        return m_user;
    }
}

// Log a command.
void
server::common::Session::logCommand(afl::sys::LogListener& log, String_t logChannel, const String_t& verb, interpreter::Arguments args, size_t censor)
{
    // Log channel name
    if (!isAdmin()) {
        logChannel += ".";
        logChannel += getUser();
    }

    // Command
    String_t text = formatWord(verb, false);
    size_t i = 1;
    while (args.getNumArgs() != 0) {
        text += " ";
        text += formatWord(toString(args.getNext()), censor==i);
        ++i;
    }

    // Log it
    log.write(afl::sys::Log::Info, logChannel, text);
}

// Format a word for logging.
String_t
server::common::Session::formatWord(const String_t& word, bool censor)
{
    if (word.empty()) {
        return "''";
    } else if (word.size() < 100 && !censor && isPrintable(word)) {
        return word;
    } else {
        return "...";
    }
}
