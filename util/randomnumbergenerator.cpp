/**
  *  \file util/randomnumbergenerator.cpp
  *  \brief Class util::RandomNumberGenerator
  */

#include "util/randomnumbergenerator.hpp"

util::RandomNumberGenerator::RandomNumberGenerator(uint32_t seed)
    : m_seed(seed)
{ }

// Get random number in range [0, 2^16).
uint16_t
util::RandomNumberGenerator::operator()()
{
    advance();
    return uint16_t(m_seed >> 16);
}

// Get random number in range [0, max).
uint16_t
util::RandomNumberGenerator::operator()(uint16_t max)
{
    advance();

    /* seed * max >> 32
        = (loword(seed) * max + hiword(seed) * max << 16) >> 32
          hence loword(result) = loword(seed)*max which is discarded anyways
        = (loword(seed) * max >> 16 + hiword(seed) * max) >> 16 */
    return uint16_t((((m_seed & 0xFFFFU) * max >> 16) + (m_seed >> 16) * max) >> 16);
}

void
util::RandomNumberGenerator::setSeed(uint32_t seed)
{
    m_seed = seed;
}

uint32_t
util::RandomNumberGenerator::getSeed() const
{
    return m_seed;
}

// Advance seed.
void
util::RandomNumberGenerator::advance()
{
    m_seed = uint32_t(134775813UL * m_seed + 1);
}
