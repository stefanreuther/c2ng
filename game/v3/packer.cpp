/**
  *  \file game/v3/packer.cpp
  *  \brief Class game::v3::Packer
  */

#include "game/v3/packer.hpp"

namespace {
    void unpackTransfer(game::map::ShipData::Transfer& out, const game::v3::structures::ShipTransfer& in)
    {
        // ex ccmain.pas:FrobShipRecord (part)
        out.neutronium = in.ore[game::v3::structures::Neutronium];
        out.tritanium  = in.ore[game::v3::structures::Tritanium];
        out.duranium   = in.ore[game::v3::structures::Duranium];
        out.molybdenum = in.ore[game::v3::structures::Molybdenum];
        out.colonists  = in.colonists;
        out.supplies   = in.supplies;

        // Handle dull transfers: sometimes a transfer has a nonempty target,
        // but no content, which can confuse things.
        if (in.ore[game::v3::structures::Neutronium] == 0
            && in.ore[game::v3::structures::Tritanium] == 0
            && in.ore[game::v3::structures::Duranium] == 0
            && in.ore[game::v3::structures::Molybdenum] == 0
            && in.colonists == 0
            && in.supplies == 0)
        {
            out.targetId = 0;
        } else {
            out.targetId = in.targetId;
        }
    }

    void copyOut(game::v3::structures::Int16_t& out, const game::IntegerProperty_t& in)
    {
        out = static_cast<int16_t>(in.orElse(-1));
    }

    void copyOut(game::v3::structures::Int32_t& out, const game::LongProperty_t& in)
    {
        out = static_cast<int32_t>(in.orElse(-1));
    }

    void copyOut(game::v3::structures::Int16_t& out, const game::NegativeProperty_t& in)
    {
        out = static_cast<int16_t>(in.orElse(0x8000));
    }

    void packTransfer(game::v3::structures::ShipTransfer& out, const game::map::ShipData::Transfer& in)
    {
        copyOut(out.ore[game::v3::structures::Neutronium], in.neutronium);
        copyOut(out.ore[game::v3::structures::Tritanium], in.tritanium);
        copyOut(out.ore[game::v3::structures::Duranium], in.duranium);
        copyOut(out.ore[game::v3::structures::Molybdenum], in.molybdenum);
        copyOut(out.colonists, in.colonists);
        copyOut(out.supplies, in.supplies);
        copyOut(out.targetId, in.targetId);
    }
}

// Constructor.
game::v3::Packer::Packer(afl::charset::Charset& cs)
    : m_charset(cs)
{ }

// Unpack a ship.
void
game::v3::Packer::unpackShip(game::map::ShipData& out, const game::v3::structures::Ship& in, bool remapExplore)
{
    // FIXME: must validate data so we don't accidentally see an unknown value
    out.owner               = in.owner;
    out.friendlyCode        = m_charset.decode(in.friendlyCode);
    out.warpFactor          = std::max(0, int(in.warpFactor));      // Lizard ships with >100% damage have negative warp
    out.waypointDX          = in.waypointDX;
    out.waypointDY          = in.waypointDY;
    out.x                   = in.x;
    out.y                   = in.y;
    out.engineType          = in.engineType;
    out.hullType            = in.hullType;
    out.beamType            = in.beamType;
    out.numBeams            = in.numBeams;
    out.numBays             = in.numBays;
    out.launcherType        = in.launcherType;
    out.ammo                = in.ammo;
    out.numLaunchers        = in.numLaunchers;
    out.mission             = in.mission;
    out.primaryEnemy        = in.primaryEnemy;
    out.missionTowParameter = in.missionTowParameter;
    out.damage              = in.damage;
    out.crew                = in.crew;
    out.colonists           = in.colonists;
    out.name                = m_charset.decode(in.name);
    out.neutronium          = in.ore[structures::Neutronium];
    out.tritanium           = in.ore[structures::Tritanium];
    out.duranium            = in.ore[structures::Duranium];
    out.molybdenum          = in.ore[structures::Molybdenum];
    out.supplies            = in.supplies;
    unpackTransfer(out.unload, in.unload);
    unpackTransfer(out.transfer, in.transfer);
    out.missionInterceptParameter = in.missionInterceptParameter;
    out.money               = in.money;

    // In SRace, mission 1 means "special".
    if (remapExplore && out.mission.isSame(1)) {
        out.mission = 9;
    }
}

// Unpack a planet.
void
game::v3::Packer::unpackPlanet(game::map::PlanetData& out, const game::v3::structures::Planet& in)
{
    // FIXME: must validate data so we don't accidentally see an unknown value
    out.owner             = in.owner;
    out.friendlyCode      = m_charset.decode(in.friendlyCode);
    out.numMines          = in.numMines;
    out.numFactories      = in.numFactories;
    out.numDefensePosts   = in.numDefensePosts;
    out.minedNeutronium   = in.minedOre[structures::Neutronium];
    out.minedTritanium    = in.minedOre[structures::Tritanium];
    out.minedDuranium     = in.minedOre[structures::Duranium];
    out.minedMolybdenum   = in.minedOre[structures::Molybdenum];
    out.colonistClans     = in.colonists;
    out.supplies          = in.supplies;
    out.money             = in.money;
    out.groundNeutronium  = in.groundOre[structures::Neutronium];
    out.groundTritanium   = in.groundOre[structures::Tritanium];
    out.groundDuranium    = in.groundOre[structures::Duranium];
    out.groundMolybdenum  = in.groundOre[structures::Molybdenum];
    out.densityNeutronium = in.oreDensity[structures::Neutronium];
    out.densityTritanium  = in.oreDensity[structures::Tritanium];
    out.densityDuranium   = in.oreDensity[structures::Duranium];
    out.densityMolybdenum = in.oreDensity[structures::Molybdenum];
    out.colonistTax       = in.colonistTax;
    out.nativeTax         = in.nativeTax;
    out.colonistHappiness = in.colonistHappiness;
    out.nativeHappiness   = in.nativeHappiness;
    out.nativeGovernment  = in.nativeGovernment;
    out.nativeClans       = in.natives;
    out.nativeRace        = in.nativeRace;
    out.temperature       = 100 - in.temperatureCode;
    out.baseFlag          = in.buildBaseFlag;
}

// Unpack a starbase.
void
game::v3::Packer::unpackBase(game::map::BaseData& out, const game::v3::structures::Base& in)
{
    // FIXME: must validate data so we don't accidentally see an unknown value
    out.numBaseDefensePosts = in.numBaseDefensePosts;
    out.damage              = in.damage;

    for (size_t i = 0; i < NUM_TECH_AREAS; ++i) {
        out.techLevels[i] = in.techLevels[i];
    }

    // Arrays
    for (int i = 1; i <= structures::NUM_ENGINE_TYPES; ++i) {
        out.engineStorage.set(i, int(in.engineStorage[i-1]));
    }
    for (int i = 1; i <= structures::NUM_HULLS_PER_PLAYER; ++i) {
        out.hullStorage.set(i, int(in.hullStorage[i-1]));
    }
    for (int i = 1; i <= structures::NUM_BEAM_TYPES; ++i) {
        out.beamStorage.set(i, int(in.beamStorage[i-1]));
    }
    for (int i = 1; i <= structures::NUM_TORPEDO_TYPES; ++i) {
        out.launcherStorage.set(i, int(in.launcherStorage[i-1]));
    }
    for (int i = 1; i <= structures::NUM_TORPEDO_TYPES; ++i) {
        out.torpedoStorage.set(i, int(in.torpedoStorage[i-1]));
    }

    out.numFighters    = in.numFighters;
    out.shipyardId     = in.shipyardId;
    out.shipyardAction = in.shipyardAction;
    out.mission        = in.mission;

    out.shipBuildOrder.setHullIndex(in.shipBuildOrder.hullIndex);
    out.shipBuildOrder.setEngineType(in.shipBuildOrder.engineType);
    out.shipBuildOrder.setBeamType(in.shipBuildOrder.beamType);
    out.shipBuildOrder.setNumBeams(in.shipBuildOrder.numBeams);
    out.shipBuildOrder.setLauncherType(in.shipBuildOrder.launcherType);
    out.shipBuildOrder.setNumLaunchers(in.shipBuildOrder.numLaunchers);
}

// Pack a ship.
void
game::v3::Packer::packShip(game::v3::structures::Ship& out, int id, const game::map::ShipData& in, bool remapExplore)
{
    // ex GShip::getShipData (sort-of)
    out.shipId = static_cast<int16_t>(id);

    copyOut(out.owner, in.owner);
    if (const String_t* fc = in.friendlyCode.get()) {
        out.friendlyCode = m_charset.encode(afl::string::toMemory(*fc));
    } else {
        afl::base::Bytes_t(out.friendlyCode.m_bytes).fill(0xFF);
    }
    copyOut(out.warpFactor, in.warpFactor);
    copyOut(out.waypointDX, in.waypointDX);
    copyOut(out.waypointDY, in.waypointDY);
    copyOut(out.x, in.x);
    copyOut(out.y, in.y);
    copyOut(out.engineType, in.engineType);
    copyOut(out.hullType, in.hullType);
    copyOut(out.beamType, in.beamType);
    copyOut(out.numBeams, in.numBeams);
    copyOut(out.numBays, in.numBays);
    copyOut(out.launcherType, in.launcherType);
    copyOut(out.ammo, in.ammo);
    copyOut(out.numLaunchers, in.numLaunchers);

    if (remapExplore && in.mission.isSame(1)) {
        out.mission = 0;
    } else if (remapExplore && in.mission.isSame(9)) {
        out.mission = 1;
    } else {
        copyOut(out.mission, in.mission);
    }
    copyOut(out.primaryEnemy, in.primaryEnemy);
    copyOut(out.missionTowParameter, in.missionTowParameter);
    copyOut(out.damage, in.damage);
    copyOut(out.crew, in.crew);
    copyOut(out.colonists, in.colonists);
    out.name = m_charset.encode(afl::string::toMemory(in.name.orElse("")));
    copyOut(out.ore[structures::Neutronium], in.neutronium);
    copyOut(out.ore[structures::Tritanium], in.tritanium);
    copyOut(out.ore[structures::Duranium], in.duranium);
    copyOut(out.ore[structures::Molybdenum], in.molybdenum);
    copyOut(out.supplies, in.supplies);
    packTransfer(out.unload, in.unload);
    packTransfer(out.transfer, in.transfer);
    copyOut(out.missionInterceptParameter, in.missionInterceptParameter);
    copyOut(out.money, in.money);
}

// Pack a planet.
void
game::v3::Packer::packPlanet(game::v3::structures::Planet& out, int id, const game::map::PlanetData& in)
{
    int temp;
    copyOut(out.owner, in.owner);
    out.planetId = static_cast<int16_t>(id);

    if (const String_t* fc = in.friendlyCode.get()) {
        out.friendlyCode = m_charset.encode(afl::string::toMemory(*fc));
    } else {
        afl::base::Bytes_t(out.friendlyCode.m_bytes).fill(0xFF);
    }
    copyOut(out.numMines,                           in.numMines);
    copyOut(out.numFactories,                       in.numFactories);
    copyOut(out.numDefensePosts,                    in.numDefensePosts);
    copyOut(out.minedOre[structures::Neutronium],   in.minedNeutronium);
    copyOut(out.minedOre[structures::Tritanium],    in.minedTritanium);
    copyOut(out.minedOre[structures::Duranium],     in.minedDuranium);
    copyOut(out.minedOre[structures::Molybdenum],   in.minedMolybdenum);
    copyOut(out.colonists,                          in.colonistClans);
    copyOut(out.supplies,                           in.supplies);
    copyOut(out.money,                              in.money);
    copyOut(out.groundOre[structures::Neutronium],  in.groundNeutronium);
    copyOut(out.groundOre[structures::Tritanium],   in.groundTritanium);
    copyOut(out.groundOre[structures::Duranium],    in.groundDuranium);
    copyOut(out.groundOre[structures::Molybdenum],  in.groundMolybdenum);
    copyOut(out.oreDensity[structures::Neutronium], in.densityNeutronium);
    copyOut(out.oreDensity[structures::Tritanium],  in.densityTritanium);
    copyOut(out.oreDensity[structures::Duranium],   in.densityDuranium);
    copyOut(out.oreDensity[structures::Molybdenum], in.densityMolybdenum);
    copyOut(out.colonistTax,                        in.colonistTax);
    copyOut(out.nativeTax,                          in.nativeTax);
    copyOut(out.colonistHappiness,                  in.colonistHappiness);
    copyOut(out.nativeHappiness,                    in.nativeHappiness);
    copyOut(out.nativeGovernment,                   in.nativeGovernment);
    copyOut(out.natives,                            in.nativeClans);
    copyOut(out.nativeRace,                         in.nativeRace);

    if (in.temperature.get(temp)) {
        out.temperatureCode = static_cast<int16_t>(100 - temp);
    } else {
        out.temperatureCode = -1;
    }
    copyOut(out.buildBaseFlag, in.baseFlag);
}

// Pack a starbase.
void
game::v3::Packer::packBase(game::v3::structures::Base& out, int id, const game::map::BaseData& in, int owner)
{
    out.baseId = static_cast<int16_t>(id);
    out.owner = static_cast<int16_t>(owner);
    copyOut(out.numBaseDefensePosts, in.numBaseDefensePosts);
    copyOut(out.damage, in.damage);
    for (size_t i = 0; i < NUM_TECH_AREAS; ++i) {
        copyOut(out.techLevels[i], in.techLevels[i]);
    }

    // Arrays
    for (int i = 1; i <= structures::NUM_ENGINE_TYPES; ++i) {
        copyOut(out.engineStorage[i-1], in.engineStorage.get(i));
    }
    for (int i = 1; i <= structures::NUM_HULLS_PER_PLAYER; ++i) {
        copyOut(out.hullStorage[i-1], in.hullStorage.get(i));
    }
    for (int i = 1; i <= structures::NUM_BEAM_TYPES; ++i) {
        copyOut(out.beamStorage[i-1], in.beamStorage.get(i));
    }
    for (int i = 1; i <= structures::NUM_TORPEDO_TYPES; ++i) {
        copyOut(out.launcherStorage[i-1], in.launcherStorage.get(i));
    }
    for (int i = 1; i <= structures::NUM_TORPEDO_TYPES; ++i) {
        copyOut(out.torpedoStorage[i-1], in.torpedoStorage.get(i));
    }

    copyOut(out.numFighters, in.numFighters);
    copyOut(out.shipyardId, in.shipyardId);
    copyOut(out.shipyardAction, in.shipyardAction);
    copyOut(out.mission, in.mission);

    out.shipBuildOrder.hullIndex    = static_cast<int16_t>(in.shipBuildOrder.getHullIndex());
    out.shipBuildOrder.engineType   = static_cast<int16_t>(in.shipBuildOrder.getEngineType());
    out.shipBuildOrder.beamType     = static_cast<int16_t>(in.shipBuildOrder.getBeamType());
    out.shipBuildOrder.numBeams     = static_cast<int16_t>(in.shipBuildOrder.getNumBeams());
    out.shipBuildOrder.launcherType = static_cast<int16_t>(in.shipBuildOrder.getLauncherType());
    out.shipBuildOrder.numLaunchers = static_cast<int16_t>(in.shipBuildOrder.getNumLaunchers());
    out.shipBuildOrder.zero = 0;
}
