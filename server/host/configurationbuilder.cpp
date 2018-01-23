/**
  *  \file server/host/configurationbuilder.cpp
  *  \brief Class server::host::ConfigurationBuilder
  */

#include "server/host/configurationbuilder.hpp"

namespace {
    /** Check for characters valid in identifiers (also see isValidIdentifier). */
    bool isIdentifierCharacter(const char c)
    {
        return (c >= '0' && c <= '9')
            || (c >= 'A' && c <= 'Z')
            || (c >= 'a' && c <= 'z')
            || c == '_';
    }

    /** Check for characters guaranteed not to be mangled by a shell.
        All other characters must be quoted somehow. */
    bool isSafeCharacter(const char c)
    {
        return isIdentifierCharacter(c)
            || c == '.'
            || c == '-'
            || c == '+'
            || c == '/'
            || c == ':'
            || c == ',';
    }

    /** Check for a valid identifier.
        Those are valid as variable names in a shell script, and thus can be possible exported values. */
    bool isValidIdentifier(const String_t& s)
    {
        /* bash(1) says:
           name   A word consisting only of  alphanumeric  characters  and  underscores,  and
           beginning  with an alphabetic character or an underscore.  Also referred to
           as an identifier.

           SUSv4 says in volume 1, 3.230 Name:
           In the shell command language, a word consisting solely of underscores, digits,
           and alphabetics from the portable character set. The first character of a name
           is not a digit. */
        if (s.empty() || (s[0] >= '0' && s[0] <= '9')) {
            return false;
        }
        for (size_t i = 0; i < s.size(); ++i) {
            if (!isIdentifierCharacter(s[i])) {
                return false;
            }
        }
        return true;
    }
}

// Constructor.
server::host::ConfigurationBuilder::ConfigurationBuilder()
    : m_buffer()
{ }

// Destructor.
server::host::ConfigurationBuilder::~ConfigurationBuilder()
{ }

// Add a value.
void
server::host::ConfigurationBuilder::addValue(const String_t& key, const String_t& value)
{
    // ex planetscentral/host/exec.cc:exportValue
    if (!isValidIdentifier(key)) {
        // This value cannot be exported.
    } else {
        m_buffer += key;
        m_buffer += '=';;
        for (size_t i = 0; i < value.size(); ++i) {
            if (uint8_t(value[i]) < ' ') {
                // embedded newline etc.
                break;
            } else if (!isSafeCharacter(value[i])) {
                m_buffer += '\\';
            }
            m_buffer += value[i];
        }
        m_buffer += '\n';
    }
}

// Get accumulated content.
afl::base::ConstBytes_t
server::host::ConfigurationBuilder::getContent() const
{
    return afl::string::toBytes(m_buffer);
}
