/**
  *  \file util/io.cpp
  *  \brief I/O-related utilities
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

String_t
util::appendFileNameExtension(afl::io::FileSystem& fs, String_t pathName, String_t ext, bool force)
{
    // ex io/dirs.cc:appendFileNameExtension
    String_t fileName = fs.getFileName(pathName);
    String_t dirName  = fs.getDirectoryName(pathName);
    if (fileName.size() == 0) {
        // pathological case
        return fs.makePathName(dirName, "." + ext);
    } else {
        // do not accept index 0 to avoid identifying ".emacs" as zero-length basename with extension EMACS
        String_t::size_type n = fileName.rfind('.');
        if (n == String_t::npos || n == 0) {
            return fs.makePathName(dirName, fileName + "." + ext);
        } else if (force) {
            return fs.makePathName(dirName, fileName.substr(0, n+1) + ext);
        } else {
            return pathName;
        }
    }
}
