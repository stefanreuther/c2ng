/**
  *  \file game/map/ship.hpp
  *  \brief Class game::map::Ship
  */
#ifndef C2NG_GAME_MAP_SHIP_HPP
#define C2NG_GAME_MAP_SHIP_HPP

#include <vector>
#include "game/element.hpp"
#include "game/map/messagelink.hpp"
#include "game/map/object.hpp"
#include "game/map/shipdata.hpp"
#include "game/map/shiphistorydata.hpp"
#include "game/parser/messageinformation.hpp"
#include "game/playerset.hpp"
#include "game/spec/modifiedhullfunctionlist.hpp"
#include "game/spec/shiplist.hpp"
#include "game/unitscoredefinitionlist.hpp"
#include "game/unitscorelist.hpp"

namespace game { namespace map {

    /** Ship.
        Represents all sorts of ship information:
        - current ships, i.e. seen this turn, and possibly played
        - guessed ships, i.e. not seen this turn but we guess it's there
        - history ships, i.e. not seen this turn */
    class Ship : public Object {
     public:
        /** Transporters. */
        enum Transporter {
            UnloadTransporter,  ///< Unload to planet.
            TransferTransporter ///< Transfer to another ship.
        };

        /** Ship kind. */
        enum Kind {
            NoShip,             ///< We do not know anything about this ship. */
            CurrentShip,        ///< This is a current ship. It is visible, we have a SHIP.DAT entry for it.
            CurrentTarget,      ///< This is a current ship. It is visible, we have a TARGET.DAT entry for it.
            CurrentUnknown,     ///< This is a current ship. It is visible, but we don't have any data but its mass (non-visual contact).
            GuessedShip,        ///< This is a guessed ship. We have no current data for it, just history, and we display that on the map.
            HistoryShip         ///< This is an old ship. We have history data for it, and it's not visible this turn.
        };

        /** History timestamp. */
        enum Timestamp {
            MilitaryTime,       ///< Arms/damage. ex ShipArmsDamage
            RestTime            ///< Rest. ex ShipRest
        };

        /*
         *  Construction
         */

        /** Constructor.
            \param id Ship Id */
        explicit Ship(Id_t id);

        /** Destructor. */
        ~Ship();

        /*
         *  Load and Save
         */

        /** Add current ship data (from SHIP file).
            \param data   Data
            \param source Source (whose file(s) this data is from) */
        void addCurrentShipData(const ShipData& data, PlayerSet_t source);

        /** Add current ship position data (from SHIPXY file).
            \param pt     Position
            \param owner  Owner
            \param mass   Mass
            \param source Source (whose file(s) this data is from) */
        void addShipXYData(Point pt, int owner, int mass, PlayerSet_t source);

        /** Add message information.
            Processes information received from messages, history, or util.dat.
            \param info   Information
            \param source Source. If nonempty, this will generate reliable data; use for processing TARGET files.
                          Pass empty (to create non-reliable data) to process scans, messages, etc. */
        void addMessageInformation(const game::parser::MessageInformation& info, PlayerSet_t source);

        /** Get current ship data for storage.
            \param [out] out data */
        void getCurrentShipData(ShipData& out) const;

        /** Do internal checks for this ship.
            Internal checks do not require a partner to interact with.
            This will fix the problems, and display appropriate messages.
            It will also fill in the ship kind.

            The previous second check, which needs a Universe, is no longer required.

            \param availablePlayers Available players (union of all source parameters used)
            \param turnNumber       Current turn number */
        void internalCheck(PlayerSet_t availablePlayers, int turnNumber);


        /*
         *  Object interface
         */

        virtual String_t getName(ObjectName which, afl::string::Translator& tx, const InterpreterInterface& iface) const;
        virtual afl::base::Optional<int> getOwner() const;
        virtual afl::base::Optional<Point> getPosition() const;


        /*
         *  Status inquiry
         */

        /** Check whether this ship is visible.
            If it is visible, it is displayed on the map.
            \return flag */
        bool isVisible() const;

        /** Check whether this ship is reliably visible (to a player).
            A ship can be unreliably visible if it guessed.
            It can also be reliably visible to one player but not another one if they are not allied.

            \param forPlayer Player to ask question for: is this ship known to that player,
                             and will host accept orders relating to it?
                             If zero, check whether ship is seen reliably by anyone.
            \return result */
        bool isReliablyVisible(int forPlayer) const;

        /** Get ship source flags.
            This is the set of players whose SHIP file contains a copy of this ship (usually a unit set).
            \return set */
        PlayerSet_t getShipSource() const;

        /** Add ship source.
            Normally, those are set using addCurrentShipData();
            use this function if you cannot use that.
            \param p Set to add */
        void addShipSource(PlayerSet_t p);

        /** Get kind of this ship.
            The kind determines how complete and reliable this ship's data is.
            \return kind */
        Kind getShipKind() const;

        /** Check whether we have any data about this ship.
            \return result */
        bool hasAnyShipData() const;

        /** Check whether we have full, playable data.
            \return result */
        bool hasFullShipData() const;


        /*
         *  History accessors
         */

        /** Get history timestamp.
            \param kind Timestamp to query
            \return turn number */
        int getHistoryTimestamp(Timestamp kind) const;

        /** Get newest history location turn.
            This is the newest turn for which we have location information.
            For iteration, use

                for (t = getHistoryNewestLocationTurn(); getHistoryLocation(t) != 0; --t)

            c2ng no longer has a getHistoryOldestTurn() equivalent.

            \return turn number */
        int getHistoryNewestLocationTurn() const;

        /** Get history entry for a turn.
            \param turnNr Turn number
            \return entry; null if turn too old or too new */
        const ShipHistoryData::Track* getHistoryLocation(int turnNr) const;


        /*
         *  Test access
         *
         *  These methods are NOT intended for consuming history.
         *  They are intended for setting up tests or for a potential host editor.
         */

        /** Set owner (for testing/host editor, not for consuming history).
            \param owner Owner. */
        void setOwner(int owner);

        /** Set position (for testing/host editor, not for consuming history).
            \param pos Position. */
        void setPosition(Point pos);


        /*
         *  Type accessors
         */

        /** Get ship mass.
            \param shipList Ship list (to resolve component masses)
            \return mass */
        IntegerProperty_t getMass(const game::spec::ShipList& shipList) const;

        /** Get hull number.
            \return hull number */
        IntegerProperty_t getHull() const;

        /** Set hull number.
            If this changes the hull type, resets all other history information.
            \param h New hull number */
        void setHull(IntegerProperty_t h);


        /*
         *  Owner accessors
         */

        /** Get real owner of ship.
            Can be different from getOwner() due to remote control.
            \return real owner */
        IntegerProperty_t getRealOwner() const;

        /** Get ship's remote control flag.
            \return flag (negative: remote-control forbidden; positive: real owner) */
        int getRemoteControlFlag() const;


        /*
         *  Course accessors
         */

        /** Get waypoint.
            \return Waypoint (absolute coordinates), if known */
        afl::base::Optional<Point> getWaypoint() const;

        /** Set waypoint.
            A waypoint can only be set when the ship's position is known; this call is ignored if it's not.
            \param pt New waypoint (absolute position) */
        void setWaypoint(Point pt);

        /** Clear waypoint (set waypoint to same as location). */
        void clearWaypoint();

        /** Get waypoint X displacement.
            \return waypoint X displacement */
        NegativeProperty_t getWaypointDX() const;

        /** Get waypoint Y displacement.
            \return waypoint Y displacement */
        NegativeProperty_t getWaypointDY() const;

        /** Get ship's heading vector.
            \return value from [0,360), corresponding to VGAP's heading angles.
                    Returns Nothing if ship doesn't move or heading not known */
        IntegerProperty_t getHeading() const;

        /** Get warp factor.
            \return warp factor, if known */
        IntegerProperty_t getWarpFactor() const;

        /** Set warp factor.
            \param warp New warp factor */
        void setWarpFactor(IntegerProperty_t warp);

        /** Check for active hyperdrive.
            \param scoreDefinitions Ship score definitions (for hull functions)
            \param shipList         Ship list (for hull functions)
            \param config           Host configuration (for hull functions)
            \return true Ship has an active hyperdrive */
        bool isHyperdriving(const UnitScoreDefinitionList& scoreDefinitions, const game::spec::ShipList& shipList, const game::config::HostConfiguration& config) const;


        /*
         *  Equipment accessors
         */

        /** Get engine type.
            \return engine type if known */
        IntegerProperty_t getEngineType() const;

        /** Set engine type (for history).
            \param engineType New engine type */
        void setEngineType(IntegerProperty_t engineType);

        /** Get beam type.
            \return beam type if known */
        IntegerProperty_t getBeamType() const;

        /** Set beam type (for history).
            \param type New beam type */
        void setBeamType(IntegerProperty_t type);

        /** Get number of beams.
            \return number if known */
        IntegerProperty_t getNumBeams() const;

        /** Set number of beams (for history).
            \param count Number */
        void setNumBeams(IntegerProperty_t count);

        /** Get number of fighter bays.
            \return number if known */
        IntegerProperty_t  getNumBays() const;

        /** Set number of fighter bays (for history).
            \param count Number */
        void setNumBays(IntegerProperty_t count);

        /** Get torpedo launcher type.
            \return torpedo launcher type if known */
        IntegerProperty_t getTorpedoType() const;

        /** Set torpedo launcher type (for history).
            \param type New torpedo launcher type */
        void setTorpedoType(IntegerProperty_t type);

        /** Get number of torpedo launchers.
            \return number if known */
        IntegerProperty_t getNumLaunchers() const;

        /** Set number of torpedo launchers (for history).
            \param count Number */
        void setNumLaunchers(IntegerProperty_t count);

        /** Check for weapons.
            \return true if ship has weapons, false if not (=it's a freighter) or not known. */
        bool hasWeapons() const;


        /*
         *  Mission accessors
         */

        /** Get ship name.
            \return Plain name as provided by user */
        String_t getName() const;

        /** Set name.
            \param str New name */
        void setName(const String_t& str);

        /** Get ship mission.
            \return mission number if known */
        IntegerProperty_t getMission() const;

        /** Set ship mission.
            \param m Mission number
            \param i Intercept parameter
            \param t Tow parameter
            \see FleetMember::setMission */
        void setMission(IntegerProperty_t m, IntegerProperty_t i, IntegerProperty_t t);

        /** Get mission parameter.
            \param which Which parameter to get
            \return parameter */
        IntegerProperty_t getMissionParameter(MissionParameter which) const;

        /** Get primary enemy.
            \return primary enemy, if known */
        IntegerProperty_t getPrimaryEnemy() const;

        /** Set primary enemy.
            \param pe New primary enemy */
        void setPrimaryEnemy(IntegerProperty_t pe);

        /** Get damage.
            \return damage, if known */
        IntegerProperty_t getDamage() const;

        /** Set damage (for history).
            \param damage New damage level */
        void setDamage(IntegerProperty_t damage);

        /** Get crew.
            \return crew size, if known */
        IntegerProperty_t getCrew() const;

        /** Set crew (for history).
            \param crew New crew amount */
        void setCrew(IntegerProperty_t crew);

        /** Get friendly code.
            \return friendly code, if known */
        const StringProperty_t& getFriendlyCode() const;

        /** Set friendly code.
            \param fc New friendly code */
        void setFriendlyCode(const StringProperty_t& fc);


        /*
         *  Cargo accessors
         */

        /** Get ammunition.
            \return number of torpedoes or fighters */
        IntegerProperty_t getAmmo() const;

        /** Set ammunition.
            \param amount New amount */
        void setAmmo(IntegerProperty_t amount);

        /** Get cargo amount.
            Use to access colonists, supplies, cash, ore, ammunition.
            Returns 0 if the ship cannot carry the type requested.
            \param type Cargo type
            \return amount (kt, clans, mc, torpedoes/fighters). */
        IntegerProperty_t getCargo(Element::Type type) const;

        /** Set cargo amount.
            An attempt to set a cargo type the ship cannot carry is ignored.
            \param type Cargo type
            \param amount Amount */
        void setCargo(Element::Type type, IntegerProperty_t amount);

        /** Get free cargo room on ship.
            \param list Ship list (to determine cargo room size)
            \return available room; Nothing if a value required for computation is not known */
        LongProperty_t getFreeCargo(const game::spec::ShipList& list) const;


        /*
         *  Transporter accesssors
         */

        /** Check whether a transporter is active.
            An active transporter has a non-empty transfer order.
            \param which Transporter to check */
        bool isTransporterActive(Transporter which) const;

        /** Get transporter target Id.
            \param which Transporter to check
            \return ship or planet Id, if known. Can be 0 for Jettison. */
        IntegerProperty_t getTransporterTargetId(Transporter which) const;

        /** Set transporter target Id.
            \param which Transporter
            \param id New target */
        void setTransporterTargetId(Transporter which, IntegerProperty_t id);

        /** Get transporter cargo amount.
            Returns 0 if the transporter cannot carry the type requested.
            \param which Transporter to check
            \param type  Element
            \return amount, if known */
        IntegerProperty_t getTransporterCargo(Transporter which, Element::Type type) const;

        /** Set transporter cargo amount.
            An attempt to set a cargo type the transporter cannot carry is ignored.
            \param which  Transporter to check
            \param type   Element
            \param amount New amount */
        void setTransporterCargo(Transporter which, Element::Type type, IntegerProperty_t amount);

        /** Cancel transporter.
            Moves all cargo from the transporter back into the cargo room.
            Note that this can cause the ship to become overloaded.
            \param which Transporter */
        void cancelTransporter(Transporter which);


        /*
         *  Fleet accessors
         */

        /** Set number of the fleet this ship is in.
            This function just sets the internal flag.
            Do not use this function directly; use FleetMember::setFleetNumber() instead.
            That function will update all dependant information.
            \param fno Fleet number */
        void setFleetNumber(int fno);

        /** Get number of the fleet this ship is in.
            \return fleet number (a ship Id), or 0 */
        int getFleetNumber() const;

        /** Set name of the fleet led by this ship.
            Valid only for fleet leaders.
            \param name Name */
        void setFleetName(String_t name);

        /** Get name of the fleet led by this ship.
            Valid only for fleet leaders.
            \return name */
        const String_t& getFleetName() const;

        /** Check for fleet leader.
            \return true if this ship is a fleet leader */
        bool isFleetLeader() const;

        /** Check for fleet member.
            \return true if this ship is a fleet member (but not leader) */
        bool isFleetMember() const;


        /*
         *  Function accessors
         */

        /** Add special function.
            \param function Function to add (modified hull function Id) */
        void addShipSpecialFunction(game::spec::ModifiedHullFunctionList::Function_t function);

        /** Check whether ship can do hull function.
            Checks functions assigned to the ship as well as automatic hull functions and racial abilities.
            \param basicFunction     Function to check, e.g. game::spec::BasicHullFunction::Cloak
            \param scoreDefinitions  Ship score definitions (to resolve experience limits)
            \param shipList          Ship list (to resolve hull/race specific functions, experience limits)
            \param config            Host configuration
            \return true if ship can currently do the function */
        bool hasSpecialFunction(int basicFunction,
                                const UnitScoreDefinitionList& scoreDefinitions,
                                const game::spec::ShipList& shipList,
                                const game::config::HostConfiguration& config) const;

        /** Enumerate this ship's functions.
            Returns all functions assigned to this ship.
            Does NOT return functions assigned to this ship's hull or race.
            \param [out] list       List of functions
            \param [in]  shipList   Ship list (to resolve experience-specific functions) */
        void enumerateShipFunctions(game::spec::HullFunctionList& list,
                                    const game::spec::ShipList& shipList) const;

        /** Check for functions assigned to this ship.
            If this returns true, and configuration is correct, enumerateShipFunctions() will return a nonempty list.
            \return result */
        bool hasAnyShipSpecialFunctions() const;


        /*
         *  Unit score accessors:
         */

        /** Access this ship's scores.
            \return scores */
        UnitScoreList& unitScores();

        /** Access this ship's scores (const).
            \return scores */
        const UnitScoreList& unitScores() const;

        /** Get score value.
            \param scoreId Score Id
            \param scoreDefinitions Score definitions
            \return Score as looked up in the scoreDefinitions; unknown if score value not known */
        NegativeProperty_t getScore(int16_t scoreId, const UnitScoreDefinitionList& scoreDefinitions) const;


        /*
         *  Message link
         */

        /** Access this ship's messages.
            \return messages */
        MessageLink& messages();

        /** Access this ship's messages (const).
            \return messages */
        const MessageLink& messages() const;

     private:
        IntegerProperty_t m_scannedMass;       // ex scanned_mass; ///< Scanned mass. Known for CurrentTarget and CurrentUnknown. For CurrentShip, taken from ship_data. */
        NegativeProperty_t m_scannedHeading;   // ex ship_info.heading
        std::vector<game::spec::ModifiedHullFunctionList::Function_t> m_specialFunctions;   // ex map_info.special_functions
        int m_remoteControlFlag;               // ex map_info.rc_flag
        Kind m_kind;                           // ex map_info.kind
        int m_fleetNumber;                     // ex map_info.fleet_nr
        String_t m_fleetName;                  // ex map_info.fleet_name

        // Data:
        ShipData m_currentData;                // ex ship_info.data
        ShipHistoryData m_historyData;         // ex ship_info.track_turn, track

        int m_historyTimestamps[2];

        // Source flags: we track the source of SHIP, TARGET, and SHIPXY records.
        PlayerSet_t m_shipSource;
        PlayerSet_t m_targetSource;
        PlayerSet_t m_xySource;

        UnitScoreList m_unitScores;
        MessageLink m_messages;

        ShipData::Transfer& getTransporter(Transporter which);
        const ShipData::Transfer& getTransporter(Transporter which) const;
    };

} }

inline game::PlayerSet_t
game::map::Ship::getShipSource() const
{
    // ex GShip::getShipSource
    return m_shipSource;
}

inline void
game::map::Ship::addShipSource(PlayerSet_t p)
{
    m_shipSource += p;
}

inline game::map::Ship::Kind
game::map::Ship::getShipKind() const
{
    // ex GShip::getShipKind
    return m_kind;
}

inline bool
game::map::Ship::hasAnyShipData() const
{
    // ex GShip::hasAnyShipData
    // Note that this is implemented differently than in PCC2!
    // PCC2: check whether any historic data is available
    // c2ng: check for known owner
    return m_currentData.owner.isValid();
}

inline bool
game::map::Ship::hasFullShipData() const
{
    // ex GShip::hasFullShipData
    return !m_shipSource.empty();
}

inline int
game::map::Ship::getHistoryNewestLocationTurn() const
{
    // ex GShip::getHistoryNewestTurn
    return m_historyData.trackTurn;
}

inline game::IntegerProperty_t
game::map::Ship::getHull() const
{
    // ex GShip::getHullId() const
    return m_currentData.hullType;
}

inline int
game::map::Ship::getRemoteControlFlag() const
{
    // ex GShip::getRemoteControlFlag
    return m_remoteControlFlag;
}

inline game::NegativeProperty_t
game::map::Ship::getWaypointDX() const
{
    // ex GShip::getWaypointDX
    return m_currentData.waypointDX;
}

inline game::NegativeProperty_t
game::map::Ship::getWaypointDY() const
{
    // ex GShip::getWaypointDY
    return m_currentData.waypointDY;
}

inline game::IntegerProperty_t
game::map::Ship::getWarpFactor() const
{
    // ex GShip::getWarp
    return m_currentData.warpFactor;
}

inline game::IntegerProperty_t
game::map::Ship::getEngineType() const
{
    // ex GShip::getEngineType
    return m_currentData.engineType;
}

inline game::IntegerProperty_t
game::map::Ship::getBeamType() const
{
    // ex GShip::getBeamType
    return m_currentData.beamType;
}

inline game::IntegerProperty_t
game::map::Ship::getNumBays() const
{
    // ex GShip::getNumBays
    return m_currentData.numBays;
}

inline game::IntegerProperty_t
game::map::Ship::getTorpedoType() const
{
    // ex GShip::getTorpType
    return m_currentData.torpedoType;
}

inline game::IntegerProperty_t
game::map::Ship::getMission() const
{
    // ex GShip::getMission
    return m_currentData.mission;
}

inline game::IntegerProperty_t
game::map::Ship::getPrimaryEnemy() const
{
    // ex GShip::getPrimaryEnemy
    return m_currentData.primaryEnemy;
}

inline game::IntegerProperty_t
game::map::Ship::getDamage() const
{
    // ex GShip::getDamage
    return m_currentData.damage;
}

inline game::IntegerProperty_t
game::map::Ship::getCrew() const
{
    // ex GShip::getCrew
    return m_currentData.crew;
}

inline const game::StringProperty_t&
game::map::Ship::getFriendlyCode() const
{
    // ex GShip::getFCode, GShip::isFCodeKnown
    return m_currentData.friendlyCode;
}

inline game::IntegerProperty_t
game::map::Ship::getAmmo() const
{
    // ex GShip::getAmmoRaw; also replaces GShip::getAmmo
    return m_currentData.ammo;
}

inline int
game::map::Ship::getFleetNumber() const
{
    // ex GShip::getFleetNumber
    return m_fleetNumber;
}

inline const String_t&
game::map::Ship::getFleetName() const
{
    // ex GShip::getFleetName
    return m_fleetName;
}

inline bool
game::map::Ship::isFleetLeader() const
{
    // ex GShip::isFleetLeader
    return m_fleetNumber == getId();
}

inline bool
game::map::Ship::hasAnyShipSpecialFunctions() const
{
    // ex GShip::hasAnyShipSpecificSpecials
    return !m_specialFunctions.empty();
}

inline game::UnitScoreList&
game::map::Ship::unitScores()
{
    return m_unitScores;
}

inline const game::UnitScoreList&
game::map::Ship::unitScores() const
{
    return m_unitScores;
}

inline game::map::MessageLink&
game::map::Ship::messages()
{
    // ex GShip::getAssociatedMessages
    return m_messages;
}

inline const game::map::MessageLink&
game::map::Ship::messages() const
{
    return m_messages;
}

#endif
