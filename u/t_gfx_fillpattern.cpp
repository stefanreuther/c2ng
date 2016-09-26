/**
  *  \file u/t_gfx_fillpattern.cpp
  *  \brief Test for gfx::FillPattern
  */

#include "gfx/fillpattern.hpp"

#include "t_gfx.hpp"

/** Test constructors, isBlank, isBlack. */
void
TestGfxFillPattern::testInit()
{
    {
        gfx::FillPattern a;
        TS_ASSERT(a.isBlank());
        TS_ASSERT(!a.isBlack());
    }
    {
        gfx::FillPattern a(uint8_t(0));
        TS_ASSERT(a.isBlank());
        TS_ASSERT(!a.isBlack());
    }
    {
        gfx::FillPattern a(1);
        TS_ASSERT(!a.isBlank());
        TS_ASSERT(!a.isBlack());
    }
    {
        gfx::FillPattern a(0xFF);
        TS_ASSERT(!a.isBlank());
        TS_ASSERT(a.isBlack());
    }
    {
        static const uint8_t black[] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
        gfx::FillPattern a(black);
        TS_ASSERT(a.isBlack());
        TS_ASSERT(!a.isBlank());
    }
    {
        static const uint8_t notQuiteBlack[] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00};
        gfx::FillPattern a(notQuiteBlack);
        TS_ASSERT(!a.isBlack());
        TS_ASSERT(!a.isBlank());
    }
}

/** Test operators. */
void
TestGfxFillPattern::testOperators()
{
    static const uint8_t pattern[] = {0x01,0,0,0,0,0,0x30,0};
    gfx::FillPattern a(pattern);
    const gfx::FillPattern& b(a);
    TS_ASSERT(!a.isBlank());
    TS_ASSERT(!a.isBlack());
    TS_ASSERT_EQUALS(a[0], 1);
    TS_ASSERT_EQUALS(a[1], 0);
    TS_ASSERT_EQUALS(a[2], 0);
    TS_ASSERT_EQUALS(a[3], 0);
    TS_ASSERT_EQUALS(a[4], 0);
    TS_ASSERT_EQUALS(a[5], 0);
    TS_ASSERT_EQUALS(a[6], 0x30);
    TS_ASSERT_EQUALS(a[7], 0);

    TS_ASSERT_EQUALS(b[0], 1);
    TS_ASSERT_EQUALS(b[1], 0);
    TS_ASSERT_EQUALS(b[2], 0);
    TS_ASSERT_EQUALS(b[3], 0);
    TS_ASSERT_EQUALS(b[4], 0);
    TS_ASSERT_EQUALS(b[5], 0);
    TS_ASSERT_EQUALS(b[6], 0x30);
    TS_ASSERT_EQUALS(b[7], 0);

    a.shiftLeft(2);
    TS_ASSERT_EQUALS(a[0], 4);
    TS_ASSERT_EQUALS(a[1], 0);
    TS_ASSERT_EQUALS(a[2], 0);
    TS_ASSERT_EQUALS(a[3], 0);
    TS_ASSERT_EQUALS(a[4], 0);
    TS_ASSERT_EQUALS(a[5], 0);
    TS_ASSERT_EQUALS(a[6], 0xC0);
    TS_ASSERT_EQUALS(a[7], 0);

    a.shiftUp(3);
    TS_ASSERT_EQUALS(a[0], 0);
    TS_ASSERT_EQUALS(a[1], 0);
    TS_ASSERT_EQUALS(a[2], 0);
    TS_ASSERT_EQUALS(a[3], 0xC0);
    TS_ASSERT_EQUALS(a[4], 0);
    TS_ASSERT_EQUALS(a[5], 4);
    TS_ASSERT_EQUALS(a[6], 0);
    TS_ASSERT_EQUALS(a[7], 0);

    a.shiftRight(5);
    TS_ASSERT_EQUALS(a[0], 0);
    TS_ASSERT_EQUALS(a[1], 0);
    TS_ASSERT_EQUALS(a[2], 0);
    TS_ASSERT_EQUALS(a[3], 6);
    TS_ASSERT_EQUALS(a[4], 0);
    TS_ASSERT_EQUALS(a[5], 0x20);
    TS_ASSERT_EQUALS(a[6], 0);
    TS_ASSERT_EQUALS(a[7], 0);

    a.shiftDown(1);
    TS_ASSERT_EQUALS(a[0], 0);
    TS_ASSERT_EQUALS(a[1], 0);
    TS_ASSERT_EQUALS(a[2], 0);
    TS_ASSERT_EQUALS(a[3], 0);
    TS_ASSERT_EQUALS(a[4], 6);
    TS_ASSERT_EQUALS(a[5], 0);
    TS_ASSERT_EQUALS(a[6], 0x20);
    TS_ASSERT_EQUALS(a[7], 0);

    a.flipVertical();
    TS_ASSERT_EQUALS(a[0], 0);
    TS_ASSERT_EQUALS(a[1], 0x20);
    TS_ASSERT_EQUALS(a[2], 0);
    TS_ASSERT_EQUALS(a[3], 6);
    TS_ASSERT_EQUALS(a[4], 0);
    TS_ASSERT_EQUALS(a[5], 0);
    TS_ASSERT_EQUALS(a[6], 0);
    TS_ASSERT_EQUALS(a[7], 0);

    a.flipHorizontal();
    TS_ASSERT_EQUALS(a[0], 0);
    TS_ASSERT_EQUALS(a[1], 4);
    TS_ASSERT_EQUALS(a[2], 0);
    TS_ASSERT_EQUALS(a[3], 0x60);
    TS_ASSERT_EQUALS(a[4], 0);
    TS_ASSERT_EQUALS(a[5], 0);
    TS_ASSERT_EQUALS(a[6], 0);
    TS_ASSERT_EQUALS(a[7], 0);

    a.invert();
    TS_ASSERT_EQUALS(a[0], 0xFF);
    TS_ASSERT_EQUALS(a[1], 0xFB);
    TS_ASSERT_EQUALS(a[2], 0xFF);
    TS_ASSERT_EQUALS(a[3], 0x9F);
    TS_ASSERT_EQUALS(a[4], 0xFF);
    TS_ASSERT_EQUALS(a[5], 0xFF);
    TS_ASSERT_EQUALS(a[6], 0xFF);
    TS_ASSERT_EQUALS(a[7], 0xFF);

    a ^= 0xF0;
    TS_ASSERT_EQUALS(a[0], 0x0F);
    TS_ASSERT_EQUALS(a[1], 0x0B);
    TS_ASSERT_EQUALS(a[2], 0x0F);
    TS_ASSERT_EQUALS(a[3], 0x6F);
    TS_ASSERT_EQUALS(a[4], 0x0F);
    TS_ASSERT_EQUALS(a[5], 0x0F);
    TS_ASSERT_EQUALS(a[6], 0x0F);
    TS_ASSERT_EQUALS(a[7], 0x0F);

    static const uint8_t toggle[] = {0x08,0x1B,0x18,0x0B,0x08,0x1B,0x18,0x0B};
    a ^= gfx::FillPattern(toggle);
    TS_ASSERT_EQUALS(a[0], 0x07);
    TS_ASSERT_EQUALS(a[1], 0x10);
    TS_ASSERT_EQUALS(a[2], 0x17);
    TS_ASSERT_EQUALS(a[3], 0x64);
    TS_ASSERT_EQUALS(a[4], 0x07);
    TS_ASSERT_EQUALS(a[5], 0x14);
    TS_ASSERT_EQUALS(a[6], 0x17);
    TS_ASSERT_EQUALS(a[7], 0x04);

    a |= 0x40;
    TS_ASSERT_EQUALS(a[0], 0x47);
    TS_ASSERT_EQUALS(a[1], 0x50);
    TS_ASSERT_EQUALS(a[2], 0x57);
    TS_ASSERT_EQUALS(a[3], 0x64);
    TS_ASSERT_EQUALS(a[4], 0x47);
    TS_ASSERT_EQUALS(a[5], 0x54);
    TS_ASSERT_EQUALS(a[6], 0x57);
    TS_ASSERT_EQUALS(a[7], 0x44);

    static const uint8_t bits[] = {0x08,0x02,0x02,0x08,0x08,0x02,0x02,0x08};
    a |= gfx::FillPattern(bits);
    TS_ASSERT_EQUALS(a[0], 0x4F);
    TS_ASSERT_EQUALS(a[1], 0x52);
    TS_ASSERT_EQUALS(a[2], 0x57);
    TS_ASSERT_EQUALS(a[3], 0x6C);
    TS_ASSERT_EQUALS(a[4], 0x4F);
    TS_ASSERT_EQUALS(a[5], 0x56);
    TS_ASSERT_EQUALS(a[6], 0x57);
    TS_ASSERT_EQUALS(a[7], 0x4C);

    a &= 0x11;
    TS_ASSERT_EQUALS(a[0], 0x01);
    TS_ASSERT_EQUALS(a[1], 0x10);
    TS_ASSERT_EQUALS(a[2], 0x11);
    TS_ASSERT_EQUALS(a[3], 0x00);
    TS_ASSERT_EQUALS(a[4], 0x01);
    TS_ASSERT_EQUALS(a[5], 0x10);
    TS_ASSERT_EQUALS(a[6], 0x11);
    TS_ASSERT_EQUALS(a[7], 0x00);

    static const uint8_t mask[] = {0xFF,0xF0,0x0F,0xFF,0xF0,0x0F,0xFF,0xF0};
    a &= gfx::FillPattern(mask);
    TS_ASSERT_EQUALS(a[0], 0x01);
    TS_ASSERT_EQUALS(a[1], 0x10);
    TS_ASSERT_EQUALS(a[2], 0x01);
    TS_ASSERT_EQUALS(a[3], 0x00);
    TS_ASSERT_EQUALS(a[4], 0x00);
    TS_ASSERT_EQUALS(a[5], 0x00);
    TS_ASSERT_EQUALS(a[6], 0x11);
    TS_ASSERT_EQUALS(a[7], 0x00);
}

/** Test predefined patterns. */
void
TestGfxFillPattern::testPredefined()
{
    TS_ASSERT(gfx::FillPattern::SOLID.isBlack());
    TS_ASSERT(!gfx::FillPattern::SOLID.isBlank());

    TS_ASSERT(!gfx::FillPattern::GRAY50.isBlack());
    TS_ASSERT(!gfx::FillPattern::GRAY50.isBlank());

    TS_ASSERT(!gfx::FillPattern::GRAY25.isBlack());
    TS_ASSERT(!gfx::FillPattern::GRAY25.isBlank());

    TS_ASSERT(!gfx::FillPattern::GRAY50_ALT.isBlack());
    TS_ASSERT(!gfx::FillPattern::GRAY50_ALT.isBlank());

    TS_ASSERT(!gfx::FillPattern::LTSLASH.isBlack());
    TS_ASSERT(!gfx::FillPattern::LTSLASH.isBlank());
}

