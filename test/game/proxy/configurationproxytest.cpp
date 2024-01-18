/**
  *  \file test/game/proxy/configurationproxytest.cpp
  *  \brief Test for game::proxy::ConfigurationProxy
  */

#include "game/proxy/configurationproxy.hpp"

#include "afl/test/testrunner.hpp"
#include "game/config/integeroption.hpp"
#include "game/config/integervalueparser.hpp"
#include "game/config/stringoption.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"

using game::config::UserConfiguration;

/** Test accessing NumberFormatter. */
AFL_TEST("game.proxy.ConfigurationProxy:getNumberFormatter", a)
{
    // Setup
    // - session thread
    game::test::SessionThread h;

    // - root
    h.session().setRoot(game::test::makeRoot(game::HostVersion()).asPtr());

    // - user configuration
    UserConfiguration& config = h.session().getRoot()->userConfiguration();
    config[UserConfiguration::Display_ThousandsSep].set(0);
    config[UserConfiguration::Display_Clans].set(1);

    // Call subject function
    game::test::WaitIndicator ind;
    game::proxy::ConfigurationProxy testee(h.gameSender());
    util::NumberFormatter fmt = testee.getNumberFormatter(ind);

    // Verify
    a.checkEqual("01. formatNumber",     fmt.formatNumber(10000), "10000");
    a.checkEqual("02. formatPopulation", fmt.formatPopulation(500), "500c");
}

/** Test accessing integer options. */
AFL_TEST("game.proxy.ConfigurationProxy:getOption:int", a)
{
    static const game::config::IntegerOptionDescriptor desc = { "name", &game::config::IntegerValueParser::instance };

    // Setup
    game::test::SessionThread h;
    h.session().setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    UserConfiguration& config = h.session().getRoot()->userConfiguration();
    config[desc].set(7);
    a.checkEqual("01. getSource", config[desc].getSource(), game::config::ConfigurationOption::Default);

    // Proxy access
    game::test::WaitIndicator ind;
    game::proxy::ConfigurationProxy testee(h.gameSender());
    a.checkEqual("11. getOption", testee.getOption(ind, desc), 7);

    // Modify and read back
    testee.setOption(desc, 12);
    a.checkEqual("21. getOption", testee.getOption(ind, desc), 12);

    // Verify placement
    a.checkEqual("31. getSource", config[desc].getSource(), game::config::ConfigurationOption::User);
}

/** Test accessing string options. */
AFL_TEST("game.proxy.ConfigurationProxy:getOption:str", a)
{
    static const game::config::StringOptionDescriptor desc = { "name" };

    // Setup
    game::test::SessionThread h;
    h.session().setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    UserConfiguration& config = h.session().getRoot()->userConfiguration();
    config[desc].set("hi");
    a.checkEqual("01. getSource", config[desc].getSource(), game::config::ConfigurationOption::Default);

    // Proxy access
    game::test::WaitIndicator ind;
    game::proxy::ConfigurationProxy testee(h.gameSender());
    a.checkEqual("11. getOption", testee.getOption(ind, desc), "hi");

    // Modify and read back
    testee.setOption(desc, "ho");
    a.checkEqual("21. getOption", testee.getOption(ind, desc), "ho");

    // Verify placement
    a.checkEqual("31. getSource", config[desc].getSource(), game::config::ConfigurationOption::User);
}

/** Test accessing marker options. */
AFL_TEST("game.proxy.ConfigurationProxy:getOption:marker", a)
{
    static const game::config::MarkerOptionDescriptor desc = { "name", 3, 7 };

    // Setup
    game::test::SessionThread h;
    h.session().setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    UserConfiguration& config = h.session().getRoot()->userConfiguration();
    a.checkEqual("01. markerKind", config[desc]().markerKind, 3);
    a.checkEqual("02. color",      config[desc]().color, 7);
    a.checkEqual("03. getSource",  config[desc].getSource(), game::config::ConfigurationOption::Default);

    // Proxy access
    game::test::WaitIndicator ind;
    game::proxy::ConfigurationProxy testee(h.gameSender());
    game::config::MarkerOption::Data d = testee.getOption(ind, desc);
    a.checkEqual("11. markerKind", d.markerKind, 3);
    a.checkEqual("12. color", d.color, 7);

    // Modify and read back
    testee.setOption(desc, game::config::MarkerOption::Data(5, 6, "ho"));
    d = testee.getOption(ind, desc);
    a.checkEqual("21. markerKind", d.markerKind, 5);
    a.checkEqual("22. color", d.color, 6);
    a.checkEqual("23. note", d.note, "ho");

    // Verify placement
    a.checkEqual("31. getSource", config[desc].getSource(), game::config::ConfigurationOption::User);
}
