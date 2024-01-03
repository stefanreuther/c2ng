/**
  *  \file util/runlengthcompress.hpp
  *  \brief Run-Length Encoding
  */
#ifndef C2NG_UTIL_RUNLENGTHCOMPRESS_HPP
#define C2NG_UTIL_RUNLENGTHCOMPRESS_HPP

#include "afl/base/growablememory.hpp"
#include "afl/base/memory.hpp"

namespace util {

    /** Compress data using run-length encoding.
        This RLE scheme was organically-grown and used in PCC1 for resource files,
        and can be decoded using RunLengthExpandTransform; see there for file format docs.

        For now, we provide only this simple one-shot function to compress an entire file at once.
        Because our RLE scheme prepends the payload size,
        splitting a block in two and compressing each separately does not produce the same result
        as processing the block at once.

        @param out [out]  Result will be appended here
        @param in  [in]   Data to compress */
    void encodeRLE(afl::base::GrowableBytes_t& out, afl::base::ConstBytes_t in);

}

#endif
