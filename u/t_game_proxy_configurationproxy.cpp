/**
  *  \file u/t_game_proxy_configurationproxy.cpp
  *  \brief Test for game::proxy::ConfigurationProxy
  */

#include "game/proxy/configurationproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/root.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/test/waitindicator.hpp"

using game::config::UserConfiguration;

void
TestGameProxyConfigurationProxy::testIt()
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

