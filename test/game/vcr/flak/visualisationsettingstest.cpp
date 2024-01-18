/**
  *  \file test/game/vcr/flak/visualisationsettingstest.cpp
  *  \brief Test for game::vcr::flak::VisualisationSettings
  */

#include "game/vcr/flak/visualisationsettings.hpp"
#include "afl/test/testrunner.hpp"

using game::vcr::flak::VisualisationSettings;
using game::vcr::flak::Visualizer;
using game::vcr::flak::Position;

namespace {
    Visualizer::ShipInfo makeShipInfo(int player, bool isPlanet)
    {
        Visualizer::ShipInfo info;
        info.player = player;
        info.isPlanet = isPlanet;
        return info;
    }
}

/** Test initial state. */
AFL_TEST("game.vcr.flak.VisualisationSettings:init", a)
{
    VisualisationSettings testee;
    a.checkEqual("01. getFollowedFleet", testee.getFollowedFleet(), Visualizer::NO_ENEMY);
    a.checkEqual("02. isAutoCamera", testee.isAutoCamera(), true);
    a.checkGreaterThan("03. getCameraDistance", testee.getCameraDistance(), 0);
    a.checkEqual("04. getCameraAzimuth", testee.getCameraAzimuth(), 0.0);
    a.checkEqual("05. getCameraRaise", testee.getCameraRaise(), 0);
}

/** Test move(). */
AFL_TEST("game.vcr.flak.VisualisationSettings:move", a)
{
    VisualisationSettings testee;
    float h = testee.getCameraHeight();

    // First move: disables auto-cam
    VisualisationSettings::Changes_t ch = testee.move(0.0, 0.25);
    a.checkEqual("01. move", ch, VisualisationSettings::Changes_t() + VisualisationSettings::ParameterChange + VisualisationSettings::CameraChange);

    // Second move
    ch = testee.move(0.0, 0.75);
    a.checkEqual("11. move", ch, VisualisationSettings::Changes_t() + VisualisationSettings::ParameterChange);

    // Verify move
    a.checkEqual("21. getCameraAzimuth", testee.getCameraAzimuth(), 1.0);
    a.checkEqual("22. getCameraHeight", testee.getCameraHeight(), h);
}

/** Test followFleet(). */
AFL_TEST("game.vcr.flak.VisualisationSettings:followFleet", a)
{
    // Environment
    game::vcr::flak::VisualisationState st;
    st.createShip(1, Position(1000, 2000, 0), makeShipInfo(3, false));
    st.createShip(2, Position(1000, -5000, 10), makeShipInfo(4, true));
    st.createFleet(0, 1000, 2000, 3, 1, 1);
    st.createFleet(1, 1000, -5000, 4, 2, 1);

    // Test
    VisualisationSettings testee;
    VisualisationSettings::Changes_t ch = testee.followFleet(1, st);
    a.checkEqual("01. followFleet", ch, VisualisationSettings::Changes_t() + VisualisationSettings::FollowChange);
    a.checkEqual("02. getFollowedFleet", testee.getFollowedFleet(), 1U);

    // Azimuth adjustment
    float old = testee.getCameraAzimuth();
    ch = testee.updateCamera(st);
    a.checkDifferent("11. getCameraAzimuth", testee.getCameraAzimuth(), old);
}

/** Test followPlayer(). */
AFL_TEST("game.vcr.flak.VisualisationSettings:followPlayer", a)
{
    // Environment
    game::vcr::flak::VisualisationState st;
    st.createShip(1, Position(1000, 2000, 0), makeShipInfo(3, false));
    st.createShip(2, Position(1000, -5000, 10), makeShipInfo(4, true));
    st.createFleet(0, 1000, 2000, 3, 1, 1);
    st.createFleet(1, 1000, -5000, 4, 2, 1);

    // Test
    VisualisationSettings testee;
    VisualisationSettings::Changes_t ch = testee.followPlayer(4, st);
    a.checkEqual("01. followPlayer", ch, VisualisationSettings::Changes_t() + VisualisationSettings::FollowChange);
    a.checkEqual("02. getFollowedFleet", testee.getFollowedFleet(), 1U);

    // Azimuth adjustment
    float old = testee.getCameraAzimuth();
    ch = testee.updateCamera(st);
    a.checkDifferent("11. getCameraAzimuth", testee.getCameraAzimuth(), old);
}

/** Test followFleet(), followPlayer(), error cases. */
AFL_TEST("game.vcr.flak.VisualisationSettings:followPlayer:error", a)
{
    // Environment
    game::vcr::flak::VisualisationState st;
    st.createShip(1, Position(1000, 2000, 0), makeShipInfo(3, false));
    st.createShip(2, Position(1000, -5000, 10), makeShipInfo(4, true));
    st.createFleet(0, 1000, 2000, 3, 1, 1);
    st.createFleet(1, 1000, -5000, 4, 2, 1);

    // Test
    VisualisationSettings testee;

    // - followPlayer() is a no-op if player does not exist
    VisualisationSettings::Changes_t ch = testee.followPlayer(7, st);
    a.checkEqual("01. followPlayer", ch, VisualisationSettings::Changes_t());

    // - followFleet() will work even for currently non-existant fleets
    ch = testee.followFleet(7, st);
    a.checkEqual("11. followFleet", ch, VisualisationSettings::Changes_t() + VisualisationSettings::FollowChange);
}

/** Test camera raise. */
AFL_TEST("game.vcr.flak.VisualisationSettings:raise", a)
{
    game::vcr::flak::VisualisationState st;
    VisualisationSettings testee;
    testee.setCameraRaiseSpeed(100);

    // Set target: no change yet
    VisualisationSettings::Changes_t ch = testee.setCameraRaiseTarget(2000);
    a.checkEqual("01. setCameraRaiseTarget", ch, VisualisationSettings::Changes_t());
    a.checkEqual("02. getCameraRaise", testee.getCameraRaise(), 0);

    // First move: moves slowly
    ch = testee.updateCamera(st);
    a.checkEqual("11. updateCamera", ch, VisualisationSettings::Changes_t() + VisualisationSettings::ParameterChange);
    a.checkEqual("12. getCameraRaise", testee.getCameraRaise(), 100);

    // Disable auto; moves instantly
    ch = testee.toggleAutoCamera();
    a.checkEqual("21. toggleAutoCamera", ch, VisualisationSettings::Changes_t() + VisualisationSettings::ParameterChange + VisualisationSettings::CameraChange);
    a.checkEqual("22. getCameraRaise", testee.getCameraRaise(), 2000);

    // New raise; moves instantly
    ch = testee.setCameraRaiseTarget(333);
    a.checkEqual("31. setCameraRaiseTarget", ch, VisualisationSettings::Changes_t() + VisualisationSettings::ParameterChange);
    a.checkEqual("32. getCameraRaise", testee.getCameraRaise(), 333);
}

/** Test zoom. */
AFL_TEST("game.vcr.flak.VisualisationSettings:zoom", a)
{
    VisualisationSettings testee;
    float dist = testee.getCameraDistance();

    // First move: disables auto-cam
    VisualisationSettings::Changes_t ch = testee.zoomIn();
    a.checkEqual("01. zoomIn", ch, VisualisationSettings::Changes_t() + VisualisationSettings::ParameterChange + VisualisationSettings::CameraChange);

    // Second move
    ch = testee.zoomOut();
    a.checkEqual("11. zoomOut", ch, VisualisationSettings::Changes_t() + VisualisationSettings::ParameterChange);

    // Verify move
    a.checkEqual("21. getCameraDistance", testee.getCameraDistance(), dist);
}

/** Test following a fleet that died. */
AFL_TEST("game.vcr.flak.VisualisationSettings:followFleet:dead", a)
{
    // Environment
    game::vcr::flak::VisualisationState st;
    st.createShip(0, Position(1000, 2000, 0), makeShipInfo(3, false));
    st.createShip(1, Position(2000, 3000, 0), makeShipInfo(4, true));
    st.createShip(2, Position(3000, 4000, 0), makeShipInfo(3, false));
    st.createShip(3, Position(4000, 5000, 0), makeShipInfo(3, false));
    st.createFleet(0, 1000, 2000, 3, 0, 1);
    st.createFleet(1, 2000, 3000, 4, 1, 1);
    st.createFleet(2, 3000, 4000, 3, 2, 1);
    st.createFleet(3, 4000, 5000, 3, 3, 1);

    // Follow fleet 0
    VisualisationSettings testee;
    testee.followFleet(0, st);

    // Kill fleet 0
    st.killShip(0);
    st.killFleet(0);

    // Update camera
    VisualisationSettings::Changes_t ch = testee.updateCamera(st);
    a.checkEqual("01. updateCamera", ch, VisualisationSettings::Changes_t() + VisualisationSettings::ParameterChange + VisualisationSettings::FollowChange);

    // Verify
    a.checkEqual("11. getFollowedFleet", testee.getFollowedFleet(), 2U);
}

AFL_TEST("game.vcr.flak.VisualisationSettings:followFleet:dead:2", a)
{
    // Environment
    game::vcr::flak::VisualisationState st;
    st.createShip(0, Position(1000, 2000, 0), makeShipInfo(3, false));
    st.createShip(1, Position(2000, 3000, 0), makeShipInfo(4, true));
    st.createShip(2, Position(3000, 4000, 0), makeShipInfo(4, false));
    st.createShip(3, Position(4000, 5000, 0), makeShipInfo(5, false));
    st.createFleet(0, 1000, 2000, 3, 0, 1);
    st.createFleet(1, 2000, 3000, 4, 1, 1);
    st.createFleet(2, 3000, 4000, 4, 2, 1);
    st.createFleet(3, 4000, 5000, 5, 3, 1);

    // Follow fleet 0
    VisualisationSettings testee;
    testee.followFleet(0, st);

    // Kill fleet 0
    st.killShip(0);
    st.killFleet(0);

    // Update camera
    VisualisationSettings::Changes_t ch = testee.updateCamera(st);
    a.checkEqual("01. updateCamera", ch, VisualisationSettings::Changes_t() + VisualisationSettings::ParameterChange + VisualisationSettings::FollowChange);

    // Verify
    a.checkEqual("11. getFollowedFleet", testee.getFollowedFleet(), 1U);
}
