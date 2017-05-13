/**
  *  \file u/t_game_config_stringoption.cpp
  *  \brief Test for game::config::StringOption
  */

#include "game/config/stringoption.hpp"

#include "t_game_config.hpp"

/** Simple test. */
void
TestGameConfigStringOption::testIt()
{
    game::config::StringOption a;
    TS_ASSERT_EQUALS(a(), "");
    TS_ASSERT_EQUALS(a.toString(), "");

    a.set("hi");
    TS_ASSERT_EQUALS(a(), "hi");
    TS_ASSERT_EQUALS(a.toString(), "hi");

    a.set(String_t("ho"));
    TS_ASSERT_EQUALS(a(), "ho");
    TS_ASSERT_EQUALS(a.toString(), "ho");

    game::config::StringOption b("hu");
    TS_ASSERT_EQUALS(b(), "hu");
    TS_ASSERT_EQUALS(b.toString(), "hu");
}

