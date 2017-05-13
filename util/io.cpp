/**
  *  \file util/io.cpp
  */

#include "util/io.hpp"
#include "afl/base/growablememory.hpp"

bool
util::storePascalString(afl::io::DataSink& out, const String_t& str, afl::charset::Charset& charset)
{
    // Encode
    afl::base::GrowableBytes_t encoded = charset.encode(afl::string::toMemory(str));

    // Can we represent the size?
    uint8_t size = uint8_t(encoded.size());
    if (size == encoded.size()) {
        out.handleFullData(afl::base::fromObject(size));
        out.handleFullData(encoded);
        return true;
    } else {
        return false;
    }
}

bool
util::storePascalStringTruncate(afl::io::DataSink& out, const String_t& str, afl::charset::Charset& charset)
{
    // Encode
    afl::base::GrowableBytes_t encoded = charset.encode(afl::string::toMemory(str));

    // Can we represent the size?
    bool ok;
    if (encoded.size() <= 255) {
        ok = true;
    } else {
        encoded.trim(255);
        ok = false;
    }
    uint8_t size = uint8_t(encoded.size());

    out.handleFullData(afl::base::fromObject(size));
    out.handleFullData(encoded);
    return ok;
}

String_t
util::loadPascalString(afl::io::Stream& in, afl::charset::Charset& charset)
{
    // Read size
    uint8_t size;
    in.fullRead(afl::base::fromObject(size));

    // Read body
    afl::base::GrowableBytes_t encodedChars;
    encodedChars.resize(size);
    in.fullRead(encodedChars);

    return charset.decode(encodedChars);
}
