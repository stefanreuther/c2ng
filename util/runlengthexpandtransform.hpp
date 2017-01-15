/**
  *  \file util/runlengthexpandtransform.hpp
  */
#ifndef C2NG_UTIL_RUNLENGTHEXPANDTRANSFORM_HPP
#define C2NG_UTIL_RUNLENGTHEXPANDTRANSFORM_HPP

#include "afl/io/transform.hpp"

namespace util {

    class RunLengthExpandTransform : public afl::io::Transform {
     public:
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

        RunLengthExpandTransform();

        virtual void transform(afl::base::ConstBytes_t& in, afl::base::Bytes_t& out);
        virtual void flush();

     private:
        State m_state;

        uint32_t m_totalSize;
        uint16_t m_chunkSize;
        uint8_t m_chunkPrefix;

        uint8_t m_byte;
        uint8_t m_counter;
    };

}

#endif
