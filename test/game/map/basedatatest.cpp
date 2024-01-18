/**
  *  \file test/game/map/basedatatest.cpp
  *  \brief Test for game::map::BaseData
  */

#include "game/map/basedata.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("game.map.BaseData:getBaseStorage", a)
{
    game::map::BaseData testee;
    a.checkEqual("01. BeamTech",    getBaseStorage(testee, game::BeamTech),    &testee.beamStorage);
    a.checkEqual("02. EngineTech",  getBaseStorage(testee, game::EngineTech),  &testee.engineStorage);
    a.checkEqual("03. HullTech",    getBaseStorage(testee, game::HullTech),    &testee.hullStorage);
    a.checkEqual("04. TorpedoTech", getBaseStorage(testee, game::TorpedoTech), &testee.launcherStorage);

    const game::map::BaseData& ct = testee;
    a.checkEqual("11. TorpedoTech", getBaseStorage(testee, game::TorpedoTech), getBaseStorage(ct, game::TorpedoTech));
}
