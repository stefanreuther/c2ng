/**
  *  \file u/t_game_vcr_flak_visualisationsettings.cpp
  *  \brief Test for game::vcr::flak::VisualisationSettings
  */

#include "game/vcr/flak/visualisationsettings.hpp"

#include "t_game_vcr_flak.hpp"

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
void
TestGameVcrFlakVisualisationSettings::testInit()
{
    VisualisationSettings testee;
    TS_ASSERT_EQUALS(testee.getFollowedFleet(), Visualizer::NO_ENEMY);
    TS_ASSERT_EQUALS(testee.isAutoCamera(), true);
    TS_ASSERT(testee.getCameraDistance() > 0);
    TS_ASSERT_EQUALS(testee.getCameraAzimuth(), 0.0);
    TS_ASSERT_EQUALS(testee.getCameraRaise(), 0);
}

/** Test move(). */
void
TestGameVcrFlakVisualisationSettings::testMove()
{
    VisualisationSettings testee;
    float h = testee.getCameraHeight();

    // First move: disables auto-cam
    VisualisationSettings::Changes_t ch = testee.move(0.0, 0.25);
    TS_ASSERT_EQUALS(ch, VisualisationSettings::Changes_t() + VisualisationSettings::ParameterChange + VisualisationSettings::CameraChange);

    // Second move
    ch = testee.move(0.0, 0.75);
    TS_ASSERT_EQUALS(ch, VisualisationSettings::Changes_t() + VisualisationSettings::ParameterChange);

    // Verify move
    TS_ASSERT_EQUALS(testee.getCameraAzimuth(), 1.0);
    TS_ASSERT_EQUALS(testee.getCameraHeight(), h);
}

/** Test followFleet(). */
void
TestGameVcrFlakVisualisationSettings::testFollowFleet()
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
    TS_ASSERT_EQUALS(ch, VisualisationSettings::Changes_t() + VisualisationSettings::FollowChange);
    TS_ASSERT_EQUALS(testee.getFollowedFleet(), 1U);

    // Azimuth adjustment
    float old = testee.getCameraAzimuth();
    ch = testee.updateCamera(st);
    TS_ASSERT_DIFFERS(testee.getCameraAzimuth(), old);
}

/** Test followPlayer(). */
void
TestGameVcrFlakVisualisationSettings::testFollowPlayer()
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
    TS_ASSERT_EQUALS(ch, VisualisationSettings::Changes_t() + VisualisationSettings::FollowChange);
    TS_ASSERT_EQUALS(testee.getFollowedFleet(), 1U);

    // Azimuth adjustment
    float old = testee.getCameraAzimuth();
    ch = testee.updateCamera(st);
    TS_ASSERT_DIFFERS(testee.getCameraAzimuth(), old);
}

/** Test followFleet(), followPlayer(), error cases. */
void
TestGameVcrFlakVisualisationSettings::testFollowError()
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
    TS_ASSERT_EQUALS(ch, VisualisationSettings::Changes_t());

    // - followFleet() will work even for currently non-existant fleets
    ch = testee.followFleet(7, st);
    TS_ASSERT_EQUALS(ch, VisualisationSettings::Changes_t() + VisualisationSettings::FollowChange);
}

/** Test camera raise. */
void
TestGameVcrFlakVisualisationSettings::testRaise()
{
    game::vcr::flak::VisualisationState st;
    VisualisationSettings testee;
    testee.setCameraRaiseSpeed(100);

    // Set target: no change yet
    VisualisationSettings::Changes_t ch = testee.setCameraRaiseTarget(2000);
    TS_ASSERT_EQUALS(ch, VisualisationSettings::Changes_t());
    TS_ASSERT_EQUALS(testee.getCameraRaise(), 0);

    // First move: moves slowly
    ch = testee.updateCamera(st);
    TS_ASSERT_EQUALS(ch, VisualisationSettings::Changes_t() + VisualisationSettings::ParameterChange);
    TS_ASSERT_EQUALS(testee.getCameraRaise(), 100);

    // Disable auto; moves instantly
    ch = testee.toggleAutoCamera();
    TS_ASSERT_EQUALS(ch, VisualisationSettings::Changes_t() + VisualisationSettings::ParameterChange + VisualisationSettings::CameraChange);
    TS_ASSERT_EQUALS(testee.getCameraRaise(), 2000);

    // New raise; moves instantly
    ch = testee.setCameraRaiseTarget(333);
    TS_ASSERT_EQUALS(ch, VisualisationSettings::Changes_t() + VisualisationSettings::ParameterChange);
    TS_ASSERT_EQUALS(testee.getCameraRaise(), 333);
}

/** Test zoom. */
void
TestGameVcrFlakVisualisationSettings::testZoom()
{
    VisualisationSettings testee;
    float dist = testee.getCameraDistance();

    // First move: disables auto-cam
    VisualisationSettings::Changes_t ch = testee.zoomIn();
    TS_ASSERT_EQUALS(ch, VisualisationSettings::Changes_t() + VisualisationSettings::ParameterChange + VisualisationSettings::CameraChange);

    // Second move
    ch = testee.zoomOut();
    TS_ASSERT_EQUALS(ch, VisualisationSettings::Changes_t() + VisualisationSettings::ParameterChange);

    // Verify move
    TS_ASSERT_EQUALS(testee.getCameraDistance(), dist);
}

/** Test following a fleet that died. */
void
TestGameVcrFlakVisualisationSettings::testFollowDead()
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
    TS_ASSERT_EQUALS(ch, VisualisationSettings::Changes_t() + VisualisationSettings::ParameterChange + VisualisationSettings::FollowChange);

    // Verify
    TS_ASSERT_EQUALS(testee.getFollowedFleet(), 2U);
}

void
TestGameVcrFlakVisualisationSettings::testFollowDead2()
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
    TS_ASSERT_EQUALS(ch, VisualisationSettings::Changes_t() + VisualisationSettings::ParameterChange + VisualisationSettings::FollowChange);

    // Verify
    TS_ASSERT_EQUALS(testee.getFollowedFleet(), 1U);
}

