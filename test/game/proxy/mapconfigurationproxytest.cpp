/**
  *  \file test/game/proxy/mapconfigurationproxytest.cpp
  *  \brief Test for game::proxy::MapConfigurationProxy
  */

#include "game/proxy/mapconfigurationproxy.hpp"

#include "afl/test/testrunner.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/game.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"

/** Test getMapConfiguration(), setMapConfiguration(). */
AFL_TEST("game.proxy.MapConfigurationProxy:getMapConfiguration", a)
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

    a.checkEqual("01. getMode",   out.getMode(), game::map::Configuration::Wrapped);
    a.checkEqual("02. getCenter", out.getCenter().getX(), 2000);
    a.checkEqual("03. getSize",   out.getSize().getX(), 1400);

    // Verify stored state - external interface
    game::config::ConfigurationOption* opt = h.session().getRoot()->userConfiguration().getOptionByName("Chart.Geo.Size");
    a.checkNonNull("11. opt", opt);
    a.checkEqual("12. toString", opt->toString(), "1400,1500");
}

/** Test getRenderOptions(), setRenderOptions(). */
AFL_TEST("game.proxy.MapConfigurationProxy:getRenderOptions", a)
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
    a.checkEqual("01. ShowIonStorms",  opts.getOption(RenderOptions::ShowIonStorms), RenderOptions::Filled);
    a.checkEqual("02. ShowMinefields", opts.getOption(RenderOptions::ShowMinefields), RenderOptions::Enabled);
    a.checkEqual("03. ShowUfos",       opts.getOption(RenderOptions::ShowUfos), RenderOptions::Disabled);

    // Verify stored state - external interface
    game::config::ConfigurationOption* opt = h.session().getRoot()->userConfiguration().getOptionByName("Chart.Normal.Fill");
    a.checkNonNull("11. opt", opt);
    a.checkEqual("12. toString", opt->toString(), "ion");
}

/** Test getMarkerConfiguration(), setMarkerConfiguration(). */
AFL_TEST("game.proxy.MapConfigurationProxy:getMarkerConfiguration", a)
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
    a.check("01. size", data.size() >= 10U);
    a.checkEqual("02. note", data[3].note, "hu");

    // Verify stored state - external interface
    game::config::ConfigurationOption* opt = h.session().getRoot()->userConfiguration().getOptionByName("Chart.Marker3");
    a.checkNonNull("11. opt", opt);
    a.checkEqual("12. toString", opt->toString(), "3,4,hu");
}
