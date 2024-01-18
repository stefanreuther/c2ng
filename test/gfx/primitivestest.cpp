/**
  *  \file test/gfx/primitivestest.cpp
  *  \brief Test for gfx::Primitives
  */

#include "gfx/primitives.hpp"

#include "afl/base/growablememory.hpp"
#include "afl/test/testrunner.hpp"
#include "gfx/types.hpp"
#include <cstring>

namespace {
    /*
     *  Traits implementation for test
     */
    class TraitsImpl {
     public:
        typedef uint8_t Pixel_t;
        typedef uint8_t Data_t;

        Data_t* get(int x, int y) const
            { return m_data.at(y*m_width + x); }
        static inline Pixel_t peek(Data_t* ptr)
            { return *ptr; }
        static inline void poke(Data_t* ptr, Pixel_t val)
            { *ptr = val; }
        Pixel_t mix(Pixel_t a, Pixel_t b, gfx::Alpha_t balpha) const
            { return gfx::mixColorComponent(a, b, balpha); }
        inline Data_t* add(Data_t* ptr, int dx, int dy) const
            { return ptr + m_width*dy + dx; }

        afl::base::Bytes_t data()
            { return m_data; }

        TraitsImpl(int w, int h)
            : m_data(),
              m_width(w)
            {
                m_data.resize(w*h);
                m_data.fill(0);
            }

     private:
        afl::base::GrowableMemory<Data_t> m_data;
        int m_width;
    };

    typedef gfx::Primitives<TraitsImpl> Primitives_t;
}

/*
 *  doHLine3
 */


// Horizontal, pattern, opaque
AFL_TEST("gfx.Primitives:doHLine:pattern-opaque", a)
{
    TraitsImpl impl(20, 3);
    Primitives_t(impl).doHLine(2, 1, 15, 7, 0xA3, gfx::OPAQUE_ALPHA);
    static const uint8_t EXPECTED[] = {
        0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
        0,0,7,0,0, 0,7,7,7,0, 7,0,0,0,7, 0,0,0,0,0,
        0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    };
    a.checkEqualContent<uint8_t>("", impl.data(), EXPECTED);
}

// Horizontal, solid, opaque
AFL_TEST("gfx.Primitives:doHLine:solid-opaque", a)
{
    TraitsImpl impl(20, 3);
    Primitives_t(impl).doHLine(2, 1, 15, 8, 0xFF, gfx::OPAQUE_ALPHA);
    static const uint8_t EXPECTED[] = {
        0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
        0,0,8,8,8, 8,8,8,8,8, 8,8,8,8,8, 0,0,0,0,0,
        0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    };
    a.checkEqualContent<uint8_t>("", impl.data(), EXPECTED);
}

// Horizontal, pattern, transparent
AFL_TEST("gfx.Primitives:doHLine:pattern-transparent", a)
{
    TraitsImpl impl(20, 3);
    Primitives_t(impl).doHLine(2, 1, 15, 10, 0xA3, 26);
    static const uint8_t EXPECTED[] = {
        0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
        0,0,1,0,0, 0,1,1,1,0, 1,0,0,0,1, 0,0,0,0,0,
        0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    };
    a.checkEqualContent<uint8_t>("", impl.data(), EXPECTED);
}

// Horizontal, solid, transparent
AFL_TEST("gfx.Primitives:doHLine:solid-transparent", a)
{
    TraitsImpl impl(20, 3);
    Primitives_t(impl).doHLine(2, 1, 15, 10, 0xFF, 26);
    static const uint8_t EXPECTED[] = {
        0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
        0,0,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 0,0,0,0,0,
        0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    };
    a.checkEqualContent<uint8_t>("", impl.data(), EXPECTED);
}

// Horizontal, solid, transparent over existing
AFL_TEST("gfx.Primitives:doHLine:solid-transparent-over-existing", a)
{
    TraitsImpl impl(20, 3);
    impl.data().fill(9);
    Primitives_t(impl).doHLine(2, 1, 15, 0, 0xFF, 40); // FIXME? Using 26 will not be visible. Rounding seems biased.
    static const uint8_t EXPECTED[] = {
        9,9,9,9,9, 9,9,9,9,9, 9,9,9,9,9, 9,9,9,9,9,
        9,9,8,8,8, 8,8,8,8,8, 8,8,8,8,8, 9,9,9,9,9,
        9,9,9,9,9, 9,9,9,9,9, 9,9,9,9,9, 9,9,9,9,9,
    };
    a.checkEqualContent<uint8_t>("", impl.data(), EXPECTED);
}

// Horizontal, solid/pattern, invisible
AFL_TEST("gfx.Primitives:doHLine:invisible", a)
{
    TraitsImpl impl(20, 3);
    Primitives_t(impl).doHLine(2, 1, 15, 10, 0xFF, 0);
    Primitives_t(impl).doHLine(2, 2, 15, 10, 0xA3, 0);
    static const uint8_t EXPECTED[] = {
        0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
        0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
        0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
    };
    a.checkEqualContent<uint8_t>("", impl.data(), EXPECTED);
}

/*
 *  doVLine
 */

// Vertical, pattern, opaque
AFL_TEST("gfx.Primitives:doVLine:pattern-opaque", a)
{
    TraitsImpl impl(3, 20);
    Primitives_t(impl).doVLine(1, 2, 17, 2, 0xF1, gfx::OPAQUE_ALPHA);
    static const uint8_t EXPECTED[] = {
        0,0,0,
        0,0,0,
        0,2,0,
        0,2,0,
        0,0,0,

        0,0,0,
        0,0,0,
        0,2,0,
        0,2,0,
        0,2,0,

        0,2,0,
        0,2,0,
        0,0,0,
        0,0,0,
        0,0,0,

        0,2,0,
        0,2,0,
        0,0,0,
        0,0,0,
        0,0,0,
    };
    a.checkEqualContent<uint8_t>("", impl.data(), EXPECTED);
}

// Vertical, solid, opaque
AFL_TEST("gfx.Primitives:solid-opaque", a)
{
    TraitsImpl impl(3, 20);
    Primitives_t(impl).doVLine(1, 2, 17, 2, 0xFF, gfx::OPAQUE_ALPHA);
    static const uint8_t EXPECTED[] = {
        0,0,0,
        0,0,0,
        0,2,0,
        0,2,0,
        0,2,0,

        0,2,0,
        0,2,0,
        0,2,0,
        0,2,0,
        0,2,0,

        0,2,0,
        0,2,0,
        0,2,0,
        0,2,0,
        0,2,0,

        0,2,0,
        0,2,0,
        0,0,0,
        0,0,0,
        0,0,0,
    };
    a.checkEqualContent<uint8_t>("", impl.data(), EXPECTED);
}

// Vertical, pattern, transparent
AFL_TEST("gfx.Primitives:pattern-transparent", a)
{
    TraitsImpl impl(3, 20);
    Primitives_t(impl).doVLine(1, 2, 17, 6, 0xF1, 85);
    static const uint8_t EXPECTED[] = {
        0,0,0,
        0,0,0,
        0,2,0,
        0,2,0,
        0,0,0,

        0,0,0,
        0,0,0,
        0,2,0,
        0,2,0,
        0,2,0,

        0,2,0,
        0,2,0,
        0,0,0,
        0,0,0,
        0,0,0,

        0,2,0,
        0,2,0,
        0,0,0,
        0,0,0,
        0,0,0,
    };
    a.checkEqualContent<uint8_t>("", impl.data(), EXPECTED);
}

// Vertical, solid, transparent
AFL_TEST("gfx.Primitives:solid-transparent", a)
{
    TraitsImpl impl(3, 20);
    Primitives_t(impl).doVLine(1, 2, 17, 15, 0xFF, 100);
    static const uint8_t EXPECTED[] = {
        0,0,0,
        0,0,0,
        0,5,0,
        0,5,0,
        0,5,0,

        0,5,0,
        0,5,0,
        0,5,0,
        0,5,0,
        0,5,0,

        0,5,0,
        0,5,0,
        0,5,0,
        0,5,0,
        0,5,0,

        0,5,0,
        0,5,0,
        0,0,0,
        0,0,0,
        0,0,0,
    };
    a.checkEqualContent<uint8_t>("", impl.data(), EXPECTED);
}

// Vertical, solid/transparent, invisible
AFL_TEST("gfx.Primitives:invisible", a)
{
    TraitsImpl impl(3, 10);
    Primitives_t(impl).doVLine(1, 2, 10, 15, 0xFF, 0);
    Primitives_t(impl).doVLine(2, 2, 10, 15, 0x1F, 0);
    static const uint8_t EXPECTED[] = {
        0,0,0,
        0,0,0,
        0,0,0,
        0,0,0,
        0,0,0,

        0,0,0,
        0,0,0,
        0,0,0,
        0,0,0,
        0,0,0,
    };
    a.checkEqualContent<uint8_t>("", impl.data(), EXPECTED);
}

/*
 *  doBar
 */

// Rock solid
AFL_TEST("gfx.Primitives:doBar:solid", a)
{
    TraitsImpl impl(10, 10);
    Primitives_t(impl).doBar(gfx::Rectangle(1, 1, 8, 8), 3, gfx::TRANSPARENT_COLOR, gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA);
    static const uint8_t EXPECTED[] = {
        0,0,0,0,0,0,0,0,0,0,
        0,3,3,3,3,3,3,3,3,0,
        0,3,3,3,3,3,3,3,3,0,
        0,3,3,3,3,3,3,3,3,0,
        0,3,3,3,3,3,3,3,3,0,
        0,3,3,3,3,3,3,3,3,0,
        0,3,3,3,3,3,3,3,3,0,
        0,3,3,3,3,3,3,3,3,0,
        0,3,3,3,3,3,3,3,3,0,
        0,0,0,0,0,0,0,0,0,0,
    };
    a.checkEqualContent<uint8_t>("", impl.data(), EXPECTED);
}

// Pattern
AFL_TEST("gfx.Primitives:doBar:pattern", a)
{
    TraitsImpl impl(10, 10);
    Primitives_t(impl).doBar(gfx::Rectangle(1, 1, 8, 8), 4, gfx::TRANSPARENT_COLOR, gfx::FillPattern::GRAY25, gfx::OPAQUE_ALPHA);
    static const uint8_t EXPECTED[] = {
        0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
        0,4,0,4,0,4,0,4,0,0,
        0,0,0,0,0,0,0,0,0,0,
        0,0,4,0,4,0,4,0,4,0,
        0,0,0,0,0,0,0,0,0,0,
        0,4,0,4,0,4,0,4,0,0,
        0,0,0,0,0,0,0,0,0,0,
        0,0,4,0,4,0,4,0,4,0,
        0,0,0,0,0,0,0,0,0,0,
    };
    a.checkEqualContent<uint8_t>("", impl.data(), EXPECTED);
}

// Pattern + color
AFL_TEST("gfx.Primitives:doBar:pattern-color", a)
{
    TraitsImpl impl(10, 10);
    Primitives_t(impl).doBar(gfx::Rectangle(1, 1, 8, 8), 4, 3, gfx::FillPattern::GRAY25, gfx::OPAQUE_ALPHA);
    static const uint8_t EXPECTED[] = {
        0,0,0,0,0,0,0,0,0,0,
        0,3,3,3,3,3,3,3,3,0,
        0,4,3,4,3,4,3,4,3,0,
        0,3,3,3,3,3,3,3,3,0,
        0,3,4,3,4,3,4,3,4,0,
        0,3,3,3,3,3,3,3,3,0,
        0,4,3,4,3,4,3,4,3,0,
        0,3,3,3,3,3,3,3,3,0,
        0,3,4,3,4,3,4,3,4,0,
        0,0,0,0,0,0,0,0,0,0,
    };
    a.checkEqualContent<uint8_t>("", impl.data(), EXPECTED);
}

// Pattern + color + alpha
AFL_TEST("gfx.Primitives:doBar:color-alpha", a)
{
    TraitsImpl impl(10, 10);
    impl.data().subrange(0, 30).fill(2);
    Primitives_t(impl).doBar(gfx::Rectangle(1, 1, 8, 8), 8, 6, gfx::FillPattern::GRAY25, 128);
    static const uint8_t EXPECTED[] = {
        2,2,2,2,2,2,2,2,2,2,
        2,4,4,4,4,4,4,4,4,2,
        2,5,4,5,4,5,4,5,4,2,
        0,3,3,3,3,3,3,3,3,0,
        0,3,4,3,4,3,4,3,4,0,
        0,3,3,3,3,3,3,3,3,0,
        0,4,3,4,3,4,3,4,3,0,
        0,3,3,3,3,3,3,3,3,0,
        0,3,4,3,4,3,4,3,4,0,
        0,0,0,0,0,0,0,0,0,0,
    };
    a.checkEqualContent<uint8_t>("", impl.data(), EXPECTED);
}

/*
 *  doBlitPattern
 */

namespace {
    static const uint8_t PATTERN[] = { 0xF3, 0x81, 0xF3 };
}

// Pattern + color
AFL_TEST("gfx.Primitives:doBlitPattern:pattern-color", a)
{
    TraitsImpl impl(10, 5);
    Primitives_t(impl).doBlitPattern(gfx::Rectangle(0, 0, 8, 3), gfx::Point(0, 0), 1, PATTERN, 5, gfx::TRANSPARENT_COLOR, gfx::OPAQUE_ALPHA);
    static const uint8_t EXPECTED[] = {
        5,5,5,5,0, 0,5,5,0,0,
        5,0,0,0,0, 0,0,5,0,0,
        5,5,5,5,0, 0,5,5,0,0,
        0,0,0,0,0, 0,0,0,0,0,
        0,0,0,0,0, 0,0,0,0,0,
    };
    a.checkEqualContent<uint8_t>("", impl.data(), EXPECTED);
}

// Part of pattern + color
AFL_TEST("gfx.Primitives:doBlitPattern:partial-pattern-color", a)
{
    TraitsImpl impl(10, 5);
    Primitives_t(impl).doBlitPattern(gfx::Rectangle(2, 0, 6, 3), gfx::Point(0, 0), 1, PATTERN, 5, gfx::TRANSPARENT_COLOR, gfx::OPAQUE_ALPHA);
    static const uint8_t EXPECTED[] = {
        0,0,5,5,0, 0,5,5,0,0,
        0,0,0,0,0, 0,0,5,0,0,
        0,0,5,5,0, 0,5,5,0,0,
        0,0,0,0,0, 0,0,0,0,0,
        0,0,0,0,0, 0,0,0,0,0,
    };
    a.checkEqualContent<uint8_t>("", impl.data(), EXPECTED);
}

// Pattern + color + background
AFL_TEST("gfx.Primitives:doBlitPattern:pattern-color-background", a)
{
    TraitsImpl impl(10, 5);
    Primitives_t(impl).doBlitPattern(gfx::Rectangle(0, 0, 8, 3), gfx::Point(0, 0), 1, PATTERN, 4, 2, gfx::OPAQUE_ALPHA);
    static const uint8_t EXPECTED[] = {
        4,4,4,4,2, 2,4,4,0,0,
        4,2,2,2,2, 2,2,4,0,0,
        4,4,4,4,2, 2,4,4,0,0,
        0,0,0,0,0, 0,0,0,0,0,
        0,0,0,0,0, 0,0,0,0,0,
    };
    a.checkEqualContent<uint8_t>("", impl.data(), EXPECTED);
}

// Pattern + color + alpha
AFL_TEST("gfx.Primitives:doBlitPattern:pattern-color-alpha", a)
{
    TraitsImpl impl(10, 5);
    Primitives_t(impl).doBlitPattern(gfx::Rectangle(0, 0, 8, 3), gfx::Point(0, 0), 1, PATTERN, 10, gfx::TRANSPARENT_COLOR, 26);
    static const uint8_t EXPECTED[] = {
        1,1,1,1,0, 0,1,1,0,0,
        1,0,0,0,0, 0,0,1,0,0,
        1,1,1,1,0, 0,1,1,0,0,
        0,0,0,0,0, 0,0,0,0,0,
        0,0,0,0,0, 0,0,0,0,0,
    };
    a.checkEqualContent<uint8_t>("", impl.data(), EXPECTED);
}

// Pattern + color + background + alpha
AFL_TEST("gfx.Primitives:doBlitPattern:pattern-color-alpha-background", a)
{
    TraitsImpl impl(10, 5);
    Primitives_t(impl).doBlitPattern(gfx::Rectangle(0, 0, 8, 3), gfx::Point(0, 0), 1, PATTERN, 12, 6, 85);
    static const uint8_t EXPECTED[] = {
        4,4,4,4,2, 2,4,4,0,0,
        4,2,2,2,2, 2,2,4,0,0,
        4,4,4,4,2, 2,4,4,0,0,
        0,0,0,0,0, 0,0,0,0,0,
        0,0,0,0,0, 0,0,0,0,0,
    };
    a.checkEqualContent<uint8_t>("", impl.data(), EXPECTED);
}

// Pattern + color + background + alpha
AFL_TEST("gfx.Primitives:doBlitPattern:pattern-color-alpha-background:offset", a)
{
    TraitsImpl impl(10, 5);
    Primitives_t(impl).doBlitPattern(gfx::Rectangle(2, 2, 6, 2), gfx::Point(1, 1), 1, PATTERN, 12, 6, 85);
    static const uint8_t EXPECTED[] = {
        0,0,0,0,0, 0,0,0,0,0,
        0,0,0,0,0, 0,0,0,0,0,
        0,0,2,2,2, 2,2,2,0,0,
        0,0,4,4,4, 2,2,4,0,0,
        0,0,0,0,0, 0,0,0,0,0,
    };
    a.checkEqualContent<uint8_t>("", impl.data(), EXPECTED);
}
