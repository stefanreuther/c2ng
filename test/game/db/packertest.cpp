/**
  *  \file test/game/db/packertest.cpp
  *  \brief Test for game::db::Packer
  */

#include "game/db/packer.hpp"

#include "afl/base/staticassert.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/ufo.hpp"

AFL_TEST("game.db.Packer:ufo", a)
{
    // An Ufo record from Pleiades 7
    static const uint8_t DATA[] = {
        0x35, 0x00, 0x02, 0x00, 0x57, 0x6F, 0x72, 0x6D, 0x68, 0x6F,
        0x6C, 0x65, 0x20, 0x23, 0x32, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x32, 0x35, 0x36, 0x32, 0x36, 0x20,
        0x4B, 0x54, 0x2F, 0x42, 0x69, 0x64, 0x69, 0x72, 0x2E, 0x20, 0x20, 0x20, 0x20, 0x20, 0x6D, 0x6F, 0x73, 0x74, 0x6C, 0x79,
        0x20, 0x73, 0x74, 0x61, 0x62, 0x6C, 0x65, 0x20, 0x28, 0x3C, 0x33, 0x30, 0x25, 0x29, 0x98, 0x04, 0x59, 0x05, 0x00, 0x00,
        0xFF, 0xFF, 0x27, 0x01, 0x27, 0x01, 0x06, 0x00, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x2A, 0x00, 0x98, 0x04, 0x59, 0x05,
        0x00, 0x00, 0x00, 0x00,
    };
    game::db::structures::Ufo ufo;
    static_assert(sizeof(ufo) == sizeof(DATA), "sizeof Ufo");
    afl::base::fromObject(ufo).copyFrom(DATA);

    // Load the Ufo
    game::Turn turn;
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    game::db::Packer(cs).addUfo(turn, ufo);

    // Verify
    game::map::Ufo* p = turn.universe().ufos().getObjectByIndex(turn.universe().ufos().findUfoIndexById(53));
    a.checkNonNull("01. ufo", p);

    a.checkEqual("11. id",                p->getId(), 53);
    a.checkEqual("12. getColorCode",      p->getColorCode(), 2);
    a.checkEqual("13. getPlainName",      p->getPlainName(), "Wormhole #2");
    a.checkEqual("14. getInfo1",          p->getInfo1(), "25626 KT/Bidir.");
    a.checkEqual("15. getInfo2",          p->getInfo2(), "mostly stable (<30%)");

    game::map::Point pt;
    a.checkEqual("21. getPosition",       p->getPosition().get(pt), true);
    a.checkEqual("22. x",                 pt.getX(), 1176);
    a.checkEqual("23. y",                 pt.getY(), 1369);
    a.checkEqual("24. getWarpFactor",     p->getWarpFactor().orElse(-1), 0);
    a.checkEqual("25. getHeading",        p->getHeading().isValid(), false);
    a.checkEqual("26. getPlanetRange",    p->getPlanetRange().orElse(-1), 295);
    a.checkEqual("27. getShipRange",      p->getShipRange().orElse(-1), 295);

    int n;
    a.checkEqual("31. getRadius",         p->getRadius().get(n), true);
    a.checkEqual("32. radius",            n, 6);
    a.checkEqual("33. getTypeCode",       p->getTypeCode().orElse(-1), 1);
    a.checkEqual("34. getRealId",         p->getRealId(), 2);

    a.checkEqual("41. getLastTurn",       p->getLastTurn(), 42);
    a.checkEqual("42. getLastPosition",   p->getLastPosition().getX(), 1176);
    a.checkEqual("43. getLastPosition",   p->getLastPosition().getY(), 1369);
    a.checkEqual("44. getMovementVector", p->getMovementVector().getX(), 0);
    a.checkEqual("45. getMovementVector", p->getMovementVector().getY(), 0);

    // Store again
    game::db::structures::Ufo newUfo;
    game::db::Packer(cs).packUfo(newUfo, *p);

    a.checkEqualContent("51. packUfo", afl::base::ConstBytes_t(afl::base::fromObject(ufo)), afl::base::ConstBytes_t(afl::base::fromObject(newUfo)));
}

AFL_TEST("game.db.Packer:planet", a)
{
    static const uint8_t DATA[] = {
        0x08, 0x00, 0x04, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x33, 0x75, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x80, 0x00, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    game::db::structures::Planet planet;
    static_assert(sizeof(planet) == sizeof(DATA), "sizeof Planet");
    afl::base::fromObject(planet).copyFrom(DATA);

    // Load the planet
    // This will NOT create the planet, we have to do.
    game::Turn turn;
    for (int i = 1; i < 10; ++i) {
        turn.universe().planets().create(i);
    }
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    game::db::Packer(cs).addPlanet(turn, planet);

    // Verify
    game::map::Planet* p = turn.universe().planets().get(4);

    a.check("01. planet", p);
    a.checkEqual("02. getId", p->getId(), 4);

    int owner;
    a.checkEqual("11. getOwner", p->getOwner().get(owner), true);
    a.checkEqual("12. getOwner", owner, 8);

    a.checkEqual("21. getFriendlyCode", p->getFriendlyCode().isValid(), false);
    a.checkEqual("22. MineBuilding", p->getNumBuildings(game::MineBuilding).isValid(), false);
    a.checkEqual("23. FactoryBuilding", p->getNumBuildings(game::FactoryBuilding).isValid(), false);
    a.checkEqual("24. DefenseBuilding", p->getNumBuildings(game::DefenseBuilding).isValid(), false);
    a.checkEqual("25. getIndustryLevel", p->getIndustryLevel(game::HostVersion()).orElse(-1), 3);
    a.checkEqual("26. getHistoryTimestamp", p->getHistoryTimestamp(game::map::Planet::ColonistTime), 42);
    a.checkEqual("27. isKnownToHaveNatives", p->isKnownToHaveNatives(), false);

    // Store again
    game::db::structures::Planet newPlanet;
    game::db::Packer(cs).packPlanet(newPlanet, *p);

    a.checkEqualContent("31. packPlanet", afl::base::ConstBytes_t(afl::base::fromObject(planet)), afl::base::ConstBytes_t(afl::base::fromObject(newPlanet)));
}

AFL_TEST("game.db.Packer:planet:full", a)
{
    static const uint8_t DATA[] = {
        0x05, 0x00, 0x06, 0x00, 0x39, 0x71, 0x29, 0x02, 0x00, 0xFF, 0xFF, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00,
        0x00, 0x04, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
        0x00, 0x7E, 0x01, 0x00, 0x00, 0x09, 0x01, 0x00, 0x00, 0x1A, 0x01, 0x00, 0x00, 0xF8, 0x01, 0x00, 0x00, 0x28, 0x00, 0x5B,
        0x00, 0x1B, 0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x35, 0x00, 0x00, 0x00, 0x2A, 0x00, 0x2A, 0x00, 0x2A, 0x00, 0x2A, 0x00, 0x00,
    };

    game::db::structures::Planet planet;
    static_assert(sizeof(planet) == sizeof(DATA), "sizeof Planet");
    afl::base::fromObject(planet).copyFrom(DATA);

    // Load the planet
    // This will NOT create the planet, we have to do.
    game::Turn turn;
    for (int i = 1; i < 10; ++i) {
        turn.universe().planets().create(i);
    }
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    game::db::Packer(cs).addPlanet(turn, planet);

    // Verify
    game::map::Planet* p = turn.universe().planets().get(6);

    a.check("01. planet", p);
    a.checkEqual("02. getId", p->getId(), 6);

    int owner;
    a.checkEqual("11. getOwner", p->getOwner().get(owner), true);
    a.checkEqual("12. getOwner", owner, 5);

    a.checkEqual("21. getFriendlyCode",      p->getFriendlyCode().orElse(""), "9q)");
    a.checkEqual("22. MineBuilding",         p->getNumBuildings(game::MineBuilding).orElse(-1), 2);
    a.checkEqual("23. FactoryBuilding",      p->getNumBuildings(game::FactoryBuilding).isValid(), false);
    a.checkEqual("24. DefenseBuilding",      p->getNumBuildings(game::DefenseBuilding).orElse(-1), 3);
    a.checkEqual("25. neu",                  p->getCargo(game::Element::Neutronium).orElse(-1), 0);
    a.checkEqual("26. tri",                  p->getCargo(game::Element::Tritanium).orElse(-1), 15);
    a.checkEqual("27. dur",                  p->getCargo(game::Element::Duranium).orElse(-1), 4);
    a.checkEqual("28. mol",                  p->getCargo(game::Element::Molybdenum).orElse(-1), 15);
    a.checkEqual("29. col",                  p->getCargo(game::Element::Colonists).orElse(-1), 3);
    a.checkEqual("30. sup",                  p->getCargo(game::Element::Supplies).orElse(-1), 44);
    a.checkEqual("31. mc",                   p->getCargo(game::Element::Money).orElse(-1), 1);
    a.checkEqual("32. getOreGround",         p->getOreGround(game::Element::Neutronium).orElse(-1), 382);
    a.checkEqual("33. getOreGround",         p->getOreGround(game::Element::Tritanium).orElse(-1), 265);
    a.checkEqual("34. getOreGround",         p->getOreGround(game::Element::Duranium).orElse(-1), 282);
    a.checkEqual("35. getOreGround",         p->getOreGround(game::Element::Molybdenum).orElse(-1), 504);
    a.checkEqual("36. getOreDensity",        p->getOreDensity(game::Element::Neutronium).orElse(-1), 40);
    a.checkEqual("37. getOreDensity",        p->getOreDensity(game::Element::Tritanium).orElse(-1), 91);
    a.checkEqual("38. getOreDensity",        p->getOreDensity(game::Element::Duranium).orElse(-1), 27);
    a.checkEqual("39. getOreDensity",        p->getOreDensity(game::Element::Molybdenum).orElse(-1), 65);
    a.checkEqual("40. getColonistTax",       p->getColonistTax().orElse(-1), 0);
    a.checkEqual("41. getNativeTax",         p->getNativeTax().orElse(-1), 0);
    a.checkEqual("42. getColonistHappiness", p->getColonistHappiness().orElse(-1), 100);
    a.checkEqual("43. getNativeHappiness",   p->getNativeHappiness().orElse(-1), 100);
    a.checkEqual("44. getNativeGovernment",  p->getNativeGovernment().orElse(-1), 0);
    a.checkEqual("45. getNatives",           p->getNatives().orElse(-1), 0);
    a.checkEqual("46. getNativeRace",        p->getNativeRace().orElse(-1), 0);
    a.checkEqual("47. getTemperature",       p->getTemperature().orElse(-1), 47);

    a.checkEqual("51. getHistoryTimestamp",  p->getHistoryTimestamp(game::map::Planet::ColonistTime), 42);
    a.checkEqual("52. getHistoryTimestamp",  p->getHistoryTimestamp(game::map::Planet::NativeTime), 42);
    a.checkEqual("53. getHistoryTimestamp",  p->getHistoryTimestamp(game::map::Planet::MineralTime), 42);
    a.checkEqual("54. getHistoryTimestamp",  p->getHistoryTimestamp(game::map::Planet::CashTime), 42);
    a.checkEqual("55. isKnownToHaveNatives", p->isKnownToHaveNatives(), false);

    // Store again
    game::db::structures::Planet newPlanet;
    game::db::Packer(cs).packPlanet(newPlanet, *p);

    a.checkEqualContent("61. packPlanet", afl::base::ConstBytes_t(afl::base::fromObject(planet)), afl::base::ConstBytes_t(afl::base::fromObject(newPlanet)));
}

AFL_TEST("game.db.Packer:ship", a)
{
    const int TURN_NR = 42;
    const int SHIP_ID = 1;

    // Ship data
    static const uint8_t DATA[] = {
        0x01, 0x00, 0x05, 0x00, 0xFF, 0xFF, 0xFF, 0x09, 0x00, 0x00, 0x80, 0x00, 0x80, 0x80, 0x0B, 0x4F, 0x0B, 0xFF, 0xFF, 0x2C,
        0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4D, 0x45, 0x4E, 0x48, 0x55, 0x4E, 0x54, 0x45, 0x52, 0x20, 0x43, 0x4C, 0x41, 0x53, 0x53,
        0x20, 0x49, 0x4E, 0x54, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x2A, 0x00,
    };
    game::db::structures::Ship ship;
    static_assert(sizeof(ship) == sizeof(DATA), "sizeof Ship");
    afl::base::fromObject(ship).copyFrom(DATA);

    // Ship track entry (current turn)
    static const uint8_t TRACK1[] = {
        0x80, 0x0B, 0x4F, 0x0B, 0x09, 0x3B, 0x01, 0x8D, 0x00
    };
    game::db::structures::ShipTrackEntry track1;
    static_assert(sizeof(track1) == sizeof(TRACK1), "sizeof ShipTrackEntry");
    afl::base::fromObject(track1).copyFrom(TRACK1);

    // Ship track entry (previous turn)
    static const uint8_t TRACK2[] = {
        0xF4, 0x0A, 0x9C, 0x0B, 0x09, 0xFF, 0xFF, 0x6E, 0x00
    };
    game::db::structures::ShipTrackEntry track2;
    static_assert(sizeof(track2) == sizeof(TRACK2), "sizeof ShipTrackEntry");
    afl::base::fromObject(track2).copyFrom(TRACK2);

    // Load the ship
    // This will NOT create the ship, we have to do.
    // We need to add ship track entries to get location/mass info for history ships.
    game::Turn turn;
    for (int i = 1; i < 10; ++i) {
        turn.universe().ships().create(i);
    }
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    game::db::Packer(cs).addShip(turn, ship);
    game::db::Packer(cs).addShipTrack(turn, SHIP_ID, TURN_NR,   track1);
    game::db::Packer(cs).addShipTrack(turn, SHIP_ID, TURN_NR-1, track2);

    // Verify
    game::map::Ship* p = turn.universe().ships().get(SHIP_ID);

    a.check("01. ship", p);
    a.checkEqual("02. getId", p->getId(), SHIP_ID);

    // We need the checkers to merge ship track into current data
    p->internalCheck(game::PlayerSet_t(), TURN_NR);

    // Proceed with verification
    int owner;
    a.checkEqual("11. getOwner", p->getOwner().get(owner), true);
    a.checkEqual("12. getOwner", owner, 5);

    a.checkEqual("21. getFriendlyCode",     p->getFriendlyCode().isValid(), false);
    a.checkEqual("22. getWaypointDX",       p->getWaypointDX().isValid(), false);
    a.checkEqual("23. getWaypointDY",       p->getWaypointDY().isValid(), false);
    a.checkEqual("24. getWarpFactor",       p->getWarpFactor().orElse(-1), 9);
    a.checkEqual("25. getHull",             p->getHull().orElse(-1), 44);
    a.checkEqual("26. getEngineType",       p->getEngineType().isValid(), false);
    a.checkEqual("27. getName",             p->getName(), "MENHUNTER CLASS INTE");
    a.checkEqual("28. getHistoryTimestamp", p->getHistoryTimestamp(game::map::Ship::RestTime), TURN_NR);

    // Store again
    game::db::structures::Ship newShip;
    game::db::Packer(cs).packShip(newShip, *p);

    a.checkEqualContent("31. packShip", afl::base::ConstBytes_t(afl::base::fromObject(ship)), afl::base::ConstBytes_t(afl::base::fromObject(newShip)));
}

AFL_TEST("game.db.Packer:ship:full", a)
{
    const int TURN_NR = 42;
    const int SHIP_ID = 25;

    // These values represent the ship-to-ship/planet transporters.
    // They are 0x00 in the file written by PCC2, but 0xFF after a round-trip through c2ng (without seeing the result file).
    // Set them to 0xFF for the test to be able to do binary comparisons.
    const uint8_t WHAT = 0xFF;

    // Ship data
    static const uint8_t DATA[] = {
        0x19, 0x00, 0x07, 0x00, 0x31, 0x31, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4D, 0x04, 0x3A, 0x09, 0x08, 0x00, 0x0F,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x14, 0x00, 0x00, 0x00, 0x53, 0x61, 0x75, 0x73, 0x73, 0x75, 0x72, 0x69, 0x74, 0x65, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x59, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, WHAT, WHAT, WHAT, WHAT, WHAT,
        WHAT, WHAT, WHAT, WHAT, WHAT, WHAT, WHAT, WHAT, WHAT, WHAT, WHAT, WHAT, WHAT, WHAT, WHAT, WHAT, WHAT, WHAT, WHAT, WHAT,
        WHAT, WHAT, WHAT, 0x00, 0x00, 0x00, 0x00, 0x2A, 0x00, 0x2A, 0x00,
    };
    game::db::structures::Ship ship;
    static_assert(sizeof(ship) == sizeof(DATA), "sizeof Ship");
    afl::base::fromObject(ship).copyFrom(DATA);

    // Ship track entry (current turn)
    static const uint8_t TRACK1[] = {
        0x4D, 0x04, 0x3A, 0x09, 0x00, 0xFF, 0xFF, 0x8B, 0x00
    };
    game::db::structures::ShipTrackEntry track1;
    static_assert(sizeof(track1) == sizeof(TRACK1), "sizeof ShipTrackEntry");
    afl::base::fromObject(track1).copyFrom(TRACK1);

    // Ship track entry (previous turn)
    static const uint8_t TRACK2[] = {
        0x14, 0x04, 0x0A, 0x09, 0x09, 0x31, 0x00, 0x96, 0x00
    };
    game::db::structures::ShipTrackEntry track2;
    static_assert(sizeof(track2) == sizeof(TRACK2), "sizeof ShipTrackEntry");
    afl::base::fromObject(track2).copyFrom(TRACK2);

    // Load the ship
    // This will NOT create the ship, we have to do.
    // We need to add ship track entries to get location/mass info for history ships.
    game::Turn turn;
    for (int i = 1; i < 100; ++i) {
        turn.universe().ships().create(i);
    }
    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
    game::db::Packer(cs).addShip(turn, ship);
    game::db::Packer(cs).addShipTrack(turn, SHIP_ID, TURN_NR,   track1);
    game::db::Packer(cs).addShipTrack(turn, SHIP_ID, TURN_NR-1, track2);

    // Verify
    game::map::Ship* p = turn.universe().ships().get(SHIP_ID);

    a.check("01. ship", p);
    a.checkEqual("02. getId", p->getId(), SHIP_ID);

    // We need the checkers to merge ship track into current data
    p->internalCheck(game::PlayerSet_t(), TURN_NR);

    // Proceed with verification
    int owner;
    a.checkEqual("11. getOwner", p->getOwner().get(owner), true);
    a.checkEqual("12. getOwner", owner, 7);

    a.checkEqual("21. getFriendlyCode",     p->getFriendlyCode().orElse(""), "113");
    a.checkEqual("22. getWaypointDX",       p->getWaypointDX().orElse(1111), 0);
    a.checkEqual("23. getWaypointDY",       p->getWaypointDY().orElse(1111), 0);
    a.checkEqual("24. getWarpFactor",       p->getWarpFactor().orElse(-1), 0);
    a.checkEqual("25. getHull",             p->getHull().orElse(-1), 15);
    a.checkEqual("26. getEngineType",       p->getEngineType().orElse(-1), 8);
    a.checkEqual("27. getName",             p->getName(), "Saussurite");
    a.checkEqual("28. neu",                 p->getCargo(game::Element::Neutronium).orElse(-1), 89);
    a.checkEqual("29. getHistoryTimestamp", p->getHistoryTimestamp(game::map::Ship::RestTime), TURN_NR);
    a.checkEqual("30. getHistoryTimestamp", p->getHistoryTimestamp(game::map::Ship::MilitaryTime), TURN_NR);

    // Store again
    game::db::structures::Ship newShip;
    game::db::Packer(cs).packShip(newShip, *p);

    a.checkEqualContent("31. packShip", afl::base::ConstBytes_t(afl::base::fromObject(ship)), afl::base::ConstBytes_t(afl::base::fromObject(newShip)));
}
