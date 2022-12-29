/**
  *  \file u/t_gfx.hpp
  *  \brief Tests for gfx
  */
#ifndef C2NG_U_T_GFX_HPP
#define C2NG_U_T_GFX_HPP

#include <cxxtest/TestSuite.h>

class TestGfxAntialiased : public CxxTest::TestSuite {
 public:
    void testLine();
    void testCircle();
};

class TestGfxApplication : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGfxBaseColorScheme : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGfxBaseContext : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGfxBitmapFont : public CxxTest::TestSuite {
 public:
    void testFile();
    void testAdd();
    void testFileErrors();
};

class TestGfxBitmapGlyph : public CxxTest::TestSuite {
 public:
    void testIt();
    void testConstruct();
};

class TestGfxCanvas : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGfxColorScheme : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGfxColorTransform : public CxxTest::TestSuite {
 public:
    void testPalette();
    void testRGBA();
};

class TestGfxComplex : public CxxTest::TestSuite {
 public:
    void testFillPolyTriangle1();
    void testFillPolyTriangle2();
    void testFillPolyTrianglePattern();
    void testFillPolyPentagram();
    void testFillPolyPolygon();
    void testFillPolySquare();
    void testFillPolyRhombe();
};

class TestGfxContext : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGfxDefaultFont : public CxxTest::TestSuite {
 public:
    void testMetrics();
    void testDrawing();
};

class TestGfxEngine : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGfxEventConsumer : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGfxFillPattern : public CxxTest::TestSuite {
 public:
    void testInit();
    void testOperators();
    void testPredefined();
};

class TestGfxFilter : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGfxFont : public CxxTest::TestSuite {
 public:
    void testIt();
    void testFitWidth();
    void testFitArea();
};

class TestGfxFontList : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testUnit();
    void testList();
};

class TestGfxFontRequest : public CxxTest::TestSuite {
 public:
    void testSet();
    void testMatch();
    void testCompare();
    void testParse();
};

class TestGfxGraphicsException : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGfxKeyEventConsumer : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGfxNullCanvas : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGfxNullColorScheme : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGfxNullEngine : public CxxTest::TestSuite {
 public:
    void testTimers();
    void testEvents();
};

class TestGfxNullResourceProvider : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGfxPalettizedPixmap : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGfxPixmapCanvasImpl : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGfxPoint : public CxxTest::TestSuite {
 public:
    void testIt();
    void testExtend();
};

class TestGfxPrimitives : public CxxTest::TestSuite {
 public:
    void testHLine();
    void testVLine();
    void testBar();
    void testBlit();
};

class TestGfxRGBAPixmap : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGfxRectangle : public CxxTest::TestSuite {
 public:
    void testIt();
    void testModify();
    void testAlign();
    void testSplit();
    void testFormat();
    void testSplit2();
    void testInclude();
};

class TestGfxResourceProvider : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGfxSave : public CxxTest::TestSuite {
 public:
    void testUnaligned();
};

class TestGfxScan : public CxxTest::TestSuite {
 public:
    void testScanEmpty();
    void testScanSmall();
    void testScanLarge();
    void testScanHuge();
};

class TestGfxTimer : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGfxTimerQueue : public CxxTest::TestSuite {
 public:
    void test1();
    void test2();
    void test3();
};

class TestGfxTypes : public CxxTest::TestSuite {
 public:
    void testMixColorComponent();
    void testMixColor();
    void testAddColor();
    void testGetColorDistance();
};

class TestGfxtypes : public CxxTest::TestSuite {
 public:
    void testParseColor();
};

#endif
