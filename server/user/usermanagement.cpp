/**
  *  \file server/user/usermanagement.cpp
  */

#include "server/user/usermanagement.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/string/format.hpp"
#include "server/common/util.hpp"
#include "server/errors.hpp"
#include "server/user/passwordencrypter.hpp"
#include "server/user/root.hpp"
#include "server/user/user.hpp"

using afl::string::Format;

const char*const LOG_NAME = "user.mgmt";

server::user::UserManagement::UserManagement(Root& root)
    : m_root(root)
{ }

String_t
server::user::UserManagement::add(String_t userName, String_t password, afl::base::Memory<const String_t> config)
{
    // Normalize user name
    String_t simplifiedName = server::common::simplifyUserName(userName);
    if (simplifiedName.empty()) {
        throw std::runtime_error(INVALID_USERNAME);
    }

    // Reserve user name
    if (!m_root.userByName(simplifiedName).setUnique("0")) {
        throw std::runtime_error(ALREADY_EXISTS);
    }

    // Allocate user Id
    String_t userId = m_root.allocateUserId();
    m_root.log().write(afl::sys::Log::Info, LOG_NAME, Format("(%s) creating user '%s'", userId, simplifiedName));

    // Initialize profile
    // - password
    setPassword(userId, password);

    // - screen name
    String_t screenNameConfig[] = { "screenname", userName };
    setProfile(userId, screenNameConfig);

    // - default profile
    afl::data::StringList_t dp;
    m_root.defaultProfileCopy().getAll(dp);
    setProfile(userId, dp);

    // - custom profile (after default to allow override)
    setProfile(userId, config);

    // Finish up the user
    User(m_root, userId).tree().stringKey("name").set(simplifiedName);
    m_root.userByName(simplifiedName).set(userId);
    m_root.allUsers().add(userId);

    return userId;
}

String_t
server::user::UserManagement::login(String_t userName, String_t password)
{
    // ex doNntpUser, server::talk::TalkNNTP::checkUser
    // Check user name
    const String_t userId = m_root.getUserIdFromLogin(userName);
    if (userId.empty()) {
        throw std::runtime_error(INVALID_USERNAME);
    }

    // Get their password
    User u(m_root, userId);
    String_t correctHash = u.passwordHash().get();
    if (correctHash.empty()) {
        throw std::runtime_error(INVALID_PASSWORD);
    }

    // Validate
    switch (m_root.encrypter().checkPassword(password, correctHash, userId)) {
     case PasswordEncrypter::Invalid:
        // Invalid password
        throw std::runtime_error(INVALID_PASSWORD);

     case PasswordEncrypter::ValidCurrent:
        // Valid password, no action needed
        break;

     case PasswordEncrypter::ValidNeedUpdate:
        // Password needs update
        m_root.log().write(afl::sys::Log::Info, LOG_NAME, Format("(%s) password upgrade user", userId));
        u.passwordHash().set(m_root.encrypter().encryptPassword(password, userId));
        break;
    }
    return userId;
}

String_t
server::user::UserManagement::getUserIdByName(String_t userName)
{
    const String_t userId = m_root.getUserIdFromLogin(userName);
    if (userId.empty()) {
        throw std::runtime_error(USER_NOT_FOUND);
    }
    return userId;
}

String_t
server::user::UserManagement::getNameByUserId(String_t userId)
{
    return User(m_root, userId).getLoginName();
}

void
server::user::UserManagement::getNamesByUserId(afl::base::Memory<const String_t> userIds, afl::data::StringList_t& userNames)
{
    while (const String_t* p = userIds.eat()) {
        userNames.push_back(getNameByUserId(*p));
    }
}

server::Value_t*
server::user::UserManagement::getProfileRaw(String_t userId, String_t key)
{
    return User(m_root, userId).getProfileRaw(key);
}

server::Value_t*
server::user::UserManagement::getProfileRaw(String_t userId, afl::base::Memory<const String_t> keys)
{
    User u(m_root, userId);
    afl::data::Vector::Ref_t vec = afl::data::Vector::create();
    while (const String_t* p = keys.eat()) {
        vec->pushBackNew(u.getProfileRaw(*p));
    }
    return new afl::data::VectorValue(vec);
}

void
server::user::UserManagement::setProfile(String_t userId, afl::base::Memory<const String_t> config)
{
    User u(m_root, userId);
    while (const String_t* pKey = config.eat()) {
        if (const String_t* pValue = config.eat()) {
            u.profile().stringField(*pKey).set(*pValue);
        }
    }
}

void
server::user::UserManagement::setPassword(String_t userId, String_t password)
{
    m_root.log().write(afl::sys::Log::Info, LOG_NAME, Format("(%s) password change", userId));
    User(m_root, userId).passwordHash().set(m_root.encrypter().encryptPassword(password, userId));
}
