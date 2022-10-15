/**
  *  \file u/t_game_parser.hpp
  *  \brief Tests for game::parser
  */
#ifndef C2NG_U_T_GAME_PARSER_HPP
#define C2NG_U_T_GAME_PARSER_HPP

#include <cxxtest/TestSuite.h>

class TestGameParserDataInterface : public CxxTest::TestSuite {
 public:
    void testIt();
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
