/**
  *  \file gfx/fillpattern.cpp
  *  \brief Class gfx::FillPattern
  */

#include <algorithm>
#include <memory>
#include "gfx/fillpattern.hpp"
#include "afl/bits/rotate.hpp"
#include "afl/bits/bits.hpp"

namespace {
    const uint8_t SOLID_INIT[]      = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    const uint8_t GRAY50_INIT[]     = { 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55 };
    const uint8_t GRAY25_INIT[]     = { 0xAA, 0x00, 0x55, 0x00, 0xAA, 0x00, 0x55, 0x00 };
    const uint8_t LTSLASH_INIT[]    = { 0x88, 0x44, 0x22, 0x11, 0x88, 0x44, 0x22, 0x11 };
    const uint8_t GRAY50_ALT_INIT[] = { 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA };
}

const gfx::FillPattern gfx::FillPattern::SOLID(SOLID_INIT);
const gfx::FillPattern gfx::FillPattern::GRAY50(GRAY50_INIT);
const gfx::FillPattern gfx::FillPattern::GRAY25(GRAY25_INIT);
const gfx::FillPattern gfx::FillPattern::LTSLASH(LTSLASH_INIT);
const gfx::FillPattern gfx::FillPattern::GRAY50_ALT(GRAY50_ALT_INIT);

// Construct blank pattern.
gfx::FillPattern::FillPattern() throw()
{
    std::fill(m_pattern, m_pattern+SIZE, uint8_t(0));
}

// Construct pattern from byte array.
gfx::FillPattern::FillPattern(const uint8_t (&init)[SIZE]) throw()
{
    std::copy(init, init+SIZE, m_pattern);
}

// Construct pattern with specified value.
gfx::FillPattern::FillPattern(uint8_t value) throw()
{
    std::fill(m_pattern, m_pattern+SIZE, value);
}

// Check for blank pattern.
bool
gfx::FillPattern::isBlank() const throw()
{
    for (size_t i = 0; i < SIZE; ++i) {
        if (m_pattern[i] != 0) {
            return false;
        }
    }
    return true;
}

// Check for black pattern.
bool
gfx::FillPattern::isBlack() const throw()
{
    for (size_t i = 0; i < SIZE; ++i) {
        if (m_pattern[i] != 255) {
            return false;
        }
    }
    return true;
}

// Shift pattern to the left.
gfx::FillPattern&
gfx::FillPattern::shiftLeft(int amount) throw()
{
    amount &= (SIZE-1);
    for (size_t i = 0; i < SIZE; ++i) {
        m_pattern[i] = afl::bits::rotateLeft8(m_pattern[i], amount);
    }
    return *this;
}

// Shift pattern to the right.
gfx::FillPattern&
gfx::FillPattern::shiftRight(int amount) throw()
{
    return shiftLeft(-amount);
}

// Shift pattern up.
gfx::FillPattern&
gfx::FillPattern::shiftUp(int amount) throw()
{
    gfx::FillPattern tmp = *this;
    for (size_t i = 0; i < SIZE; ++i) {
        m_pattern[i] = tmp.m_pattern[(i + size_t(amount)) % SIZE];
    }
    return *this;
}

// Shift pattern down.
gfx::FillPattern&
gfx::FillPattern::shiftDown(int amount) throw()
{
    return shiftUp(-amount);
}

// "Or" two patterns.
gfx::FillPattern&
gfx::FillPattern::operator|=(const FillPattern& rhs) throw()
{
    for (size_t i = 0; i < SIZE; ++i) {
        m_pattern[i] |= rhs.m_pattern[i];
    }
    return *this;
}

// "Or" pattern with value.
gfx::FillPattern&
gfx::FillPattern::operator|=(uint8_t value) throw()
{
    for (size_t i = 0; i < SIZE; ++i) {
        m_pattern[i] |= value;
    }
    return *this;
}

// "And" two patterns.
gfx::FillPattern&
gfx::FillPattern::operator&=(const FillPattern& rhs) throw()
{
    for (size_t i = 0; i < SIZE; ++i) {
        m_pattern[i] &= rhs.m_pattern[i];
    }
    return *this;
}

// "And" pattern with value.
gfx::FillPattern&
gfx::FillPattern::operator&=(uint8_t value) throw()
{
    for (size_t i = 0; i < SIZE; ++i) {
        m_pattern[i] &= value;
    }
    return *this;
}

// "Xor" two patterns.
gfx::FillPattern&
gfx::FillPattern::operator^=(const FillPattern& rhs) throw()
{
    for (size_t i = 0; i < SIZE; ++i) {
        m_pattern[i] ^= rhs.m_pattern[i];
    }
    return *this;
}

// "Xor" pattern with value.
gfx::FillPattern&
gfx::FillPattern::operator^=(uint8_t value) throw()
{
    for (size_t i = 0; i < SIZE; ++i) {
        m_pattern[i] ^= value;
    }
    return *this;
}

// Invert pattern.
gfx::FillPattern&
gfx::FillPattern::invert() throw()
{
    for (size_t i = 0; i < SIZE; ++i) {
        m_pattern[i] = uint8_t(~m_pattern[i]);
    }
    return *this;
}

// Flip pattern horizontally.
gfx::FillPattern&
gfx::FillPattern::flipHorizontal() throw()
{
    for (size_t i = 0; i < SIZE; ++i) {
        m_pattern[i] = afl::bits::bitReverse8(m_pattern[i]);
    }
    return *this;
}

// Flip pattern vertically.
gfx::FillPattern&
gfx::FillPattern::flipVertical() throw()
{
    std::swap(m_pattern[0], m_pattern[7]);
    std::swap(m_pattern[1], m_pattern[6]);
    std::swap(m_pattern[2], m_pattern[5]);
    std::swap(m_pattern[3], m_pattern[4]);
    return *this;
}
