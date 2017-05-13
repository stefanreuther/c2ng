/**
  *  \file u/t_server_talk_parse.hpp
  *  \brief Tests for server::talk::parse
  */
#ifndef C2NG_U_T_SERVER_TALK_PARSE_HPP
#define C2NG_U_T_SERVER_TALK_PARSE_HPP

#include <cxxtest/TestSuite.h>

class TestServerTalkParseBBLexer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testPara();
    void testTags();
    void testAtLink();
    void testPartials();
};

class TestServerTalkParseBBParser : public CxxTest::TestSuite {
 public:
    void testIt();
    void testRecog();
};

#endif
