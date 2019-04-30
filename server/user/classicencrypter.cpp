/**
  *  \file server/user/classicencrypter.cpp
  *  \brief Class server::user::ClassicEncrypter
  */

#include "server/user/classicencrypter.hpp"
#include "afl/checksums/md5.hpp"
#include "afl/charset/base64.hpp"

server::user::ClassicEncrypter::ClassicEncrypter(String_t userKey)
    : m_userKey(userKey)
{ }

String_t
server::user::ClassicEncrypter::encryptPassword(String_t password, String_t /*userId*/)
{
    afl::checksums::MD5 ctx;
    ctx.add(afl::string::toBytes(m_userKey));
    ctx.add(afl::string::toBytes(password));

    uint8_t hashBuffer[afl::checksums::MD5::MAX_HASH_SIZE];
    String_t userHash = "1," + afl::string::fromBytes(afl::charset::Base64().encode(afl::string::toMemory(afl::string::fromBytes(ctx.getHash(hashBuffer)))));
    while (userHash.size() > 0 && userHash[userHash.size()-1] == '=') {
        userHash.erase(userHash.size()-1);
    }
    return userHash;
}

server::user::PasswordEncrypter::Result
server::user::ClassicEncrypter::checkPassword(String_t password, String_t hash, String_t userId)
{
    if (encryptPassword(password, userId) == hash) {
        return ValidCurrent;
    } else {
        return Invalid;
    }
}
