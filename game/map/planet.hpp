/**
  *  \file game/map/planet.hpp
  *  \brief Class game::map::Planet
  */
#ifndef C2NG_GAME_MAP_PLANET_HPP
#define C2NG_GAME_MAP_PLANET_HPP

#include "afl/base/optional.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/element.hpp"
#include "game/hostversion.hpp"
#include "game/map/basedata.hpp"
#include "game/map/messagelink.hpp"
#include "game/map/object.hpp"
#include "game/map/planetdata.hpp"
#include "game/parser/messageinformation.hpp"
#include "game/playerset.hpp"
#include "game/spec/hullassignmentlist.hpp"
#include "game/unitscoredefinitionlist.hpp"
#include "game/unitscorelist.hpp"

namespace game { namespace map {

    class Configuration;

    /** Planet.
        This stores data of a planet and possibly a starbase.
        It is used for played and scanned planets.

        - We always know name and Id of all planets
        - A planet can exist or not:
          - if it exists, we know its position, but not necessarily its other data (!= NoPlanet)
          - if it does not exist, we do not know its position. We may still know some other data (from scanning) (== NoPlanet)
        - A planet can have three levels of data:
          - if we're playing it, we have full data (hasFullPlanetData() == CurrentPlanet); we may have full data for some other planets as well
          - if we've seen it somehow, we have partial data (hasAnyPlanetData() == KnownPlanet).
          - otherwise, we don't know anything about it == UnknownPlanet, NoPlanet
        - A starbase can have two levels of data:
          - if we're playing the planet, the base is either present or not, and if it's present (haveBaseData()), we have full data
          - if we're not playing the planet, we only know whether there is a base or not.

        Changes from PCC2:
        - we no longer distinguish dat/dis by using two instances; instead, we store before and after in the same object.
          This has the advantage that we don't have to duplicate and maintain information like X/Y, name, and status.
        - removed orbit flags
        - removed cargo arbiter */
    class Planet : public Object {
     public:
        /** Overall type of starbase information. */
        enum BaseKind {
            UnknownBase,            ///< We do not know whether there is a base.
            NoBase,                 ///< We know that there is no base.
            ExistingBase,           ///< We know that there is a base.
            KnownBase,              ///< We know that there is a base, and have (partial or full) data.
            CurrentBase             ///< We have a BDATA.DAT entry for this base.
        };

        /** Overall type of planet information. */
        enum PlanetKind {
            NoPlanet,               ///< The planet does not exist. We do not know its position.
            HiddenPlanet,           ///< We do not know this planet's position, but we still have some data from sensor scans.
            UnknownPlanet,          ///< This planet exists but we do not know anything about it but its position.
            KnownPlanet,            ///< This planet exists and we have some information about it.
            CurrentPlanet           ///< We have a PDATAx.DAT entry for this planet.
        };

        /** Identification of history timestamp. */
        enum Timestamp {
            MineralTime,            ///< Mined/ground/density
            ColonistTime,           ///< Population/owner/industry
            NativeTime,             ///< Native gov/pop/race
            CashTime                ///< Cash/supplies
        };

        /** Number of history timestamps. */
        static const size_t NUM_TIMESTAMPS = 4;

        /** Autobuild settings. */
        struct AutobuildSettings {
            IntegerProperty_t goal[NUM_PLANETARY_BUILDING_TYPES];    ///< Build goals for each structure type.
            IntegerProperty_t speed[NUM_PLANETARY_BUILDING_TYPES];   ///< Build speeds for each structure type.
        };


        /** Construct new planet.
            \param id Id */
        explicit Planet(Id_t id);

        /** Copy a planet.
            \param other Other planet */
        Planet(const Planet& other);

        /** Destructor. */
        ~Planet();


        /*
         *  Load and Save
         */

        /** Add planet .dat file entry.
            \param data Parsed data file
            \param source Source flag to use for this entry */
        void addCurrentPlanetData(const PlanetData& data, PlayerSet_t source);

        /** Add starbase .dat file entry.
            \param data Parsed data file
            \param source Source flag to use for this entry */
        void addCurrentBaseData(const BaseData& data, PlayerSet_t source);

        /** Add message information.
            Processes information received from messages, history, or util.dat.
            \param info Information */
        void addMessageInformation(const game::parser::MessageInformation& info);

        /** Set position.
            \param pos Position */
        void setPosition(Point pos);

        /** Set planet name.
            \param name Name */
        void setName(const String_t& name);

        /** Get name.
            \param tx Translator */
        String_t getName(afl::string::Translator& tx) const;

        /** Set whether non-existance of this planet is known.
            There is no way to explicitly specify that a planet does not exist.
            To build maps with fewer than 500 planets, people move planets to far-away positions.
            Recent PHosts send a util.dat message whenever they consider a planet to be non-existant,
            to make sure that the clients' idea of which planets do exist agrees with PHost's.
            \param value flag (true: planet does not exist) */
        void setKnownToNotExist(bool value);

        /** Get current planet data for storage.
            \param [out] data */
        void getCurrentPlanetData(PlanetData& data) const;

        /** Get current starbase data for storage.
            \param [out] data */
        void getCurrentBaseData(BaseData& data) const;

        /** Do internal checks for this planet.
            Internal checks do not require a partner to interact with.
            This will determine the planet kind,
            fix possible the problems, and log appropriate messages.

            The previous second check, which needs a Universe, is no longer required.

            \param config Map configuration
            \param availablePlayers Players for which we have current data
            \param turnNumber Turn number
            \param tx Translator
            \param log Logger */
        void internalCheck(const Configuration& config, PlayerSet_t availablePlayers, int turnNumber, afl::string::Translator& tx, afl::sys::LogListener& log);


        /*
         *  Object interface:
         */

        virtual String_t getName(ObjectName which, afl::string::Translator& tx, const InterpreterInterface& iface) const;
        virtual afl::base::Optional<int> getOwner() const;
        virtual afl::base::Optional<Point> getPosition() const;


        /*
         *  Planet status accessors:
         */

        /** Check whether planet is visible.
            \return true if planet is visible */
        bool isVisible() const;

        /** Get planet source flags.
            This is the set of players whose PDATA file contains a copy of this planet
            (usually a unit set, but may be larger for unowned planets).
            \return set */
        PlayerSet_t getPlanetSource() const;

        /** Add planet source.
            Normally, those are set using addCurrentPlanetData();
            use this function if you cannot use that.
            \param p Set to add */
        void addPlanetSource(PlayerSet_t p);

        /** Check whether we have any information about this planet.
            Note that the planet may not be visible and therefore treated as nonexistant
            even if it has information (HiddenPlanet).
            \return true iff we have any information, full or partial */
        bool hasAnyPlanetData() const;

        /** Check whether we have full planet data.
            \return flag */
        bool hasFullPlanetData() const;

        /** Get history timestamp.
            \param kind Timestamp to query
            \return turn number */
        int getHistoryTimestamp(Timestamp kind) const;


        /*
         *  Base status accessors:
         */

        /** Get base source flags.
            This is the set of players whose BDATA file contains a copy of this base (usually a unit set).
            \return set */
        PlayerSet_t getBaseSource() const;

        /** Add base source.
            Normally, those are set using addCurrentBaseData();
            use this function if you cannot use that.
            \param p Set to add */
        void addBaseSource(PlayerSet_t p);

        /** Check for starbase.
            \retval true this planet has a starbase
            \retval false this planet has no starbase, or we don't know */
        bool hasBase() const;

        /** Check for full starbase information.
            \return true iff we have full, playable data. If yes, all base accessors will work. */
        bool hasFullBaseData() const;


        /*
         *  Owner accessors:
         */

        /** Set owner.
            \param owner new owner */
        void setOwner(IntegerProperty_t owner);


        /*
         *  Structure accessors:
         */

        /** Get number of buildings.
            \param kind Structure kind
            \return number */
        IntegerProperty_t getNumBuildings(PlanetaryBuilding kind) const;

        /** Set number of buildings.
            \param kind Structure kind
            \param n    New amount */
        void setNumBuildings(PlanetaryBuilding kind, IntegerProperty_t n);

        /** Get industry level of this planet.
            Reports the industry level from known structure counts if available, otherwise from sensor scans.
            \param host Host version (for interpretation of levels)
            \return level */
        IntegerProperty_t getIndustryLevel(const HostVersion& host) const;

        /** Get industry level for a given structure count.
            \param mifa Mines+Factories
            \param host Host version (for interpretation of levels)
            \return level */
        static int getIndustryLevel(int mifa, const HostVersion& host);

        /** Set industry level for this planet.
            This routine only makes sense for planets we do not play.
            \param level New level
            \param host  Host version (for interpretation of levels) */
        void setIndustryLevel(IntegerProperty_t level, const HostVersion& host);


        /*
         *  Colonist accessors:
         */

        /** Get colonist happiness.
            \return happiness */
        NegativeProperty_t getColonistHappiness() const;

        /** Set colonist happiness.
            \param happiness New value [MIN_HAPPINESS, MAX_HAPPINESS] */
        void setColonistHappiness(NegativeProperty_t happiness);

        /** Get colonist tax rate
            \return tax rate */
        IntegerProperty_t getColonistTax() const;

        /** Set colonist tax rate.
            \param tax New value [0,100] */
        void setColonistTax(IntegerProperty_t tax);


        /*
         *  Native accessors:
         */

        /** Get native government type.
            \return Native government type; see game::tables::NativeGovernmentName */
        IntegerProperty_t getNativeGovernment() const;

        /** Set native government type.
            \param gov Native government type */
        void setNativeGovernment(IntegerProperty_t gov);

        /** Get native happiness.
            \return happiness */
        NegativeProperty_t getNativeHappiness() const;

        /** Set native happiness.
            \param happiness New value [MIN_HAPPINESS, MAX_HAPPINESS] */
        void setNativeHappiness(NegativeProperty_t happiness);

        /** Get native race.
            \return Native race
            \see game::NativeRace
            \see game::tables::NativeRaceName */
        IntegerProperty_t getNativeRace() const;

        /** Set native race.
            \param race Native race */
        void setNativeRace(IntegerProperty_t race);

        /** Set native tax rate.
            \return Native tax rate */
        IntegerProperty_t getNativeTax() const;

        /** Set native tax rate.
            \param tax Tax rate */
        void setNativeTax(IntegerProperty_t tax);

        /** Get native population.
            \return Number of native clans */
        LongProperty_t getNatives() const;

        /** Set number of natives.
            \param natives Number of native clans */
        void setNatives(LongProperty_t natives);

        /** Check known-to-have-natives status.
            \return true if there is any indication that the planet has known natives
                    (actual population/race might still be unknonw) */
        bool isKnownToHaveNatives() const;

        /** Set known-to-have-natives status.
            \param known Status */
        void setKnownToHaveNatives(bool known);


        /*
         *  FCode accessors:
         */

        /** Get friendly code.
            \return friendly code */
        StringProperty_t getFriendlyCode() const;

        /** Set friendly code.
            \param fc Friendly code */
        void setFriendlyCode(StringProperty_t fc);


        /*
         *  Starbase building accessors:
         */

        /** Check whether planet is building a base.
            \return status */
        bool isBuildingBase() const;

        /** Set build-base order.
            \param b true to build base, false to do not build/cancel */
        void setBuildBaseFlag(bool b);


        /*
         *  Environment accessors:
         */

        /** Get ground ore density.
            \param type Ore type
            \return density */
        IntegerProperty_t getOreDensity(Element::Type type) const;

        /** Set ground ore density.
            \param type    Ore type
            \param density Density */
        void setOreDensity(Element::Type type, IntegerProperty_t density);

        /** Get ground ore amount.
            \param type Ore type
            \return amount, kilotons */
        LongProperty_t getOreGround(Element::Type type) const;

        /** Set ground ore amount.
            \param type Ore type
            \param amount Amount, kilotons */
        void setOreGround(Element::Type type, LongProperty_t amount);

        /** Get planet temperature.
            \return temperature [0=ice, 100=desert] */
        IntegerProperty_t getTemperature() const;

        /** Set planet temperature.
            \param value Temperature [0=ice, 100=desert]
            \see game::tables::TemperatureName */
        void setTemperature(IntegerProperty_t value);


        /*
         *  Cargo accessors:
         */

        /** Get available cargo amount.
            Use to access colonists, supplies, cash, mined ore, starbase ammo storage.
            \param type Cargo type
            \return amount (kt, clans, mc, torpedoes/fighters) */
        LongProperty_t getCargo(Element::Type type) const;

        /** Set cargo amount.
            \param type   Cargo type
            \param amount Amount */
        void setCargo(Element::Type type, LongProperty_t amount);


        /*
         *  Simple base accessors:
         */

        /** Get starbase damage level.
            \return damage */
        IntegerProperty_t getBaseDamage() const;

        /** Set starbase damage level.
            \param n Damage level [0,100] */
        void setBaseDamage(IntegerProperty_t n);

        /** Get starbase mission.
            \return mission
            \see game::tables::BaseMissionName */
        IntegerProperty_t getBaseMission() const;

        /** Set starbase mission.
            \param mission mission */
        void setBaseMission(IntegerProperty_t mission);

        /** Get tech level.
            \param level Area to query
            \return tech level */
        IntegerProperty_t getBaseTechLevel(TechLevel level) const;

        /** Get current maximum tech level.
            \return maximum tech level currently on base (NOT maximum achievable!) */
        IntegerProperty_t getMaxBaseTechLevel() const;

        /** Set tech level.
            \param level Area to set
            \param value Tech level */
        void setBaseTechLevel(TechLevel level, IntegerProperty_t value);


        /*
         *  Shipyard accessors:
         */

        /** Get shipyard action.
            \return action
            \see game::ShipyardAction */
        IntegerProperty_t getBaseShipyardAction() const;

        /** Get Id of ship being worked on in shipyard.
            \return ship Id or 0 */
        IntegerProperty_t getBaseShipyardId() const;

        /** Set shipyard action.
            \param action Action
            \param id     Ship Id
            \see game::ShipyardAction */
        void setBaseShipyardOrder(IntegerProperty_t action, IntegerProperty_t id);


        /*
         *  Component storage accessors:
         */

        /** Get starbase component storage.
            \param area Area to query
            \param slot Slot number (for hulls, truehull slot, NOT hull number)
            \return amount */
        IntegerProperty_t getBaseStorage(TechLevel area, int slot) const;

        /** Get starbase component storage maximum index.
            All values at this slot or higher are unknown.
            Therefore, use `for (i = 0; i < getBaseStorage(L); ++i)` for iterating over a storage.
            \param area Area to query */
        int getBaseStorageLimit(TechLevel area) const;

        /** Set starbase component storage.

            Note that this function will NOT create base component storage slots, accesses to an invalid slot are ignored.
            Slots can be created only using addCurrentBaseData().

            \param area Area to query
            \param slot Slot number (for hulls, truehull slot, NOT hull number)
            \param amount New amount */
        void setBaseStorage(TechLevel area, int slot, IntegerProperty_t amount);


        /*
         *  Build order accessors:
         */

        /** Get ship being built.
            \param config Host configuration (for resolution of truehull slots to hull numbers)
            \param map    HullAssignmentList (for resolution of truehull slots to hull numbers)
            \return Hull number; Nothing if no ship being built */
        IntegerProperty_t getBaseBuildHull(const game::config::HostConfiguration& config, const game::spec::HullAssignmentList& map) const;

        /** Get ship build order.
            \return order (using truehull slot) */
        ShipBuildOrder getBaseBuildOrder() const;

        /** Set ship build order.
            \param order New order (using truehull slot) */
        void setBaseBuildOrder(const ShipBuildOrder& order);

        /** Get truehull slot for ship being built.
            \return slot */
        IntegerProperty_t getBaseBuildOrderHullIndex() const;


        /*
         *  Build queue accessors:
         */

        /** Get build queue position.
            \return position (1=first, next ship to be built) */
        IntegerProperty_t getBaseQueuePosition() const;

        /** Set build queue position.
            \param pos Position */
        void setBaseQueuePosition(IntegerProperty_t pos);

        /** Get build queue priority.
            This is the priority value associated with the base; higher values mean build earlier.
            \return priority value */
        LongProperty_t getBaseQueuePriority() const;

        /** Set build queue priority.
            \param pri Priority */
        void setBaseQueuePriority(LongProperty_t pri);


        /*
         *  Auto build accessors:
         */

        /** Get autobuild goal for a structure.
            Known for all planets.
            \param ps Structure type */
        int getAutobuildGoal(PlanetaryBuilding ps) const;

        /** Set autobuild goal for a structure.
            \param ps Structure type
            \param value New goal */
        void setAutobuildGoal(PlanetaryBuilding ps, int value);

        /** Get autobuild speed for a structure.
            Known for all planets.
            \param ps Structure type */
        int getAutobuildSpeed(PlanetaryBuilding ps) const;

        /** Set autobuild speed for a structure.
            \param ps Structure type
            \param value New speed */
        void setAutobuildSpeed(PlanetaryBuilding ps, int value);

        /** Apply auto-build settings.
            Updates all goals and speeds from the values that are set in \c settings.
            \param settings New settings */
        void applyAutobuildSettings(const AutobuildSettings& settings);


        /*
         *  Unit score accessors:
         */

        /** Access this planet's scores.
            \return scores */
        UnitScoreList& unitScores();

        /** Access this planet's scores (const).
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

        /** Access this planet's messages.
            \return messages */
        MessageLink& messages();

        /** Access this planet's messages (const).
            \return messages */
        const MessageLink& messages() const;

     private:
        String_t m_name;        // Name, always known

        afl::base::Optional<Point> m_position;    // Position
        bool m_knownToNotExist; // Override saying this planet does not exist

        PlanetData m_currentPlanetData;

        BaseData m_currentBaseData;

        BaseKind m_baseKind;
        PlanetKind m_planetKind;

        // Source flags. These specify which players' .dat files contained
        // the PDATA/BDATA records that make up this planet. They do NOT
        // necessarily mean that we (a) play this unit and (b) know
        // everything about it. We will, however, assume that we know
        // everything about the units we play.
        PlayerSet_t m_planetSource;
        PlayerSet_t m_baseSource;

        // Planet extra info
        int m_historyTimestamps[4];
        bool m_isPlanetKnownToHaveNatives;
        IntegerProperty_t m_industryLevel;

        int m_autobuildGoals[NUM_PLANETARY_BUILDING_TYPES];
        int m_autobuildSpeeds[NUM_PLANETARY_BUILDING_TYPES];

        // Base extra info
        IntegerProperty_t m_queuePosition;
        LongProperty_t m_queuePriority;

        UnitScoreList m_unitScores;
        MessageLink m_messages;
    };

} }

#endif
