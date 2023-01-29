/**
  *  \file u/t_game_parser.hpp
  *  \brief Tests for game::parser
  */
#ifndef C2NG_U_T_GAME_PARSER_HPP
#define C2NG_U_T_GAME_PARSER_HPP

#include <cxxtest/TestSuite.h>

class TestGameParserBinaryTransfer : public CxxTest::TestSuite {
 public:
    void testPackBinaryMinefield();
    void testPackBinaryDrawing();
    void testPackBinaryDrawing2();
    void testPackBinaryDrawing3();
    void testPackBinaryDrawing4();
    void testPackBinaryPlanet();
    void testUnpackMinefield();
    void testUnpackPlanet();
    void testUnpackPlanet2();
    void testUnpackDrawing();
    void testUnpackDrawing2();
    void testUnpackDrawing3();
    void testUnpackDrawing4();
    void testDrawingColors();
    void testDrawingMarkerShapes();
    void testUnpackVPA1();
    void testUnpackVPA2();
    void testUnpackVPA3();
    void testUnpackStatistic();
    void testDecodeErrors();
};

class TestGameParserDataInterface : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameParserInformationConsumer : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGameParserMessageInformation : public CxxTest::TestSuite {
 public:
    void testIt();
    void testPlayerScore();
    void testConfiguration();
    void testGetValue();
};

class TestGameParserMessageParser : public CxxTest::TestSuite {
 public:
    void testHostVersion();
    void testConfig();
    void testObjects();
    void testMulti();
    void testScore();
    void testDelta();
    void testTimAllies();
    void testFailId();
    void testMarker();
};

class TestGameParserMessageTemplate : public CxxTest::TestSuite {
 public:
    void testValues();
    void testValuesX100();
    void testValuesEnum();
    void testValuesFormat();
    void testGetMessageHeaderInformation();
    void testSplitMessage();
    void testParseInteger();
    void testMatchMeta();
    void testMatchMetaSubId();
    void testMatchMetaBigId();
    void testMatchCheck();
    void testMatchParseValues();
    void testMatchArray();
    void testMatchArrayFixed();
};

class TestGameParserMessageValue : public CxxTest::TestSuite {
 public:
    void testValues();
    void testNames();
    void testKeywords();
};

#endif
