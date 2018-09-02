/**
  *  \file game/db/packer.cpp
  */

#include "game/db/packer.hpp"
#include "game/map/planet.hpp"
#include "game/map/ufo.hpp"
#include "game/map/universe.hpp"
#include "game/parser/messageinformation.hpp"
#include "game/map/ship.hpp"
#include "game/v3/packer.hpp"

namespace gp = game::parser;
namespace gm = game::map;
namespace gt = game::v3::structures;
namespace dt = game::db::structures;

namespace {
    const int16_t UnknownInteger = -1;
    const int16_t UnknownNegative = int16_t(0x8000);
    const int32_t UnknownLong = -1;


    void addValueMaybe(gp::MessageInformation& info,
                       gp::MessageIntegerIndex ii,
                       int32_t value,
                       int32_t unknownMarker)
    {
        if (value != unknownMarker) {
            info.addValue(ii, value);
        }
    }
}

game::db::Packer::Packer(Turn& turn, afl::charset::Charset& cs)
    : m_turn(turn),
      m_charset(cs)
{ }

void
game::db::Packer::addUfo(const structures::Ufo& ufo)
{
    // Add Ufo through message interface instead of directly setting properties.
    // This allows it to reject obsolete data.
    gp::MessageInformation info(gp::MessageInformation::Ufo, ufo.id, ufo.turnLastSeen);

    // Scalars
    // FIXME: deal with -1 values?
    info.addValue(gp::mi_UfoColor,       ufo.ufo.color);
    info.addValue(gp::mi_UfoRealId,      ufo.realId);
    info.addValue(gp::mi_Speed,          ufo.ufo.warpFactor);
    if (ufo.ufo.heading >= 0) {
        info.addValue(gp::mi_Heading,    ufo.ufo.heading);
    }
    info.addValue(gp::mi_UfoShipRange,   ufo.ufo.shipRange);
    info.addValue(gp::mi_UfoPlanetRange, ufo.ufo.planetRange);
    info.addValue(gp::mi_Radius,         ufo.ufo.radius);
    info.addValue(gp::mi_Type,           ufo.ufo.typeCode);

    // Strings
    info.addValue(gp::ms_Name,           m_charset.decode(ufo.ufo.name));
    info.addValue(gp::ms_UfoInfo1,       m_charset.decode(ufo.ufo.info1));
    info.addValue(gp::ms_UfoInfo2,       m_charset.decode(ufo.ufo.info2));

    // Pairs (coordinates).
    // We map xLastSeen,yLastSeen to X,Y, because that matches the turnLastSeen.
    // The x,y fields correspond to whatever turn the Ufo is seen.
    info.addValue(gp::mi_X, ufo.xLastSeen);
    info.addValue(gp::mi_Y, ufo.yLastSeen);
    info.addValue(gp::mi_UfoSpeedX, ufo.speedX);
    info.addValue(gp::mi_UfoSpeedY, ufo.speedY);

    // Add it
    if (gm::Ufo* pUfo = m_turn.universe().ufos().addUfo(ufo.id, ufo.ufo.typeCode, ufo.ufo.color)) {
        pUfo->addMessageInformation(info);
        pUfo->setIsStoredInHistory(true);
    }
}

void
game::db::Packer::addPlanet(const structures::Planet& planet)
{
    // ex GPlanet::addHistoryData (remotely related)
    // Fetch the planet
    const int id = planet.planet.planetId;
    gm::Planet*const p = m_turn.universe().planets().get(id);
    if (p == 0) {
        return;
    }

    // Temperature is added to both Colonists and Natives batch.
    // It will be checked against timestamps, but not for itself update the timestamp.
    const int rawTemp = planet.planet.temperatureCode;

    // Colonists
    {
        gp::MessageInformation info(gp::MessageInformation::Planet, id, planet.turn[structures::PlanetColonists]);
        if (planet.planet.friendlyCode.m_bytes[0] != 0xFF) {
            info.addValue(gp::ms_FriendlyCode, m_charset.decode(planet.planet.friendlyCode));
        }

        // Factories can also mean just a level
        int factoriesOrLevel = planet.planet.numFactories;
        if (factoriesOrLevel >= 30000) {
            info.addValue(gp::mi_PlanetActivity, factoriesOrLevel - 30000);
        } else {
            addValueMaybe(info, gp::mi_PlanetMines,     planet.planet.numMines,     UnknownInteger);
            addValueMaybe(info, gp::mi_PlanetFactories, planet.planet.numFactories, UnknownInteger);
        }

        addValueMaybe(info, gp::mi_Owner,                   planet.planet.owner,             UnknownInteger);
        addValueMaybe(info, gp::mi_PlanetDefense,           planet.planet.numDefensePosts,   UnknownInteger);
        addValueMaybe(info, gp::mi_PlanetHasBase,           planet.planet.buildBaseFlag,     UnknownInteger);
        addValueMaybe(info, gp::mi_PlanetColonists,         planet.planet.colonists,         UnknownLong);
        addValueMaybe(info, gp::mi_PlanetColonistTax,       planet.planet.colonistTax,       UnknownInteger);
        addValueMaybe(info, gp::mi_PlanetColonistHappiness, planet.planet.colonistHappiness, UnknownNegative);
        if (rawTemp >= 0) {
            info.addValue(gp::mi_PlanetTemperature, 100 - rawTemp);
        }
        p->addMessageInformation(info);
    }

    // Minerals
    {
        gp::MessageInformation info(gp::MessageInformation::Planet, id, planet.turn[structures::PlanetMinerals]);
        addValueMaybe(info, gp::mi_PlanetTotalN,   planet.planet.groundOre[gt::Neutronium],  UnknownLong);
        addValueMaybe(info, gp::mi_PlanetTotalT,   planet.planet.groundOre[gt::Tritanium],   UnknownLong);
        addValueMaybe(info, gp::mi_PlanetTotalD,   planet.planet.groundOre[gt::Duranium],    UnknownLong);
        addValueMaybe(info, gp::mi_PlanetTotalM,   planet.planet.groundOre[gt::Molybdenum],  UnknownLong);
        addValueMaybe(info, gp::mi_PlanetMinedN,   planet.planet.minedOre[gt::Neutronium],   UnknownLong);
        addValueMaybe(info, gp::mi_PlanetMinedT,   planet.planet.minedOre[gt::Tritanium],    UnknownLong);
        addValueMaybe(info, gp::mi_PlanetMinedD,   planet.planet.minedOre[gt::Duranium],     UnknownLong);
        addValueMaybe(info, gp::mi_PlanetMinedM,   planet.planet.minedOre[gt::Molybdenum],   UnknownLong);
        addValueMaybe(info, gp::mi_PlanetDensityN, planet.planet.oreDensity[gt::Neutronium], UnknownInteger);
        addValueMaybe(info, gp::mi_PlanetDensityT, planet.planet.oreDensity[gt::Tritanium],  UnknownInteger);
        addValueMaybe(info, gp::mi_PlanetDensityD, planet.planet.oreDensity[gt::Duranium],   UnknownInteger);
        addValueMaybe(info, gp::mi_PlanetDensityM, planet.planet.oreDensity[gt::Molybdenum], UnknownInteger);
        p->addMessageInformation(info);
    }

    // Cash
    {
        gp::MessageInformation info(gp::MessageInformation::Planet, id, planet.turn[structures::PlanetCash]);
        addValueMaybe(info, gp::mi_PlanetCash,     planet.planet.money,    UnknownLong);
        addValueMaybe(info, gp::mi_PlanetSupplies, planet.planet.supplies, UnknownLong);
        p->addMessageInformation(info);
    }

    // Natives
    {
        gp::MessageInformation info(gp::MessageInformation::Planet, id, planet.turn[structures::PlanetNatives]);
        addValueMaybe(info, gp::mi_PlanetNativeRace,      planet.planet.nativeRace,       UnknownInteger);
        addValueMaybe(info, gp::mi_PlanetNativeGov,       planet.planet.nativeGovernment, UnknownInteger);
        addValueMaybe(info, gp::mi_PlanetNatives,         planet.planet.natives,          UnknownLong);
        addValueMaybe(info, gp::mi_PlanetNativeTax,       planet.planet.nativeTax,        UnknownInteger);
        addValueMaybe(info, gp::mi_PlanetNativeHappiness, planet.planet.nativeHappiness,  UnknownNegative);
        if (planet.knownToHaveNatives != 0) {
            info.addValue(gp::mi_PlanetHasNatives, 1);
        }
        if (rawTemp >= 0) {
            info.addValue(gp::mi_PlanetTemperature, 100 - rawTemp);
        }
        p->addMessageInformation(info);
    }
}

void
game::db::Packer::addShip(const structures::Ship& ship)
{
    // ex GPlanet::addHistoryData (remotely related)
    /*
     *  Note: since we're using the addMessageInformation() interface, this will NOT restore the unload/transfer orders.
     *  These have no mapping to MessageInformation, and because they're pretty useless as history information,
     *  I didn't bother adding them.
     */
    /*
     *  This does NOT unpack X,Y,speed.
     *  Those are handled by addShipTrack().
     */
    // Fetch the ship
    const int id = ship.ship.shipId;
    gm::Ship*const sh = m_turn.universe().ships().get(id);
    if (sh == 0) {
        return;
    }

    // Military
    {
        gp::MessageInformation info(gp::MessageInformation::Ship, id, ship.turn[structures::ShipArmsDamage]);
        addValueMaybe(info, gp::mi_ShipBeamType,     ship.ship.beamType,     -1);
        addValueMaybe(info, gp::mi_ShipNumBeams,     ship.ship.numBeams,     -1);
        addValueMaybe(info, gp::mi_ShipNumBays,      ship.ship.numBays,      -1);
        addValueMaybe(info, gp::mi_ShipLauncherType, ship.ship.launcherType, -1);
        addValueMaybe(info, gp::mi_ShipAmmo,         ship.ship.ammo,         -1);
        addValueMaybe(info, gp::mi_ShipNumLaunchers, ship.ship.numLaunchers, -1);
        addValueMaybe(info, gp::mi_Damage,           ship.ship.damage,       -1);
        addValueMaybe(info, gp::mi_ShipCrew,         ship.ship.crew,         -1);
        sh->addMessageInformation(info, PlayerSet_t());
    }

    // Rest
    {
        gp::MessageInformation info(gp::MessageInformation::Ship, id, ship.turn[structures::ShipRest]);
        if (ship.ship.friendlyCode.m_bytes[0] != 0xFF) {
            info.addValue(gp::ms_FriendlyCode, m_charset.decode(ship.ship.friendlyCode));
        }
        if (ship.ship.name.m_bytes[0] != 0xFF) {
            info.addValue(gp::ms_Name, m_charset.decode(ship.ship.name));
        }
        addValueMaybe(info, gp::mi_Owner,          ship.ship.owner,                     -1);
        addValueMaybe(info, gp::mi_ShipWaypointDX, ship.ship.waypointDX,                0x8000);
        addValueMaybe(info, gp::mi_ShipWaypointDY, ship.ship.waypointDY,                0x8000);
        addValueMaybe(info, gp::mi_ShipEngineType, ship.ship.engineType,                -1);
        addValueMaybe(info, gp::mi_ShipHull,       ship.ship.hullType,                  -1);
        addValueMaybe(info, gp::mi_ShipMission,    ship.ship.mission,                   -1);
        addValueMaybe(info, gp::mi_ShipEnemy,      ship.ship.primaryEnemy,              -1);
        addValueMaybe(info, gp::mi_ShipTow,        ship.ship.missionTowParameter,       -1);
        addValueMaybe(info, gp::mi_ShipColonists,  ship.ship.colonists,                 -1);
        addValueMaybe(info, gp::mi_ShipFuel,       ship.ship.ore[gt::Neutronium],       -1);
        addValueMaybe(info, gp::mi_ShipCargoT,     ship.ship.ore[gt::Tritanium],        -1);
        addValueMaybe(info, gp::mi_ShipCargoD,     ship.ship.ore[gt::Duranium],         -1);
        addValueMaybe(info, gp::mi_ShipCargoM,     ship.ship.ore[gt::Molybdenum],       -1);
        addValueMaybe(info, gp::mi_ShipSupplies,   ship.ship.supplies,                  -1);
        addValueMaybe(info, gp::mi_ShipIntercept,  ship.ship.missionInterceptParameter, -1);
        addValueMaybe(info, gp::mi_ShipMoney,      ship.ship.money,                     -1);
        sh->addMessageInformation(info, PlayerSet_t());
    }
}

void
game::db::Packer::addShipTrack(int id, int turn, const structures::ShipTrackEntry& entry)
{
    gm::Ship*const sh = m_turn.universe().ships().get(id);
    if (sh == 0) {
        return;
    }

    // Create it
    gp::MessageInformation info(gp::MessageInformation::Ship, id, turn);
    addValueMaybe(info, gp::mi_X,        entry.x,       -1);
    addValueMaybe(info, gp::mi_Y,        entry.y,       -1);
    addValueMaybe(info, gp::mi_Speed,    entry.speed,   -1);
    addValueMaybe(info, gp::mi_Heading,  entry.heading, -1);
    addValueMaybe(info, gp::mi_Mass,     entry.mass,    -1);
    sh->addMessageInformation(info, PlayerSet_t());
}

void
game::db::Packer::packUfo(structures::Ufo& out, const game::map::Ufo& in)
{
    // ex GUfo::getHistoryData
    // Read position (will not fail).
    gm::Point pos;
    in.getPosition(pos);

    int radius = 0;
    in.getRadius(radius);

    // Populate structure
    out.id              = static_cast<int16_t>(in.getId());
    out.ufo.color       = static_cast<int16_t>(in.getColorCode());
    out.ufo.name        = m_charset.encode(afl::string::toMemory(in.getPlainName()));
    out.ufo.info1       = m_charset.encode(afl::string::toMemory(in.getInfo1()));
    out.ufo.info2       = m_charset.encode(afl::string::toMemory(in.getInfo2()));
    out.ufo.x           = static_cast<int16_t>(pos.getX());
    out.ufo.y           = static_cast<int16_t>(pos.getY());
    out.ufo.warpFactor  = static_cast<int16_t>(in.getSpeed().orElse(-1));
    out.ufo.heading     = static_cast<int16_t>(in.getHeading().orElse(-1));
    out.ufo.planetRange = static_cast<int16_t>(in.getPlanetRange().orElse(-1));
    out.ufo.shipRange   = static_cast<int16_t>(in.getShipRange().orElse(-1));
    out.ufo.radius      = static_cast<int16_t>(radius);
    out.ufo.typeCode    = static_cast<int16_t>(in.getTypeCode().orElse(-1));
    out.realId          = in.getRealId();
    out.turnLastSeen    = static_cast<int16_t>(in.getLastTurn());
    out.xLastSeen       = static_cast<int16_t>(in.getLastPosition().getX());
    out.yLastSeen       = static_cast<int16_t>(in.getLastPosition().getY());
    out.speedX          = static_cast<int16_t>(in.getMovementVector().getX());
    out.speedY          = static_cast<int16_t>(in.getMovementVector().getY());
}

void
game::db::Packer::packPlanet(structures::Planet& out, const game::map::Planet& in)
{
    // ex GPlanet::getHistoryData
    // Pack planet using v3 packer
    gm::PlanetData data;
    in.getCurrentPlanetData(data);
    game::v3::Packer(m_charset).packPlanet(out.planet, in.getId(), data);

    // Industry level
    if (!data.numFactories.isValid()) {
        IntegerProperty_t level = in.getIndustryLevel(HostVersion());
        if (level.isValid()) {
            out.planet.numFactories = static_cast<int16_t>(level.orElse(0) + 30000);
        }
    }

    // Known-to-have-natives flag
    // FIXME: reconsider? This differes from PCC2. PCC writes the raw "isKnownToHaveNatives" flag,
    // we write the processed value.
    out.knownToHaveNatives = in.isKnownToHaveNatives();

    // Base flag
    if (in.hasBase()) {
        out.planet.buildBaseFlag = 1;
    }

    // Timestamps
    out.turn[dt::PlanetMinerals]  = static_cast<int16_t>(in.getHistoryTimestamp(gm::Planet::MineralTime));
    out.turn[dt::PlanetColonists] = static_cast<int16_t>(in.getHistoryTimestamp(gm::Planet::ColonistTime));
    out.turn[dt::PlanetNatives]   = static_cast<int16_t>(in.getHistoryTimestamp(gm::Planet::NativeTime));
    out.turn[dt::PlanetCash]      = static_cast<int16_t>(in.getHistoryTimestamp(gm::Planet::CashTime));
}

void
game::db::Packer::packShip(structures::Ship& out, const game::map::Ship& in)
{
    // ex GShip::getShipHistoryData
    gm::ShipData data;
    in.getCurrentShipData(data);
    game::v3::Packer(m_charset).packShip(out.ship, in.getId(), data, false);

    // Timestamp
    out.turn[dt::ShipArmsDamage] = static_cast<int16_t>(in.getHistoryTimestamp(gm::Ship::MilitaryTime));
    out.turn[dt::ShipRest]       = static_cast<int16_t>(in.getHistoryTimestamp(gm::Ship::RestTime));
}

