/**
  *  \file gfx/fillpattern.hpp
  *  \brief Class gfx::FillPattern
  */
#ifndef C2NG_GFX_FILLPATTERN_HPP
#define C2NG_GFX_FILLPATTERN_HPP

#include "afl/base/types.hpp"

namespace gfx {

    /** Fill Pattern.
        An 8x8 pattern usable to fill a rectangle.
        Consists of 8 bytes (one per line, first byte is topmost),
        the most-significant bit of each byte is leftmost. */
    class FillPattern {
     public:
        static const size_t SIZE = 8;

        /*
         *  Predefined patterns
         */

        /** Solid fill pattern. */
        static const FillPattern SOLID;

        /** 50% gray fill pattern. */
        static const FillPattern GRAY50;

        /** 25% gray fill pattern. */
        static const FillPattern GRAY25;

        /** 50% gray fill pattern, alternative version.
            This is the inverse of GRAY50. */
        static const FillPattern GRAY50_ALT;

        /** Slashed fill pattern. */
        static const FillPattern LTSLASH;



        /** Construct blank pattern.
            Constructs a pattern that is all-zero. */
        FillPattern() throw();

        /** Construct pattern from byte array.
            \param init Byte array */
        FillPattern(const uint8_t (&init)[SIZE]) throw();

        /** Construct pattern with specified value.
            Constructs a pattern that contains the specified value in each line.
            \param value Value */
        FillPattern(uint8_t value) throw();

        /** Check for blank pattern.
            \return true iff the pattern is all-zero,
            i.e. when it is used for filling something, nothing would happen. */
        bool isBlank() const throw();

        /** Check for black pattern.
            \return true iff the pattern is all-one, i.e. filling something with it would result in a completely filled surface. */
        bool isBlack() const throw();

        /** Shift pattern to the left.
            \param amount Number of columns to shift
            \return *this */
        FillPattern& shiftLeft(int amount) throw();

        /** Shift pattern to the right.
            \param amount Number of columns to shift
            \return *this */
        FillPattern& shiftRight(int amount) throw();

        /** Shift pattern up.
            \param amount Number of lines to shift
            \return *this */
        FillPattern& shiftUp(int amount) throw();

        /** Shift pattern down.
            \param amount Number of lines to shift
            \return *this */
        FillPattern& shiftDown(int amount) throw();

        /** "Or" two patterns.
            The result has 1-bits where either of the two patterns had 1-bits.
            \param rhs Other pattern
            \return *this */
        FillPattern& operator|=(const FillPattern& rhs) throw();

        /** "Or" pattern with value.
            \param value Value
            \return *this */
        FillPattern& operator|=(uint8_t value) throw();

        /** "And" two patterns.
            The result has 1-bits where both patterns have 1-bits.
            \param rhs Other pattern
            \return *this */
        FillPattern& operator&=(const FillPattern& rhs) throw();

        /** "And" pattern with value.
            \param value Value
            \return *this */
        FillPattern& operator&=(uint8_t value) throw();

        /** "Xor" two patterns.
            The result has 1-bits where either of the two patterns (but not both) had 1-bits.
            \return *this */
        FillPattern& operator^=(const FillPattern& rhs) throw();

        /** "Xor" pattern with value.
            \param value Value
            \return *this */
        FillPattern& operator^=(uint8_t value) throw();

        /** Invert pattern.
            Turns black into white and vice versa.
            \return *this */
        FillPattern& invert() throw();

        /** Flip pattern horizontally.
            Swaps left and right.
            \return *this */
        FillPattern& flipHorizontal() throw();

        /** Flip pattern vertically.
            Swaps top and bottom.
            \return *this */
        FillPattern& flipVertical() throw();

        /** Read/write access to line i.
            \param i Index (automatic modulo)
            \return Element */
        uint8_t& operator[](size_t i);

        /** Read access to line i.
            \param i Index (automatic modulo)
            \return Element */
        uint8_t operator[](size_t i) const;

     private:
        uint8_t m_pattern[SIZE];
    };

}

inline uint8_t&
gfx::FillPattern::operator[](size_t i)
{
    return m_pattern[i % SIZE];
}

inline uint8_t
gfx::FillPattern::operator[](size_t i) const
{
    return m_pattern[i % SIZE];
}

#endif
