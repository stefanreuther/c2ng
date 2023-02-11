/**
  *  \file u/t_game_map_basedata.cpp
  *  \brief Test for game::map::BaseData
  */

#include "game/map/basedata.hpp"

#include "t_game_map.hpp"

void
TestGameMapBaseData::testGetBaseStorage()
{
    game::map::BaseData testee;
    TS_ASSERT_EQUALS(getBaseStorage(testee, game::BeamTech),    &testee.beamStorage);
    TS_ASSERT_EQUALS(getBaseStorage(testee, game::EngineTech),  &testee.engineStorage);
    TS_ASSERT_EQUALS(getBaseStorage(testee, game::HullTech),    &testee.hullStorage);
    TS_ASSERT_EQUALS(getBaseStorage(testee, game::TorpedoTech), &testee.launcherStorage);

    const game::map::BaseData& ct = testee;
    TS_ASSERT_EQUALS(getBaseStorage(testee, game::TorpedoTech), getBaseStorage(ct, game::TorpedoTech));
}

