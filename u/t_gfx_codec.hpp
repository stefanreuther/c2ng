/**
  *  \file u/t_gfx_codec.hpp
  *  \brief Tests for gfx::codec
  */
#ifndef C2NG_U_T_GFX_CODEC_HPP
#define C2NG_U_T_GFX_CODEC_HPP

#include <cxxtest/TestSuite.h>

class TestGfxCodecApplication : public CxxTest::TestSuite {
 public:
    void testNoArgs();
    void testConvert();
    void testConvertFileNotFound();
    void testConvertBadSyntax();
    void testConvertToPlain4();
    void testConvertToPlain8();
    void testConvertToPacked4();
    void testConvertToPacked8();
    void testConvertTooFew();
    void testConvertTooMany();
    void testCreate();
    void testCreateTooFew();
    void testCreateSyntax();
    void testBadCommand();
    void testHelp();
};

class TestGfxCodecBMP : public CxxTest::TestSuite {
 public:
    void testSave();
    void testLoad8();
    void testLoad24();
    void testError();
};

class TestGfxCodecCodec : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGfxCodecCustom : public CxxTest::TestSuite {
 public:
    void testSave();
};

#endif
