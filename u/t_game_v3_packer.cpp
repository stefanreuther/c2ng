/**
  *  \file u/t_game_v3_packer.cpp
  *  \brief Test for game::v3::Packer
  */

#include "game/v3/packer.hpp"

#include "t_game_v3.hpp"
#include "afl/charset/utf8charset.hpp"

/** Test unpackShip(). */
void
TestGameV3Packer::testUnpackShip()
{
    // Prepare an object
    static const uint8_t DATA[] = {
        0x58, 0x00, 0x02, 0x00, 0x37, 0x28, 0x77, 0x02, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x58, 0x08, 0x5d, 0x08, 0x06, 0x00, 0x38, 0x00, 0x06, 0x00, 0x06,
        0x00, 0x00, 0x00, 0x06, 0x00, 0x1e, 0x00, 0x02, 0x00, 0x05, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0xec, 0x00, 0x00, 0x00, 0x43, 0x2e, 0x53,
        0x2e, 0x53, 0x2e, 0x20, 0x54, 0x72, 0x61, 0x64, 0x65, 0x48, 0x73, 0x30,
        0x34, 0x20, 0x20, 0x20, 0x20, 0x2c, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    game::v3::structures::Ship in;
    TS_ASSERT_EQUALS(sizeof(DATA), sizeof(in));
    afl::base::fromObject(in).copyFrom(DATA);

    // Do it
    game::map::ShipData out;
    afl::charset::Utf8Charset cs;
    game::v3::Packer(cs).unpackShip(out, in, false);

    // Verify
    TS_ASSERT_EQUALS(out.owner.orElse(-1),               2);
    TS_ASSERT_EQUALS(out.friendlyCode.orElse(""),        "7(w");
    TS_ASSERT_EQUALS(out.warpFactor.orElse(-1),          2);
    TS_ASSERT_EQUALS(out.waypointDX.orElse(-1),          0);
    TS_ASSERT_EQUALS(out.waypointDY.orElse(-1),          0);
    TS_ASSERT_EQUALS(out.x.orElse(-1),                   2136);
    TS_ASSERT_EQUALS(out.y.orElse(-1),                   2141);
    TS_ASSERT_EQUALS(out.engineType.orElse(-1),          6);
    TS_ASSERT_EQUALS(out.hullType.orElse(-1),            56);
    TS_ASSERT_EQUALS(out.beamType.orElse(-1),            6);
    TS_ASSERT_EQUALS(out.numBeams.orElse(-1),            6);
    TS_ASSERT_EQUALS(out.numBays.orElse(-1),             0);
    TS_ASSERT_EQUALS(out.torpedoType.orElse(-1),         6);
    TS_ASSERT_EQUALS(out.ammo.orElse(-1),                30);
    TS_ASSERT_EQUALS(out.numLaunchers.orElse(-1),        2);
    TS_ASSERT_EQUALS(out.mission.orElse(-1),             5);
    TS_ASSERT_EQUALS(out.primaryEnemy.orElse(-1),        0);
    TS_ASSERT_EQUALS(out.missionTowParameter.orElse(-1), 0);
    TS_ASSERT_EQUALS(out.damage.orElse(-1),              0);
    TS_ASSERT_EQUALS(out.crew.orElse(-1),                236);
    TS_ASSERT_EQUALS(out.colonists.orElse(-1),           0);
    TS_ASSERT_EQUALS(out.name.orElse(""),                "C.S.S. TradeHs04");
    TS_ASSERT_EQUALS(out.neutronium.orElse(-1),          300);
    TS_ASSERT_EQUALS(out.tritanium.orElse(-1),           0);
    TS_ASSERT_EQUALS(out.duranium.orElse(-1),            0);
    TS_ASSERT_EQUALS(out.molybdenum.orElse(-1),          0);
}

/** Test unpackPlanet(). */
void
TestGameV3Packer::testUnpackPlanet()
{
    // Prepare an object
    static const uint8_t DATA[] = {
        0x04, 0x00, 0xd9, 0x00, 0x2e, 0x41, 0x3a, 0x82, 0x00, 0x83, 0x00, 0x2b,
        0x00, 0xd8, 0x04, 0x00, 0x00, 0x9f, 0x02, 0x00, 0x00, 0xb8, 0x04, 0x00,
        0x00, 0x81, 0x06, 0x00, 0x00, 0x56, 0x04, 0x00, 0x00, 0xf4, 0x01, 0x00,
        0x00, 0xd6, 0x02, 0x00, 0x00, 0x6f, 0x00, 0x00, 0x00, 0x37, 0x03, 0x00,
        0x00, 0x72, 0x04, 0x00, 0x00, 0x74, 0x00, 0x00, 0x00, 0x46, 0x00, 0x10,
        0x00, 0x35, 0x00, 0x55, 0x00, 0x0b, 0x00, 0x04, 0x00, 0x64, 0x00, 0x64,
        0x00, 0x03, 0x00, 0x32, 0xbf, 0x00, 0x00, 0x06, 0x00, 0x11, 0x00, 0x00,
        0x00
    };
    game::v3::structures::Planet in;
    TS_ASSERT_EQUALS(sizeof(DATA), sizeof(in));
    afl::base::fromObject(in).copyFrom(DATA);

    // Do it
    game::map::PlanetData out;
    afl::charset::Utf8Charset cs;
    game::v3::Packer(cs).unpackPlanet(out, in);

    // Verify
    TS_ASSERT_EQUALS(out.owner.orElse(-1), 4);
    TS_ASSERT_EQUALS(out.friendlyCode.orElse(""), ".A:");
    TS_ASSERT_EQUALS(out.numMines.orElse(-1), 130);
    TS_ASSERT_EQUALS(out.numFactories.orElse(-1), 131);
    TS_ASSERT_EQUALS(out.numDefensePosts.orElse(-1), 43);
    TS_ASSERT_EQUALS(out.minedNeutronium.orElse(-1), 1240);
    TS_ASSERT_EQUALS(out.minedTritanium.orElse(-1), 671);
    TS_ASSERT_EQUALS(out.minedDuranium.orElse(-1), 1208);
    TS_ASSERT_EQUALS(out.minedMolybdenum.orElse(-1), 1665);
    TS_ASSERT_EQUALS(out.colonistClans.orElse(-1), 1110);
    TS_ASSERT_EQUALS(out.supplies.orElse(-1), 500);
    TS_ASSERT_EQUALS(out.money.orElse(-1), 726);
    TS_ASSERT_EQUALS(out.groundNeutronium.orElse(-1), 111);
    TS_ASSERT_EQUALS(out.groundTritanium.orElse(-1), 823);
    TS_ASSERT_EQUALS(out.groundDuranium.orElse(-1), 1138);
    TS_ASSERT_EQUALS(out.groundMolybdenum.orElse(-1), 116);
    TS_ASSERT_EQUALS(out.densityNeutronium.orElse(-1), 70);
    TS_ASSERT_EQUALS(out.densityTritanium.orElse(-1), 16);
    TS_ASSERT_EQUALS(out.densityDuranium.orElse(-1), 53);
    TS_ASSERT_EQUALS(out.densityMolybdenum.orElse(-1), 85);
    TS_ASSERT_EQUALS(out.colonistTax.orElse(-1), 11);
    TS_ASSERT_EQUALS(out.nativeTax.orElse(-1), 4);
    TS_ASSERT_EQUALS(out.colonistHappiness.orElse(-1), 100);
    TS_ASSERT_EQUALS(out.nativeHappiness.orElse(-1), 100);
    TS_ASSERT_EQUALS(out.nativeGovernment.orElse(-1), 3);
    TS_ASSERT_EQUALS(out.nativeClans.orElse(-1), 48946);
    TS_ASSERT_EQUALS(out.nativeRace.orElse(-1), 6);
    TS_ASSERT_EQUALS(out.temperature.orElse(-1), 100 - 17);
    TS_ASSERT_EQUALS(out.baseFlag.orElse(-1), 0);
}

/** Test unpackBase(). */
void
TestGameV3Packer::testUnpackBase()
{
    // Prepare an object
    static const uint8_t DATA[] = {
        0xd9, 0x00, 0x04, 0x00, 0xc8, 0x00, 0x00, 0x00, 0x06, 0x00, 0x06, 0x00,
        0x05, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x06, 0x00,
        0x06, 0x00, 0x06, 0x00, 0x04, 0x00, 0x06, 0x00, 0x02, 0x00, 0x00, 0x00,
    };
    game::v3::structures::Base in;
    TS_ASSERT_EQUALS(sizeof(DATA), sizeof(in));
    afl::base::fromObject(in).copyFrom(DATA);

    // Do it
    game::map::BaseData out;
    afl::charset::Utf8Charset cs;
    game::v3::Packer(cs).unpackBase(out, in);

    // Verify
    TS_ASSERT_EQUALS(out.numBaseDefensePosts.orElse(-1),    200);
    TS_ASSERT_EQUALS(out.damage.orElse(-1),                 0);
    TS_ASSERT_EQUALS(out.techLevels[0].orElse(-1),          6);
    TS_ASSERT_EQUALS(out.techLevels[1].orElse(-1),          6);
    TS_ASSERT_EQUALS(out.techLevels[2].orElse(-1),          5);
    TS_ASSERT_EQUALS(out.techLevels[3].orElse(-1),          5);
    TS_ASSERT_EQUALS(out.engineStorage.get(1).orElse(-1),   0);
    TS_ASSERT_EQUALS(out.engineStorage.get(6).orElse(-1),   2);
    TS_ASSERT_EQUALS(out.hullStorage.get(1).orElse(-1),     0);
    TS_ASSERT_EQUALS(out.hullStorage.get(6).orElse(-1),     1);
    TS_ASSERT_EQUALS(out.beamStorage.get(1).orElse(-1),     0);
    TS_ASSERT_EQUALS(out.beamStorage.get(6).orElse(-1),     4);
    TS_ASSERT_EQUALS(out.launcherStorage.get(1).orElse(-1), 0);
    TS_ASSERT_EQUALS(out.launcherStorage.get(6).orElse(-1), 2);
    TS_ASSERT_EQUALS(out.torpedoStorage.get(1).orElse(-1),  0);
    TS_ASSERT_EQUALS(out.torpedoStorage.get(6).orElse(-1),  50);
    TS_ASSERT_EQUALS(out.numFighters.orElse(-1),            0);
    TS_ASSERT_EQUALS(out.shipyardId.orElse(-1),             0);
    TS_ASSERT_EQUALS(out.shipyardAction.orElse(-1),         0);
    TS_ASSERT_EQUALS(out.mission.orElse(-1),                6);
    TS_ASSERT_EQUALS(out.shipBuildOrder.getHullIndex(),     6);
    TS_ASSERT_EQUALS(out.shipBuildOrder.getEngineType(),    6);
    TS_ASSERT_EQUALS(out.shipBuildOrder.getBeamType(),      6);
    TS_ASSERT_EQUALS(out.shipBuildOrder.getNumBeams(),      4);
    TS_ASSERT_EQUALS(out.shipBuildOrder.getTorpedoType(),   6);
    TS_ASSERT_EQUALS(out.shipBuildOrder.getNumLaunchers(),  2);
}

