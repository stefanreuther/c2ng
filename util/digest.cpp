/**
  *  \file util/digest.cpp
  *  \brief Class util::Digest
  *
  *  This checksum is used by PHost to compute hashes over
  *  specification files. Don't ask me about the theory behind it: it
  *  looks like being derived from CRC, but then it uses operations
  *  (arithmetic multiply/add) that do not appear in the Galois field
  *  that CRC works in.
  */

#include "util/digest.hpp"

util::Digest::Digest()
{
    init();
}

util::Digest::~Digest()
{ }

uint32_t
util::Digest::add(Memory_t data, uint32_t prev) const
{
    // ex game/checksum.h:getDigest
    while (const uint8_t* pValue = data.eat()) {
        prev += m_table[(prev ^ *pValue) & 255] + (prev >> 16);
    }
    return prev;
}

size_t
util::Digest::bits() const
{
    return 32;
}

util::Digest&
util::Digest::getDefaultInstance()
{
    static Digest instance;
    return instance;
}

uint32_t
util::Digest::addImpl(Memory_t data, uint32_t prev) const
{
    return add(data, prev);
}

size_t
util::Digest::bitsImpl() const
{
    return bits();
}

void
util::Digest::init()
{
    const uint32_t POLY = 0x10811;
    for (uint32_t b = 0; b < 256; ++b) {
        uint32_t x = b;
        m_table[b] = 0;
        for (int i = 0; i < 8; ++i) {
            if (x & 1) {
                x ^= POLY;
            }
            x >>= 1;
            m_table[b] += (x+1) * POLY;
        }
    }
}
