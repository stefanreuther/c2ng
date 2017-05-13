/**
  *  \file game/browser/account.cpp
  *  \brief game::browser::Account
  */

#include "game/browser/account.hpp"
#include "afl/charset/urlencoding.hpp"
#include "afl/charset/base64.hpp"

namespace {
    const char USER_KEY[] = "user";
    const char TYPE_KEY[] = "type";
    const char HOST_KEY[] = "host";
    const char GAME_KEY_PREFIX[] = "game:";
}

// Default constructor.
game::browser::Account::Account()
    : m_name(),
      m_data()
{ }

// Destructor.
game::browser::Account::~Account()
{ }

// Set name.
void
game::browser::Account::setName(String_t name)
{
    m_name = name;
}

// Get name.
String_t
game::browser::Account::getName() const
{
    return m_name;
}

// Set attribute.
void
game::browser::Account::set(String_t key, String_t value, bool persistent)
{
    m_data.insert(std::make_pair(key, Item_t(value, persistent)));
}

// Get attribute.
const String_t*
game::browser::Account::get(String_t key) const
{
    std::map<String_t, Item_t>::const_iterator it = m_data.find(key);
    if (it != m_data.end()) {
        return &it->second.first;
    } else {
        return 0;
    }
}

// Get attribute, with default.
String_t
game::browser::Account::get(String_t key, String_t defaultValue) const
{
    if (const String_t* p = get(key)) {
        return *p;
    } else {
        return defaultValue;
    }
}

// Set encoded attribute.
void
game::browser::Account::setEncoded(String_t key, String_t value, bool persistent)
{
    set(key, afl::string::fromBytes(afl::charset::Base64().encode(afl::string::toMemory(value))), persistent);
}

// Get encoded attribute.
bool
game::browser::Account::getEncoded(String_t key, String_t& result) const
{
    if (const String_t* p = get(key)) {
        result = afl::charset::Base64().decode(afl::string::toBytes(*p));
        return true;
    } else {
        return false;
    }
}

// Check validity.
bool
game::browser::Account::isValid() const
{
    // FIXME: should verify thet USER,TYPE,HOST are actually persistent
    return get(USER_KEY) != 0
        && get(TYPE_KEY) != 0
        && get(HOST_KEY) != 0;
}

// Get user name.
String_t
game::browser::Account::getUser() const
{
    return get(USER_KEY, String_t());
}

// Set user name.
void
game::browser::Account::setUser(String_t user)
{
    set(USER_KEY, user, true);
}

// Get account type.
String_t
game::browser::Account::getType() const
{
    return get(TYPE_KEY, String_t());
}

// Set account type.
void
game::browser::Account::setType(String_t type)
{
    set(TYPE_KEY, type, true);
}

// Get host name.
String_t
game::browser::Account::getHost() const
{
    return get(HOST_KEY, String_t());
}

// Set host name.
void
game::browser::Account::setHost(String_t host)
{
    set(HOST_KEY, host, true);
}

// Set game folder name.
void
game::browser::Account::setGameFolderName(String_t gameId, String_t folderName)
{
    set(GAME_KEY_PREFIX + afl::string::fromBytes(afl::charset::UrlEncoding().encode(afl::string::toMemory(gameId))), folderName, true);
}

// Get game folder name.
const String_t*
game::browser::Account::getGameFolderName(String_t gameId) const
{
    return get(GAME_KEY_PREFIX + afl::string::fromBytes(afl::charset::UrlEncoding().encode(afl::string::toMemory(gameId))));
}

// Remove game folder name.
void
game::browser::Account::removeGameFolderName(String_t gameId)
{
    m_data.erase(GAME_KEY_PREFIX + afl::string::fromBytes(afl::charset::UrlEncoding().encode(afl::string::toMemory(gameId))));
}

// Save this account's data to a text file.
void
game::browser::Account::write(afl::io::TextFile& file) const
{
    file.writeText("[");
    file.writeText(m_name);
    file.writeText("]");
    file.writeLine();
    for (std::map<String_t, Item_t>::const_iterator it = m_data.begin(); it != m_data.end(); ++it) {
        if (it->second.second) {
            file.writeText(it->first);
            file.writeText("=");
            file.writeText(it->second.first);
            file.writeLine();
        }
    }
}
