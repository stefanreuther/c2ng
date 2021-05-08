/**
  *  \file util/runlengthexpandtransform.cpp
  *  \brief Class util::RunLengthExpandTransform
  */

#include "util/runlengthexpandtransform.hpp"

util::RunLengthExpandTransform::RunLengthExpandTransform()
    : m_state(Read_TotalSize1),
      m_totalSize(0),
      m_chunkSize(0),
      m_chunkPrefix(0),
      m_byte(0),
      m_counter(0)
{ }

void
util::RunLengthExpandTransform::transform(afl::base::ConstBytes_t& in, afl::base::Bytes_t& out)
{
    // ex RleReaderStream::get, sort-of
    size_t outIndex = 0;
    while (1) {
        switch (m_state) {
         case Read_TotalSize1:
            if (const uint8_t* p = in.eat()) {
                m_totalSize = *p;
                m_state = Read_TotalSize2;
                break;
            } else {
                goto out;
            }
         case Read_TotalSize2:
            if (const uint8_t* p = in.eat()) {
                m_totalSize |= uint32_t(*p) << 8;
                m_state = Read_TotalSize3;
                break;
            } else {
                goto out;
            }
         case Read_TotalSize3:
            if (const uint8_t* p = in.eat()) {
                m_totalSize |= uint32_t(*p) << 16;
                m_state = Read_TotalSize4;
                break;
            } else {
                goto out;
            }

         case Read_TotalSize4:
            if (const uint8_t* p = in.eat()) {
                m_totalSize |= uint32_t(*p) << 24;
                m_state = Cond_TotalSize;
                break;
            } else {
                goto out;
            }

            // Loop header
         case Cond_TotalSize:
            if (m_totalSize == 0) {
                m_state = Final;
            } else {
                m_state = Read_ChunkSize1;
            }
            break;

            // Read chunk header
         case Read_ChunkSize1:
            if (const uint8_t* p = in.eat()) {
                m_chunkSize = *p;
                m_state = Read_ChunkSize2;
                break;
            } else {
                goto out;
            }
         case Read_ChunkSize2:
            if (const uint8_t* p = in.eat()) {
                m_chunkSize = static_cast<uint16_t>(m_chunkSize + (256 * *p));
                m_state = Read_ChunkPrefix;
                break;
            } else {
                goto out;
            }
         case Read_ChunkPrefix:
            if (const uint8_t* p = in.eat()) {
                m_chunkPrefix = *p;
                m_state = Cond_ChunkSize;
                break;
            } else {
                goto out;
            }

            // Loop header
         case Cond_ChunkSize:
            if (m_chunkSize == 0) {
                m_state = Cond_TotalSize;
            } else {
                m_state = Read_Byte;
            }
            break;

            // Decompression
         case Read_Byte:
            if (const uint8_t* p = in.eat()) {
                if (*p == m_chunkPrefix) {
                    m_state = Read_Counter;
                } else {
                    m_byte = *p;
                    m_counter = 1;
                    m_state = Store_Byte;
                }
                break;
            } else {
                goto out;
            }
         case Read_Counter:
            if (const uint8_t* p = in.eat()) {
                m_counter = *p;
                m_state = Read_Value;
                break;
            } else {
                goto out;
            }

         case Read_Value:
            if (const uint8_t* p = in.eat()) {
                m_byte = *p;
                m_state = (m_counter > 0 ? Store_Byte : Cond_ChunkSize);
                break;
            } else {
                goto out;
            }
         case Store_Byte:
            if (uint8_t* p = out.at(outIndex)) {
                *p = m_byte;
                if (m_chunkSize) {
                    --m_chunkSize;
                }
                if (m_totalSize) {
                    --m_totalSize;
                }
                ++outIndex;
                --m_counter;
                if (m_counter == 0) {
                    m_state = Cond_ChunkSize;
                }
                break;
            } else {
                goto out;
            }

         case Final:
            in.reset();
            goto out;
        }
    }
 out:
    out.trim(outIndex);
}

void
util::RunLengthExpandTransform::flush()
{ }
