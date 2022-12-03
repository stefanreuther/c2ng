/**
  *  \file u/t_game_db_packer.cpp
  *  \brief Test for game::db::Packer
  */

#include "game/db/packer.hpp"

#include "t_game_db.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "game/map/ufo.hpp"

void
TestGameDbPacker::testUfo()
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
    TS_ASSERT(p);

    TS_ASSERT_EQUALS(p->getId(), 53);
    TS_ASSERT_EQUALS(p->getColorCode(), 2);
    TS_ASSERT_EQUALS(p->getPlainName(), "Wormhole #2");
    TS_ASSERT_EQUALS(p->getInfo1(), "25626 KT/Bidir.");
    TS_ASSERT_EQUALS(p->getInfo2(), "mostly stable (<30%)");

    game::map::Point pt;
    TS_ASSERT_EQUALS(p->getPosition().get(pt), true);
    TS_ASSERT_EQUALS(pt.getX(), 1176);
    TS_ASSERT_EQUALS(pt.getY(), 1369);
    TS_ASSERT_EQUALS(p->getSpeed().orElse(-1), 0);
    TS_ASSERT_EQUALS(p->getHeading().isValid(), false);
    TS_ASSERT_EQUALS(p->getPlanetRange().orElse(-1), 295);
    TS_ASSERT_EQUALS(p->getShipRange().orElse(-1), 295);

    int n;
    TS_ASSERT_EQUALS(p->getRadius().get(n), true);
    TS_ASSERT_EQUALS(n, 6);
    TS_ASSERT_EQUALS(p->getTypeCode().orElse(-1), 1);
    TS_ASSERT_EQUALS(p->getRealId(), 2);

    TS_ASSERT_EQUALS(p->getLastTurn(), 42);
    TS_ASSERT_EQUALS(p->getLastPosition().getX(), 1176);
    TS_ASSERT_EQUALS(p->getLastPosition().getY(), 1369);
    TS_ASSERT_EQUALS(p->getMovementVector().getX(), 0);
    TS_ASSERT_EQUALS(p->getMovementVector().getY(), 0);

    // Store again
    game::db::structures::Ufo newUfo;
    game::db::Packer(cs).packUfo(newUfo, *p);

    TS_ASSERT_SAME_DATA(&ufo, &newUfo, sizeof(ufo));
}

void
TestGameDbPacker::testPlanet()
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

    TS_ASSERT(p);
    TS_ASSERT_EQUALS(p->getId(), 4);

    int owner;
    TS_ASSERT_EQUALS(p->getOwner().get(owner), true);
    TS_ASSERT_EQUALS(owner, 8);

    TS_ASSERT_EQUALS(p->getFriendlyCode().isValid(), false);
    TS_ASSERT_EQUALS(p->getNumBuildings(game::MineBuilding).isValid(), false);
    TS_ASSERT_EQUALS(p->getNumBuildings(game::FactoryBuilding).isValid(), false);
    TS_ASSERT_EQUALS(p->getNumBuildings(game::DefenseBuilding).isValid(), false);
    TS_ASSERT_EQUALS(p->getIndustryLevel(game::HostVersion()).orElse(-1), 3);
    TS_ASSERT_EQUALS(p->getHistoryTimestamp(game::map::Planet::ColonistTime), 42);
    TS_ASSERT_EQUALS(p->isKnownToHaveNatives(), false);

    // Store again
    game::db::structures::Planet newPlanet;
    game::db::Packer(cs).packPlanet(newPlanet, *p);

    TS_ASSERT_SAME_DATA(&planet, &newPlanet, sizeof(planet));
}
void
TestGameDbPacker::testFullPlanet()
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

    TS_ASSERT(p);
    TS_ASSERT_EQUALS(p->getId(), 6);

    int owner;
    TS_ASSERT_EQUALS(p->getOwner().get(owner), true);
    TS_ASSERT_EQUALS(owner, 5);

    TS_ASSERT_EQUALS(p->getFriendlyCode().orElse(""), "9q)");
    TS_ASSERT_EQUALS(p->getNumBuildings(game::MineBuilding).orElse(-1), 2);
    TS_ASSERT_EQUALS(p->getNumBuildings(game::FactoryBuilding).isValid(), false);
    TS_ASSERT_EQUALS(p->getNumBuildings(game::DefenseBuilding).orElse(-1), 3);
    TS_ASSERT_EQUALS(p->getCargo(game::Element::Neutronium).orElse(-1), 0);
    TS_ASSERT_EQUALS(p->getCargo(game::Element::Tritanium).orElse(-1), 15);
    TS_ASSERT_EQUALS(p->getCargo(game::Element::Duranium).orElse(-1), 4);
    TS_ASSERT_EQUALS(p->getCargo(game::Element::Molybdenum).orElse(-1), 15);
    TS_ASSERT_EQUALS(p->getCargo(game::Element::Colonists).orElse(-1), 3);
    TS_ASSERT_EQUALS(p->getCargo(game::Element::Supplies).orElse(-1), 44);
    TS_ASSERT_EQUALS(p->getCargo(game::Element::Money).orElse(-1), 1);
    TS_ASSERT_EQUALS(p->getOreGround(game::Element::Neutronium).orElse(-1), 382);
    TS_ASSERT_EQUALS(p->getOreGround(game::Element::Tritanium).orElse(-1), 265);
    TS_ASSERT_EQUALS(p->getOreGround(game::Element::Duranium).orElse(-1), 282);
    TS_ASSERT_EQUALS(p->getOreGround(game::Element::Molybdenum).orElse(-1), 504);
    TS_ASSERT_EQUALS(p->getOreDensity(game::Element::Neutronium).orElse(-1), 40);
    TS_ASSERT_EQUALS(p->getOreDensity(game::Element::Tritanium).orElse(-1), 91);
    TS_ASSERT_EQUALS(p->getOreDensity(game::Element::Duranium).orElse(-1), 27);
    TS_ASSERT_EQUALS(p->getOreDensity(game::Element::Molybdenum).orElse(-1), 65);
    TS_ASSERT_EQUALS(p->getColonistTax().orElse(-1), 0);
    TS_ASSERT_EQUALS(p->getNativeTax().orElse(-1), 0);
    TS_ASSERT_EQUALS(p->getColonistHappiness().orElse(-1), 100);
    TS_ASSERT_EQUALS(p->getNativeHappiness().orElse(-1), 100);
    TS_ASSERT_EQUALS(p->getNativeGovernment().orElse(-1), 0);
    TS_ASSERT_EQUALS(p->getNatives().orElse(-1), 0);
    TS_ASSERT_EQUALS(p->getNativeRace().orElse(-1), 0);
    TS_ASSERT_EQUALS(p->getTemperature().orElse(-1), 47);

    TS_ASSERT_EQUALS(p->getHistoryTimestamp(game::map::Planet::ColonistTime), 42);
    TS_ASSERT_EQUALS(p->getHistoryTimestamp(game::map::Planet::NativeTime), 42);
    TS_ASSERT_EQUALS(p->getHistoryTimestamp(game::map::Planet::MineralTime), 42);
    TS_ASSERT_EQUALS(p->getHistoryTimestamp(game::map::Planet::CashTime), 42);
    TS_ASSERT_EQUALS(p->isKnownToHaveNatives(), false);

    // Store again
    game::db::structures::Planet newPlanet;
    game::db::Packer(cs).packPlanet(newPlanet, *p);

    TS_ASSERT_SAME_DATA(&planet, &newPlanet, sizeof(planet));
}

void
TestGameDbPacker::testShip()
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

    TS_ASSERT(p);
    TS_ASSERT_EQUALS(p->getId(), SHIP_ID);

    // We need the checkers to merge ship track into current data
    p->internalCheck();
    p->combinedCheck1(turn.universe(), game::PlayerSet_t(), TURN_NR);

    // Proceed with verification
    int owner;
    TS_ASSERT_EQUALS(p->getOwner().get(owner), true);
    TS_ASSERT_EQUALS(owner, 5);

    TS_ASSERT_EQUALS(p->getFriendlyCode().isValid(), false);
    TS_ASSERT_EQUALS(p->getWaypointDX().isValid(), false);
    TS_ASSERT_EQUALS(p->getWaypointDY().isValid(), false);
    TS_ASSERT_EQUALS(p->getWarpFactor().orElse(-1), 9);
    TS_ASSERT_EQUALS(p->getHull().orElse(-1), 44);
    TS_ASSERT_EQUALS(p->getEngineType().isValid(), false);
    TS_ASSERT_EQUALS(p->getName(), "MENHUNTER CLASS INTE");
    TS_ASSERT_EQUALS(p->getHistoryTimestamp(game::map::Ship::RestTime), TURN_NR);

    // Store again
    game::db::structures::Ship newShip;
    game::db::Packer(cs).packShip(newShip, *p);

    TS_ASSERT_SAME_DATA(&ship, &newShip, sizeof(ship));
}

void
TestGameDbPacker::testFullShip()
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

    TS_ASSERT(p);
    TS_ASSERT_EQUALS(p->getId(), SHIP_ID);

    // We need the checkers to merge ship track into current data
    p->internalCheck();
    p->combinedCheck1(turn.universe(), game::PlayerSet_t(), TURN_NR);

    // Proceed with verification
    int owner;
    TS_ASSERT_EQUALS(p->getOwner().get(owner), true);
    TS_ASSERT_EQUALS(owner, 7);

    TS_ASSERT_EQUALS(p->getFriendlyCode().orElse(""), "113");
    TS_ASSERT_EQUALS(p->getWaypointDX().orElse(1111), 0);
    TS_ASSERT_EQUALS(p->getWaypointDY().orElse(1111), 0);
    TS_ASSERT_EQUALS(p->getWarpFactor().orElse(-1), 0);
    TS_ASSERT_EQUALS(p->getHull().orElse(-1), 15);
    TS_ASSERT_EQUALS(p->getEngineType().orElse(-1), 8);
    TS_ASSERT_EQUALS(p->getName(), "Saussurite");
    TS_ASSERT_EQUALS(p->getCargo(game::Element::Neutronium).orElse(-1), 89);
    TS_ASSERT_EQUALS(p->getHistoryTimestamp(game::map::Ship::RestTime), TURN_NR);
    TS_ASSERT_EQUALS(p->getHistoryTimestamp(game::map::Ship::MilitaryTime), TURN_NR);

    // Store again
    game::db::structures::Ship newShip;
    game::db::Packer(cs).packShip(newShip, *p);

    TS_ASSERT_SAME_DATA(&ship, &newShip, sizeof(ship));
}

