/**
  *  \file test/gfx/graphicsexceptiontest.cpp
  *  \brief Test for gfx::GraphicsException
  */

#include "gfx/graphicsexception.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("gfx.GraphicsException", a)
{
    gfx::GraphicsException aa("lit");
    gfx::GraphicsException bb(String_t("str"));
    gfx::GraphicsException cc(aa);
    std::exception& ee = cc;

    a.checkEqual("01. what", aa.what(), String_t("lit"));
    a.checkEqual("02. what", bb.what(), String_t("str"));
    a.checkEqual("03. what", cc.what(), String_t("lit"));
    a.checkEqual("04. what", ee.what(), String_t("lit"));
}
