/**
  *  \file test/util/runlengthcompresstest.cpp
  *  \brief Test for util::RunLengthCompress
  */

#include "util/runlengthcompress.hpp"

#include "afl/base/memory.hpp"
#include "afl/test/testrunner.hpp"
#include "util/runlengthexpandtransform.hpp"
#include <vector>

using afl::base::Bytes_t;
using afl::base::ConstBytes_t;
using afl::base::GrowableBytes_t;

namespace {
    void verifyRoundTrip(afl::test::Assert a, ConstBytes_t data, size_t maxSize)
    {
        // Encode
        GrowableBytes_t packedData;
        util::encodeRLE(packedData, data);

        // Verify size limit
        a.checkLessThan("01. size", packedData.size(), maxSize);

        // Verify correct decompression
        std::vector<uint8_t> unpackedData;
        unpackedData.resize(data.size() + 10);

        ConstBytes_t in(packedData);
        Bytes_t out(unpackedData);
        util::RunLengthExpandTransform().transform(in, out);

        // Must have processed all compressed input
        a.check("11. empty", in.empty());

        // Must have produced exact uncompressed input data
        a.checkEqual("21. size", out.size(), data.size());
        if (data.size() > 0) {
            a.checkEqualContent<uint8_t>("22. content", out, data);
        }
    }
}

/** Test compression of empty block.
    Expected size is 6 bytes (total + chunk size). */
AFL_TEST("util.RunLengthCompress:empty", a)
{
    ConstBytes_t data;
    verifyRoundTrip(a, data, 20);
}

/** Test compression of compressible data.
    Expected size is ~42 bytes (12x3 bytes covering up to 255 input bytes, plus headers). */
AFL_TEST("util.RunLengthCompress:compressible", a)
{
    for (int i = 0; i < 256; ++i) {
        uint8_t data[3000];
        Bytes_t(data).fill(uint8_t(i));
        verifyRoundTrip(a, data, 50);
    }
}

/** Test compression of incompressible data.
    Expected expansion is 1/128, plus headers. */
AFL_TEST("util.RunLengthCompress:incompressible", a)
{
    uint8_t data[3000];
    for (size_t i = 0; i < sizeof(data); ++i) {
        data[i] = uint8_t(i);
    }
    verifyRoundTrip(a, data, sizeof(data) * 65/64);
}

/** Test compression of large data.
    At more than 64k input, the encoder is required to emit multiple chunks.
    Expected size is ~1200 bytes (~400x3 bytes covering up to 255 input bytes, plus headers). */
AFL_TEST("util.RunLengthCompress:large", a)
{
    uint8_t data[100000];
    Bytes_t(data).fill(0);
    verifyRoundTrip(a, data, 1300);
}
