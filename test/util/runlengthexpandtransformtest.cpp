/**
  *  \file test/util/runlengthexpandtransformtest.cpp
  *  \brief Test for util::RunLengthExpandTransform
  */

#include "util/runlengthexpandtransform.hpp"

#include "afl/base/growablememory.hpp"
#include "afl/base/memory.hpp"
#include "afl/string/string.hpp"
#include "afl/test/testrunner.hpp"

namespace {
    /** Test good compression. Tests an input segment against an output segment. */
    void testGood(afl::test::Assert a,
                  const afl::base::ConstBytes_t in,
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
            a.checkEqual("01. size", out.size(), result.size());
            a.checkEqualContent<uint8_t>("02. content", out, result);
            a.check("03. empty", inputReader.empty());
            AFL_CHECK_SUCCEEDS(a("04. flush"), testee.flush());
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
                    a.check("11. empty", thisInput.empty());
                } else {
                    // End reached
                    break;
                }
            }
            a.checkEqual("12. size", out.size(), result.size());
            a.checkEqualContent<uint8_t>("13. content", out, result);
            AFL_CHECK_SUCCEEDS(a("14. flush"), testee.flush());
        }
    }

    /** Test bad compression.
        Just tests that we can process the data, with no assumption about output. */
    void testBadData(afl::test::Assert a, afl::base::ConstBytes_t in)
    {
        uint8_t buffer[1000];
        afl::base::Bytes_t result(buffer);

        util::RunLengthExpandTransform testee;
        testee.transform(in, result);
        a.check("21. empty", in.empty());
        AFL_CHECK_SUCCEEDS(a("22. flush"), testee.flush());
    }
}

/*
 *  Some tests for well-formed compression.
 */
AFL_TEST("util.RunLengthExpandTransform:good:empty", a)
{
    static const uint8_t in[] = {0,0,0,0};
    testGood(a, in, afl::base::ConstBytes_t());
}

AFL_TEST("util.RunLengthExpandTransform:good:plain", a)
{
    static const uint8_t in[] = {8,0,0,0,8,0,3,'f','o','o'};
    static const uint8_t out[] = {'f','o','o'};
    testGood(a, in, out);
}

AFL_TEST("util.RunLengthExpandTransform:good:one-run", a)
{
    static const uint8_t in[] = {8,0,0,0,8,0,3,'f',3,5,'o','x'};
    static const uint8_t out[] = {'f','o','o','o','o','o','x'};
    testGood(a, in, out);
}

AFL_TEST("util.RunLengthExpandTransform:good:two-chunks", a)
{
    static const uint8_t in[] = {8,0,0,0,4,0,3,'f',3,3,'o',4,0,7,7,3,'o','x'};
    static const uint8_t out[] = {'f','o','o','o','o','o','o','x'};
    testGood(a, in, out);
}

/** Some test for bad compression.
    In PCC2, these throw an exception.
    c2ng does not do that; most errors are a truncated stream which a Transform cannot detect by design. */

AFL_TEST("util.RunLengthExpandTransform:bad:empty", a)
{
    testBadData(a, afl::base::ConstBytes_t());
}

AFL_TEST("util.RunLengthExpandTransform:bad:bad-size", a)
{
    // Truncated (and overly long) total size
    static const uint8_t in[] = {'x','x','x'};
    testBadData(a, in);
}

AFL_TEST("util.RunLengthExpandTransform:bad:truncated-size", a)
{
    // Truncated total size
    static const uint8_t in[] = {0,0,0};
    testBadData(a, in);
}

AFL_TEST("util.RunLengthExpandTransform:bad:excess-data", a)
{
    // Excess bytes after zero-length block
    static const uint8_t in[] = {0,0,0,0,0};
    testBadData(a, in);
}

AFL_TEST("util.RunLengthExpandTransform:bad:truncated-chunk-size", a)
{
    // Truncated chunk header (within size)
    static const uint8_t in[] = {1,0,0,0,0};
    testBadData(a, in);
}

AFL_TEST("util.RunLengthExpandTransform:bad:truncated-chunk-header", a)
{
    // Truncated chunk header
    static const uint8_t in[] = {8,0,0,0,8,0};
    testBadData(a, in);
}

AFL_TEST("util.RunLengthExpandTransform:bad:missing-data", a)
{
    // Missing compressed data
    static const uint8_t in[] = {8,0,0,0,8,0,3};
    testBadData(a, in);
}

AFL_TEST("util.RunLengthExpandTransform:bad:truncated-run", a)
{
    // Truncated run
    static const uint8_t in[] = {8,0,0,0,8,0,3,3};
    testBadData(a, in);
}

AFL_TEST("util.RunLengthExpandTransform:bad:truncated-run-2", a)
{
    // Truncated run
    static const uint8_t in[] = {8,0,0,0,8,0,3,3,3};
    testBadData(a, in);
}

AFL_TEST("util.RunLengthExpandTransform:bad:truncated-after-literal", a)
{
    // Truncated run after literal
    static const uint8_t in[] = {8,0,0,0,8,0,3,'x',3,3};
    testBadData(a, in);
}

AFL_TEST("util.RunLengthExpandTransform:bad:overlong-run", a)
{
    // Truncated run that exceeds its chunk
    static const uint8_t in[] = {8,0,0,0,8,0,3,'a','b','c','d','e','f',2,3};
    testBadData(a, in);
}

AFL_TEST("util.RunLengthExpandTransform:bad:overlong-run:2", a)
{
    // Truncated run that exceeds its chunk
    static const uint8_t in[] = {8,0,0,0,8,0,3,'a','b','c','d','e','f',2,3,3};
    testBadData(a, in);
}
