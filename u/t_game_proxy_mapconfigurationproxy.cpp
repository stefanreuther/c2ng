/**
  *  \file u/t_game_proxy_mapconfigurationproxy.cpp
  *  \brief Test for game::proxy::MapConfigurationProxy
  */

#include "game/proxy/mapconfigurationproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/game.hpp"

/** Test getMapConfiguration(), setMapConfiguration(). */
void
TestGameProxyMapConfigurationProxy::testMapConfig()
{
    // Setup
    game::test::SessionThread h;
    h.session().setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    h.session().setGame(new game::Game());

    // Testee
    game::proxy::MapConfigurationProxy testee(h.gameSender());

    // Set
    game::map::Configuration in;
    in.setConfiguration(game::map::Configuration::Wrapped, game::map::Point(2000, 1900), game::map::Point(1400, 1500));
    testee.setMapConfiguration(in);

    // Load back
    game::test::WaitIndicator ind;
    game::map::Configuration out;
    testee.getMapConfiguration(ind, out);

    TS_ASSERT_EQUALS(out.getMode(), game::map::Configuration::Wrapped);
    TS_ASSERT_EQUALS(out.getCenter().getX(), 2000);
    TS_ASSERT_EQUALS(out.getSize().getX(), 1400);

    // Verify stored state - external interface
    game::config::ConfigurationOption* opt = h.session().getRoot()->userConfiguration().getOptionByName("Chart.Geo.Size");
    TS_ASSERT(opt != 0);
    TS_ASSERT_EQUALS(opt->toString(), "1400,1500");
}

/** Test getRenderOptions(), setRenderOptions(). */
void
TestGameProxyMapConfigurationProxy::testRenderOptions()
{
    using game::map::RenderOptions;

    // Setup
    game::test::SessionThread h;
    h.session().setRoot(game::test::makeRoot(game::HostVersion()).asPtr());

    // Testee
    game::proxy::MapConfigurationProxy testee(h.gameSender());

    // Set
    testee.setRenderOptions(RenderOptions::Normal,
                            RenderOptions(RenderOptions::Options_t() + RenderOptions::ShowIonStorms + RenderOptions::ShowMinefields,
                                          RenderOptions::Options_t() + RenderOptions::ShowIonStorms));

    // Load back
    game::test::WaitIndicator ind;
    RenderOptions opts = testee.getRenderOptions(ind, RenderOptions::Normal);
    TS_ASSERT_EQUALS(opts.getOption(RenderOptions::ShowIonStorms), RenderOptions::Filled);
    TS_ASSERT_EQUALS(opts.getOption(RenderOptions::ShowMinefields), RenderOptions::Enabled);
    TS_ASSERT_EQUALS(opts.getOption(RenderOptions::ShowUfos), RenderOptions::Disabled);

    // Verify stored state - external interface
    game::config::ConfigurationOption* opt = h.session().getRoot()->userConfiguration().getOptionByName("Chart.Normal.Fill");
    TS_ASSERT(opt != 0);
    TS_ASSERT_EQUALS(opt->toString(), "ion");
}

/** Test getMarkerConfiguration(), setMarkerConfiguration(). */
void
TestGameProxyMapConfigurationProxy::testMarkerConfig()
{
    // Setup
    game::test::SessionThread h;
    h.session().setRoot(game::test::makeRoot(game::HostVersion()).asPtr());

    // Testee
    game::proxy::MapConfigurationProxy testee(h.gameSender());

    // Set
    testee.setMarkerConfiguration(3, game::config::MarkerOption::Data(3, 4, "hu"));

    // Load back
    game::test::WaitIndicator ind;
    std::vector<game::config::MarkerOption::Data> data;
    testee.getMarkerConfiguration(ind, data);
    TS_ASSERT(data.size() >= 10U);
    TS_ASSERT_EQUALS(data[3].note, "hu");

    // Verify stored state - external interface
    game::config::ConfigurationOption* opt = h.session().getRoot()->userConfiguration().getOptionByName("Chart.Marker3");
    TS_ASSERT(opt != 0);
    TS_ASSERT_EQUALS(opt->toString(), "3,4,hu");
}

