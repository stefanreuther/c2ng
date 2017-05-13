/**
  *  \file server/format/stringpacker.cpp
  *  \brief Class server::format::StringPacker
  */

#include "server/format/stringpacker.hpp"
#include "server/types.hpp"

String_t
server::format::StringPacker::pack(afl::data::Value* data, afl::charset::Charset& cs)
{
    // ex StringPacker::pack
    return afl::string::fromBytes(cs.encode(afl::string::toMemory(toString(data))));
}

afl::data::Value*
server::format::StringPacker::unpack(const String_t& data, afl::charset::Charset& cs)
{
    // ex StringPacker::unpack
    return makeStringValue(cs.decode(afl::string::toBytes(data)));
}
