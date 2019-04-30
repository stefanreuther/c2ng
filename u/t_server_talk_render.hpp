/**
  *  \file u/t_server_talk_render.hpp
  *  \brief Tests for server::talk::render
  */
#ifndef C2NG_U_T_SERVER_TALK_RENDER_HPP
#define C2NG_U_T_SERVER_TALK_RENDER_HPP

#include <cxxtest/TestSuite.h>

class TestServerTalkRenderBBRenderer : public CxxTest::TestSuite {
 public:
    void testPlaintext();
    void testText();
    void testLink();
    void testSpecial();
};

class TestServerTalkRenderContext : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerTalkRenderHTMLRenderer : public CxxTest::TestSuite {
 public:
    void testPlaintext();
    void testText();
    void testLink();
    void testSpecial();
    void testUser();
};

class TestServerTalkRenderHtmlRenderer : public CxxTest::TestSuite {
 public:
    void testCode();
};

class TestServerTalkRenderOptions : public CxxTest::TestSuite {
 public:
    void testIt();
};

#endif
