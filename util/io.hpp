/**
  *  \file util/io.hpp
  */
#ifndef C2NG_UTIL_IO_HPP
#define C2NG_UTIL_IO_HPP

#include "afl/io/stream.hpp"
#include "afl/string/string.hpp"
#include "afl/charset/charset.hpp"
#include "afl/io/datasink.hpp"

namespace util {

    bool storePascalString(afl::io::DataSink& out, const String_t& str, afl::charset::Charset& charset);
    bool storePascalStringTruncate(afl::io::DataSink& out, const String_t& str, afl::charset::Charset& charset);

    String_t loadPascalString(afl::io::Stream& in, afl::charset::Charset& charset);

}

#endif
