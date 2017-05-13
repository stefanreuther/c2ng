/**
  *  \file gfx/fillpattern.cpp
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

// /** Construct blank pattern. Constructs a pattern that is all-zero. */
gfx::FillPattern::FillPattern() throw()
{
    std::fill(m_pattern, m_pattern+SIZE, uint8_t(0));
}

// /** Construct pattern from byte array.
//     \param init pointer to 8 bytes */
gfx::FillPattern::FillPattern(const uint8_t (&init)[SIZE]) throw()
{
    std::copy(init, init+SIZE, m_pattern);
}

// /** Construct pattern with specified value. Constructs a pattern that
//     contains the specified value in each line. */
gfx::FillPattern::FillPattern(uint8_t value) throw()
{
    std::fill(m_pattern, m_pattern+SIZE, value);
}

// /** Check for blank pattern. \returns true iff the pattern is all-zero,
//     i.e. when it is used for filling something, nothing would happen. */
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

// /** Check for black pattern. \returns true iff the pattern is all-one,
//     i.e. filling something with it would result in a completely filled
//     surface. */
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

// /** Shift pattern to the left. \returns reference to this pattern
//     to allow chaining. */
gfx::FillPattern&
gfx::FillPattern::shiftLeft(int amount) throw()
{
    amount &= (SIZE-1);
    for (size_t i = 0; i < SIZE; ++i) {
        m_pattern[i] = afl::bits::rotateLeft8(m_pattern[i], amount);
    }
    return *this;
}

// /** Shift pattern to the right. \returns reference to this pattern
//     to allow chaining. */
gfx::FillPattern&
gfx::FillPattern::shiftRight(int amount) throw()
{
    amount &= (SIZE-1);
    for (size_t i = 0; i < SIZE; ++i) {
        m_pattern[i] = afl::bits::rotateRight8(m_pattern[i], amount);
    }
    return *this;
}

// /** Shift pattern up. \returns reference to this pattern
//     to allow chaining. */
gfx::FillPattern&
gfx::FillPattern::shiftUp(int amount) throw()
{
    gfx::FillPattern tmp = *this;
    for (size_t i = 0; i < SIZE; ++i) {
        m_pattern[i] = tmp.m_pattern[(i + size_t(amount)) % SIZE];
    }
    return *this;
}

// /** Shift pattern down. \returns reference to this pattern
//     to allow chaining. */
gfx::FillPattern&
gfx::FillPattern::shiftDown(int amount) throw()
{
    FillPattern tmp = *this;
    for (size_t i = 0; i < SIZE; ++i) {
        m_pattern[i] = tmp.m_pattern[(i - size_t(amount)) % SIZE];
    }
    return *this;
}

// /** `Or' two patterns. The result has 1-bits where either of the two
//     patterns had 1-bits. \returns reference to this pattern to allow
//     chaining. */
gfx::FillPattern&
gfx::FillPattern::operator|=(const FillPattern& rhs) throw()
{
    for (size_t i = 0; i < SIZE; ++i) {
        m_pattern[i] |= rhs.m_pattern[i];
    }
    return *this;
}

// /** `Or' pattern with value. \overload */
gfx::FillPattern&
gfx::FillPattern::operator|=(uint8_t value) throw()
{
    for (size_t i = 0; i < SIZE; ++i) {
        m_pattern[i] |= value;
    }
    return *this;
}

// /** `And' two patterns. The result has 1-bits where both of the two
//     patterns had 1-bits. \returns reference to this pattern to allow
//     chaining. */
gfx::FillPattern&
gfx::FillPattern::operator&=(const FillPattern& rhs) throw()
{
    for (size_t i = 0; i < SIZE; ++i) {
        m_pattern[i] &= rhs.m_pattern[i];
    }
    return *this;
}

// /** `And' pattern and value. \overload */
gfx::FillPattern&
gfx::FillPattern::operator&=(uint8_t value) throw()
{
    for (size_t i = 0; i < SIZE; ++i) {
        m_pattern[i] &= value;
    }
    return *this;
}

// /** `Xor' two patterns. The result has 1-bits where either of the two
//     patterns (but not both) had 1-bits. \returns reference to this
//     pattern to allow chaining. */
gfx::FillPattern&
gfx::FillPattern::operator^=(const FillPattern& rhs) throw()
{
    for (size_t i = 0; i < SIZE; ++i) {
        m_pattern[i] ^= rhs.m_pattern[i];
    }
    return *this;
}

// /** `Xor' pattern and value. \overload */
gfx::FillPattern&
gfx::FillPattern::operator^=(uint8_t value) throw()
{
    for (size_t i = 0; i < SIZE; ++i) {
        m_pattern[i] ^= value;
    }
    return *this;
}

// /** Invert pattern. Turns black into white and vice versa.
//     \returns reference to this pattern to allow chaining */
gfx::FillPattern&
gfx::FillPattern::invert() throw()
{
    for (size_t i = 0; i < SIZE; ++i) {
        m_pattern[i] = uint8_t(~m_pattern[i]);
    }
    return *this;
}

// /** Flip pattern horizontally. Swaps left and right.
//     \returns reference to this pattern to allow chaining */
gfx::FillPattern&
gfx::FillPattern::flipHorizontal() throw()
{
    for (size_t i = 0; i < SIZE; ++i) {
        m_pattern[i] = afl::bits::bitReverse8(m_pattern[i]);
    }
    return *this;
}

// /** Flip pattern vertically. Swaps top and bottom.
//     \returns reference to this pattern to allow chaining */
gfx::FillPattern&
gfx::FillPattern::flipVertical() throw()
{
    std::swap(m_pattern[0], m_pattern[7]);
    std::swap(m_pattern[1], m_pattern[6]);
    std::swap(m_pattern[2], m_pattern[5]);
    std::swap(m_pattern[3], m_pattern[4]);
    return *this;
}
