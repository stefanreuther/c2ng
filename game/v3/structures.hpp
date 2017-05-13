/**
  *  \file game/v3/structures.hpp
  *  \brief v3 Structures
  */
#ifndef C2NG_GAME_V3_STRUCTURES_HPP
#define C2NG_GAME_V3_STRUCTURES_HPP

#include "afl/base/staticassert.hpp"
#include "afl/bits/fixedstring.hpp"
#include "afl/bits/int16le.hpp"
#include "afl/bits/int32le.hpp"
#include "afl/bits/uint16le.hpp"
#include "afl/bits/uint32le.hpp"
#include "afl/bits/value.hpp"

/** \namespace game::v3::structures
    \brief v3 Structure Definitions

    This namespace defines all binary structures used to build and parse regular ("v3") data files,
    including our local files such as "chartX.cc". */

namespace game { namespace v3 { namespace structures {

    typedef afl::bits::Value<afl::bits::UInt16LE> UInt16_t;
    typedef afl::bits::Value<afl::bits::Int16LE> Int16_t;
    typedef afl::bits::Value<afl::bits::UInt32LE> UInt32_t;
    typedef afl::bits::Value<afl::bits::Int32LE> Int32_t;
    typedef afl::bits::Value<afl::bits::FixedString<3> > String3_t;
    typedef afl::bits::Value<afl::bits::FixedString<12> > String12_t;
    typedef afl::bits::Value<afl::bits::FixedString<20> > String20_t;
    typedef afl::bits::Value<afl::bits::FixedString<25> > String25_t;
    typedef afl::bits::Value<afl::bits::FixedString<30> > String30_t;
    typedef afl::bits::Value<afl::bits::FixedString<50> > String50_t;


    /** Manifest Constants. */
    const int NUM_BEAM_TYPES = 10;                          ///< Number of beams in BEAMSPEC.
    const int NUM_TORPEDO_TYPES = 10;                       ///< Number of torpedoes in TORPSPEC.
    const int NUM_ENGINE_TYPES = 9;                         ///< Number of engines in ENGSPEC.
    const int NUM_WARP_FACTORS = 9;                         ///< Number of warp factors.

    const int NUM_SHIPS   = 999;              ///< Maximum number of ships.
    const int NUM_PLANETS = 500;              ///< Maximum number of planets.
    const int NUM_ION_STORMS = 50;            ///< Maximum number of ion storms.

    const int NUM_PLAYERS = 11;                             ///< Number of players in standard game.
    const int NUM_OWNERS  =  12;              ///< Permitted range for owners: include Aliens.
    const int NUM_HULLS_PER_PLAYER = 20;                   ///< Number of hulls per player.

    const size_t MAX_TRN_ATTACHMENTS = 10;

//     MAX_HULLS   = NUM_PLAYERS*NUM_HULLS_PER_PLAYER, ///< Maximum number of hulls.

//     MAX_AUTOBUILD_SPEED = 100,      ///< Maximum speed for autobuild.
//     MAX_AUTOBUILD_GOAL  = 1000,     ///< Maximum goal for autobuild. Displayed as "Max".

//     /* Manifest Constants for VCRs */
//     MAX_FIGHTER_LIMIT = 50,         ///< Maximum number of fighters active in a VCR.
//     MAX_BEAM_LIMIT    = 20,         ///< Maximum number of beams in a VCR.
//     MAX_TORP_LIMIT    = 20,         ///< Maximum number of torpedo launchers in a VCR.
//     MAX_BAY_LIMIT     = 20          ///< Maximum number of fighter bays in a VCR.
// };

    enum Section {
        ShipSection,
        PlanetSection,
        BaseSection
    };

    enum Ore {
        Neutronium,
        Tritanium,
        Duranium,
        Molybdenum
    };

    struct Cost {
        Int16_t     money;
        Int16_t     tritanium;
        Int16_t     duranium;
        Int16_t     molybdenum;
    };
    static_assert(sizeof(Cost) == 8, "sizeof Cost");

    /** Beam structure. BEAMSPEC consists of 10 of these. */
    struct Beam {
        String20_t  name;
        Cost        cost;
        Int16_t     mass;
        Int16_t     techLevel;
        Int16_t     killPower;
        Int16_t     damagePower;
    };
    static_assert(sizeof(Beam) == 36, "sizeof Beam");

    /** Engine structure. ENGSPEC consists of 9 of these. */
    struct Engine {
        String20_t  name;                                       ///< Engine name.
        Cost        cost;                                       ///< Engine cost.
        Int16_t     techLevel;                                  ///< Tech level.
        Int32_t     fuelFactors[NUM_WARP_FACTORS];              ///< Fuel usage for warp 1-9. Fuel usage for 100MT ship for one turn.
    };
    static_assert(sizeof(Engine) == 66, "sizeof Engine");

    /** Hull structure. HULLSPEC consists of 105 of these. */
    struct Hull {
        String30_t  name;                                       ///< Hull name.
        Int16_t     pictureNumber;                              ///< RESOURCE.PLN index.
        Int16_t     zero;                                       ///< Unused.
        Int16_t     tritanium;                                  ///< Minerals needed to build.
        Int16_t     duranium;                                   ///< Minerals needed to build.
        Int16_t     molybdenum;                                 ///< Minerals needed to build.
        Int16_t     maxFuel;                                    ///< Fuel tank size.
        Int16_t     maxCrew;                                    ///< Normal crew.
        Int16_t     numEngines;                                 ///< Number of engines.
        Int16_t     mass;                                       ///< Empty hull mass.
        Int16_t     techLevel;                                  ///< Tech level.
        Int16_t     maxCargo;                                   ///< Cargo space.
        Int16_t     numBays;                                    ///< Number of fighter bays built into hull.
        Int16_t     maxLaunchers;                               ///< Maximum number of torpedo launchers.
        Int16_t     maxBeams;                                   ///< Maximum number of beams.
        Int16_t     money;                                      ///< Monetary cost.
    };
    static_assert(sizeof(Hull) == 60, "sizeof Hull");

     /** Torpedo structure. TORPSPEC contains 10 of these. */
    struct Torpedo {
        String20_t  name;                                       ///< Torpedo system name.
        Int16_t     torpedoCost;                                ///< Torpedo monetary cost (mineral cost is fixed to 1TDM).
        Cost        launcherCost;                               ///< Launcher cost.
        Int16_t     launcherMass;                               ///< Launcher mass (torp mass is fixed to 1kt).
        Int16_t     techLevel;                                  ///< Tech level.
        Int16_t     killPower;                                  ///< Effect.
        Int16_t     damagePower;                                ///< Effect.
    };
    static_assert(sizeof(Torpedo) == 38, "sizeof Torpedo");

    /** Build order. This is a member of the BDATA record. */
    struct BuildOrder {
        Int16_t     hullIndex;                                  ///< In BDATA record, index into truehull. Partly used differently within this program.
        Int16_t     engineType;                                 ///< Engine type (1..9).
        Int16_t     beamType;                                   ///< Beam type. Might be zero if count is zero.
        Int16_t     numBeams;                                   ///< Beam count.
        Int16_t     launcherType;                               ///< Torpedo launcher type. Might be zero if count is zero.
        Int16_t     numLaunchers;                               ///< Torpedo launcher count.
        Int16_t     zero;                                       ///< Called "fighter count" in cplayer.bas. Unused, actually, and must be zero.
    };
    static_assert(sizeof(BuildOrder) == 14, "sizeof BuildOrder");

    /** Tech level index. */
    enum TechLevel {
        EngineTech,                                             ///< Engine tech level.
        HullTech,                                               ///< Hull tech level.
        BeamTech,                                               ///< Beam weapon tech level.
        TorpedoTech                                             ///< Torpedo tech level.
    };

    /** Starbase. BDATA contains these. */
    struct Base {
        Int16_t     baseId;                                     ///< Starbase Id number. Same as planet Id.
        Int16_t     owner;                                      ///< Starbase owner. Same as planet owner.
        Int16_t     numBaseDefensePosts;                        ///< Starbase defense posts.
        Int16_t     damage;                                     ///< Starbase damage.
        Int16_t     techLevels[4];                              ///< Tech levels. Indexed by TechLevel (eng, hull, beam, torp).
        Int16_t     engineStorage[NUM_ENGINE_TYPES];            ///< Engines in storage. Indexed by engspec slot.
        Int16_t     hullStorage[NUM_HULLS_PER_PLAYER];          ///< Hulls in storage. Indexed by truehull slot.
        Int16_t     beamStorage[NUM_BEAM_TYPES];                ///< Beams in storage. Indexed by beamspec slot.
        Int16_t     launcherStorage[NUM_TORPEDO_TYPES];         ///< Torpedo launchers in storage. Indexed by torpspec slot.
        Int16_t     torpedoStorage[NUM_TORPEDO_TYPES];          ///< Torpedoes in storage. Indexed by torpspec slot.
        Int16_t     numFighters;                                ///< Fighters in storage.
        Int16_t     shipyardId;                                 ///< Ship to fix/recycle.
        Int16_t     shipyardAction;                             ///< What to do with the ship on the shipyard.
        Int16_t     mission;                                    ///< Starbase mission.
        BuildOrder  shipBuildOrder;                             ///< Build order.
    };
    static_assert(sizeof(Base) == 156, "sizeof Starbase");

    /** Score item. GEN contains one per player. */
    struct GenScore {
        Int16_t     numPlanets;                                 ///< Number of planets. 10 points each.
        Int16_t     numCapitalShips;                            ///< Number of capital ships. 10 points each.
        Int16_t     numFreighters;                              ///< Number of freighters. 1 point each.
        Int16_t     numBases;                                   ///< Number of starbases. 120 points each.
    };
    static_assert(sizeof(GenScore) == 8, "sizeof GenScore");

    /** Game info. The GEN file contains one such record. \see GGen. */
    struct Gen {
        uint8_t     timestamp[18];                              ///< Host time stamp.
        GenScore    scores[NUM_PLAYERS];                        ///< Scores.
        Int16_t     playerId;                                   ///< Player number.
        char        password[20];                               ///< Encoded password. @sa GGen
        char        zero;
        Int32_t     shipChecksum;                               ///< Checksum over all SHIP files.
        Int32_t     planetChecksum;                             ///< Checksum over all PDATA files.
        Int32_t     baseChecksum;                               ///< Checksum over all BDATA files.
        Int16_t     newPasswordFlag;                            ///< 13 iff new password set, zero otherwise.
        char        newPassword[10];                            ///< Encoded new password. @sa GGen
        Int16_t     turnNumber;                                 ///< Turn number.
        Int16_t     timestampChecksum;                          ///< Checksum over host time stamp.
    };
    static_assert(sizeof(Gen) == 157, "sizeof Gen");

    /** Game info in RST file. Same as TGen, but lacks a few fields. \see GGen */
    struct ResultGen {
        uint8_t     timestamp[18];                              ///< Host time stamp.
        GenScore    scores[NUM_PLAYERS];                        ///< Scores.
        Int16_t     playerId;                                   ///< Player number.
        char        password[20];                               ///< Encoded password. @sa GGen
        Int32_t     shipChecksum;                               ///< Checksum over SHIP section.
        Int32_t     planetChecksum;                             ///< Checksum over PDATA section.
        Int32_t     baseChecksum;                               ///< Checksum over BDATA section.
        Int16_t     turnNumber;                                 ///< Turn number.
        Int16_t     timestampChecksum;                          ///< Checksum over host time stamp.
    };
    static_assert(sizeof(ResultGen) == 144, "sizeof ResultGen");

    /** Incoming Message header. MDATA contains these. */
    struct IncomingMessageHeader {
        Int32_t     address;                                    ///< Position of message text. Starts with 1, not 0.
        Int16_t     length;                                     ///< Length of message in bytes.
    };
    static_assert(sizeof(IncomingMessageHeader) == 6, "sizeof IncomingMessageHeader");

    /** Outgoing message header. MESS contains these (DOS format messages). */
    struct OutgoingMessageHeader {
        Int32_t     address;                                    ///< Position of message text. Starts with 1, not 0.
        Int16_t     length;                                     ///< Length of message in bytes.
        Int16_t     from;                                       ///< Sender race.
        Int16_t     to;                                         ///< Receiver race.
    };
    static_assert(sizeof(OutgoingMessageHeader) == 10, "sizeof OutgoingMessageHeader");

    /** Planet. PDATA contains these. */
    struct Planet {
        Int16_t     owner;                                      ///< Planet owner.
        Int16_t     planetId;                                   ///< Planet Id.
        String3_t   friendlyCode;                               ///< Friendly code.
        Int16_t     numMines;                                   ///< Mineral mines.
        Int16_t     numFactories;                               ///< Factories.
        Int16_t     numDefensePosts;                            ///< Defense posts.
        Int32_t     minedOre[4];                                ///< Mined ore, order N, T, D, M.
        Int32_t     colonists;                                  ///< Colonist clans.
        Int32_t     supplies;                                   ///< Supplies.
        Int32_t     money;                                      ///< Money.
        Int32_t     groundOre[4];                               ///< Ground ore, order N, T, D, M.
        Int16_t     oreDensity[4];                              ///< Density of ground ore, order N, T, D, M.
        Int16_t     colonistTax;                                ///< Colonist tax rate.
        Int16_t     nativeTax;                                  ///< Native tax rate.
        Int16_t     colonistHappiness;                          ///< Colonist happiness.
        Int16_t     nativeHappiness;                            ///< Native happiness.
        Int16_t     nativeGovernment;                           ///< Native government.
        Int32_t     natives;                                    ///< Native clans.
        Int16_t     nativeRace;                                 ///< Native race.
        Int16_t     temperatureCode;                            ///< 100-temp, actually.
        Int16_t     buildBaseFlag;                              ///< 1 iff base being built, zero otherwise.
    };
    static_assert(sizeof(Planet) == 85, "sizeof Planet");

    /** Ship Transporter. Each ship has two of these (unload/transfer). */
    struct ShipTransfer {
        Int16_t     ore[4];                                     ///< Ore to transfer, N, T, D, M.
        Int16_t     colonists;                                  ///< Colonist clans to transfer.
        Int16_t     supplies;                                   ///< Supplies to transfer.
        Int16_t     targetId;                                   ///< Receiver Id.
    };
    static_assert(sizeof(ShipTransfer) == 14, "sizeof ShipTransfer");

    /** Player-owned Ship. SHIP contains these records. */
    struct Ship {
        Int16_t     shipId;                                     ///< Ship Id.
        Int16_t     owner;                                      ///< Ship owner.
        String3_t   friendlyCode;                               ///< Friendly code.
        Int16_t     warpFactor;                                 ///< Warp factor.
        Int16_t     waypointDX, waypointDY;                     ///< Waypoint displacement.
        Int16_t     x, y;                                       ///< Position.
        Int16_t     engineType;                                 ///< Engine type.
        Int16_t     hullType;                                   ///< Hull type.
        Int16_t     beamType;                                   ///< Beam type.
        Int16_t     numBeams;                                   ///< Number of beams.
        Int16_t     numBays;                                    ///< Number of fighter bays.
        Int16_t     launcherType;                               ///< Torpedo type.
        Int16_t     ammo;                                       ///< Number of torpedoes or fighters.
        Int16_t     numLaunchers;                               ///< Number of torpedo launchers.
        Int16_t     mission;                                    ///< Mission.
        Int16_t     primaryEnemy;                               ///< Primary enemy.
        Int16_t     missionTowParameter;                        ///< Mission: tow Id.
        Int16_t     damage;                                     ///< Damage.
        Int16_t     crew;                                       ///< Current crew.
        Int16_t     colonists;                                  ///< Colonists in cargo room.
        String20_t  name;                                       ///< Ship name.
        Int16_t     ore[4];                                     ///< Ore in cargo room.
        Int16_t     supplies;                                   ///< Supplies in cargo room.
        ShipTransfer unload;                                    ///< Unload transporter. For jettison / transfer to planet.
        ShipTransfer transfer;                                  ///< Transfer transporters. For enemy-ship transfer.
        Int16_t     missionInterceptParameter;                  ///< Mission: intercept Id.
        Int16_t     money;                                      ///< Money in cargo room.
    };
    static_assert(sizeof(Ship) == 107, "sizeof Ship");

    /** Non-visual contact. The SHIPXY file contains 500 / 999 of these. */
    struct ShipXY {
        Int16_t     x, y;                                       ///< Position.
        Int16_t     owner;                                      ///< Owner.
        Int16_t     mass;                                       ///< Total mass.
    };
    static_assert(sizeof(ShipXY) == 8, "sizeof ShipXY");

    /** Visual contact. TARGET contains these. */
    struct ShipTarget {
        Int16_t     shipId;                                     ///< Ship Id.
        Int16_t     owner;                                      ///< Ship owner.
        Int16_t     warpFactor;                                 ///< Warp factor, may be -1.
        Int16_t     x, y;                                       ///< Position.
        Int16_t     hullType;                                   ///< Hull number.
        Int16_t     heading;                                    ///< Heading in degrees, -1 if not moving.
        String20_t  name;                                       ///< Ship name, possibly encrypted.
    };
    static_assert(sizeof(ShipTarget) == 34, "sizeof ShipTarget");

    /** VCR object. Each VCR contains two of them. */
    struct VcrObject {
        String20_t  name;                                       ///< Name.
        Int16_t     damage;                                     ///< Initial damage.
        Int16_t     crew;                                       ///< Crew. For planets in host, zero means no shields.
        Int16_t     id;                                         ///< Id number.
        uint8_t     owner;                                      ///< Owner.
        uint8_t     raceOrZero;                                 ///< Race. Zero if same as player number.
        uint8_t     pictureNumber;                              ///< resource.pln index.
        uint8_t     hullTypeOrZero;                             ///< Hull number. May be zero if not known.
        Int16_t     beamType;                                   ///< Beam type.
        uint8_t     numBeams;                                   ///< Beam count.
        uint8_t     experienceLevel;                            ///< Experience level.
        Int16_t     numBays;                                    ///< Number of fighter bays.
        Int16_t     launcherType;                               ///< Torpedo type.
        Int16_t     ammo;                                       ///< Fighters, or torps.
        Int16_t     numLaunchersPacked;                         ///< Number of torpedo launchers. Encoded when PlanetsHaveTubes is used.
    };
    static_assert(sizeof(VcrObject) == 42, "sizeof VcrObject");

    /** Visual Combat recording.  */
    struct Vcr {
        Int16_t     randomSeed;                                 ///< Initial random seed.
        Int16_t     signature;                                  ///< Signature for PHost combat. Zero for HOST.
        UInt16_t    flags;                                      ///< Temperature of planet in HOST, capability flags in PHost 3.4d/4.0+.
        Int16_t     battleType;                                 ///< Type of battle. 0=ship/ship, 1=ship/planet.
        Int16_t     mass[2];                                    ///< Both units' combat mass.
        VcrObject   objects[2];                                 ///< Both units.
        Int16_t     shield[2];                                  ///< Both units' shields.
    };
    static_assert(sizeof(Vcr) == 100, "sizeof Vcr");

    /** VCR capabilities. */
    const uint16_t ValidCapabilities    = 0x8000;  ///< Valid bit. Treat everything as zero if this is not set.
    const uint16_t DeathRayCapability   = 1;       ///< Death rays in use.
    const uint16_t ExperienceCapability = 2;       ///< Experience in use.
    const uint16_t BeamCapability       = 4;       ///< New beam/fighter behaviour from 4.0k.

    /** Ufo. Stored in the KORE/SKORE files. */
    struct Ufo {
        Int16_t     color;                                      ///< Color. VGA color number, [1,15]. Zero for non-existant Ufos.
        String20_t  name;                                       ///< Name of object.
        String20_t  info1, info2;                               ///< Additional information.
        Int16_t     x, y;                                       ///< Object location.
        Int16_t     warpFactor;                                 ///< Warp factor. Can be larger than 9!
        Int16_t     heading;                                    ///< Heading (degrees).
        Int16_t     planetRange, shipRange;                     ///< Visibility ranges.
        Int16_t     radius;                                     ///< Radius of object.
        Int16_t     typeCode;                                   ///< Type code. Identifies the add-on which owns the object.
    };
    static_assert(sizeof(Ufo) == 78, "sizeof Ufo");

    /** Minefield in KOREx.DAT. Note that these (a) contain only a radius,
        no unit count, and (b) can not transmit non-crystalline Webs. */
    struct KoreMine {
        Int16_t     x, y;                                       ///< Center location.
        Int16_t     radius;                                     ///< Radius of minefield.
        Int16_t     ownerTypeFlag;                              ///< Owner. [1,11] for normal mines, 12 for crystalline webs.
    };
    static_assert(sizeof(KoreMine) == 8, "sizeof KoreMine");

    /** Ion storm in KOREx.DAT. */
    struct KoreStorm {
        Int16_t     x, y;                                       ///< Center location.
        Int16_t     radius;                                     ///< Radius.
        Int16_t     voltage;                                    ///< Voltage. Even: weakening, odd: growing
        Int16_t     warpFactor;                                 ///< Speed (warp factor).
        Int16_t     heading;                                    ///< Heading (angle).
    };
    static_assert(sizeof(KoreStorm) == 12, "sizeof KoreStorm");

    /** Explosion in KOREx.DAT. */
    struct KoreExplosion {
        Int16_t    x, y;                                        ///< Position.
    };
    static_assert(sizeof(KoreExplosion) == 4, "sizeof KoreExplosion");

    /** Header of SKOREx.DAT file. */
    struct SkoreHeader {
        char        reserved[96];                               ///< Not used.
        char        signature[5];                               ///< Signature "yAmsz" if file is valid.
        Int16_t     numUfos;                                    ///< Total number of Ufos (including the 100 from KOREx.DAT).
        Int16_t     resultVersion;                              ///< RST version. Must be 1 or higher.
    };
    static_assert(sizeof(SkoreHeader) == 105, "sizeof SkoreHeader");

    /** Truehull. */
    struct Truehull {
        /** Hull assignments for each player. First index is player
            number, second index is slot number. 0 means entry not
            allocated. */
        Int16_t     hulls[NUM_PLAYERS][NUM_HULLS_PER_PLAYER];
    };
    static_assert(sizeof(Truehull) == 440, "sizeof Truehull");

    /** Turn file header, standard version. */
    struct TurnHeader {
        Int16_t     playerId;                                   ///< Player number.
        Int32_t     numCommands;                                ///< Number of commands.
        uint8_t     timestamp[18];                              ///< Timestamp.
        Int16_t     unused;                                     ///< Unused. Contains random value.
        Int16_t     timeChecksum;                               ///< Checksum over timestamp.
    };
    static_assert(sizeof(TurnHeader) == 28, "sizeof TurnHeader");

    /** A file in a "Taccom-enhanced" TRN. */
    struct TaccomTurnFile {
        Int32_t     address;                                    ///< File position of entry. 1-based as usual.
        Int32_t     length;                                     ///< Size of entry. Can't use "size", already taken.
        String12_t  name;                                       ///< File name. Blank for empty entry.
    };
    static_assert(sizeof(TaccomTurnFile) == 20, "sizeof TaccomTurnFile");

    /** Turn file header, "Taccom-enhanced". */
    struct TaccomTurnHeader {
        char        magic[10];                                  ///< Magic number.
        Int32_t     turnAddress;                                ///< Position of standard turn file, 1-based.
        Int32_t     turnSize;                                   ///< Size of standard turn file.
        TaccomTurnFile attachments[MAX_TRN_ATTACHMENTS];        ///< Attachments.
    };
    static_assert(sizeof(TaccomTurnHeader) == 218, "sizeof TaccomTurnHeader");

    /** Turn file trailer, DOS version. */
    struct TurnDosTrailer {
        UInt32_t    checksum;                                   ///< Checksum over turn file, up to just before DOS trailer.
        UInt32_t    signature;                                  ///< Signature inserted by maketurn program, undefined normally.
        UInt32_t    registrationKey[51];                        ///< Registration string from FIZZ.BIN.
        UInt32_t    playerSecret[NUM_PLAYERS];                  ///< "Player secret" (templock).
    };
    static_assert(sizeof(TurnDosTrailer) == 256, "sizeof TurnDosTrailer");

    /** Turn file trailer, Windows version. Always followed by DOS trailer. */
    struct TurnWindowsTrailer {
        char        magic[8];                                   ///< "VER3.5xx".
        UInt32_t    vphKey[2];                                  ///< VPH.DLL values.
        String25_t  regstr1[2];                                 ///< Serial number. Same as standard reg string 1.
        String25_t  regstr2[2];                                 ///< Reg date. Same as standard reg string 2.
        String50_t  regstr3;                                    ///< Player name. Player-settable.
        String50_t  regstr4;                                    ///< Player address. Player-settable.
        uint8_t     unused[100];                                ///< Zero (?).
    };
    static_assert(sizeof(TurnWindowsTrailer) == 316, "sizeof TurnWindowsTrailer");

    /** Header of RST file. */
    struct ResultHeader {
        Int32_t     address[8];                                 ///< Offsets of "standard" sections.
        char        signature[8];                               ///< "VER3.500" or "VER3.501".
        Int32_t     addressWindows;                             ///< Winplan data (KOREx.DAT, RACE.NM).
        Int32_t     addressLeech;                               ///< LEECH.DAT.
        Int32_t     addressSkore;                               ///< Extended UFO database (SKOREx.DAT).
    };
    static_assert(sizeof(ResultHeader) == 52, "sizeof ResultHeader");

    /** Race name file. */
    struct RaceNames {
        String30_t  longNames[NUM_PLAYERS];
        String20_t  shortNames[NUM_PLAYERS];
        String12_t  adjectiveNames[NUM_PLAYERS];
    };
    static_assert(sizeof(RaceNames) == 682, "sizeof RaceNames");

    /* Host-specific */

    /// Minefield (MINES.HST).
    struct HostMinefield {
        Int16_t    x, y;                                        ///< Position.
        Int16_t    owner;                                       ///< Owner.
        Int32_t    units;                                       ///< Units.
        Int16_t    type;                                        ///< Type (0=normal, 1=web).
    };
    static_assert(sizeof(HostMinefield) == 12, "sizeof HostMinefield");

    /// Ion storm (GREY.HST)..
    struct HostIonStorm {
        Int16_t     x, y;                                       ///< Position.
        Int16_t     radius;                                     ///< Radius.
        Int16_t     voltage;                                    ///< Voltage.
        Int16_t     heading;                                    ///< Heading (0-360).
        Int16_t     growthFlag;                                 ///< Growing flag (0=weakening, 1=growing).
        Int32_t     _pad;
    };
    static_assert(sizeof(HostIonStorm) == 16, "sizeof HostIonStorm");

    /* Chart DB entries */

    /// Planet history record (rPlanetHistory, 1).
    struct DatabasePlanet {
        Planet      planet;
        Int16_t     turn[4];                                    ///< Timestamps.
        uint8_t     knownToHaveNatives;                         ///< true if we know this planet has natives.
    };
    static_assert(sizeof(DatabasePlanet) == 94, "sizeof DatabasePlanet");

    /// Indexes for TDbPlanet::turn.
    // FIXME: here?
    enum DatabasePlanetTimestamp {
        PlanetMinerals,                                         ///< Mined/ground/density fields.
        PlanetColonists,                                        ///< Population/owner/industry fields.
        PlanetNatives,                                          ///< Native gov/pop/race fields.
        PlanetCash                                              ///< Cash/supplies fields.
    };

    /// Ship history record (rShipHistory, 2).
    struct DatabaseShip {
        Ship        ship;                                       ///< Ship data.
        Int16_t     turn[2];                                    ///< Timestamps.
    };
    static_assert(sizeof(DatabaseShip) == 111, "sizeof DatabaseShip");

    /// Indexes for TDbShip::turn.
    // FIXME: here?
    enum DatabaseShipTimestamp {
        ShipArmsDamage,                                         ///< Arms/damage.
        ShipRest                                                ///< Cargo etc.
    };

    /// Ship Track entry (part of rShipTrack, 3).
    struct DatabaseShipTrackEntry {
        Int16_t     x, y;                                       ///< Ship position.
        int8_t      speed;                                      ///< Ship speed.
        Int16_t     heading;                                    ///< Ship heading (angle, degrees).
        Int16_t     mass;                                       ///< Ship mass.
    };
    static_assert(sizeof(DatabaseShipTrackEntry) == 9, "sizeof DatabaseShipTrackEntry");

    /// Ship Track header (rShipTrack, 3).
    struct DatabaseShipTrack {
        Int16_t     id;                                         ///< Ship Id.
        Int16_t     turn;                                       ///< Reference turn, i.e.\ turn of first TDbShipTrackEntry that follows (entries in reverse chronological order).
    };
    static_assert(sizeof(DatabaseShipTrack) == 4, "sizeof DatabaseShipTrack");

    /// Minefield History Record (rMinefield, 4).
    struct DatabaseMinefield {
        Int16_t     id;                                         ///< Minefield Id.
        Int16_t     x, y;                                       ///< Minefield center.
        Int16_t     owner;                                      ///< Minefield owner.
        Int32_t     units;                                      ///< Minefield units.
        Int16_t     type;                                       ///< Minefield type: 0=normal, 1=web.
        Int16_t     turn;                                       ///< Turn number for which this information holds.
    };
    static_assert(sizeof(DatabaseMinefield) == 16, "sizeof DatabaseMinefield");

    /// Ufo history (rUfoHistory, 12).
    struct DatabaseUfo {
        Int16_t     id;                                         ///< Ufo Id.
        Ufo         ufo;                                        ///< Ufo data as last seen.
        Int32_t     realId;                                     ///< Real ID of object represented by Ufo.
        Int16_t     turnLastSeen;                               ///< Turn in which Ufo was last seen.
        Int16_t     xLastSeen, yLastSeen;                       ///< Location at which Ufo was last seen.
        Int16_t     speedX, speedY;                             ///< Movement vector, if known.
    };
    static_assert(sizeof(DatabaseUfo) == 94, "sizeof DatabaseUfo");

    /// hconfig.hst.
    struct HConfig {
        Int16_t RecycleRate;
        Int16_t RandomMeteorRate;
        Int16_t AllowMinefields;
        Int16_t AllowAlchemy;
        Int16_t DeleteOldMessages;
        // -- 10 bytes
        Int16_t DisablePasswords;
        Int16_t GroundKillFactor[NUM_PLAYERS+1];                ///< GroundKillFactor. Note dummy element at beginning.
        Int16_t GroundDefenseFactor[NUM_PLAYERS+1];
        Int16_t FreeFighters[NUM_PLAYERS+1];
        Int16_t RaceMiningRate[NUM_PLAYERS+1];
        Int16_t ColonistTaxRate[NUM_PLAYERS+1];
        Int16_t RebelsBuildFighters;
        Int16_t ColoniesBuildFighters;
        Int16_t RobotsBuildFighters;
        Int16_t CloakFailureRate;
        Int16_t RobCloakedShips;
        Int16_t ScanRange;
        Int16_t DarkSenseRange;
        Int16_t AllowHiss;
        Int16_t AllowRebelGroundAttack;
        Int16_t AllowSuperRefit;
        Int16_t AllowWebMines;
        Int16_t CloakFuelBurn;
        Int16_t SensorRange;
        Int16_t AllowNewNatives;
        Int16_t AllowPlanetAttacks;
        Int16_t BorgAssimilationRate;
        Int16_t WebMineDecayRate;
        Int16_t MineDecayRate;
        Int16_t MaximumMinefieldRadius;
        Int16_t TransuraniumDecayRate;
        Int16_t StructureDecayPerTurn;
        Int16_t AllowEatingSupplies;
        Int16_t AllowNoFuelMovement;
        Int16_t MineHitOdds;
        Int16_t WebMineHitOdds;
        Int16_t MineScanRange;
        Int16_t AllowMinesDestroyMines;
        // -- 186 bytes
        Int16_t AllowEngineShieldBonus;
        Int16_t EngineShieldBonusRate;
        Int16_t _ColonialFighterSweepRate; // FIXME: handle this option
        Int16_t AllowColoniesSweepWebs;
        Int16_t MineSweepRate;
        Int16_t WebMineSweepRate;
        Int16_t HissEffectRate;
        Int16_t RobFailureOdds;
        Int16_t PlanetsAttackRebels;
        Int16_t PlanetsAttackKlingons;
        Int16_t MineSweepRange;
        Int16_t WebMineSweepRange;
        Int16_t AllowScienceMissions;
        Int16_t MineHitOddsWhenCloakedX10;
        Int16_t DamageLevelForCloakFail;
        Int16_t AllowFedCombatBonus;
        Int16_t MeteorShowerOdds;
        Int32_t MeteorShowerOreRanges[8];           // Min N/T/D/M, Max N/T/D/M
        Int16_t LargeMeteorsImpacting;
        Int32_t LargeMeteorOreRanges[8];            // Min N/T/D/M, Max N/T/D/M
        Int16_t AllowMeteorMessages;
        // -- 288 bytes
        Int16_t AllowOneEngineTowing;
        Int16_t AllowHyperWarps;
        Int16_t ClimateDeathRate;
        Int16_t AllowGravityWells;
        Int16_t CrystalsPreferDeserts;
        // -- 298 bytes
        Int16_t AllowMinesDestroyWebs;
        Int16_t ClimateLimitsPopulation;
        // -- 302 bytes
        Int32_t MaxPlanetaryIncome;
        Int16_t IonStormActivity;
        Int16_t AllowChunneling;
        Int16_t AllowDeluxeSuperSpy;
        Int16_t IonStormsHideMines;
        Int16_t AllowGloryDevice;
        Int16_t AllowAntiCloakShips;
        Int16_t AllowGamblingShips;
        Int16_t AllowCloakedShipsAttack;
        Int16_t AllowShipCloning;
        Int16_t AllowBoardingParties;
        Int16_t AllowImperialAssault;
        // -- 328 bytes
        Int16_t RamScoopFuelPerLY;
        Int16_t AllowAdvancedRefinery;
        Int16_t AllowBioscanners;
        Int16_t HullTechNotSlowedByMines;
        // -- 336 bytes
        Int16_t _LokiDecloaksBirds; // FIXME: handle this option
        // -- 338 bytes
        Int16_t AllowVPAFeatures;
        // -- 340 bytes
    };
    static_assert(sizeof(HConfig) == 340, "sizeof HConfig");

} } }

#endif
