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
#include "game/types.hpp"

/** \namespace game::v3::structures
    \brief v3 Structure Definitions

    This namespace defines all binary structures used to build and parse regular ("v3") data files,
    including our local files such as "chartX.cc". */

namespace game { namespace v3 { namespace structures {

    /*
     *  Type Aliases
     */

    typedef afl::bits::Value<afl::bits::UInt16LE> UInt16_t;
    typedef afl::bits::Value<afl::bits::Int16LE> Int16_t;
    typedef afl::bits::Value<afl::bits::UInt32LE> UInt32_t;
    typedef afl::bits::Value<afl::bits::Int32LE> Int32_t;
    typedef afl::bits::Value<afl::bits::FixedString<3> > String3_t;
    typedef afl::bits::Value<afl::bits::FixedString<12> > String12_t;
    typedef afl::bits::Value<afl::bits::FixedString<20> > String20_t;
    typedef afl::bits::Value<afl::bits::FixedString<25> > String25_t;
    typedef afl::bits::Value<afl::bits::FixedString<30> > String30_t;
    typedef afl::bits::Value<afl::bits::FixedString<32> > String32_t;
    typedef afl::bits::Value<afl::bits::FixedString<50> > String50_t;

    typedef uint8_t Timestamp_t[18];


    /*
     *  Manifest Constants
     */

    const int NUM_BEAM_TYPES       = 10;                        ///< Number of beams in BEAMSPEC.
    const int NUM_TORPEDO_TYPES    = 10;                        ///< Number of torpedoes in TORPSPEC.
    const int NUM_ENGINE_TYPES     = 9;                         ///< Number of engines in ENGSPEC.
    const int NUM_WARP_FACTORS     = 9;                         ///< Number of warp factors.

    const int NUM_SHIPS            = 999;                       ///< Maximum number of ships.
    const int NUM_PLANETS          = 500;                       ///< Maximum number of planets.
    const int NUM_ION_STORMS       = 50;                        ///< Maximum number of ion storms.

    const int NUM_PLAYERS          = 11;                        ///< Number of players in standard game.
    const int NUM_OWNERS           = 12;                        ///< Permitted range for owners: include Aliens.
    const int NUM_HULLS_PER_PLAYER = 20;                        ///< Number of hulls per player.

    const size_t MAX_TRN_ATTACHMENTS = 10;                      ///< Maximum attachments in a turn file.

    /** Maximum size of a message (file format limit).
        This is actually a totally arbitrary limit.
        It defines our cutoff point when our file parsers reject a file as invalid.
        Host's limits are much lower. */
    const int MAX_MESSAGE_SIZE = 16000;

    // make a constant for 600 = default message size

//     MAX_HULLS   = NUM_PLAYERS*NUM_HULLS_PER_PLAYER, ///< Maximum number of hulls.

//     /* Manifest Constants for VCRs */
//     MAX_FIGHTER_LIMIT = 50,         ///< Maximum number of fighters active in a VCR.
//     MAX_BEAM_LIMIT    = 20,         ///< Maximum number of beams in a VCR.
//     MAX_TORP_LIMIT    = 20,         ///< Maximum number of torpedo launchers in a VCR.
//     MAX_BAY_LIMIT     = 20          ///< Maximum number of fighter bays in a VCR.
// };

    /** File section.
        The values and order of this enum are fixed and are used in file formats. */
    enum Section {
        ShipSection,
        PlanetSection,
        BaseSection
    };

    /** Ore.
        The values and order of this enum are fixed and are used in file formats. */
    enum Ore {
        Neutronium,
        Tritanium,
        Duranium,
        Molybdenum
    };

    /** Item cost.
        Used in various specification files. */
    struct Cost {
        Int16_t     money;                                      ///< Money cost.
        Int16_t     tritanium;                                  ///< Tritanium cost.
        Int16_t     duranium;                                   ///< Duranium cost.
        Int16_t     molybdenum;                                 ///< Molybdenum cost.
    };
    static_assert(sizeof(Cost) == 8, "sizeof Cost");

    /** Beam structure. BEAMSPEC consists of 10 of these. */
    struct Beam {
        String20_t  name;                                       ///< Beam name.
        Cost        cost;                                       ///< Beam cost.
        Int16_t     mass;                                       ///< Beam mass.
        Int16_t     techLevel;                                  ///< Tech level.
        Int16_t     killPower;                                  ///< Kill power.
        Int16_t     damagePower;                                ///< Damage power.
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

    // Tech levels must match binary format.
    // If they didn't, we'd have to translate.
    static_assert(EngineTech  == 0, "EngineTech");
    static_assert(HullTech    == 1, "HullTech");
    static_assert(BeamTech    == 2, "BeamTech");
    static_assert(TorpedoTech == 3, "TorpedoTech");

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
        Timestamp_t timestamp;                                  ///< Host time stamp.
        GenScore    scores[NUM_PLAYERS];                        ///< Scores.
        Int16_t     playerId;                                   ///< Player number.
        uint8_t     password[20];                               ///< Encoded password. @sa GGen
        char        zero;
        Int32_t     checksums[3];                               ///< Checksum over files, indexed by Section.
        Int16_t     newPasswordFlag;                            ///< 13 iff new password set, zero otherwise.
        uint8_t     newPassword[10];                            ///< Encoded new password. @sa GGen
        Int16_t     turnNumber;                                 ///< Turn number.
        Int16_t     timestampChecksum;                          ///< Checksum over host time stamp.
    };
    static_assert(sizeof(Gen) == 157, "sizeof Gen");

    /** Game info in RST file. Same as TGen, but lacks a few fields. \see GGen */
    struct ResultGen {
        Timestamp_t timestamp;                                  ///< Host time stamp.
        GenScore    scores[NUM_PLAYERS];                        ///< Scores.
        Int16_t     playerId;                                   ///< Player number.
        uint8_t     password[20];                               ///< Encoded password. @sa GGen
        Int32_t     checksums[3];                               ///< Checksum over files, indexed by Section.
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

    /** Outgoing message file header (v3.5).
        MESS35 starts with one of these, followed by a sequence of Outbox35MessageHeader + messages;
        the empty message file may have a truncated or missing header. */
    struct Outbox35FileHeader {
        Int16_t numMessages;                                    ///< Number of messages.
        uint8_t pad[17];                                        ///< Padding; indeterminate.
    };
    static_assert(sizeof(Outbox35FileHeader) == 19, "sizeof Outbox35FileHeader");

    /** Outgoing message header (v3.5).
        Followed by the encrypted messsage data. */
    struct Outbox35MessageHeader {
        uint8_t pad;                                            ///< Padding; indeterminate.
        uint8_t validFlag;                                      ///< Validity flag, '1' for valid.
        uint8_t receivers[NUM_OWNERS];                          ///< Receivers (player 1..11, host). '1' to send to that receiver.
        Int16_t messageLength;                                  ///< Length of the message. Defaults to 600.
    };
    static_assert(sizeof(Outbox35MessageHeader) == 16, "sizeof Outbox35MessageHeader");

    /** Planet position (XYPLAN.DAT). */
    struct PlanetXY {
        Int16_t     x, y;                                       ///< Position.
        Int16_t     owner;                                      ///< Owner (mostly unset).
    };
    static_assert(sizeof(PlanetXY) == 6, "sizeof PlanetXY");

    /** Planet. PDATA contains these. */
    struct Planet {
        Int16_t     owner;                                      ///< Planet owner.
        Int16_t     planetId;                                   ///< Planet Id.
        String3_t   friendlyCode;                               ///< Friendly code.
        Int16_t     numMines;                                   ///< Mineral mines.
        Int16_t     numFactories;                               ///< Factories.
        Int16_t     numDefensePosts;                            ///< Defense posts.
        Int32_t     minedOre[4];                                ///< Mined ore. @see Ore.
        Int32_t     colonists;                                  ///< Colonist clans.
        Int32_t     supplies;                                   ///< Supplies.
        Int32_t     money;                                      ///< Money.
        Int32_t     groundOre[4];                               ///< Ground ore. @see Ore.
        Int16_t     oreDensity[4];                              ///< Density of ground ore. @see Ore.
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
        Int16_t     ore[4];                                     ///< Ore to transfer. @see Ore.
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
    const uint16_t ValidCapabilities    = 0x8000;               ///< Valid bit. Treat everything as zero if this is not set.
    const uint16_t DeathRayCapability   = 1;                    ///< Death rays in use.
    const uint16_t ExperienceCapability = 2;                    ///< Experience in use.
    const uint16_t BeamCapability       = 4;                    ///< New beam/fighter behaviour from 4.0k.

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

    /** Header of KOREx.DAT. */
    struct KoreHeader {
        Int16_t     turnNumber;                                 ///< Turn number.
        uint8_t     unused1[7];                                 ///< Unused (random/zero).
        uint8_t     signature2[10];                             ///< Signature 2.
        uint8_t     unused2[83];                                ///< Unused (random/zero).
    };
    static_assert(sizeof(KoreHeader) == 102, "sizeof KoreHeader");

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
        Timestamp_t timestamp;                                  ///< Timestamp.
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

    /** Turn file trailer, player secret. */
    struct TurnPlayerSecret {
        UInt32_t    data[NUM_PLAYERS];                          ///< "Player secret" (templock/playerlog).
    };

    /** Turn file trailer, DOS version. */
    struct TurnDosTrailer {
        UInt32_t    checksum;                                   ///< Checksum over turn file, up to just before DOS trailer.
        UInt32_t    signature;                                  ///< Signature inserted by maketurn program, undefined normally.
        UInt32_t    registrationKey[51];                        ///< Registration string from FIZZ.BIN.
        TurnPlayerSecret playerSecret;                          ///< "Player secret" (templock/playerlog).
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
        String30_t  longNames[NUM_PLAYERS];                     ///< Long names ("The Vorticons of Fribbulus Xax").
        String20_t  shortNames[NUM_PLAYERS];                    ///< Short names ("The Vorticons").
        String12_t  adjectiveNames[NUM_PLAYERS];                ///< Adjectives ("Vorticon").
    };
    static_assert(sizeof(RaceNames) == 682, "sizeof RaceNames");


    /*
     *  Host-Side Files
     */

    /** Minefield (MINES.HST). */
    struct HostMinefield {
        Int16_t    x, y;                                        ///< Position.
        Int16_t    owner;                                       ///< Owner.
        Int32_t    units;                                       ///< Units.
        Int16_t    type;                                        ///< Type (0=normal, 1=web).
    };
    static_assert(sizeof(HostMinefield) == 12, "sizeof HostMinefield");

    /** Ion storm (GREY.HST). */
    struct HostIonStorm {
        Int16_t     x, y;                                       ///< Position.
        Int16_t     radius;                                     ///< Radius.
        Int16_t     voltage;                                    ///< Voltage.
        Int16_t     heading;                                    ///< Heading (0-360).
        Int16_t     growthFlag;                                 ///< Growing flag (0=weakening, 1=growing).
        Int32_t     _pad;
    };
    static_assert(sizeof(HostIonStorm) == 16, "sizeof HostIonStorm");

    /** hconfig.hst. */
    struct HConfig {
        Int16_t     RecycleRate;
        Int16_t     RandomMeteorRate;
        Int16_t     AllowMinefields;
        Int16_t     AllowAlchemy;
        Int16_t     DeleteOldMessages;
        // -- 10 bytes
        Int16_t     DisablePasswords;
        Int16_t     GroundKillFactor[NUM_PLAYERS+1];            ///< GroundKillFactor. Note dummy element at beginning.
        Int16_t     GroundDefenseFactor[NUM_PLAYERS+1];
        Int16_t     FreeFighters[NUM_PLAYERS+1];
        Int16_t     RaceMiningRate[NUM_PLAYERS+1];
        Int16_t     ColonistTaxRate[NUM_PLAYERS+1];
        Int16_t     RebelsBuildFighters;
        Int16_t     ColoniesBuildFighters;
        Int16_t     RobotsBuildFighters;
        Int16_t     CloakFailureRate;
        Int16_t     RobCloakedShips;
        Int16_t     ScanRange;
        Int16_t     DarkSenseRange;
        Int16_t     AllowHiss;
        Int16_t     AllowRebelGroundAttack;
        Int16_t     AllowSuperRefit;
        Int16_t     AllowWebMines;
        Int16_t     CloakFuelBurn;
        Int16_t     SensorRange;
        Int16_t     AllowNewNatives;
        Int16_t     AllowPlanetAttacks;
        Int16_t     BorgAssimilationRate;
        Int16_t     WebMineDecayRate;
        Int16_t     MineDecayRate;
        Int16_t     MaximumMinefieldRadius;
        Int16_t     TransuraniumDecayRate;
        Int16_t     StructureDecayPerTurn;
        Int16_t     AllowEatingSupplies;
        Int16_t     AllowNoFuelMovement;
        Int16_t     MineHitOdds;
        Int16_t     WebMineHitOdds;
        Int16_t     MineScanRange;
        Int16_t     AllowMinesDestroyMines;
        // -- 186 bytes
        Int16_t     AllowEngineShieldBonus;
        Int16_t     EngineShieldBonusRate;
        Int16_t     _ColonialFighterSweepRate; // FIXME: handle this option
        Int16_t     AllowColoniesSweepWebs;
        Int16_t     MineSweepRate;
        Int16_t     WebMineSweepRate;
        Int16_t     HissEffectRate;
        Int16_t     RobFailureOdds;
        Int16_t     PlanetsAttackRebels;
        Int16_t     PlanetsAttackKlingons;
        Int16_t     MineSweepRange;
        Int16_t     WebMineSweepRange;
        Int16_t     AllowScienceMissions;
        Int16_t     MineHitOddsWhenCloakedX10;
        Int16_t     DamageLevelForCloakFail;
        Int16_t     AllowFedCombatBonus;
        Int16_t     MeteorShowerOdds;
        Int32_t     MeteorShowerOreRanges[8];           // Min N/T/D/M, Max N/T/D/M
        Int16_t     LargeMeteorsImpacting;
        Int32_t     LargeMeteorOreRanges[8];            // Min N/T/D/M, Max N/T/D/M
        Int16_t     AllowMeteorMessages;
        // -- 288 bytes
        Int16_t     AllowOneEngineTowing;
        Int16_t     AllowHyperWarps;
        Int16_t     ClimateDeathRate;
        Int16_t     AllowGravityWells;
        Int16_t     CrystalsPreferDeserts;
        // -- 298 bytes
        Int16_t     AllowMinesDestroyWebs;
        Int16_t     ClimateLimitsPopulation;
        // -- 302 bytes
        Int32_t     MaxPlanetaryIncome;
        Int16_t     IonStormActivity;
        Int16_t     AllowChunneling;
        Int16_t     AllowDeluxeSuperSpy;
        Int16_t     IonStormsHideMines;
        Int16_t     AllowGloryDevice;
        Int16_t     AllowAntiCloakShips;
        Int16_t     AllowGamblingShips;
        Int16_t     AllowCloakedShipsAttack;
        Int16_t     AllowShipCloning;
        Int16_t     AllowBoardingParties;
        Int16_t     AllowImperialAssault;
        // -- 328 bytes
        Int16_t     RamScoopFuelPerLY;
        Int16_t     AllowAdvancedRefinery;
        Int16_t     AllowBioscanners;
        Int16_t     HullTechNotSlowedByMines;
        // -- 336 bytes
        Int16_t     _LokiDecloaksBirds; // FIXME: handle this option
        // -- 338 bytes
        Int16_t     AllowVPAFeatures;
        // -- 340 bytes
    };
    static_assert(sizeof(HConfig) == 340, "sizeof HConfig");


    /*
     *  VPA
     */

    /** VPA Turn Header.
        A VPA database consists of a signature, followed by list of entries of this type. */
    struct VpaTurn {
        UInt32_t    signature;                                  ///< Block identifier. @see VPA_TURN_MAGIC.
        UInt32_t    size;                                       ///< Size of payload (everything after this header).
        UInt16_t    turnNumber;                                 ///< Turn number.
        Timestamp_t timestamp;                                  ///< Time stamp.
        GenScore    scores[NUM_PLAYERS];                        ///< Scores.
    };
    static_assert(sizeof(VpaTurn) == 116, "sizeof VpaTurn");

    const uint32_t VPA_TURN_MAGIC = 0x4E525554;                 ///< Value for VpaTurn::signature.

    /** VPA Chunk.
        A VpaTurn's payload consists of a sequence of chunks.
        This is the header, followed by a content-dependant payload. */
    struct VpaChunk {
        UInt32_t type;                                          ///< Chunk type.
        UInt32_t size;                                          ///< Size of payload (everything after this header).
        UInt16_t count;                                         ///< Number of elements, if applicable.
    };
    static_assert(sizeof(VpaChunk) == 10, "sizeof VpaChunk");

    const uint32_t VPA_BASE_CHUNK_MAGIC = 0x45534142;           ///< VPA chunk Id: Starbase data (BASE).
    const uint32_t VPA_EPLN_CHUNK_MAGIC = 0x4E4C5045;           ///< VPA chunk Id: Planet scans (EPLN).
    const uint32_t VPA_IMSG_CHUNK_MAGIC = 0x47534D49;           ///< VPA chunk Id: Incoming messages (IMSG).
    const uint32_t VPA_IONS_CHUNK_MAGIC = 0x534E4F49;           ///< VPA chunk Id: Ion storms (IONS).
    const uint32_t VPA_MARK_CHUNK_MAGIC = 0x4B52414D;           ///< VPA chunk Id: Drawings (MARK).
    const uint32_t VPA_MINE_CHUNK_MAGIC = 0x454E494D;           ///< VPA chunk Id: Minefields (MINE).
    const uint32_t VPA_MSGO_CHUNK_MAGIC = 0x4F47534D;           ///< VPA chunk Id: Message associations (MSGO).
    const uint32_t VPA_NPLN_CHUNK_MAGIC = 0x4E4C504E;           ///< VPA chunk Id: Planet flags (NPLN).
    const uint32_t VPA_OMSG_CHUNK_MAGIC = 0x47534D4F;           ///< VPA chunk Id: Outgoing messages (OMSG).
    const uint32_t VPA_PASS_CHUNK_MAGIC = 0x53534150;           ///< VPA chunk Id: Password (PASS).
    const uint32_t VPA_PBPS_CHUNK_MAGIC = 0x53504250;           ///< VPA chunk Id: PBPs (PBPS).
    const uint32_t VPA_PEXP_CHUNK_MAGIC = 0x50584550;           ///< VPA chunk Id: Planet experience (PEXP).
    const uint32_t VPA_PHST_CHUNK_MAGIC = 0x54534850;           ///< VPA chunk Id: PHost version (PHST).
    const uint32_t VPA_PLAN_CHUNK_MAGIC = 0x4E414C50;           ///< VPA chunk Id: Planet data (PLAN).
    const uint32_t VPA_REFS_CHUNK_MAGIC = 0x53464552;           ///< VPA chunk Id: Reserved (REFS).
    const uint32_t VPA_SCOR_CHUNK_MAGIC = 0x524F4353;           ///< VPA chunk Id: Reserved (SCOR).
    const uint32_t VPA_SEXP_CHUNK_MAGIC = 0x50584553;           ///< VPA chunk Id: Ship experience (SEXP).
    const uint32_t VPA_SHIP_CHUNK_MAGIC = 0x50494853;           ///< VPA chunk Id: Ship data (SHIP).
    const uint32_t VPA_UFOS_CHUNK_MAGIC = 0x534F4655;           ///< VPA chunk Id: Ufo data (UFOS).
    const uint32_t VPA_VCRS_CHUNK_MAGIC = 0x53524356;           ///< VPA chunk Id: VCR data (VCRS).
    const uint32_t VPA_VERS_CHUNK_MAGIC = 0x53524556;           ///< VPA chunk Id: Version number (VERS).
    const uint32_t VPA_WORM_CHUNK_MAGIC = 0x4D524F57;           ///< VPA chunk Id: Wormholes (WORM).
    const uint32_t VPA_XYPL_CHUNK_MAGIC = 0x4C505958;           ///< VPA chunk Id: Planet positions (XYPL).


    /*
     *  util.dat
     */

    /** UTIL.DAT chunk header.
        A UTIL.DAT file consists of a sequence of chunks, each preceded by this header. */
    struct UtilChunkHeader {
        Int16_t  recordType;                                    ///< Record type.
        UInt16_t recordSize;                                    ///< Record size.
    };
    static_assert(sizeof(UtilChunkHeader) == 4, "sizeof UtilChunkHeader");

    const uint16_t UTIL_CONTROL_ID = 13;                         ///< UTIL.DAT chunk Id: control header. @see Util13Control.

    /** Record type 0: Minefield. Reports a minefield scan/lay/sweep action. */
    struct Util0Minefield {
        Int16_t     minefieldId;                                ///< Minefield Id.
        Int16_t     x, y;                                       ///< Position.
        Int16_t     owner;                                      ///< Owner of minefield.
        Int32_t     units;                                      ///< Number of mine units at time of report.
        Int16_t     type;                                       ///< Minefield type. 0=normal, 1=web.
        Int16_t     planetId;                                   ///< (2.0+) Controlling planet Id. 0 if not known.
        Int16_t     scanReason;                                 ///< (2.6d+) Reason for this scan.
    };
    static_assert(sizeof(Util0Minefield) == 18, "sizeof Util0Minefield");

    /** Record type 1: Explosion. */
    struct Util1Bang {
        Int16_t     x, y;                                       ///< Position.
        Int16_t     shipId;                                     ///< Ship Id.
        String20_t  shipName;                                   ///< (3.4+) Ship name.
    };
    static_assert(sizeof(Util1Bang) == 26, "sizeof Util1Bang");

    /** Record type 2: Mine hit. */
    struct Util2MineHit {
        Int16_t     shipId;                                     ///< Ship Id.
        Int16_t     x, y;                                       ///< Position.
        Int16_t     damage;                                     ///< Total damage.
        String20_t  shipName;                                   ///< (3.4b+) Ship name.
    };
    static_assert(sizeof(Util2MineHit) == 28, "sizeof Util2MineHit");

    /** Record type 3: Dark sense report. */
    struct Util3DarkSense {
        Int16_t     planetId;                                   ///< Planet Id.
        Int16_t     owner;                                      ///< Planet owner.
        Int32_t     totalOre[4];                                ///< Total minerals (n, t, d, m).
        Int32_t     money;                                      ///< Money.
        Int16_t     baseFlag;                                   ///< Starbase flag (0=no, 1=yes).
    };
    static_assert(sizeof(Util3DarkSense) == 26, "sizeof Util3DarkSense");

    /** Record type 4: Super spy report. */
    struct Util4SuperSpy {
        Int16_t     planetId;                                   ///< Planet Id.
        Int16_t     numMines;                                   ///< Number of mines.
        Int16_t     numFactories;                               ///< Number of factories.
        Int16_t     numDefensePosts;                            ///< Number of defense posts.
        String3_t   friendlyCode;                               ///< Friendly code.
        Int32_t     totalOre[4];                                ///< Total minerals (n, t, d, m).
        Int32_t     money;                                      ///< Money.
        Int32_t     supplies;                                   ///< (3.0+) Supplies.
    };
    static_assert(sizeof(Util4SuperSpy) == 35, "sizeof Util4SuperSpy");

    /** Record type 5: Planet exploration. */
    struct Util5Planet {
        Int16_t     planetId;                                   ///< Planet Id.
        Int16_t     temperature;                                ///< Temperature (real value, not 100-F as usual).
        Int16_t     owner;                                      ///< Owner.
        Int32_t     numColonists;                               ///< Number of colonists (not clans!)
        Int16_t     baseFlag;                                   ///< Starbase flag.
    };
    static_assert(sizeof(Util5Planet) == 12, "sizeof Util5Planet");

    /** Record type 6: Sensor sweep report. */
    struct Util6SensorSweep {
        Int16_t     planetId;                                   ///< Planet Id.
        Int16_t     owner;                                      ///< Planet owner.
        Int16_t     activity;                                   ///< Activity rating. See game::IndustryLevel.
    };
    static_assert(sizeof(Util6SensorSweep) == 6, "sizeof Util6SensorSweep");

    /** Record type 7: Battle result. */
    struct Util7Battle {
        Int16_t     ids[2];                                     ///< Id numbers.
        Int16_t     battleType;                                 ///< Battle type. 0=ship/ship, 1=ship/planet.
        Int16_t     owners[2];                                  ///< Unit owners.
        Int16_t     damageAfter[2];                             ///< Damage after fight for both units.
        Int16_t     numTorpedoesAfter[2];                       ///< Torpedoes left after fight for both units.
        Int16_t     numFightersAfter[2];                        ///< Fighters left after fight for both units.
        Int16_t     result[2];                                  ///< Results for both units.
        Int16_t     x, y;                                       ///< (1.3+) Location of battle.
        Int16_t     seed;                                       ///< (3.4b+) Random seed, same as in VCR.
    };
    static_assert(sizeof(Util7Battle) == 32, "sizeof Util7Battle");

    static const int16_t UNIT_WON = 0;                          ///< Battle result: this unit won/survived.
    static const int16_t UNIT_CAPTURED = 1;                     ///< Battle result: this unit was captured.
    static const int16_t UNIT_DESTROYED = 2;                    ///< Battle result: this unit got destroyed.
    static const int16_t UNIT_NO_AMMO = 3;                      ///< Battle result: this unit ran out of ammo (and the other, too).

    /** Record type 8: Meteor. */
    struct Util8Meteor {
        Int16_t     planetId;                                   ///< Planet Id.
        Int32_t     addedOre[4];                                ///< New ore (N,T,D,M).
    };
    static_assert(sizeof(Util8Meteor) == 18, "sizeof Util8Meteor");

    /** Record type 9: Meteorite. Same as type 8. */
    typedef Util8Meteor Util9Meteorite;

    // Util10Target = TShipTarget

    /** Record type 11: Allied starbase. */
    struct Util11AlliedBase {
        Int16_t     baseId;                                     ///< Base Id.
        Int16_t     owner;                                      ///< Owner.
    };

    /** Record type 12: Allied planet. */
    struct Util12AlliedPlanet {
        Int16_t     planetId;                                   ///< Planet Id.
        Int16_t     owner;                                      ///< Owner.
        Int16_t     temperature;                                ///< Temperature (real value, not 100-F as usual).
        Int16_t     nativeRace;                                 ///< Native race.
        Int16_t     nativeGovernment;                           ///< Native government type.
        Int32_t     numNatives;                                 ///< Native population (people, not clans!).
        Int32_t     minedOre[4];                                ///< Mined ore (N,T,D,M).
        Int32_t     numColonists;                               ///< Colonist population (people, not clans!).
        Int32_t     supplies, money;                            ///< Funds.
    };

    /** Record type 13, minimal version (PHost 1.1).
        This is the minimum set of information we expect. */
    struct Util13ControlMinimal {
        Timestamp_t timestamp;                                  ///< Timestamp.
        Int16_t     turnNumber;                                 ///< Turn number.
        Int16_t     playerId;                                   ///< Player number.
        uint8_t     majorVersion;                               ///< PHost major version.
        uint8_t     minorVersion;                               ///< PHost minor version.
        UInt32_t    digest[8];                                  ///< Spec file digests.
    };
    static_assert(sizeof(Util13ControlMinimal) == 56, "sizeof Util13ControlMinimal");

    /** Record type 13: Control. First record in file. */
    struct Util13Control {
        Util13ControlMinimal base;                              ///< Basic data.
        String32_t  gameName;                                   ///< Game name. \todo Unsure whether to recode this one
        char        releaseVersion;                             ///< (2.11h+) Release code, a letter or space.
    };
    static_assert(sizeof(Util13Control) == 89, "sizeof Util13Control");

    /** Record type 14: Wormhole scan. */
    struct Util14Wormhole {
        Int16_t     x, y;                                       ///< Position.
        Int16_t     mass;                                       ///< Mass (kt).
        Int16_t     stabilityCode;                              ///< Stability code.
        Int16_t     wormholeId;                                 ///< Wormhole Id (starts at 0!).
        Int16_t     ufoId;                                      ///< (3.4h/4.0e+) Associated Ufo Id.
        Int16_t     bidirFlag;                                  ///< (3.4h/4.0e+) Bidirectionality flag.
    };
    static_assert(sizeof(Util14Wormhole) == 14, "sizeof Util14Wormhole");

    /** Record type 15: Wormhole travel. */
    struct Util15WormholeTravel {
        Int16_t     shipId;                                     ///< Ship Id.
        Int16_t     x, y;                                       ///< Position.
        Int16_t     damageAdded;                                ///< Damage taken.
        Int16_t     damage;                                     ///< Total damage.
        Int16_t     wormholeId;                                 ///< Wormhole Id.
    };
    static_assert(sizeof(Util15WormholeTravel) == 12, "sizeof Util15WormholeTravel");

    /** Record type 16: Ship recycled. */
    struct Util16Recycled {
        Int16_t     shipId;                                     ///< Ship Id.
        Int16_t     baseId;                                     ///< Starbase Id.
    };
    static_assert(sizeof(Util16Recycled) == 4, "sizeof Util16Recycled");

    /** Record type 17: Ion storm. */
    struct Util17Storm {
        Int16_t     stormId;                                    ///< Storm Id.
        Int16_t     x, y;                                       ///< Position.
        Int16_t     voltage;                                    ///< Voltage.
        Int16_t     heading;                                    ///< Heading (degrees).
        Int16_t     warpFactor;                                 ///< Warp factor.
        Int16_t     radius;                                     ///< Radius.
        Int16_t     stormClass;                                 ///< Danger class.
        Int16_t     growthFlag;                                 ///< Growth flag. 0=weakening, 1=growing.
    };
    static_assert(sizeof(Util17Storm) == 18, "sizeof Util17Storm");

    /** Record type 18: Ship colonized. */
    typedef Util16Recycled Util18Colonized;

    /** Record type 19: Ship surrendered. Note that this record is
        different in PHost 1.3 and below. */
    struct Util19Surrender {
        Int16_t     shipId;                                     ///< Ship Id.
        Int16_t     oldOwner;                                   ///< Old ship owner.
        Int16_t     baseId;                                     ///< Base Id.
        Int16_t     newOwner;                                   ///< New ship owner.
    };
    static_assert(sizeof(Util19Surrender) == 8, "sizeof Util19Surrender");

    /** Record type 20: Ship built. Note that this record is
        different in PHost 1.3 and below. */
    struct Util20ShipBuilt {
        Int16_t     shipId;                                     ///< Ship Id.
        Int16_t     baseId;                                     ///< Base Id.
        Int16_t     cloneFlag;                                  ///< Clone flag. 0=normal build, 1=cloned.
    };
    static_assert(sizeof(Util20ShipBuilt) == 6, "sizeof Util20ShipBuilt");

    /** Record type 21: Ship given away. */
    struct Util21ShipGiven {
        Int16_t     shipId;                                     ///< Ship Id.
        Int16_t     oldOwner;                                   ///< Old owner.
        Int16_t     newOwner;                                   ///< New owner.
    };
    static_assert(sizeof(Util21ShipGiven) == 6, "sizeof Util21ShipGiven");

    /** Record type 22: Alliance status. */
    struct Util22Alliance {
        uint8_t     offeredTo[NUM_PLAYERS];                     ///< Our offers.
        uint8_t     offeredFrom[NUM_PLAYERS];                   ///< Received offers.
        uint8_t     conditionalTo[NUM_PLAYERS];                 ///< Our conditional offers.
        uint8_t     conditionalFrom[NUM_PLAYERS];               ///< Received conditional offers.
    };
    static_assert(sizeof(Util22Alliance) == 44, "sizeof Util22Alliance");

    /** Record type 23: Bioscan result. */
    struct Util23Bioscan {
        Int16_t     planetId;                                   ///< Planet Id.
        Int16_t     nativeRace;                                 ///< Native race.
        Int32_t     numNatives;                                 ///< Native population (people, not clans).
        Int16_t     temperature;                                ///< Temperature (real value, not 100-F as usual).
    };
    static_assert(sizeof(Util23Bioscan) == 10, "sizeof Util23Bioscan");

    /** Record type 24: Glory device set off. */
    struct Util24GD {
        Int16_t     shipId;                                     ///< Glory ship Id.
        Int16_t     x, y;                                       ///< Position.
    };
    static_assert(sizeof(Util24GD) == 6, "sizeof Util24GD");

    /** Record type 25: Ship hit by glory device. */
    struct Util25GDHit {
        Int16_t     shipId;                                     ///< Victim ship Id.
        Int16_t     x, y;                                       ///< Position.
        Int16_t     damage;                                     ///< Total damage.
        Int16_t     owner;                                      ///< Owner of victim ship.
        Int16_t     hullType;                                   ///< (3.4b+) Ship hull type.
        String20_t  name;                                       ///< (3.4b+) Ship name.
    };
    static_assert(sizeof(Util25GDHit) == 32, "sizeof Util25GDHit");

    /** Record type 26: Ship boarded (tow capture). */
    struct Util26Boarded {
        Int16_t     shipId;                                     ///< Victim ship Id.
        Int16_t     oldOwner;                                   ///< Old owner.
        Int16_t     newOwner;                                   ///< New owner. Same as owner of boarding ship.
        Int16_t     boardingShipId;                             ///< (2.9e+) Boarding ship Id.
    };
    static_assert(sizeof(Util26Boarded) == 8, "sizeof Util26Boarded");

    // --- Type 27 (unused) ---
    // This record isn't in use any more since PHost 2.10.
    // verbatim copy of pconfig.src

    /** Record type 28: Ground combat result. */
    struct Util28GroundCombat {
        Int16_t     planetId;                                   ///< Planet Id.
        Int16_t     owner;                                      ///< Original planet owner.
        Int16_t     attacker;                                   ///< Attacking race.
        Int16_t     result;                                     ///< Result of fight.
    };
    static_assert(sizeof(Util28GroundCombat) == 8, "sizeof Util28GroundCombat");

    /** Record type 29: Minefield explosions. */
    struct Util29MinesExplode {
        Int16_t     x1, y1, id1;                                ///< First minefield data.
        Int16_t     x2, y2, id2;                                ///< Second minefield data.
        Int32_t     explodedUnits;                              ///< Number of units destroyed.
    };
    static_assert(sizeof(Util29MinesExplode) == 16, "sizeof Util29MinesExplode");

    // --- Type 30 (End of PHost info) ---
    // This record doesn't contain data.

    /** Record type 31: Mine scoop result. */
    struct Util31MineScoop {
        Int16_t     shipId;                                     ///< Ship Id.
        Int16_t     mineId;                                     ///< Minefield Id.
        Int16_t     torpedoesMade;                              ///< Torpedoes scooped.
        Int32_t     unitsSwept;                                 ///< Mine units removed.
        Int32_t     unitsBefore;                                ///< (2.11h+) Original mine units.
    };
    static_assert(sizeof(Util31MineScoop) == 14, "sizeof Util31MineScoop");

    /** Record type 32: Pillage result. */
    struct Util32Pillage {
        Int16_t     planetId;                                   ///< Planet Id.
        Int32_t     colonists;                                  ///< Colonist clans after pillage.
        Int32_t     natives;                                    ///< Native clans after pillage.
        Int16_t     shipOwner;                                  ///< (3.4g/4.0c+) Owner of pillaging ship.
    };
    static_assert(sizeof(Util32Pillage) == 12, "sizeof Util32Pillage");

    /** Record type 33: General object (Ufo). */
    struct Util33GO {
        Int16_t     ufoId;                                      ///< Object Id.
        Int16_t     x, y;                                       ///< Object position.
        Int16_t     color;                                      ///< Color. Standard VGA color number.
        Int16_t     radius;                                     ///< Radius.
        Int16_t     warpFactor;                                 ///< Warp factor.
        Int16_t     heading;                                    ///< Heading angle.
        String20_t  name;                                       ///< Object name.
        String20_t  info1, info2;                               ///< Additional information.
        Int16_t     typeCode;                                   ///< Object type code.
    };
    static_assert(sizeof(Util33GO) == 76, "sizeof Util33GO");

    /** Record type 34: File transmission. */
    struct Util34FTP {
        String12_t  fileName;                                   ///< File name.
        uint8_t     flags;                                      ///< File type information (flags). Bit 0: binary flag.
    };
    static_assert(sizeof(Util34FTP) == 13, "sizeof Util34FTP");

    /** Record type 35: Cloak failure. */
    struct Util35CloakFail {
        Int16_t     shipId;                                     ///< Ship Id.
        Int16_t     cause;                                      ///< Cause of failure.
    };
    static_assert(sizeof(Util35CloakFail) == 4, "sizeof Util35CloakFail");

    /** Record type 36: Decloak. */
    struct Util36Loki {
        Int16_t     shipId;                                     ///< De-cloaked ship Id.
        Int16_t     x, y;                                       ///< De-cloaked ship location.
        Int16_t     owner;                                      ///< De-cloaked ship owner.
        Int16_t     beforeMovementFlag;                         ///< (3.4e/4.0a+) Status flag. 0=after movement, 1=before.
    };
    static_assert(sizeof(Util36Loki) == 10, "sizeof Util36Loki");

    /** Record type 37: Remote control. This record is an array of these structures. */
    struct Util37RemoteEntry {
        Int16_t     shipId;                                     ///< Ship Id.
        Int16_t     flag;                                       ///< Remote-control flag: true owner, or -1 if disabled.
    };
    static_assert(sizeof(Util37RemoteEntry) == 4, "sizeof Util37RemoteEntry");

    /** Record type 38: PAL report. */
    struct Util38PAL {
        Int32_t     old;                                        ///< Old score.
        Int32_t     decay;                                      ///< Decayed points.
        Int32_t     gain;                                       ///< New points.
        Int32_t     total;                                      ///< Total points.
    };
    static_assert(sizeof(Util38PAL) == 16, "sizeof Util38PAL");

    /** Record type 39: Build queue entry.
        The file contains many instance of this structures. */
    struct Util39Queue {
        Int16_t     baseId;                                     ///< Starbase Id.
        Int16_t     hullType;                                   ///< Hull being built.
        Int16_t     queuePosition;                              ///< Position in build queue.
        Int32_t     priority;                                   ///< Priority value.
    };
    static_assert(sizeof(Util39Queue) == 10, "sizeof Util39Queue");

    /** Record type 40: Web drain complete. */
    struct Util40WebDrainComplete {
        Int16_t     shipId;                                     ///< Victim ship Id.
    };
    static_assert(sizeof(Util40WebDrainComplete) == 2, "sizeof Util40WebDrainComplete");

    /** Record type 41: RGA result. */
    struct Util41RGA {
        Int16_t     planetId;                                   ///< Planet Id.
        Int16_t     hasNativesFlag;                             ///< Natives flag. 1=has natives, 0=not.
        Int16_t     shipOwner;                                  ///< (3.4g/4.0d+) Owner of RGA ship.
    };
    static_assert(sizeof(Util41RGA) == 6, "sizeof Util41RGA");

    /** Record type 42: Object destroyed. */
    struct Util42GODestroyed {
        Int16_t     ufoId;                                      ///< Object Id.
        Int16_t     typeCode;                                   ///< Object type.
    };
    static_assert(sizeof(Util42GODestroyed) == 4, "sizeof Util42GODestroyed");

    /** Record type 43: Minefield quota report. */
    struct Util43MinefieldQuota {
        Int16_t     allowed[NUM_PLAYERS];                       ///< Allowed minefields.
        Int16_t     used[NUM_PLAYERS];                          ///< Used minefields. -1=not known.
    };
    static_assert(sizeof(Util43MinefieldQuota) == 44, "sizeof Util43MinefieldQuota");

    /** Record type 44: Failure. */
    struct Util44Failure {
        Int16_t     action;                                     ///< Which action failed.
        Int16_t     shipId, planetId;                           ///< Participating ship/planet Id.
        Int16_t     cause;                                      ///< Cause of failure.
    };
    static_assert(sizeof(Util44Failure) == 8, "sizeof Util44Failure");

    /** Record type 45: Planet trade. */
    struct Util45PlanetGiven {
        Int16_t     planetId;                                   ///< Planet Id.
        Int16_t     oldOwner;                                   ///< Old owner.
        Int16_t     newOwner;                                   ///< New owner.
    };
    static_assert(sizeof(Util45PlanetGiven) == 6, "sizeof Util45PlanetGiven");

    /** Record type 46: Mine field. Used for minefields > 500. */
    typedef Util0Minefield Util46Minefield;

    // 47 = Non-existant planets = Int16_t[]

    /** Record type 48: PAL summary. */
    struct Util48PALSummary {
        Int32_t     scores[NUM_PLAYERS];                        ///< Scores for each player. -1 if not known.
    };
    static_assert(sizeof(Util48PALSummary) == 44, "sizeof Util48PALSummary");

    /** Record type 49: Per-unit score. */
    struct Util49UnitScoreHeader {
        String50_t  name;                                       ///< Name of score. Identifies the score to humans.
        Int16_t     scoreType;                                  ///< Type of score. Identifies the score to programs.
        Int16_t     scoreLimit;                                 ///< Maximum possible value.
        // Util49UnitScoreEntry[]
    };
    static_assert(sizeof(Util49UnitScoreHeader) == 54, "sizeof Util49UnitScoreHeader");

    struct Util49UnitScoreEntry {
        Int16_t     id;                                         ///< Unit Id.
        Int16_t     value;                                      ///< Score value.
    };

    // 50 = 49

    /** Record type 51: Player scores. */
    struct Util51PlayerScore {
        String50_t  name;                                       ///< Name of score. Identifies the score to humans.
        Int16_t     scoreId;                                    ///< Type of score. Identifies the score to programs.
        Int16_t     turnLimit;                                  ///< Turns to keep win limit.
        Int32_t     winLimit;                                   ///< Win limit. If somebody exceeds this limit for turn_limit turns, he wins. -1=no such limit.
        Int32_t     scores[NUM_PLAYERS];                        ///< Current scores. -1=not known.
    };
    static_assert(sizeof(Util51PlayerScore) == 102, "sizeof Util51PlayerScore");

    // 52 = ship abilities, Int16_t[] where first is ship id

    /** Record type 53: One minefield explodes. */
    struct Util53OneMineExplodes {
        Int16_t     x, y;                                       ///< Position of minefield.
        Int16_t     mineId;                                     ///< Minefield Id.
        Int32_t     explodedUnits;                              ///< Units lost.
    };
    static_assert(sizeof(Util53OneMineExplodes) == 10, "sizeof Util53OneMineExplodes");

    // 54 - enemies, one Int16_t

    /** Record type 55: Production report. */
    struct Util55Production {
        Int16_t     shipId;                                     ///< Ship Id.
        Int16_t     what;                                       ///< Type of item produced.
        Int16_t     how;                                        ///< How item was produced.
        Int16_t     amount;                                     ///< Number of produced items.
    };
    static_assert(sizeof(Util55Production) == 8, "sizeof Util55Production");

    /** Record type 56: Repair report. */
    struct Util56Repair {
        Int16_t     shipId;                                     ///< Ship Id (of ship that got repaired).
        Int16_t     how;                                        ///< How ship was repaired.
        Int16_t     otherId;                                    ///< Id of unit that did the repair.
        Int16_t     damageRepaired;                             ///< Damage points repaired.
        Int16_t     crewAdded;                                  ///< Crew members added.
    };
    static_assert(sizeof(Util56Repair) == 10, "sizeof Util56Repair");

    /** Record type 57: Special function report. */
    struct Util57Special {
        Int16_t   functionId;                                   ///< New function Id.
        Int16_t   basicId;                                      ///< Basic function Id.
        Int16_t   experienceMask;                               ///< Experience level mask.
    };
    static_assert(sizeof(Util57Special) == 6, "sizeof Util57Special");

    /** Record type 58: Minefield explosion. */
    struct Util58Explosion {
        Int16_t   x, y;                                         ///< Location.
    };
    static_assert(sizeof(Util58Explosion) == 4, "sizeof Util58Explosion");

} } }

#endif
