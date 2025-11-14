/**
  *  \file test/game/map/renderoptionstest.cpp
  *  \brief Test for game::map::RenderOptions
  */

#include "game/map/renderoptions.hpp"

#include "afl/test/testrunner.hpp"
#include "game/config/userconfiguration.hpp"

using afl::base::Ref;
using game::config::UserConfiguration;
using game::map::RenderOptions;

/** Test setOptions(), getOption(), toggleOptions(). */
AFL_TEST("game.map.RenderOptions:set", a)
{
    RenderOptions t;

    t.setOptions(RenderOptions::Options_t(RenderOptions::ShowIonStorms));
    a.checkEqual("01", t.getOption(RenderOptions::ShowIonStorms), RenderOptions::Enabled);

    t.toggleOptions(RenderOptions::Options_t(RenderOptions::ShowIonStorms));
    a.checkEqual("11", t.getOption(RenderOptions::ShowIonStorms), RenderOptions::Filled);

    t.toggleOptions(RenderOptions::Options_t(RenderOptions::ShowIonStorms));
    a.checkEqual("21", t.getOption(RenderOptions::ShowIonStorms), RenderOptions::Disabled);

    t.toggleOptions(RenderOptions::Options_t(RenderOptions::ShowIonStorms));
    a.checkEqual("31", t.getOption(RenderOptions::ShowIonStorms), RenderOptions::Enabled);
}

/** Test transfer to/from configuration. */
AFL_TEST("game.map.RenderOptions:config-transfer", a)
{
    Ref<UserConfiguration> rconfig = UserConfiguration::create();
    UserConfiguration& config = *rconfig;
    config.setOption("Chart.Small.Show", "mine,shipdots,ion,warpwells", game::config::ConfigurationOption::System);
    config.setOption("Chart.Small.Fill", "mine,shipdots", game::config::ConfigurationOption::System);

    RenderOptions t(RenderOptions::fromConfiguration(config, RenderOptions::Small));
    a.checkEqual("01. getOption", t.getOption(RenderOptions::ShowMinefields), RenderOptions::Filled);
    a.checkEqual("02. getOption", t.getOption(RenderOptions::ShowShipDots),   RenderOptions::Enabled);   // .fill ignored here
    a.checkEqual("03. getOption", t.getOption(RenderOptions::ShowIonStorms),  RenderOptions::Enabled);
    a.checkEqual("04. getOption", t.getOption(RenderOptions::ShowWarpWells),  RenderOptions::Enabled);
    a.checkEqual("05. getOption", t.getOption(RenderOptions::ShowUfos),       RenderOptions::Disabled);
    a.checkEqual("06. getOption", t.getOption(RenderOptions::ShowBorders),    RenderOptions::Disabled);

    using game::map::Viewport;
    Viewport::Options_t vo = t.getViewportOptions();
    a.checkEqual("11. getViewportOptions", vo, Viewport::Options_t()
                 + Viewport::ShowMinefields
                 + Viewport::FillMinefields
                 + Viewport::ShowIonStorms
                 + Viewport::ShowShipDots
                 + Viewport::ShowWarpWells);

    t.storeToConfiguration(config, RenderOptions::Normal);
    game::config::ConfigurationOption* opt = config.getOptionByName("Chart.Normal.Show");
    a.checkNonNull("21. getOptionByName", opt);
    a.checkEqual("22. toString", opt->toString(), "ion,mine,shipdots,warpwells");  // The order is not contractual.
}

/** Test translation of individual options. */
AFL_TEST("game.map.RenderOptions:translation", a)
{
    using game::map::Viewport;
    typedef RenderOptions::Options_t Rs;

    a.check("01", RenderOptions(Rs(RenderOptions::ShowIonStorms),  Rs()).getViewportOptions().contains(Viewport::ShowIonStorms));
    a.check("02", RenderOptions(Rs(RenderOptions::ShowMinefields), Rs()).getViewportOptions().contains(Viewport::ShowMinefields));
    a.check("03", RenderOptions(Rs(RenderOptions::ShowUfos),       Rs()).getViewportOptions().contains(Viewport::ShowUfos));
    a.check("04", RenderOptions(Rs(RenderOptions::ShowGrid),       Rs()).getViewportOptions().contains(Viewport::ShowGrid));
    a.check("05", RenderOptions(Rs(RenderOptions::ShowBorders),    Rs()).getViewportOptions().contains(Viewport::ShowBorders));
    a.check("06", RenderOptions(Rs(RenderOptions::ShowDrawings),   Rs()).getViewportOptions().contains(Viewport::ShowDrawings));
    a.check("07", RenderOptions(Rs(RenderOptions::ShowSelection),  Rs()).getViewportOptions().contains(Viewport::ShowSelection));
    a.check("08", RenderOptions(Rs(RenderOptions::ShowLabels),     Rs()).getViewportOptions().contains(Viewport::ShowLabels));
    a.check("09", RenderOptions(Rs(RenderOptions::ShowTrails),     Rs()).getViewportOptions().contains(Viewport::ShowTrails));
    a.check("10", RenderOptions(Rs(RenderOptions::ShowShipDots),   Rs()).getViewportOptions().contains(Viewport::ShowShipDots));
    a.check("11", RenderOptions(Rs(RenderOptions::ShowWarpWells),  Rs()).getViewportOptions().contains(Viewport::ShowWarpWells));

    a.check("21", RenderOptions(Rs(RenderOptions::ShowGrid),       Rs()).getViewportOptions().contains(Viewport::ShowOutsideGrid));
    a.check("22", !RenderOptions(Rs(RenderOptions::ShowGrid),      Rs(RenderOptions::ShowGrid)).getViewportOptions().contains(Viewport::ShowOutsideGrid));
    a.check("23", !RenderOptions(Rs(),                             Rs()).getViewportOptions().contains(Viewport::ShowOutsideGrid));
    a.check("24", !RenderOptions(Rs(),                             Rs(RenderOptions::ShowGrid)).getViewportOptions().contains(Viewport::ShowOutsideGrid));

    a.check("31", RenderOptions(Rs(RenderOptions::ShowIonStorms),  Rs(RenderOptions::ShowIonStorms)).getViewportOptions().contains(Viewport::FillIonStorms));
    a.check("32", RenderOptions(Rs(RenderOptions::ShowMinefields), Rs(RenderOptions::ShowMinefields)).getViewportOptions().contains(Viewport::FillMinefields));
    a.check("33", RenderOptions(Rs(RenderOptions::ShowUfos),       Rs(RenderOptions::ShowUfos)).getViewportOptions().contains(Viewport::FillUfos));
}

/** Test copyOptions(). */
AFL_TEST("game.map.RenderOptions:copyOptions", a)
{
    typedef RenderOptions::Options_t Rs;
    RenderOptions ra(Rs() + RenderOptions::ShowIonStorms + RenderOptions::ShowMinefields,
                     Rs() + RenderOptions::ShowIonStorms + RenderOptions::ShowMinefields);
    RenderOptions rb(Rs() + RenderOptions::ShowIonStorms + RenderOptions::ShowUfos,
                     Rs());

    ra.copyOptions(rb, Rs() + RenderOptions::ShowIonStorms + RenderOptions::ShowUfos);

    a.checkEqual("01", ra.getOption(RenderOptions::ShowIonStorms), RenderOptions::Enabled);
    a.checkEqual("02", ra.getOption(RenderOptions::ShowMinefields), RenderOptions::Filled);
    a.checkEqual("03", ra.getOption(RenderOptions::ShowUfos), RenderOptions::Enabled);
}

/** Test getOptionFromKey(). */
AFL_TEST("game.map.RenderOptions:getOptionFromKey", a)
{
    a.check("01", RenderOptions::getOptionFromKey(util::Key_F5).empty());
    a.check("02", RenderOptions::getOptionFromKey('a').contains(RenderOptions::ShowShipDots));
    a.check("03", RenderOptions::getOptionFromKey('b').contains(RenderOptions::ShowBorders));
    a.check("04", RenderOptions::getOptionFromKey('d').contains(RenderOptions::ShowLabels));
    a.check("05", RenderOptions::getOptionFromKey('i').contains(RenderOptions::ShowIonStorms));
    a.check("06", RenderOptions::getOptionFromKey('m').contains(RenderOptions::ShowMinefields));
    a.check("07", RenderOptions::getOptionFromKey('n').contains(RenderOptions::ShowMessages));
    a.check("08", RenderOptions::getOptionFromKey('p').contains(RenderOptions::ShowDrawings));
    a.check("09", RenderOptions::getOptionFromKey('s').contains(RenderOptions::ShowGrid));
    a.check("10", RenderOptions::getOptionFromKey('t').contains(RenderOptions::ShowSelection));
    a.check("11", RenderOptions::getOptionFromKey('u').contains(RenderOptions::ShowUfos));
    a.check("12", RenderOptions::getOptionFromKey('v').contains(RenderOptions::ShowTrails));
    a.check("13", RenderOptions::getOptionFromKey('w').contains(RenderOptions::ShowWarpWells));
    a.check("14", RenderOptions::getOptionFromKey('y').contains(RenderOptions::ShowMineDecay));
}
