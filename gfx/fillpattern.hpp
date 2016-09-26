/**
  *  \file gfx/fillpattern.hpp
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

        

        FillPattern() throw();
        FillPattern(const uint8_t (&init)[SIZE]) throw();
        FillPattern(uint8_t value) throw();

        bool isBlank() const throw();
        bool isBlack() const throw();
    
        FillPattern& shiftLeft(int amount) throw();
        FillPattern& shiftRight(int amount) throw();
        FillPattern& shiftUp(int amount) throw();
        FillPattern& shiftDown(int amount) throw();

        FillPattern& operator|=(const FillPattern& rhs) throw();
        FillPattern& operator|=(uint8_t value) throw();
        FillPattern& operator&=(const FillPattern& rhs) throw();
        FillPattern& operator&=(uint8_t value) throw();
        FillPattern& operator^=(const FillPattern& rhs) throw();
        FillPattern& operator^=(uint8_t value) throw();

        FillPattern& invert() throw();

        FillPattern& flipHorizontal() throw();
        FillPattern& flipVertical() throw();

        /** Read/write access to line i. */
        uint8_t& operator[](size_t i);

        /** Read access to line i. */
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
