/**
  *  \file u/t_util_runlengthexpandtransform.cpp
  *  \brief Test for util::RunLengthExpandTransform
  */

#include "util/runlengthexpandtransform.hpp"

#include "t_util.hpp"
#include "afl/base/growablememory.hpp"
#include "afl/base/memory.hpp"
#include "afl/string/string.hpp"

namespace {
    /** Test good compression. Tests an input segment against an output segment. */
    void testGood(const afl::base::ConstBytes_t in,
                  const afl::base::ConstBytes_t out)
    {
        // Test regular transformation (large block in and out)
        {
            afl::base::GrowableMemory<uint8_t> buffer;
            afl::base::ConstBytes_t inputReader = in;
            buffer.resize(2*out.size());
            afl::base::Bytes_t result(buffer);

            util::RunLengthExpandTransform testee;
            testee.transform(inputReader, result);
            TS_ASSERT_EQUALS(out.size(), result.size());
            TS_ASSERT(out.equalContent(result));
            TS_ASSERT(inputReader.empty());
        }

        // Test byte-wise operation (maximum context switches)
        {
            afl::base::GrowableMemory<uint8_t> result;
            afl::base::ConstBytes_t inputReader = in;
            util::RunLengthExpandTransform testee;
            while (1) {
                // Attempt to get a byte out
                uint8_t byte[1];
                afl::base::ConstBytes_t thisInput;
                afl::base::Bytes_t thisOutput(byte);
                testee.transform(thisInput, thisOutput);
                if (!thisOutput.empty()) {
                    // Got a byte!
                    result.append(thisOutput);
                } else if (!inputReader.empty()) {
                    // Didn't get a byte, but have byte to send
                    thisInput = inputReader.split(1);
                    thisOutput = afl::base::Bytes_t();
                    testee.transform(thisInput, thisOutput);
                    TS_ASSERT(thisInput.empty());
                } else {
                    // End reached
                    break;
                }
            }
            TS_ASSERT_EQUALS(out.size(), result.size());
            TS_ASSERT(out.equalContent(result));
        }
    }

    /** Test bad compression.
        Just tests that we can process the data, with no assumption about output. */
    void testBadData(afl::base::ConstBytes_t in)
    {
        uint8_t buffer[1000];
        afl::base::Bytes_t result(buffer);

        util::RunLengthExpandTransform testee;
        testee.transform(in, result);
        TS_ASSERT(in.empty());
    }
}

/** Some tests for well-formed compression. */
void
TestUtilRunLengthExpandTransform::testIt()
{
    {
        static const uint8_t in[] = {0,0,0,0};
        testGood(in, afl::base::ConstBytes_t());
    }
    {
        static const uint8_t in[] = {8,0,0,0,8,0,3,'f','o','o'};
        static const uint8_t out[] = {'f','o','o'};
        testGood(in, out);
    }
    {
        static const uint8_t in[] = {8,0,0,0,8,0,3,'f',3,5,'o','x'};
        static const uint8_t out[] = {'f','o','o','o','o','o','x'};
        testGood(in, out);
    }
    {
        static const uint8_t in[] = {8,0,0,0,4,0,3,'f',3,3,'o',4,0,7,7,3,'o','x'};
        static const uint8_t out[] = {'f','o','o','o','o','o','o','x'};
        testGood(in, out);
    }
}

/** Some test for bad compression.
    In PCC2, these throw an exception.
    c2ng does not do that; most errors are a truncated stream which a Transform cannot detect by design. */
void
TestUtilRunLengthExpandTransform::testBad()
{
    {
        testBadData(afl::base::ConstBytes_t());
    }
    {
        // Truncated (and overly long) total size
        static const uint8_t in[] = {'x','x','x'};
        testBadData(in);
    }
    {
        // Truncated total size
        static const uint8_t in[] = {0,0,0};
        testBadData(in);
    }
    {
        // Excess bytes after zero-length block
        static const uint8_t in[] = {0,0,0,0,0};
        testBadData(in);
    }
    {
        // Truncated chunk header (within size)
        static const uint8_t in[] = {1,0,0,0,0};
        testBadData(in);
    }
    {
        // Truncated chunk header
        static const uint8_t in[] = {8,0,0,0,8,0};
        testBadData(in);
    }
    {
        // Missing compressed data
        static const uint8_t in[] = {8,0,0,0,8,0,3};
        testBadData(in);
    }
    {
        // Truncated run
        static const uint8_t in[] = {8,0,0,0,8,0,3,3};
        testBadData(in);
    }
    {
        // Truncated run
        static const uint8_t in[] = {8,0,0,0,8,0,3,3,3};
        testBadData(in);
    }
    {
        // Truncated run after literal
        static const uint8_t in[] = {8,0,0,0,8,0,3,'x',3,3};
        testBadData(in);
    }
    {
        // Truncated run that exceeds its chunk
        static const uint8_t in[] = {8,0,0,0,8,0,3,'a','b','c','d','e','f',2,3};
        testBadData(in);
    }
    {
        // Truncated run that exceeds its chunk
        static const uint8_t in[] = {8,0,0,0,8,0,3,'a','b','c','d','e','f',2,3,3};
        testBadData(in);
    }
}

