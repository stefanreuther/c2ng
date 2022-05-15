/**
  *  \file u/t_game_proxy_configurationproxy.cpp
  *  \brief Test for game::proxy::ConfigurationProxy
  */

#include "game/proxy/configurationproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/config/integeroption.hpp"
#include "game/config/integervalueparser.hpp"
#include "game/config/stringoption.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"

using game::config::UserConfiguration;

/** Test accessing NumberFormatter. */
void
TestGameProxyConfigurationProxy::testNumberFormatter()
{
    // Setup
    // - session thread
    game::test::SessionThread h;

    // - root
    h.session().setRoot(new game::test::Root(game::HostVersion()));

    // - user configuration
    UserConfiguration& config = h.session().getRoot()->userConfiguration();
    config[UserConfiguration::Display_ThousandsSep].set(0);
    config[UserConfiguration::Display_Clans].set(1);

    // Call subject function
    game::test::WaitIndicator ind;
    game::proxy::ConfigurationProxy testee(h.gameSender());
    util::NumberFormatter fmt = testee.getNumberFormatter(ind);

    // Verify
    TS_ASSERT_EQUALS(fmt.formatNumber(10000), "10000");
    TS_ASSERT_EQUALS(fmt.formatPopulation(500), "500c");
}

/** Test accessing integer options. */
void
TestGameProxyConfigurationProxy::testIntAccess()
{
    static const game::config::IntegerOptionDescriptor desc = { "name", &game::config::IntegerValueParser::instance };

    // Setup
    game::test::SessionThread h;
    h.session().setRoot(new game::test::Root(game::HostVersion()));
    UserConfiguration& config = h.session().getRoot()->userConfiguration();
    config[desc].set(7);
    TS_ASSERT_EQUALS(config[desc].getSource(), game::config::ConfigurationOption::Default);

    // Proxy access
    game::test::WaitIndicator ind;
    game::proxy::ConfigurationProxy testee(h.gameSender());
    TS_ASSERT_EQUALS(testee.getOption(ind, desc), 7);

    // Modify and read back
    testee.setOption(desc, 12);
    TS_ASSERT_EQUALS(testee.getOption(ind, desc), 12);

    // Verify placement
    TS_ASSERT_EQUALS(config[desc].getSource(), game::config::ConfigurationOption::User);
}

/** Test accessing string options. */
void
TestGameProxyConfigurationProxy::testStringAccess()
{
    static const game::config::StringOptionDescriptor desc = { "name" };

    // Setup
    game::test::SessionThread h;
    h.session().setRoot(new game::test::Root(game::HostVersion()));
    UserConfiguration& config = h.session().getRoot()->userConfiguration();
    config[desc].set("hi");
    TS_ASSERT_EQUALS(config[desc].getSource(), game::config::ConfigurationOption::Default);

    // Proxy access
    game::test::WaitIndicator ind;
    game::proxy::ConfigurationProxy testee(h.gameSender());
    TS_ASSERT_EQUALS(testee.getOption(ind, desc), "hi");

    // Modify and read back
    testee.setOption(desc, "ho");
    TS_ASSERT_EQUALS(testee.getOption(ind, desc), "ho");

    // Verify placement
    TS_ASSERT_EQUALS(config[desc].getSource(), game::config::ConfigurationOption::User);
}

/** Test accessing marker options. */
void
TestGameProxyConfigurationProxy::testMarkerAccess()
{
    static const game::config::MarkerOptionDescriptor desc = { "name", 3, 7 };

    // Setup
    game::test::SessionThread h;
    h.session().setRoot(new game::test::Root(game::HostVersion()));
    UserConfiguration& config = h.session().getRoot()->userConfiguration();
    TS_ASSERT_EQUALS(config[desc]().markerKind, 3);
    TS_ASSERT_EQUALS(config[desc]().color, 7);
    TS_ASSERT_EQUALS(config[desc].getSource(), game::config::ConfigurationOption::Default);

    // Proxy access
    game::test::WaitIndicator ind;
    game::proxy::ConfigurationProxy testee(h.gameSender());
    game::config::MarkerOption::Data d = testee.getOption(ind, desc);
    TS_ASSERT_EQUALS(d.markerKind, 3);
    TS_ASSERT_EQUALS(d.color, 7);

    // Modify and read back
    testee.setOption(desc, game::config::MarkerOption::Data(5, 6, "ho"));
    d = testee.getOption(ind, desc);
    TS_ASSERT_EQUALS(d.markerKind, 5);
    TS_ASSERT_EQUALS(d.color, 6);
    TS_ASSERT_EQUALS(d.note, "ho");

    // Verify placement
    TS_ASSERT_EQUALS(config[desc].getSource(), game::config::ConfigurationOption::User);
}

