/**
  *  \file u/t_util_runlengthcompress.cpp
  *  \brief Test for util::RunLengthCompress
  */

#include <vector>
#include "util/runlengthcompress.hpp"

#include "t_util.hpp"
#include "afl/base/memory.hpp"
#include "util/runlengthexpandtransform.hpp"

using afl::base::Bytes_t;
using afl::base::ConstBytes_t;
using afl::base::GrowableBytes_t;

namespace {
    void verifyRoundTrip(ConstBytes_t data, size_t maxSize)
    {
        // Encode
        GrowableBytes_t packedData;
        util::encodeRLE(packedData, data);

        // Verify size limit
        TS_ASSERT_LESS_THAN(packedData.size(), maxSize);

        // Verify correct decompression
        std::vector<uint8_t> unpackedData;
        unpackedData.resize(data.size() + 10);

        ConstBytes_t in(packedData);
        Bytes_t out(unpackedData);
        util::RunLengthExpandTransform().transform(in, out);

        // Must have processed all compressed input
        TS_ASSERT(in.empty());

        // Must have produced exact uncompressed input data
        TS_ASSERT_EQUALS(out.size(), data.size());
        if (data.size() > 0) {
            TS_ASSERT(out.equalContent(data));
            TS_ASSERT_SAME_DATA(out.unsafeData(), data.unsafeData(), (unsigned int) data.size());
        }
    }
}

/** Test compression of empty block.
    Expected size is 6 bytes (total + chunk size). */
void
TestUtilRunLengthCompress::testEmpty()
{
    ConstBytes_t data;
    verifyRoundTrip(data, 20);
}

/** Test compression of compressible data.
    Expected size is ~42 bytes (12x3 bytes covering up to 255 input bytes, plus headers). */
void
TestUtilRunLengthCompress::testCompressible()
{
    for (int i = 0; i < 256; ++i) {
        uint8_t data[3000];
        Bytes_t(data).fill(uint8_t(i));
        verifyRoundTrip(data, 50);
    }
}

/** Test compression of incompressible data.
    Expected expansion is 1/128, plus headers. */
void
TestUtilRunLengthCompress::testIncompressible()
{
    uint8_t data[3000];
    for (size_t i = 0; i < sizeof(data); ++i) {
        data[i] = uint8_t(i);
    }
    verifyRoundTrip(data, sizeof(data) * 65/64);
}

/** Test compression of large data.
    At more than 64k input, the encoder is required to emit multiple chunks.
    Expected size is ~1200 bytes (~400x3 bytes covering up to 255 input bytes, plus headers). */
void
TestUtilRunLengthCompress::testLarge()
{
    uint8_t data[100000];
    Bytes_t(data).fill(0);
    verifyRoundTrip(data, 1300);
}

