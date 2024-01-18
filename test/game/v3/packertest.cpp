/**
  *  \file test/game/v3/packertest.cpp
  *  \brief Test for game::v3::Packer
  */

#include "game/v3/packer.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/test/testrunner.hpp"

/** Test unpackShip(). */
AFL_TEST("game.v3.Packer:unpackShip", a)
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
    a.checkEqual("01. size", sizeof(DATA), sizeof(in));
    afl::base::fromObject(in).copyFrom(DATA);

    // Do it
    game::map::ShipData out;
    afl::charset::Utf8Charset cs;
    game::v3::Packer(cs).unpackShip(out, in, false);

    // Verify
    a.checkEqual("11. owner",               out.owner.orElse(-1),               2);
    a.checkEqual("12. friendlyCode",        out.friendlyCode.orElse(""),        "7(w");
    a.checkEqual("13. warpFactor",          out.warpFactor.orElse(-1),          2);
    a.checkEqual("14. waypointDX",          out.waypointDX.orElse(-1),          0);
    a.checkEqual("15. waypointDY",          out.waypointDY.orElse(-1),          0);
    a.checkEqual("16. x",                   out.x.orElse(-1),                   2136);
    a.checkEqual("17. y",                   out.y.orElse(-1),                   2141);
    a.checkEqual("18. engineType",          out.engineType.orElse(-1),          6);
    a.checkEqual("19. hullType",            out.hullType.orElse(-1),            56);
    a.checkEqual("20. beamType",            out.beamType.orElse(-1),            6);
    a.checkEqual("21. numBeams",            out.numBeams.orElse(-1),            6);
    a.checkEqual("22. numBays",             out.numBays.orElse(-1),             0);
    a.checkEqual("23. torpedoType",         out.torpedoType.orElse(-1),         6);
    a.checkEqual("24. ammo",                out.ammo.orElse(-1),                30);
    a.checkEqual("25. numLaunchers",        out.numLaunchers.orElse(-1),        2);
    a.checkEqual("26. mission",             out.mission.orElse(-1),             5);
    a.checkEqual("27. primaryEnemy",        out.primaryEnemy.orElse(-1),        0);
    a.checkEqual("28. missionTowParameter", out.missionTowParameter.orElse(-1), 0);
    a.checkEqual("29. damage",              out.damage.orElse(-1),              0);
    a.checkEqual("30. crew",                out.crew.orElse(-1),                236);
    a.checkEqual("31. colonists",           out.colonists.orElse(-1),           0);
    a.checkEqual("32. name",                out.name.orElse(""),                "C.S.S. TradeHs04");
    a.checkEqual("33. neutronium",          out.neutronium.orElse(-1),          300);
    a.checkEqual("34. tritanium",           out.tritanium.orElse(-1),           0);
    a.checkEqual("35. duranium",            out.duranium.orElse(-1),            0);
    a.checkEqual("36. molybdenum",          out.molybdenum.orElse(-1),          0);
}

/** Test unpackPlanet(). */
AFL_TEST("game.v3.Packer:unpackPlanet", a)
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
    a.checkEqual("01. size", sizeof(DATA), sizeof(in));
    afl::base::fromObject(in).copyFrom(DATA);

    // Do it
    game::map::PlanetData out;
    afl::charset::Utf8Charset cs;
    game::v3::Packer(cs).unpackPlanet(out, in);

    // Verify
    a.checkEqual("11. owner",             out.owner.orElse(-1), 4);
    a.checkEqual("12. friendlyCode",      out.friendlyCode.orElse(""), ".A:");
    a.checkEqual("13. numMines",          out.numMines.orElse(-1), 130);
    a.checkEqual("14. numFactories",      out.numFactories.orElse(-1), 131);
    a.checkEqual("15. numDefensePosts",   out.numDefensePosts.orElse(-1), 43);
    a.checkEqual("16. minedNeutronium",   out.minedNeutronium.orElse(-1), 1240);
    a.checkEqual("17. minedTritanium",    out.minedTritanium.orElse(-1), 671);
    a.checkEqual("18. minedDuranium",     out.minedDuranium.orElse(-1), 1208);
    a.checkEqual("19. minedMolybdenum",   out.minedMolybdenum.orElse(-1), 1665);
    a.checkEqual("20. colonistClans",     out.colonistClans.orElse(-1), 1110);
    a.checkEqual("21. supplies",          out.supplies.orElse(-1), 500);
    a.checkEqual("22. money",             out.money.orElse(-1), 726);
    a.checkEqual("23. groundNeutronium",  out.groundNeutronium.orElse(-1), 111);
    a.checkEqual("24. groundTritanium",   out.groundTritanium.orElse(-1), 823);
    a.checkEqual("25. groundDuranium",    out.groundDuranium.orElse(-1), 1138);
    a.checkEqual("26. groundMolybdenum",  out.groundMolybdenum.orElse(-1), 116);
    a.checkEqual("27. densityNeutronium", out.densityNeutronium.orElse(-1), 70);
    a.checkEqual("28. densityTritanium",  out.densityTritanium.orElse(-1), 16);
    a.checkEqual("29. densityDuranium",   out.densityDuranium.orElse(-1), 53);
    a.checkEqual("30. densityMolybdenum", out.densityMolybdenum.orElse(-1), 85);
    a.checkEqual("31. colonistTax",       out.colonistTax.orElse(-1), 11);
    a.checkEqual("32. nativeTax",         out.nativeTax.orElse(-1), 4);
    a.checkEqual("33. colonistHappiness", out.colonistHappiness.orElse(-1), 100);
    a.checkEqual("34. nativeHappiness",   out.nativeHappiness.orElse(-1), 100);
    a.checkEqual("35. nativeGovernment",  out.nativeGovernment.orElse(-1), 3);
    a.checkEqual("36. nativeClans",       out.nativeClans.orElse(-1), 48946);
    a.checkEqual("37. nativeRace",        out.nativeRace.orElse(-1), 6);
    a.checkEqual("38. temperature",       out.temperature.orElse(-1), 100 - 17);
    a.checkEqual("39. baseFlag",          out.baseFlag.orElse(-1), 0);
}

/** Test unpackBase(). */
AFL_TEST("game.v3.Packer:unpackBase", a)
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
    a.checkEqual("01. size", sizeof(DATA), sizeof(in));
    afl::base::fromObject(in).copyFrom(DATA);

    // Do it
    game::map::BaseData out;
    afl::charset::Utf8Charset cs;
    game::v3::Packer(cs).unpackBase(out, in);

    // Verify
    a.checkEqual("11. numBaseDefensePosts", out.numBaseDefensePosts.orElse(-1),    200);
    a.checkEqual("12. damage",              out.damage.orElse(-1),                 0);
    a.checkEqual("13. techLevels",          out.techLevels[0].orElse(-1),          6);
    a.checkEqual("14. techLevels",          out.techLevels[1].orElse(-1),          6);
    a.checkEqual("15. techLevels",          out.techLevels[2].orElse(-1),          5);
    a.checkEqual("16. techLevels",          out.techLevels[3].orElse(-1),          5);
    a.checkEqual("17. engineStorage",       out.engineStorage.get(1).orElse(-1),   0);
    a.checkEqual("18. engineStorage",       out.engineStorage.get(6).orElse(-1),   2);
    a.checkEqual("19. hullStorage",         out.hullStorage.get(1).orElse(-1),     0);
    a.checkEqual("20. hullStorage",         out.hullStorage.get(6).orElse(-1),     1);
    a.checkEqual("21. beamStorage",         out.beamStorage.get(1).orElse(-1),     0);
    a.checkEqual("22. beamStorage",         out.beamStorage.get(6).orElse(-1),     4);
    a.checkEqual("23. launcherStorage",     out.launcherStorage.get(1).orElse(-1), 0);
    a.checkEqual("24. launcherStorage",     out.launcherStorage.get(6).orElse(-1), 2);
    a.checkEqual("25. torpedoStorage",      out.torpedoStorage.get(1).orElse(-1),  0);
    a.checkEqual("26. torpedoStorage",      out.torpedoStorage.get(6).orElse(-1),  50);
    a.checkEqual("27. numFighters",         out.numFighters.orElse(-1),            0);
    a.checkEqual("28. shipyardId",          out.shipyardId.orElse(-1),             0);
    a.checkEqual("29. shipyardAction",      out.shipyardAction.orElse(-1),         0);
    a.checkEqual("30. mission",             out.mission.orElse(-1),                6);
    a.checkEqual("31. shipBuildOrder",      out.shipBuildOrder.getHullIndex(),     6);
    a.checkEqual("32. shipBuildOrder",      out.shipBuildOrder.getEngineType(),    6);
    a.checkEqual("33. shipBuildOrder",      out.shipBuildOrder.getBeamType(),      6);
    a.checkEqual("34. shipBuildOrder",      out.shipBuildOrder.getNumBeams(),      4);
    a.checkEqual("35. shipBuildOrder",      out.shipBuildOrder.getTorpedoType(),   6);
    a.checkEqual("36. shipBuildOrder",      out.shipBuildOrder.getNumLaunchers(),  2);
}
