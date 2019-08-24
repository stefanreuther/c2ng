/**
  *  \file server/user/saltedpasswordencrypter.cpp
  *  \brief Class server::user::SaltedPasswordEncrypter
  */

#include "server/user/saltedpasswordencrypter.hpp"
#include "afl/checksums/sha1.hpp"

server::user::SaltedPasswordEncrypter::SaltedPasswordEncrypter(server::common::IdGenerator& saltGenerator)
    : PasswordEncrypter(),
      m_saltGenerator(saltGenerator)
{ }

String_t
server::user::SaltedPasswordEncrypter::encryptPassword(String_t password, String_t userId)
{
    String_t salt = m_saltGenerator.createId();

    String_t result = "2,";
    result += salt;
    result += ",";

    afl::checksums::SHA1 ctx;
    ctx.add(afl::string::toBytes(result));
    ctx.add(afl::string::toBytes(userId));
    ctx.add(afl::string::toBytes(","));
    ctx.add(afl::string::toBytes(password));

    result += ctx.getHashAsHexString();
    return result;
}

server::user::PasswordEncrypter::Result
server::user::SaltedPasswordEncrypter::checkPassword(String_t password, String_t hash, String_t userId)
{
    // Quick check
    if (hash.size() < 3 || hash.compare(0, 2, "2,", 2) != 0) {
        return Invalid;
    }

    // Identify salt/checksum split
    String_t::size_type p = hash.find(',', 2);
    if (p == String_t::npos) {
        return Invalid;
    }
    ++p;

    // Checksum
    afl::checksums::SHA1 ctx;
    ctx.add(afl::string::toBytes(hash).subrange(0, p));
    ctx.add(afl::string::toBytes(userId));
    ctx.add(afl::string::toBytes(","));
    ctx.add(afl::string::toBytes(password));

    if (ctx.getHashAsHexString() != hash.substr(p)) {
        return Invalid;
    }

    return ValidCurrent;
}
