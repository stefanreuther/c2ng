/**
  *  \file server/file/ca/objectid.cpp
  */

#include <cstring>
#include "server/file/ca/objectid.hpp"
#include "afl/charset/hexencoding.hpp"
#include "afl/string/hex.hpp"

const server::file::ca::ObjectId server::file::ca::ObjectId::nil = {{ 0xda, 0x39, 0xa3, 0xee, 0x5e, 0x6b, 0x4b, 0x0d, 0x32, 0x55, 0xbf, 0xef, 0x95, 0x60, 0x18, 0x90, 0xaf, 0xd8, 0x07, 0x09 }};

server::file::ca::ObjectId
server::file::ca::ObjectId::fromHash(afl::checksums::Hash& hash)
{
    ObjectId result;
    afl::base::Bytes_t desc(result.m_bytes);
    desc.subrange(hash.getHash(desc).size()).fill(0);
    return result;
}

server::file::ca::ObjectId
server::file::ca::ObjectId::fromHex(const String_t& str)
{
    ObjectId result;
    String_t unpacked = afl::charset::HexEncoding().decode(afl::string::toBytes(str));

    afl::base::Bytes_t resultMemory(result.m_bytes);
    resultMemory.subrange(resultMemory.copyFrom(afl::string::toBytes(unpacked)).size()).fill(0);
    return result;
}

String_t
server::file::ca::ObjectId::toHex() const
{
    // FIXME: can we do better? Probably this is because the signature of Charset.encode sucks.
    return afl::string::fromBytes(afl::charset::HexEncoding(afl::string::HEX_DIGITS_LOWER).encode(afl::string::ConstStringMemory_t::unsafeCreate(reinterpret_cast<const char*>(m_bytes), sizeof(m_bytes))));
}

bool
server::file::ca::ObjectId::operator==(const ObjectId& other) const
{
    return std::memcmp(m_bytes, other.m_bytes, sizeof(m_bytes)) == 0;
}

bool
server::file::ca::ObjectId::operator!=(const ObjectId& other) const
{
    return !operator==(other);
}

bool
server::file::ca::ObjectId::operator<(const ObjectId& other) const
{
    return std::memcmp(m_bytes, other.m_bytes, sizeof(m_bytes)) < 0;
}
