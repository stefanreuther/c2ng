/**
  *  \file u/t_gfx_primitives.cpp
  *  \brief Test for gfx::Primitives
  */

#include <cstring>
#include "gfx/primitives.hpp"

#include "t_gfx.hpp"
#include "gfx/types.hpp"
#include "afl/base/growablememory.hpp"

#define TS_ASSERT_SAME(got, expected) \
    TS_ASSERT_EQUALS(got.size(), sizeof(expected)); \
    TS_ASSERT_SAME_DATA(got.unsafeData(), expected, sizeof(expected))

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

/** Test doHLine(). */
void
TestGfxPrimitives::testHLine()
{
    // Horizontal, pattern, opaque
    {
        TraitsImpl impl(20, 3);
        Primitives_t(impl).doHLine(2, 1, 15, 7, 0xA3, gfx::OPAQUE_ALPHA);
        static const uint8_t EXPECTED[] = {
            0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
            0,0,7,0,0, 0,7,7,7,0, 7,0,0,0,7, 0,0,0,0,0,
            0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
        };
        TS_ASSERT_SAME(impl.data(), EXPECTED);
    }

    // Horizontal, solid, opaque
    {
        TraitsImpl impl(20, 3);
        Primitives_t(impl).doHLine(2, 1, 15, 8, 0xFF, gfx::OPAQUE_ALPHA);
        static const uint8_t EXPECTED[] = {
            0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
            0,0,8,8,8, 8,8,8,8,8, 8,8,8,8,8, 0,0,0,0,0,
            0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
        };
        TS_ASSERT_SAME(impl.data(), EXPECTED);
    }

    // Horizontal, pattern, transparent
    {
        TraitsImpl impl(20, 3);
        Primitives_t(impl).doHLine(2, 1, 15, 10, 0xA3, 26);
        static const uint8_t EXPECTED[] = {
            0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
            0,0,1,0,0, 0,1,1,1,0, 1,0,0,0,1, 0,0,0,0,0,
            0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
        };
        TS_ASSERT_SAME(impl.data(), EXPECTED);
    }

    // Horizontal, solid, transparent
    {
        TraitsImpl impl(20, 3);
        Primitives_t(impl).doHLine(2, 1, 15, 10, 0xFF, 26);
        static const uint8_t EXPECTED[] = {
            0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
            0,0,1,1,1, 1,1,1,1,1, 1,1,1,1,1, 0,0,0,0,0,
            0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
        };
        TS_ASSERT_SAME(impl.data(), EXPECTED);
    }

    // Horizontal, solid, transparent over existing
    {
        TraitsImpl impl(20, 3);
        impl.data().fill(9);
        Primitives_t(impl).doHLine(2, 1, 15, 0, 0xFF, 40); // FIXME? Using 26 will not be visible. Rounding seems biased.
        static const uint8_t EXPECTED[] = {
            9,9,9,9,9, 9,9,9,9,9, 9,9,9,9,9, 9,9,9,9,9,
            9,9,8,8,8, 8,8,8,8,8, 8,8,8,8,8, 9,9,9,9,9,
            9,9,9,9,9, 9,9,9,9,9, 9,9,9,9,9, 9,9,9,9,9,
        };
        TS_ASSERT_SAME(impl.data(), EXPECTED);
    }

    // Horizontal, solid/pattern, invisible
    {
        TraitsImpl impl(20, 3);
        Primitives_t(impl).doHLine(2, 1, 15, 10, 0xFF, 0);
        Primitives_t(impl).doHLine(2, 2, 15, 10, 0xA3, 0);
        static const uint8_t EXPECTED[] = {
            0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
            0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
            0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
        };
        TS_ASSERT_SAME(impl.data(), EXPECTED);
    }
}

/** Test doVLine(). */
void
TestGfxPrimitives::testVLine()
{
    // Vertical, pattern, opaque
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
        TS_ASSERT_SAME(impl.data(), EXPECTED);
    }

    // Vertical, solid, opaque
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
        TS_ASSERT_SAME(impl.data(), EXPECTED);
    }

    // Vertical, pattern, transparent
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
        TS_ASSERT_SAME(impl.data(), EXPECTED);
    }

    // Vertical, solid, transparent
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
        TS_ASSERT_SAME(impl.data(), EXPECTED);
    }

    // Vertical, solid/transparent, invisible
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
        TS_ASSERT_SAME(impl.data(), EXPECTED);
    }
}

/** Test doBar(). */
void
TestGfxPrimitives::testBar()
{
    // Rock solid
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
        TS_ASSERT_SAME(impl.data(), EXPECTED);
    }

    // Pattern
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
        TS_ASSERT_SAME(impl.data(), EXPECTED);
    }

    // Pattern + color
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
        TS_ASSERT_SAME(impl.data(), EXPECTED);
    }

    // Pattern + color + alpha
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
        TS_ASSERT_SAME(impl.data(), EXPECTED);
    }
}

/** Test doBlitPattern(). */
void
TestGfxPrimitives::testBlit()
{
    static const uint8_t PATTERN[] = { 0xF3, 0x81, 0xF3 };

    // Pattern + color
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
        TS_ASSERT_SAME(impl.data(), EXPECTED);
    }

    // Part of pattern + color
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
        TS_ASSERT_SAME(impl.data(), EXPECTED);
    }

    // Pattern + color + background
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
        TS_ASSERT_SAME(impl.data(), EXPECTED);
    }

    // Pattern + color + alpha
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
        TS_ASSERT_SAME(impl.data(), EXPECTED);
    }

    // Pattern + color + background + alpha
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
        TS_ASSERT_SAME(impl.data(), EXPECTED);
    }

    // Pattern + color + background + alpha
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
        TS_ASSERT_SAME(impl.data(), EXPECTED);
    }
}

