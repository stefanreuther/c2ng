/**
  *  \file u/t_game_map_renderoptions.cpp
  *  \brief Test for game::map::RenderOptions
  */

#include "game/map/renderoptions.hpp"

#include "t_game_map.hpp"
#include "game/config/userconfiguration.hpp"

using game::map::RenderOptions;

/** Test setOptions(), getOption(), toggleOptions(). */
void
TestGameMapRenderOptions::testSet()
{
    RenderOptions t;

    t.setOptions(RenderOptions::Options_t(RenderOptions::ShowIonStorms));
    TS_ASSERT_EQUALS(t.getOption(RenderOptions::ShowIonStorms), RenderOptions::Enabled);

    t.toggleOptions(RenderOptions::Options_t(RenderOptions::ShowIonStorms));
    TS_ASSERT_EQUALS(t.getOption(RenderOptions::ShowIonStorms), RenderOptions::Filled);

    t.toggleOptions(RenderOptions::Options_t(RenderOptions::ShowIonStorms));
    TS_ASSERT_EQUALS(t.getOption(RenderOptions::ShowIonStorms), RenderOptions::Disabled);

    t.toggleOptions(RenderOptions::Options_t(RenderOptions::ShowIonStorms));
    TS_ASSERT_EQUALS(t.getOption(RenderOptions::ShowIonStorms), RenderOptions::Enabled);
}

/** Test transfer to/from configuration. */
void
TestGameMapRenderOptions::testTransfer()
{
    game::config::UserConfiguration config;
    config.setOption("Chart.Small.Show", "mine,shipdots,ion,warpwells", game::config::ConfigurationOption::System);
    config.setOption("Chart.Small.Fill", "mine,shipdots", game::config::ConfigurationOption::System);

    RenderOptions t(RenderOptions::fromConfiguration(config, RenderOptions::Small));
    TS_ASSERT_EQUALS(t.getOption(RenderOptions::ShowMinefields), RenderOptions::Filled);
    TS_ASSERT_EQUALS(t.getOption(RenderOptions::ShowShipDots),   RenderOptions::Enabled);   // .fill ignored here
    TS_ASSERT_EQUALS(t.getOption(RenderOptions::ShowIonStorms),  RenderOptions::Enabled);
    TS_ASSERT_EQUALS(t.getOption(RenderOptions::ShowWarpWells),  RenderOptions::Enabled);
    TS_ASSERT_EQUALS(t.getOption(RenderOptions::ShowUfos),       RenderOptions::Disabled);
    TS_ASSERT_EQUALS(t.getOption(RenderOptions::ShowBorders),    RenderOptions::Disabled);

    using game::map::Viewport;
    Viewport::Options_t vo = t.getViewportOptions();
    TS_ASSERT_EQUALS(vo, Viewport::Options_t()
                     + Viewport::ShowMinefields
                     + Viewport::FillMinefields
                     + Viewport::ShowIonStorms
                     + Viewport::ShowShipDots
                     + Viewport::ShowWarpWells);

    t.storeToConfiguration(config, RenderOptions::Normal);
    game::config::ConfigurationOption* opt = config.getOptionByName("Chart.Normal.Show");
    TS_ASSERT(opt != 0);
    TS_ASSERT_EQUALS(opt->toString(), "ion,mine,shipdots,warpwells");  // The order is not contractual.
}

/** Test translation of individual options. */
void
TestGameMapRenderOptions::testTranslation()
{
    using game::map::Viewport;
    typedef RenderOptions::Options_t Rs;

    TS_ASSERT(RenderOptions(Rs(RenderOptions::ShowIonStorms),  Rs()).getViewportOptions().contains(Viewport::ShowIonStorms));
    TS_ASSERT(RenderOptions(Rs(RenderOptions::ShowMinefields), Rs()).getViewportOptions().contains(Viewport::ShowMinefields));
    TS_ASSERT(RenderOptions(Rs(RenderOptions::ShowUfos),       Rs()).getViewportOptions().contains(Viewport::ShowUfos));
    TS_ASSERT(RenderOptions(Rs(RenderOptions::ShowGrid),       Rs()).getViewportOptions().contains(Viewport::ShowGrid));
    TS_ASSERT(RenderOptions(Rs(RenderOptions::ShowBorders),    Rs()).getViewportOptions().contains(Viewport::ShowBorders));
    TS_ASSERT(RenderOptions(Rs(RenderOptions::ShowDrawings),   Rs()).getViewportOptions().contains(Viewport::ShowDrawings));
    TS_ASSERT(RenderOptions(Rs(RenderOptions::ShowSelection),  Rs()).getViewportOptions().contains(Viewport::ShowSelection));
    TS_ASSERT(RenderOptions(Rs(RenderOptions::ShowLabels),     Rs()).getViewportOptions().contains(Viewport::ShowLabels));
    TS_ASSERT(RenderOptions(Rs(RenderOptions::ShowTrails),     Rs()).getViewportOptions().contains(Viewport::ShowTrails));
    TS_ASSERT(RenderOptions(Rs(RenderOptions::ShowShipDots),   Rs()).getViewportOptions().contains(Viewport::ShowShipDots));
    TS_ASSERT(RenderOptions(Rs(RenderOptions::ShowWarpWells),  Rs()).getViewportOptions().contains(Viewport::ShowWarpWells));

    TS_ASSERT(RenderOptions(Rs(RenderOptions::ShowGrid),       Rs()).getViewportOptions().contains(Viewport::ShowOutsideGrid));
    TS_ASSERT(!RenderOptions(Rs(RenderOptions::ShowGrid),      Rs(RenderOptions::ShowGrid)).getViewportOptions().contains(Viewport::ShowOutsideGrid));
    TS_ASSERT(!RenderOptions(Rs(),                             Rs()).getViewportOptions().contains(Viewport::ShowOutsideGrid));
    TS_ASSERT(!RenderOptions(Rs(),                             Rs(RenderOptions::ShowGrid)).getViewportOptions().contains(Viewport::ShowOutsideGrid));

    TS_ASSERT(RenderOptions(Rs(RenderOptions::ShowIonStorms),  Rs(RenderOptions::ShowIonStorms)).getViewportOptions().contains(Viewport::FillIonStorms));
    TS_ASSERT(RenderOptions(Rs(RenderOptions::ShowMinefields), Rs(RenderOptions::ShowMinefields)).getViewportOptions().contains(Viewport::FillMinefields));
    TS_ASSERT(RenderOptions(Rs(RenderOptions::ShowUfos),       Rs(RenderOptions::ShowUfos)).getViewportOptions().contains(Viewport::FillUfos));
}

/** Test copyOptions(). */
void
TestGameMapRenderOptions::testCopy()
{
    typedef RenderOptions::Options_t Rs;
    RenderOptions a(Rs() + RenderOptions::ShowIonStorms + RenderOptions::ShowMinefields,
                    Rs() + RenderOptions::ShowIonStorms + RenderOptions::ShowMinefields);
    RenderOptions b(Rs() + RenderOptions::ShowIonStorms + RenderOptions::ShowUfos,
                    Rs());

    a.copyOptions(b, Rs() + RenderOptions::ShowIonStorms + RenderOptions::ShowUfos);

    TS_ASSERT_EQUALS(a.getOption(RenderOptions::ShowIonStorms), RenderOptions::Enabled);
    TS_ASSERT_EQUALS(a.getOption(RenderOptions::ShowMinefields), RenderOptions::Filled);
    TS_ASSERT_EQUALS(a.getOption(RenderOptions::ShowUfos), RenderOptions::Enabled);
}

/** Test getOptionFromKey(). */
void
TestGameMapRenderOptions::testKey()
{
    TS_ASSERT(RenderOptions::getOptionFromKey(util::Key_F5).empty());
    TS_ASSERT(RenderOptions::getOptionFromKey('m').contains(RenderOptions::ShowMinefields));
    TS_ASSERT(RenderOptions::getOptionFromKey('u').contains(RenderOptions::ShowUfos));
    TS_ASSERT(RenderOptions::getOptionFromKey('a').contains(RenderOptions::ShowShipDots));
}

