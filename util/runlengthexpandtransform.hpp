/**
  *  \file util/runlengthexpandtransform.hpp
  *  \brief Class util::RunLengthExpandTransform
  */
#ifndef C2NG_UTIL_RUNLENGTHEXPANDTRANSFORM_HPP
#define C2NG_UTIL_RUNLENGTHEXPANDTRANSFORM_HPP

#include "afl/io/transform.hpp"

namespace util {

    /** Transform implementation for run-length encoded images.

        This expansion scheme is used for PCC 1.x resource files and has thus survived into PCC2.
        This implements the expander as a rather dull state machine with no effort on performance.
        The files we decode are a few kilobytes each only.
        In PCC2, this was a Stream descendant; we implement a Transform instead.

        We encode bitmap data and other stuff using a simple RLE variant.

        Each file has the following format:
        - one dword total size (uncompressed)
        - sequence of compressed chunks
        - zero-length chunk (one word of value zero)

        Each chunk has the following format:
        - word with chunk size (uncompressed)
        - byte with prefix code for this chunk (chosen dynamically for each chunk)
        - compressed data. Either a byte to be copied verbatim, or a (prefix,counter,value) triples */
    class RunLengthExpandTransform : public afl::io::Transform {
     public:
        /** Constructor. */
        RunLengthExpandTransform();

        // Transform:
        virtual void transform(afl::base::ConstBytes_t& in, afl::base::Bytes_t& out);
        virtual void flush();

     private:
        enum State {
            // Read 4 bytes of total size
            Read_TotalSize1,
            Read_TotalSize2,
            Read_TotalSize3,
            Read_TotalSize4,

            // Loop header
            Cond_TotalSize,

            // Read chunk header
            Read_ChunkSize1,
            Read_ChunkSize2,
            Read_ChunkPrefix,

            // Loop header
            Cond_ChunkSize,

            // Decompression
            Read_Byte,
            Read_Counter,
            Read_Value,
            Store_Byte,

            Final
        };

        State m_state;

        uint32_t m_totalSize;
        uint16_t m_chunkSize;
        uint8_t m_chunkPrefix;

        uint8_t m_byte;
        uint8_t m_counter;
    };

}

#endif
