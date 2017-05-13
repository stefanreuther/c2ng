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

    /** Store Pascal string, all-or-nothing.
        Stores a length byte followed by the string data if the length can be correctly represented,
        nothing if the string is too long.
        \param out Data sink
        \param str String to store
        \param charset Character set
        \retval true String stored successfully
        \retval false String too long, nothing stored */
    bool storePascalString(afl::io::DataSink& out, const String_t& str, afl::charset::Charset& charset);

    /** Store Pascal string, truncating version.
        Stores a length byte followed by the string data if the length can be correctly represented.
        If the string is too long, truncates it to 255 characters (in target character set!).
        \param out Data sink
        \param str String to store
        \param charset Character set
        \retval true String stored entirely
        \retval false String too long, stored truncated */
    bool storePascalStringTruncate(afl::io::DataSink& out, const String_t& str, afl::charset::Charset& charset);

    String_t loadPascalString(afl::io::Stream& in, afl::charset::Charset& charset);

}

#endif
