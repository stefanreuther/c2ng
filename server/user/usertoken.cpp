/**
  *  \file server/user/usertoken.cpp
  */

#include <memory>
#include <stdexcept>
#include "server/user/usertoken.hpp"
#include "afl/data/access.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/net/redis/sortoperation.hpp"
#include "afl/string/format.hpp"
#include "server/errors.hpp"
#include "server/user/root.hpp"
#include "server/user/token.hpp"
#include "server/user/user.hpp"

namespace {
    const char*const LOG_NAME = "user.token";

    /*
     *  Token definitions
     */
    struct Descriptor {
        /** Maximum age: token expires this time after creation. */
        int maxAge;

        /** Minimum age: token is renewed if less than this time remains in its life. */
        int minAge;
    };

    const Descriptor& getDescriptor(const String_t& tokenType)
    {
        /* Log-in token: for interactive use.
           Automatically renewed when user re-visits the website within the given time period.
           Expiry means user needs to re-enter their password. */
        static const Descriptor LOGIN_TOKEN = { 6*31*24*60, 3*31*24*60 };

        /* API token: for API use.
           Same behaviour as log-in token for now; expiry means user needs to re-enter their password. */
        static const Descriptor API_TOKEN = { 6*31*24*60, 3*31*24*60 };

        /* Password reset token.
           Officially valid for 3 days. Not automatically renewed. */
        static const Descriptor RESET_TOKEN = { 4*24*60, 3*24*60 };

        if (tokenType == "login") {
            return LOGIN_TOKEN;
        } else if (tokenType == "api") {
            return API_TOKEN;
        } else if (tokenType == "reset") {
            return RESET_TOKEN;
        } else {
            throw std::runtime_error(server::BAD_TOKEN_TYPE);
        }
    }
}


server::user::UserToken::UserToken(Root& root)
    : m_root(root)
{ }

String_t
server::user::UserToken::getToken(String_t userId, String_t tokenType)
{
    Token item(m_root.tokenById("*"));
    User u(m_root, userId);
    const Descriptor& desc = getDescriptor(tokenType);
    Time_t now = m_root.getTime();

    // Get all tokens and their expiration time
    std::auto_ptr<afl::data::Value> data(u.tokensByType(tokenType).
                                         sort().
                                         sortDisable().
                                         get().
                                         get(item.validUntil()).
                                         getResult());
    afl::data::Access a(data);

    // Process data:
    // - delete expired tokens
    // - find newest token
    // We do not rely on server-side sorting because it just doesn't matter.
    // Normally, there shouldn't be more than a handful of active tokens per user,
    // so we shouldn't have to worry about data size here. Maximum number of tokens
    // would be defined as (maxAge / (maxAge - minAge)), which would be 4 for "reset" tokens.
    String_t bestToken;
    Time_t bestTime = 0;
    for (size_t i = 0, n = a.getArraySize(); i < n; i += 2) {
        Time_t thisTime = a[i+1].toInteger();
        String_t thisToken = a[i].toString();
        if (thisTime < now) {
            // Token is expired, remove it
            deleteToken(userId, tokenType, thisToken);
        } else if (thisTime > bestTime) {
            // Not expired
            bestToken = thisToken;
            bestTime = thisTime;
        } else {
            // Not expired but not better than previous one
        }
    }

    // Do we need to create a new token?
    if (bestTime == 0 || (bestTime - now) < desc.minAge) {
        return createToken(userId, tokenType, now + desc.maxAge);
    } else {
        return bestToken;
    }
}

server::interface::UserToken::Info
server::user::UserToken::checkToken(String_t token, afl::base::Optional<String_t> requiredType, bool autoRenew)
{
    // Token must exist
    if (!m_root.allTokens().contains(token)) {
        throw std::runtime_error(TOKEN_EXPIRED);
    }

    // Fetch token's data
    Info result;
    Token t(m_root.tokenById(token));
    result.userId = t.userId().get();
    result.tokenType = t.tokenType().get();
    if (const String_t* pType = requiredType.get()) {
        if (result.tokenType != *pType) {
            throw std::runtime_error(TOKEN_EXPIRED);
        }
    }

    // Check for expired token
    Time_t validUntil = t.validUntil().get();
    Time_t now = m_root.getTime();
    if (validUntil < now) {
        deleteToken(result.userId, result.tokenType, token);
        throw std::runtime_error(TOKEN_EXPIRED);
    }

    // Check for token needing to be renewed
    if (autoRenew) {
        const Descriptor& desc = getDescriptor(result.tokenType);
        if ((validUntil - now) < desc.minAge) {
            result.newToken = createToken(result.userId, result.tokenType, now + desc.maxAge);
        }
    }

    return result;
}

void
server::user::UserToken::clearToken(String_t userId, afl::base::Memory<const String_t> tokenTypes)
{
    // FIXME: validate that userId is valid?
    User u(m_root, userId);

    // Iterate through all token types.
    // Do NOT validate the types here, so we can get rid of token types that are no longer valid.
    while (const String_t* pType = tokenTypes.eat()) {
        afl::data::StringList_t list;
        u.tokensByType(*pType).getAll(list);
        for (size_t i = 0, n = list.size(); i < n; ++i) {
            deleteToken(userId, *pType, list[i]);
        }
    }
}

void
server::user::UserToken::deleteToken(String_t userId, String_t tokenType, String_t token)
{
    m_root.log().write(afl::sys::LogListener::Debug, LOG_NAME, afl::string::Format("(%s) remove '%s' token", userId, tokenType));
    m_root.allTokens().remove(token);
    User(m_root, userId).tokensByType(tokenType).remove(token);
    m_root.tokenById(token).remove();
}

String_t
server::user::UserToken::createToken(String_t userId, String_t tokenType, Time_t validUntil)
{
    // Create the token.
    // There ought not to be any collisions, but retry anyway if we get one.
    // Do NOT use the atomic add() operation here, because we need to add to allTokens() last.
    m_root.log().write(afl::sys::LogListener::Debug, LOG_NAME, afl::string::Format("(%s) create '%s' token", userId, tokenType));
    String_t token;
    do {
        token = m_root.generator().createId();
    } while (m_root.allTokens().contains(token));

    Token t = m_root.tokenById(token);
    t.userId().set(userId);
    t.tokenType().set(tokenType);
    t.validUntil().set(validUntil);

    User(m_root, userId).tokensByType(tokenType).add(token);
    m_root.allTokens().add(token);

    return token;
}
