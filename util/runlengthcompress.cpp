/**
  *  \file util/runlengthcompress.cpp
  *  \brief Run-Length Encoding
  */

#include "util/runlengthcompress.hpp"
#include "afl/bits/uint16le.hpp"
#include "afl/bits/uint32le.hpp"

using afl::bits::UInt32LE;
using afl::bits::UInt16LE;

namespace {
    // Size of a chunk. This size is essentially arbitrary, but must fit in 16 bits.
    // PCC1 used 10k for compression, but can decode any size.
    const size_t CHUNK_SIZE = 20000;

    /* Find least-frequent character to use as escape character. */
    uint8_t findEscapeCharacter(afl::base::ConstBytes_t chunk)
    {
        // Count frequencies
        uint16_t counters[256];
        afl::base::Memory<uint16_t>(counters).fill(0);
        while (const uint8_t* p = chunk.eat()) {
            ++counters[*p];
        }

        // Find least frequent; default is 255
        uint8_t result = 255;
        for (size_t i = 0; i < 255; ++i) {
            if (counters[i] < counters[result]) {
                result = static_cast<uint8_t>(i);
            }
        }
        return result;
    }

    /* Compress a chunk using a given escape character.
       Just the plain compression, no framing. */
    void compressChunk(afl::base::GrowableBytes_t& out, afl::base::ConstBytes_t in, uint8_t escape)
    {
        while (const uint8_t* ele = in.eat()) {
            // ex replaceb.pas:EndRun, sort-of
            const uint8_t byte = *ele;
            size_t n = in.findNot(byte);
            in.split(n);
            ++n;
            while (n > 0) {
                if (n > 255) {
                    // Big repetition
                    out.append(escape);
                    out.append(255);
                    out.append(byte);
                    n -= 255;
                } else if (n > 2 || byte == escape) {
                    // Repetition, or must be escaped
                    out.append(escape);
                    out.append(static_cast<uint8_t>(n));
                    out.append(byte);
                    n = 0;
                } else {
                    // Single character
                    out.append(byte);
                    --n;
                }
            }
        }
    }
}

void
util::encodeRLE(afl::base::GrowableBytes_t& out, afl::base::ConstBytes_t in)
{
    // Total size
    UInt32LE::Bytes_t totalSize;
    UInt32LE::pack(totalSize, static_cast<uint32_t>(in.size()));
    out.append(totalSize);

    // Chunks
    while (1) {
        // Chunk size (or terminator)
        afl::base::ConstBytes_t chunk = in.split(CHUNK_SIZE);
        UInt16LE::Bytes_t chunkSize;
        UInt16LE::pack(chunkSize, static_cast<uint16_t>(chunk.size()));
        out.append(chunkSize);
        if (chunk.empty()) {
            break;
        }

        // Compress this chunk
        uint8_t escape = findEscapeCharacter(chunk);
        out.append(escape);
        compressChunk(out, chunk, escape);
    }
}
