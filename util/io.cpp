/**
  *  \file util/io.cpp
  */

#include "util/io.hpp"
#include "afl/base/growablememory.hpp"

bool
util::storePascalString(afl::io::DataSink& out, const String_t& str, afl::charset::Charset& charset)
{
    // Encode
    String_t encoded = charset.encode(afl::string::toMemory(str));
    afl::base::ConstBytes_t encodedBytes = afl::string::toBytes(encoded);

    // Can we represent the size?
    uint8_t size = uint8_t(encodedBytes.size());
    if (size == encodedBytes.size()) {
        out.handleFullData("<string>", afl::base::fromObject(size));
        out.handleFullData("<string>", encodedBytes);
        return true;
    } else {
        return false;
    }
}

bool
util::storePascalStringTruncate(afl::io::DataSink& out, const String_t& str, afl::charset::Charset& charset)
{
    // Encode
    String_t encoded = charset.encode(afl::string::toMemory(str));
    afl::base::ConstBytes_t encodedBytes = afl::string::toBytes(encoded);

    // Can we represent the size?
    bool ok;
    if (encodedBytes.size() <= 255) {
        ok = true;
    } else {
        encodedBytes.trim(255);
        ok = false;
    }
    uint8_t size = uint8_t(encodedBytes.size());

    out.handleFullData("<string>", afl::base::fromObject(size));
    out.handleFullData("<string>", encodedBytes);
    return ok;
}

String_t
util::loadPascalString(afl::io::Stream& in, afl::charset::Charset& charset)
{
    // Read size
    uint8_t size;
    in.fullRead(afl::base::fromObject(size));

    // Read body
    afl::base::GrowableMemory<char> encodedChars;
    encodedChars.resize(size);
    in.fullRead(encodedChars.toBytes());

    return charset.decode(encodedChars);
}
