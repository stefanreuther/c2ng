/**
  *  \file test/game/config/stringoptiontest.cpp
  *  \brief Test for game::config::StringOption
  */

#include "game/config/stringoption.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("game.config.StringOption", a)
{
    game::config::StringOption opt;
    a.checkEqual("01. value", opt(), "");
    a.checkEqual("02. toString", opt.toString(), "");

    opt.set("hi");
    a.checkEqual("11. value", opt(), "hi");
    a.checkEqual("12. toString", opt.toString(), "hi");

    opt.set(String_t("ho"));
    a.checkEqual("21. value", opt(), "ho");
    a.checkEqual("22. toString", opt.toString(), "ho");

    game::config::StringOption b("hu");
    a.checkEqual("31. value", b(), "hu");
    a.checkEqual("32. value", b.toString(), "hu");
}
