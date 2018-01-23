/**
  *  \file game/map/ship.hpp
  */
#ifndef C2NG_GAME_MAP_SHIP_HPP
#define C2NG_GAME_MAP_SHIP_HPP

#include <vector>
#include "game/map/shipdata.hpp"
#include "game/map/mapobject.hpp"
#include "game/playerset.hpp"
#include "game/spec/shiplist.hpp"
#include "game/map/shiphistorydata.hpp"
#include "game/element.hpp"
#include "game/unitscorelist.hpp"
#include "game/parser/messageinformation.hpp"
#include "game/spec/modifiedhullfunctionlist.hpp"
#include "game/unitscoredefinitionlist.hpp"

namespace game { namespace map {

    // /** Ship. We essentially have three kinds of ship:
    //     - current ship, i.e. one seen this turn, and possibly played
    //     - guessed ship, i.e. not seen this turn but we guess it's there
    //     - history ship, i.e. not seen this turn */
    class Ship : public MapObject {
     public:
        /// Transporters.
        enum Transporter {
            UnloadTransporter,
            TransferTransporter
        };
        enum Kind {
            NoShip,             /**< We do not know anything about this ship. */
            CurrentShip,        /**< This is a current ship. It is visible, we have a SHIP.DAT entry for it. */
            CurrentTarget,      /**< This is a current ship. It is visible, we have a TARGET.DAT entry for it. */
            CurrentUnknown,     /**< This is a current ship. It is visible, but we don't have any data but its mass (non-visual contact). */
            GuessedShip,        /**< This is a guessed ship. We have no current data for it, just history, and we display that on the map. */
            HistoryShip         /**< This is an old ship. We have history data for it, and it's not visible this turn. */
        };
        enum Timestamp {
            MilitaryTime,       // Arms/damage. ex ShipArmsDamage
            RestTime            // Rest. ex ShipRest
        };

        // Construction
        explicit Ship(int id);
    //     explicit GShip(const GShip& other);
        ~Ship();

        // Load and Save
        void       addCurrentShipData(const ShipData& data, PlayerSet_t source);
    //     void       addTargetData(const TShipTarget& target, GPlayerSet source);
        void       addShipXYData(Point pt, int owner, int mass, PlayerSet_t source);
    //     void       addRCEntry(int flag);
    //     void       addShipHistoryData(const TDbShip& data);
    //     void       addShipTrackEntry(const TDbShipTrackEntry& data, int turn);
        void       addMessageInformation(const game::parser::MessageInformation& info, PlayerSet_t source);

        void       getCurrentShipData(ShipData& out) const;
    //     void       getShipData(TShip& data) const;
    //     int        getShipTrackHeader(TDbShipTrack& header) const;
    //     void       getShipTrackEntry(TDbShipTrackEntry& entry, int slot) const;
    //     void       getShipHistoryData(TDbShip& the_data) const;

        void       internalCheck();
        void       combinedCheck1(Universe& univ, int turnNumber);

        // MapObject interface
        virtual String_t getName(Name which, afl::string::Translator& tx, InterpreterInterface& iface) const;
        virtual Id_t getId() const;
        virtual bool getOwner(int& result) const;
        virtual bool getPosition(Point& result) const;

        // Status inquiry
        bool       isVisible() const;
        bool       isReliablyVisible(int forPlayer) const;
        PlayerSet_t getShipSource() const;
        void       addShipSource(PlayerSet_t p);
        Kind       getShipKind() const;
        bool       hasAnyShipData() const;
        bool       hasFullShipData() const;

    //     // History accessors
    //     int        getHistoryTimestamp(TDbShipTimestamp which_one) const;
    //     const TDbShipTrackEntry& getHistoryEntry(int turn) const;
    //     int        getHistoryNewestTurn() const;
    //     int        getHistoryOldestTurn() const;

        // Type accessors
        IntegerProperty_t getMass(const game::spec::ShipList& shipList) const;
        IntegerProperty_t getHull() const;
        void              setHull(IntegerProperty_t h);

    //     // Location accessors
    //     int        getOrbitPlanetId() const;
    //     bool       isCloningAt(const GPlanet& planet) const;

        // Owner accessors
        IntegerProperty_t getRealOwner() const;
        int               getRemoteControlFlag() const;

        // Course accessors
        afl::base::Optional<Point> getWaypoint() const;
        void              setWaypoint(afl::base::Optional<Point> pt);
        void              clearWaypoint();
        NegativeProperty_t getWaypointDX() const;
        NegativeProperty_t getWaypointDY() const;
        IntegerProperty_t  getHeading() const;

        IntegerProperty_t  getWarpFactor() const;
        void               setWarpFactor(IntegerProperty_t warp);
        bool               isHyperdriving(const UnitScoreDefinitionList& scoreDefinitions,
                                          const game::spec::ShipList& shipList,
                                          const game::config::HostConfiguration& config) const;

        // Equipment accessors
        IntegerProperty_t  getEngineType() const;
        void               setEngineType(IntegerProperty_t engineType);
        IntegerProperty_t  getBeamType() const;
        void               setBeamType(IntegerProperty_t type);
        IntegerProperty_t  getNumBeams() const;
        void               setNumBeams(IntegerProperty_t count);
        IntegerProperty_t  getNumBays() const;
        void               setNumBays(IntegerProperty_t count);
        IntegerProperty_t  getTorpedoType() const;
        void               setTorpedoType(IntegerProperty_t type);
        IntegerProperty_t  getNumLaunchers() const;
        void               setNumLaunchers(IntegerProperty_t count);

        // Mission accessors
        void               setName(const String_t& str);
        IntegerProperty_t  getMission() const;
        void               setMission(IntegerProperty_t m, IntegerProperty_t i, IntegerProperty_t t);
        IntegerProperty_t  getMissionParameter(MissionParameter which) const;
        IntegerProperty_t  getPrimaryEnemy() const;
        void               setPrimaryEnemy(IntegerProperty_t pe);
        IntegerProperty_t  getDamage() const;
        void               setDamage(IntegerProperty_t damage);
        IntegerProperty_t  getCrew() const;
        void               setCrew(IntegerProperty_t crew);
        StringProperty_t   getFriendlyCode() const;
        void               setFriendlyCode(StringProperty_t fc);

        // Cargo accessors
        IntegerProperty_t  getAmmo() const;
        void               setAmmo(IntegerProperty_t amount);
        IntegerProperty_t  getCargo(Element::Type type) const;
        void               setCargo(Element::Type type, IntegerProperty_t amount);
        LongProperty_t     getFreeCargo(const game::spec::ShipList& list) const;

        // Transporter accesssors
        bool               isTransporterActive(Transporter which) const;
        IntegerProperty_t  getTransporterTargetId(Transporter which) const;
        void               setTransporterTargetId(Transporter which, IntegerProperty_t id);
        IntegerProperty_t  getTransporterCargo(Transporter which, Element::Type type) const;
        void               setTransporterCargo(Transporter which, Element::Type type, IntegerProperty_t amount);
    //     bool       isTransporterCancellable(Transporter which_one) const throw();
        void               cancelTransporter(Transporter which);

        // Fleet accessors
        void               setFleetNumber(int fno);
        int                getFleetNumber() const;
        void               setFleetName(String_t name);
        const String_t&    getFleetName() const;
        bool               isFleetLeader() const;
        bool               isFleetMember() const;

    //     // Prediction tracking
    //     void       setPredictedPos(GPoint pt);
    //     GPoint     getPredictedPos() const;

        // Function accessors
        void addShipSpecialFunction(game::spec::ModifiedHullFunctionList::Function_t function);
        bool hasSpecialFunction(int basicFunction,
                                const UnitScoreDefinitionList& scoreDefinitions,
                                const game::spec::ShipList& shipList,
                                const game::config::HostConfiguration& config) const;
    //     void       enumShipSpecificSpecials(GHullFunctionList& list) const;
        bool hasAnyShipSpecialFunctions() const;

        // Public members:
        UnitScoreList& unitScores();
        const UnitScoreList& unitScores() const;

     private:
    //     // Copy protection
    //     GShip& operator=(const GShip&);

        Id_t m_id;
        IntegerProperty_t m_scannedMass;       // ex scanned_mass; ///< Scanned mass. Known for CurrentTarget and CurrentUnknown. For CurrentShip, taken from ship_data. */
        NegativeProperty_t m_scannedHeading;   // ex ship_info.heading
        std::vector<game::spec::ModifiedHullFunctionList::Function_t> m_specialFunctions;   // ex map_info.special_functions
        int m_remoteControlFlag;               // ex map_info.rc_flag
        Kind m_kind;                           // ex map_info.kind
        int m_fleetNumber;                     // ex map_info.fleet_nr
        String_t m_fleetName;                  // ex map_info.fleet_name

    //         // Prediction:
    //         GPoint predicted_pos;

        // Data:
        ShipData m_currentData;                // ex ship_info.data
        ShipHistoryData m_historyData;         // ex ship_info.track_turn, track

        int m_historyTimestamps[2];

        // Source flags: we track the source of SHIP, TARGET, and SHIPXY records.
        PlayerSet_t m_shipSource;
        PlayerSet_t m_targetSource;
        PlayerSet_t m_xySource;

        UnitScoreList m_unitScores;

        ShipData::Transfer& getTransporter(Transporter which);
        const ShipData::Transfer& getTransporter(Transporter which) const;
    };

} }

#endif
