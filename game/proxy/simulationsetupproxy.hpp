/**
  *  \file game/proxy/simulationsetupproxy.hpp
  *  \brief Class game::proxy::SimulationSetupProxy
  */
#ifndef C2NG_GAME_PROXY_SIMULATIONSETUPPROXY_HPP
#define C2NG_GAME_PROXY_SIMULATIONSETUPPROXY_HPP

#include <vector>
#include <utility>
#include "afl/base/optional.hpp"
#include "afl/base/signal.hpp"
#include "afl/string/string.hpp"
#include "game/map/point.hpp"
#include "game/proxy/simulationadaptor.hpp"
#include "game/proxy/waitindicator.hpp"
#include "game/session.hpp"
#include "game/sim/ability.hpp"
#include "game/sim/configuration.hpp"
#include "game/sim/gameinterface.hpp"
#include "game/sim/setup.hpp"
#include "game/spec/friendlycodelist.hpp"
#include "util/range.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"
#include "util/vector.hpp"

namespace game { namespace proxy {

    /** Simulation setup proxy.
        Provides access to the object list of a game::sim::Setup.
        Objects are provided in a uniform list indexed with 0-based slot numbers.
        The planet, if any, is always last.

        In addition, provides access to the associated game::sim::Configuration.
        The configuration object can be (mostly) modified on the UI side,
        so there are no methods to modify individual elements.

        Bidirectional, synchronous:
        - retrieving the list of units (getList())
        - adding units (addShip(), addPlanet())
        - loading and saving (load(), save())
        - retrieving object information (getObject())
        - retrieving choices and ranges

        Bidirectional, asynchronous:
        - moving/removing units (swapShips(), removeObject(), clear())
        - observing a unit (setSlot())
        - modifying a unit

        Changes to the list of objects are reported using sig_listChange; see there.

        Users can select an object for observation using setSlot();
        changes to that object will be reported using sig_objectChange.

        Modifications are mostly asynchronous and will answer with a change on sig_objectChange (plus sig_listChange if applicable).
        To retrieve possible values, functions "getXxxChoice" or "getXxxRange" are provided.
        - choices can be a list of number/name pairs to choose from, or structured information.
        - ranges can be dynamic ranges for numbers.

        General rules:
        - calls that target a nonexistant object or object of wrong type are ignored
          (e.g. out-of-range slot numbers, or a ship call targeting a planet)
        - values are not validated, e.g. out-of-range hull numbers or shield levels are not rejected
        - setting some values updates others, e.g. setting damage may drop shield levels */
    class SimulationSetupProxy {
     public:
        /** Slot number. */
        typedef game::sim::Setup::Slot_t Slot_t;

        /** List item (representing a unit). */
        struct ListItem {
            Id_t id;                           ///< Id number (setId()).
            bool disabled;                     ///< true if unit is disabled (toggleDisabled()).
            bool isPlanet;                     ///< true if this is a planet.
            String_t name;                     ///< Ship or planet name ("USS Fred", "Organia").
            String_t info;                     ///< Type and owner ("Fed Nova", "Rebel planet").
            ListItem()
                : id(), disabled(), isPlanet(), name(), info()
                { }
        };

        /** List of items. */
        typedef std::vector<ListItem> ListItems_t;

        /** An element of a number/name mapping. */
        typedef std::pair<int32_t, String_t> Element_t;

        /** A number/name mapping. */
        typedef std::vector<Element_t> Elements_t;

        /** A range. */
        typedef util::Range<int32_t> Range_t;

        /** Information about an object.
            \see sig_objectChange, getObject() */
        struct ObjectInfo {
            bool isPlanet;
            Id_t id;
            String_t name;
            String_t friendlyCode;
            int damage;
            int shield;
            Element_t owner;
            Element_t experienceLevel;
            int32_t flags;
            int32_t flakRatingOverride;
            int flakCompensationOverride;

            game::sim::Abilities_t abilities;
            bool hasAnyNonstandardAbility;

            game::sim::GameInterface::Relation relation;
            afl::base::Optional<game::map::Point> position;

            // Flags
            bool allowPrimaryWeapons;
            bool allowSecondaryWeapons;
            bool hasBase;

            // Ship:
            int crew;
            Element_t hullType;
            int hullPicture;
            int mass;
            Element_t beamType;
            int numBeams;
            Element_t torpedoType;
            int numLaunchers;
            int numBays;
            int ammo;
            Element_t engineType;
            Element_t aggressiveness;
            Element_t interceptId;

            // Rating defaults
            int32_t defaultFlakRating;
            int defaultFlakCompensation;

            // Planet:
            int defense;
            int baseDefense;
            int baseBeamTech;
            int baseTorpedoTech;
            int numBaseFighters;
            int effBaseTorpedoes;
        };

        /** Choices for editing ship primary weapons.
            \see getPrimaryChoices() */
        struct PrimaryChoices {
            Elements_t beamTypes;              ///< Choices for beam type (setBeamType()).
            Range_t numBeams;                  ///< Choices for number of beams (setNumBeams()). Unit-0 if ship cannot have beams.
        };

        /** Choices for editing ship secondary weapons.
            \see getSecondaryChoices() */
        struct SecondaryChoices {
            Elements_t torpedoTypes;           ///< Choices for torpedo type (setTorpedoType()).
            Range_t numLaunchers;              ///< Choices for number of torpedo launchers (setNumLaunchers()). Unit-0 if ship cannot have torpedoes.
            Range_t numBays;                   ///< Choices for number of fighter bays (setNumBays()). Unit-X if ship has X bays.
            Range_t ammo;                      ///< Choices for ammo (number of torpedoes/fighters, setAmmo()).
        };

        /** Choices for editing unit abilities.
            \see getAbilityChoices() */
        struct AbilityChoices {
            game::sim::Abilities_t available;  ///< Abilities that can ever be set for this unit. Example: only ships can be Commander.
            game::sim::Abilities_t set;        ///< Abilities that are explicitly set. If set, active bit is valid, otherwise, implied bit is valid.
            game::sim::Abilities_t active;     ///< Status for explicitly set abilities.
            game::sim::Abilities_t implied;    ///< Status for implicitly set abilities.
        };

        /** Choices for population.
            \see getPopulationChoices() */
        struct PopulationChoices {
            int32_t population;                ///< Current population.
            Range_t range;                     ///< Range for population.
            int sampleDefense;                 ///< Sample number of defense posts.
            int32_t samplePopulation;          ///< Sample population, corresponding to sampleDefense.
        };

        /** Sort orders. */
        enum SortOrder {
            SortById,
            SortByOwner,
            SortByHull,
            SortByBattleOrder,
            SortByName
        };

        /** Player relations info package. */
        struct PlayerRelations {
            PlayerBitMatrix alliances;
            PlayerBitMatrix enemies;
            bool usePlayerRelations;

            PlayerRelations()
                : alliances(), enemies(), usePlayerRelations()
                { }
        };


        /** Constructor.
            \param adaptorSender Access to SimulationAdaptor
            \param reply RequestDispatcher to receive replies back */
        SimulationSetupProxy(util::RequestSender<SimulationAdaptor> adaptorSender, util::RequestDispatcher& reply);


        /*
         *  List Operations
         */

        /** Retrieve list of units.
            \param [in]  ind WaitIndicator for UI synchronisation
            \param [out] out List produced here  */
        void getList(WaitIndicator& ind, ListItems_t& out);

        /** Add a planet.
            The sig_listChange callback including this new planet is guaranteed to arrive before this function returns.
            \param ind WaitIndicator for UI synchronisation
            \return Slot number with the planet
            \see game::sim::Setup::addPlanet */
        Slot_t addPlanet(WaitIndicator& ind);

        /** Add (N copies of a) ship.
            The sig_listChange callback including this new ship(s) is guaranteed to arrive before this function returns.
            \param ind  WaitIndicator for UI synchronisation
            \param slot Slot number of ship to copy. If out of range, a custom ship is added.
            \param count Number of repetitions
            \return Slot number of last ship added
            \see game::sim::Setup::addShip, game::sim::Setup::duplicateShip */
        Slot_t addShip(WaitIndicator& ind, Slot_t slot, int count);

        /** Swap ships.
            The sig_listChange callback with the new list content will arrive asynchronously.
            \param a First slot
            \param b Second slot
            \see game::sim::Setup::swapShips */
        void swapShips(Slot_t a, Slot_t b);

        /** Remove object.
            The sig_listChange callback with the new list content will arrive asynchronously.
            \param slot Slot number
            \see game::sim::Setup::removeShip, game::sim::Setup::removePlanet */
        void removeObject(Slot_t slot);

        /** Clear setup.
            The sig_listChange callback with the new list content will arrive asynchronously. */
        void clear();

        /** Sort ships.
            The sig_listChange callback with the new list content will arrive asynchronously.
            \param order Sort order
            \see game::sim::Setup::sortShips */
        void sortShips(SortOrder order);

        /** Copy to game using a GameInterface.
            \param ind WaitIndicator for UI synchronisation
            \param from Index of first unit to copy
            \param to Index of first unit not to copy
            \return number of succeeded/failed units
            \see game::sim::Setup::copyToGame */
        game::sim::Setup::Status copyToGame(WaitIndicator& ind, Slot_t from, Slot_t to);

        /** Copy from game using a GameInterface.
            \param ind WaitIndicator for UI synchronisation
            \param from Index of first unit to copy
            \param to Index of first unit not to copy
            \return number of succeeded/failed units
            \see game::sim::Setup::copyToGame */
        game::sim::Setup::Status copyFromGame(WaitIndicator& ind, Slot_t from, Slot_t to);

        /** Load setup from file.
            The sig_listChange callback with the new list content will arrive asynchronously.
            \param [in]  ind           WaitIndicator for UI synchronisation
            \param [in]  fileName      File name to load
            \param [out] errorMessage  Error message
            \return true on success, false on error (message set) */
        bool load(WaitIndicator& ind, String_t fileName, String_t& errorMessage);

        /** Save setup to file.
            \param [in]  ind           WaitIndicator for UI synchronisation
            \param [in]  fileName      File name to save
            \param [out] errorMessage  Error message
            \return true on success, false on error (message set) */
        bool save(WaitIndicator& ind, String_t fileName, String_t& errorMessage);

        /** Check for matching ship list.
            \param [in]  ind           WaitIndicator for UI synchronisation
            \return false if mismatch detected; otherwise, true */
        bool isMatchingShipList(WaitIndicator& ind);


        /*
         *  Unit Operations
         */

        /** Select slot for updates.
            There is a guaranteed sig_objectChange callback immediately after the setSlot().
            Otherwise, changes to the observed unit will produce asynchronous callbacks.

            The selected slot will track movement (swapShips(), addShip(), addPlanet(), removeObject()).
            If the currently-selected slot is removed, notifications will stop.

            \param slot 0-based slot number */
        void setSlot(Slot_t slot);

        /** Get object information.
            \param [in]  ind  WaitIndicator for UI synchronisation
            \param [in]  slot Slot number
            \param [out] info Object information
            \return true on success, false on error (info not set) */
        bool getObject(WaitIndicator& ind, Slot_t slot, ObjectInfo& info);

        /** Check for duplicate Id.
            Use to check an Id before setting it to avoid setting duplicates.
            \param ind       WaitIndicator for UI synchronisation
            \param slot      Slot number
            \param candidate Candidate Id
            \retval true  candidate Id already exists on another object of the same type
            \retval false Id can be set */
        bool isDuplicateId(WaitIndicator& ind, Slot_t slot, Id_t candidate);

        /** Get base torpedoes.
            \param [in]  ind    WaitIndicator for UI synchronisation
            \param [in]  slot   Slot number
            \param [out] result List of amount,name pairs */
        void getNumBaseTorpedoes(WaitIndicator& ind, Slot_t slot, Elements_t& result);


        /*
         *  Setters
         */

        /** Set flags.
            For each bit,
            - keep=0, toggle=0 -> clear it
            - keep=0, toggle=1 -> set it
            - keep=1, toggle=0 -> keep it
            - keep=1, toggle=1 -> invert it
            \param slot   Slot number
            \param keep   Keep these bits
            \param toggle Invert these bits
            \see game::sim::Object::setFlags */
        void setFlags(Slot_t slot, uint32_t keep, uint32_t toggle);

        /** Toggle "fl_Deactivated" bit.
            \param slot Slot number */
        void toggleDisabled(Slot_t slot);

        /** Toggle "fl_Cloaked" bit.
            Enabling this bit will cancel a conflicting intercept or aggressiveness setting.
            \param slot Slot number */
        void toggleCloak(Slot_t slot);

        /** Toggle "fl_RandomFC" bit.
            \param slot Slot number */
        void toggleRandomFriendlyCode(Slot_t slot);

        /** Set ability flags.
            Only abilities mentioned in choices.available are processed.
            For those, the "set" and "active" bits are updated in the unit's flags.
            \param slot    Slot number
            \param choices Choices */
        void setAbilities(Slot_t slot, const AbilityChoices& choices);

        /** Set a sequential friendly code.
            \param slot Slot number
            \see game::sim::Setup::setSequentialFriendlyCode */
        void setSequentialFriendlyCode(Slot_t slot);

        /** Set Id.
            Changing the Id will update a planet's (and possibly, a ship's) name.
            \param slot Slot number
            \param id   New Id
            \see game::sim::Object::setId */
        void setId(Slot_t slot, int id);

        /** Set name.
            \param slot Slot number
            \param name New name
            \see game::sim::Object::setName */
        void setName(Slot_t slot, String_t name);

        /** Set friendly code.
            \param slot  Slot number
            \param fcode New friendly code
            \see game::sim::Object::setFriendlyCode, game::sim::Ship::setRandomFriendlyCodeFlags */
        void setFriendlyCode(Slot_t slot, String_t fcode);

        /** Set damage.
            Changing the damage may limit shield level.
            \param slot   Slot number
            \param damage Damage
            \see game::sim::Object::setDamage */
        void setDamage(Slot_t slot, int damage);

        /** Set shield level.
            \param slot   Slot number
            \param shield New shield level
            \see game::sim::Object::setShield */
        void setShield(Slot_t slot, int shield);

        /** Set owner.
            Changing the owner of a damaged ship may limit shield level.
            \param slot  Slot number
            \param owner New owner
            \see game::sim::Object::setOwner */
        void setOwner(Slot_t slot, int owner);

        /** Set experience level.
            \param slot  Slot number
            \param level New experience level
            \see game::sim::Object::setExperienceLevel */
        void setExperienceLevel(Slot_t slot, int level);

        /** Set FLAK rating override.
            \param slot  Slot number
            \param r     New rating
            \see game::sim::Object::setFlakRatingOverride */
        void setFlakRatingOverride(Slot_t slot, int32_t r);

        /** Set FLAK compensation override.
            \param slot  Slot number
            \param r     New rating
            \see game::sim::Object::setFlakCompensationOverride */
        void setFlakCompensationOverride(Slot_t slot, int r);

        /** Set ship crew.
            \param slot  Slot number
            \param crew  New crew
            \see game::sim::Ship::setCrew */
        void setCrew(Slot_t slot, int crew);

        /** Set ship hull type.
            \param slot     Slot number
            \param hullType New hull type
            \param afterAdd Special processing for newly-added ships: try to implicitly set the owner
            \see game::sim::Ship::setHullType */
        void setHullType(Slot_t slot, int hullType, bool afterAdd);

        /** Set ship mass.
            \param slot  Slot number
            \param mass  New mass
            \see game::sim::Ship::setMass */
        void setMass(Slot_t slot, int mass);

        /** Set ship beam type.
            \param slot      Slot number
            \param beamType  New beam type
            \see game::sim::Ship::setBeamType */
        void setBeamType(Slot_t slot, int beamType);

        /** Set ship number of beams.
            \param slot     Slot number
            \param numBeams New number of beams
            \see game::sim::Ship::setNumBeams */
        void setNumBeams(Slot_t slot, int numBeams);

        /** Set ship torpedo type.
            \param slot        Slot number
            \param torpedoType New crew
            \see game::sim::Ship::setTorpedoType */
        void setTorpedoType(Slot_t slot, int torpedoType);

        /** Set ship number of torpedo launchers.
            \param slot         Slot number
            \param numLaunchers New number of torpedo launchers
            \see game::sim::Ship::setNumLaunchers */
        void setNumLaunchers(Slot_t slot, int numLaunchers);

        /** Set ship number of fighter bays.
            \param slot    Slot number
            \param numBays New number of bays
            \see game::sim::Ship::setNumBeams */
        void setNumBays(Slot_t slot, int numBays);

        /** Set ship ammo.
            \param slot  Slot number
            \param ammo  New number of torpedoes/fighters.
            \see game::sim::Ship::setAmmo */
        void setAmmo(Slot_t slot, int ammo);

        /** Set ship engine type.
            \param slot       Slot number
            \param engineType New engine type
            \see game::sim::Ship::setEngineType */
        void setEngineType(Slot_t slot, int engineType);

        /** Set ship aggressiveness.
            \param slot           Slot number
            \param aggressiveness New aggressiveness
            \see game::sim::Ship::setAggressiveness */
        void setAggressiveness(Slot_t slot, int aggressiveness);

        /** Set ship intercept Id.
            \param slot  Slot number
            \param id    New Intercept Id
            \see game::sim::Ship::setInterceptId */
        void setInterceptId(Slot_t slot, int id);

        /** Set planet defense.
            \param slot    Slot number
            \param defense New defense
            \see game::sim::Planet::setDefense */
        void setDefense(Slot_t slot, int defense);

        /** Set planet population.
            Sets the maximum possible defense (setDefense()) according to the given population.
            \param slot  Slot number
            \param pop   New population
            \see game::sim::Planet::setDefense */
        void setPopulation(Slot_t slot, int32_t pop);

        /** Set base defense.
            \param slot    Slot number
            \param defense New defense
            \see game::sim::Planet::setBaseDefense */
        void setBaseDefense(Slot_t slot, int defense);

        /** Set base beam tech level.
            \param slot    Slot number
            \param level   New beam tech level
            \see game::sim::Planet::setBaseBeamTech */
        void setBaseBeamTech(Slot_t slot, int level);

        /** Set base torpedo tech level.
            \param slot    Slot number
            \param level   New torpedo tech level
            \see game::sim::Planet::setBaseTorpedoTech */
        void setBaseTorpedoTech(Slot_t slot, int level);

        /** Set base number of fighters.
            \param slot         Slot number
            \param baseFighters New number of fighters
            \see game::sim::Planet::setNumBaseFighters */
        void setNumBaseFighters(Slot_t slot, int baseFighters);

        /** Set number of starbase torpedoes.
            \param slot Slot number
            \param list Number of torpedoes for each type in the .first member (.second is ignored)
            \see game::sim::Planet::setNumBaseTorpedoes */
        void setNumBaseTorpedoes(Slot_t slot, const Elements_t& list);


        /*
         *  Choice Inquiry
         */

        /** Get choices for setAbilities().
            \param [in]  ind    WaitIndicator for UI synchronisation
            \param [in]  slot   Slot number
            \param [out] result Result */
        void getAbilityChoices(WaitIndicator& ind, Slot_t slot, AbilityChoices& result);

        /** Get choices for setFriendlyCode() (special friendly codes).
            \param [in]  ind    WaitIndicator for UI synchronisation
            \param [in]  slot   Slot number
            \param [out] result Result */
        void getFriendlyCodeChoices(WaitIndicator& ind, Slot_t slot, game::spec::FriendlyCodeList::Infos_t& result);

        /** Get choices for setOwner().
            \param [in]  ind    WaitIndicator for UI synchronisation
            \param [out] result Result (player Id/name pairs) */
        void getOwnerChoices(WaitIndicator& ind, Elements_t& result);

        /** Get choices for setExperienceLevel().
            \param [in]  ind    WaitIndicator for UI synchronisation
            \param [out] result Result (experience level/name pairs) */
        void getExperienceLevelChoices(WaitIndicator& ind, Elements_t& result);

        /** Get choices for setHullType().
            \param [in]  ind    WaitIndicator for UI synchronisation
            \param [out] result Result (hull number/name pairs) */
        void getHullTypeChoices(WaitIndicator& ind, Elements_t& result);

        /** Get choices for setBeamType(), setNumBeams().
            \param [in]  ind    WaitIndicator for UI synchronisation
            \param [in]  slot   Slot number
            \param [out] result Result */
        void getPrimaryChoices(WaitIndicator& ind, Slot_t slot, PrimaryChoices& result);

        /** Get choices for setTorpedoType(), setNumLaunchers(), setNumBays(), setAmmo().
            \param [in]  ind    WaitIndicator for UI synchronisation
            \param [in]  slot   Slot number
            \param [out] result Result */
        void getSecondaryChoices(WaitIndicator& ind, Slot_t slot, SecondaryChoices& result);

        /** Get choices for setEngineType().
            \param [in]  ind    WaitIndicator for UI synchronisation
            \param [out] result Result (engine type/name pairs) */
        void getEngineTypeChoices(WaitIndicator& ind, Elements_t& result);

        /** Get choices for setAggressiveness().
            \param [in]  ind    WaitIndicator for UI synchronisation
            \param [out] result Result (aggressiveness/label pairs) */
        void getAggressivenessChoices(WaitIndicator& ind, Elements_t& result);

        /** Get choices for setBaseBeamTech().
            \param [in]  ind    WaitIndicator for UI synchronisation
            \param [out] result Result (level/label pairs) */
        void getBaseBeamLevelChoices(WaitIndicator& ind, Elements_t& result);

        /** Get choices for setBaseTorpedoTech().
            \param [in]  ind    WaitIndicator for UI synchronisation
            \param [out] result Result (level/label pairs) */
        void getBaseTorpedoLevelChoices(WaitIndicator& ind, Elements_t& result);

        /** Get choices for setId() for planets.
            Setting the Id will implicitly set the name.
            \param [in]  ind    WaitIndicator for UI synchronisation
            \param [out] result Result (Id/name pairs) */
        void getPlanetNameChoices(WaitIndicator& ind, Elements_t& result);

        /** Get choices for setPopulation().
            \param [in]  ind    WaitIndicator for UI synchronisation
            \param [in]  slot   Slot number
            \param [out] result Result */
        void getPopulationChoices(WaitIndicator& ind, Slot_t slot, PopulationChoices& result);


        /*
         *  Range Inquiry
         */

        /** Get range for setId().
            \param ind  WaitIndicator for UI synchronisation
            \param slot Slot number (decides ship/planet range)
            \return range */
        Range_t getIdRange(WaitIndicator& ind, Slot_t slot);

        /** Get range for setDamage().
            \param ind  WaitIndicator for UI synchronisation
            \param slot Slot number (decides range depending on owner)
            \return range */
        Range_t getDamageRange(WaitIndicator& ind, Slot_t slot);

        /** Get range for setShield().
            \param ind  WaitIndicator for UI synchronisation
            \param slot Slot number (decides range depending on damage)
            \return range */
        Range_t getShieldRange(WaitIndicator& ind, Slot_t slot);

        /** Get range for setCrew().
            \param ind  WaitIndicator for UI synchronisation
            \param slot Slot number (decides range)
            \return range */
        Range_t getCrewRange(WaitIndicator& ind, Slot_t slot);

        /** Get range for setInterceptId().
            \param ind  WaitIndicator for UI synchronisation
            \param slot Slot number
            \return range */
        Range_t getInterceptIdRange(WaitIndicator& ind, Slot_t slot);

        /** Get range for setBaseDefense().
            \param ind  WaitIndicator for UI synchronisation
            \param slot Slot number (decides range depending on owner)
            \return range */
        Range_t getBaseDefenseRange(WaitIndicator& ind, Slot_t slot);

        /** Get range for setNumBaseFighters().
            \param ind  WaitIndicator for UI synchronisation
            \param slot Slot number (decides range depending on owner)
            \return range */
        Range_t getNumBaseFightersRange(WaitIndicator& ind, Slot_t slot);


        /*
         *  Configuration Access
         */

        /** Get configuration.
            \param [in]  ind    WaitIndicator for UI synchronisation
            \param [out] config Configuration */
        void getConfiguration(WaitIndicator& ind, game::sim::Configuration& config);

        /** Set configuration.
            \param config Configuration
            \param areas  Areas of configuration to update */
        void setConfiguration(const game::sim::Configuration& config, game::sim::Configuration::Areas_t areas);

        /** Get player relations.
            Retrieves the current player relations and status of the "use player relations" setting,
            to provide defaults for setConfiguration() and setUsePlayerRelations(), respectively.
            \param [in]  ind  WaitIndicator for UI synchronisation
            \param [out] rel  Relations
            \see game::sim::Session::getPlayerRelations() */
        void getPlayerRelations(WaitIndicator& ind, PlayerRelations& rel);

        /** Configure use of game's player relations.
            If enabled, usePlayerRelations() will use the actual game's relations;
            if disabled, configuration remains unchanged.
            This function call only configures the settings, but does not yet use them.
            \param flag Flag
            \see game::sim::Session::setUsePlayerRelations() */
        void setUsePlayerRelations(bool flag);

        /** Use player relations.
            If use of player relations is enabled, updates the configuration accordingly
            (the updated configuration can be retrieved using getConfiguration()).
            Should be called whenever the simulation editor is opened. */
        void usePlayerRelations();


        /** Signal: list changed.
            Reported whenever a list item changed, or the list itself changed (items added or removed).

            Primarily, this mirrors game::sim::Setup::sig_structureChange.
            However, a ListItem also contains information that is not reported by sig_structureChange.
            If the change was initiated by SimulationSetupProxy, such changes will be reported as sig_listChange.
            However, if another component modifies the Setup, this may not trigger a sig_listChange.

            \param list New list
            \see getList() */
        afl::base::Signal<void(const ListItems_t&)> sig_listChange;

        /** Signal: object changed.
            Reported whenever the object observed using setSlot() changes. */
        afl::base::Signal<void(Slot_t, const ObjectInfo&)> sig_objectChange;



        /** Access the underlying SimulationAdaptor sender.
            \return SimulationAdaptor */
        util::RequestSender<SimulationAdaptor> adaptorSender();


     private:
        class TrampolineFromAdaptor;
        class AdaptorFromSession;
        class Trampoline;
        util::RequestSender<SimulationAdaptor> m_adaptorSender;
        util::RequestReceiver<SimulationSetupProxy> m_reply;
        util::RequestSender<Trampoline> m_trampoline;

        template<typename Object, typename Property>
        void setProperty(Slot_t slot, void (Object::*set)(Property), Property value, uint32_t update);

        template<typename Object, typename Result>
        void getChoices(WaitIndicator& ind, void (Object::*get)(Result&), Result& result);

        template<typename Object>
        Range_t getRange(WaitIndicator& ind, Range_t (Object::*get)(Slot_t), Slot_t slot);

        template<typename Object>
        game::sim::Setup::Status copyGame(WaitIndicator& ind, Slot_t from, Slot_t to, game::sim::Setup::Status (Object::*copy)(Slot_t, Slot_t));
    };

} }

#endif
