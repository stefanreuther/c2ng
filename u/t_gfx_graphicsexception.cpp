/**
  *  \file u/t_gfx_graphicsexception.cpp
  *  \brief Test for gfx::GraphicsException
  */

#include "gfx/graphicsexception.hpp"

#include "t_gfx.hpp"

/** Simple test. */
void
TestGfxGraphicsException::testIt()
{
    gfx::GraphicsException a("lit");
    gfx::GraphicsException b(String_t("str"));
    gfx::GraphicsException c(a);
    std::exception& e = c;

    TS_ASSERT_EQUALS(a.what(), String_t("lit"));
    TS_ASSERT_EQUALS(b.what(), String_t("str"));
    TS_ASSERT_EQUALS(c.what(), String_t("lit"));
    TS_ASSERT_EQUALS(e.what(), String_t("lit"));
}

