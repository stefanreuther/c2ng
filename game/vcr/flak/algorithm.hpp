/**
  *  \file game/vcr/flak/algorithm.hpp
  *  \brief Class game::vcr::flak::Algorithm
  */
#ifndef C2NG_GAME_VCR_FLAK_ALGORITHM_HPP
#define C2NG_GAME_VCR_FLAK_ALGORITHM_HPP

#include <iosfwd>
#include <vector>
#include "afl/base/optional.hpp"
#include "afl/container/ptrvector.hpp"
#include "game/vcr/flak/definitions.hpp"
#include "game/vcr/flak/object.hpp"
#include "game/vcr/flak/position.hpp"
#include "game/vcr/flak/setup.hpp"
#include "game/vcr/statistic.hpp"
#include "util/randomnumbergenerator.hpp"

namespace game { namespace vcr { namespace flak {

    class Environment;
    class Visualizer;

    /** FLAK Battle Player.
        This contains the playback engine.
        It takes as input a Setup and a HostConfiguration, as well as an Environment.
        It does NOT require a FLAK Configuration, that is only used by Setup.

        Playback calls take a Visualizer to receive visualisation callbacks.

        General conventions:
        - public methods have been modeled after game::vcr::classic::Algorithm.
        - a participant is traditionally called "Ship" in FLAK, although in the rest of c2ng, it is called "Object".
          In FLAK, an "Object" is a torpedo or fighter.

        Note that this class contains methods to determine captors.
        This is host-side logic that is NOT part of the algorithm, and therefore uses an outside RandomNumberGenerator.
        Clients receive Host's results via the endingStatus field for each ship. */
    class Algorithm {
        // ex FlakBattle
     public:
        // Internal structures.
        struct Ship;
        struct Fleet;
        struct Object;
        struct Player;
        class StatusTokenImpl;
        enum ObjectKind { oFighter, oTorpedo, oDeleteMe };

        /** Status token. */
        class StatusToken : public afl::base::Deletable {
         public:
            virtual void storeTo(Algorithm& battle) const = 0;
        };


        /*
         *  Construction and Playback
         */

        /** Constructor.
            \param battle  Battle setup. Will be copied as far as needed.
            \param env     Environment. */
        Algorithm(const Setup& battle, const Environment& env);
        ~Algorithm();

        /** Initialize player.
            If battle was already advanced a bit, rewinds back to the beginning.
            \param env Environment
            \param vis Visualizer */
        void init(const Environment& env, Visualizer& vis);

        /** Play one cycle.
            Either does nothing and returns false (last cycle),
            or advances time, does something and returns true.
            \param env Environment
            \param vis Visualizer
            \return true if time was advanced; false if nothing was done (last cycle) */
        bool playCycle(const Environment& env, Visualizer& vis);

        /** Create a status token.
            The token can be used to rewind the battle to the current place.
            The status token can only be applied to the Algorithm instance it was derived from.
            \return newly-allocated status token */
        StatusToken* createStatusToken() const;

        /** Get current time.
         \return number of elapsed battle ticks */
        int32_t getTime() const;


        /*
         *  Ship Access
         */

        size_t getNumShips() const;
        bool isPlanet(size_t shipIndex) const;
        int getShipId(size_t shipIndex) const;
        int getBeamStatus(size_t shipIndex, int id) const;
        int getLauncherStatus(size_t shipIndex, int id) const;
        int getNumTorpedoes(size_t shipIndex) const;
        int getNumFighters(size_t shipIndex) const;
        int getNumFightersLaunched(size_t shipIndex) const;
        int getFighterLaunchCountdown(size_t shipIndex) const;
        int getShield(size_t shipIndex) const;
        int getDamage(size_t shipIndex) const;
        int getCrew(size_t shipIndex) const;
        Statistic getStatistic(size_t shipIndex) const;


        /*
         *  Fleet Access
         */

        size_t getNumFleets() const;
        Position getFleetPosition(size_t number) const;
        bool isFleetAlive(size_t number) const;


        /*
         *  Result Access
         */

        /** Find captor.
            This function shall be called once for each captured ship; when called multiple times, it might return different results.
            \param [in]     shipIndex     Index to a captured ship
            \param [in,out] rng           Host-side RNG
            \return Captor, if any found */
        afl::base::Optional<size_t> findCaptor(size_t shipIndex, util::RandomNumberGenerator& rng) const;

        /** Copy result to Object.
            Updates shield/damage/crew/ammo.
            This can be used to produce the "after" object of a fight.
            \param [in]    shipIndex   Index
            \param [out]   out         Object to update */
        void copyResult(size_t shipIndex, game::vcr::flak::Object& out) const;

        /** Set ending status.
            Populates the ending status fields (Object::setEndingStatus()) of the given battle setup.
            Calls findEndingStatus() (and therefore findCaptor()) for all units.
            This function is intended for simulations;
            it produces correct owner outputs but loses the identity of the captors which is needed for experience production.
            \param [out]    battle    Battle to update
            \param [in]     env       Environment
            \param [in,out] rng       Host-side RNG */
        void setEndingStatus(Setup& battle, const Environment& env, util::RandomNumberGenerator& rng) const;

        /** Find ending status.
            Finds the ending status (Object::setEndingStatus()) for a single unit.
            Calls findCaptor(). This function is intended for simulations; see setEndingStatus().
            \param [in]     shipIndex Index to ship
            \param [in]     env       Environment
            \param [in,out] rng       Host-side RNG
            \return ending status (-1: destroyed, 0: survived, otherwise: captured) */
        int findEndingStatus(size_t shipIndex, const Environment& env, util::RandomNumberGenerator& rng) const;

     private:
        /* Fleets (includes attack lists) */
        afl::container::PtrVector<Fleet> m_fleets;

        /* Ships */
        afl::container::PtrVector<Ship> m_ships;

        /* Player status, indexed by player number-1; use to access a player by number. */
        afl::container::PtrVector<Player> m_playerStatus;

        /* List of all live players; use to iterate through all players. */
        std::vector<Player*> m_playerIndex;

        /* Host configuration options */
        const bool m_alternativeCombat;
        const bool m_fireOnAttackFighters;
        const int m_standoffDistance;

        /* Pool of unused objects.
           New objects are taken from here to recycle object Ids. */
        afl::container::PtrVector<Object> m_unusedObjects;
        size_t m_objectId;

        /* Random number generator. */
        uint32_t m_seed;
        const uint32_t m_originalSeed;

        /* Time/status */
        int32_t m_time;
        bool m_isTerminated;

        /* Torpedo status for doFleetGC.
           Used as instance variable to avoid allocations in the inner loop. */
        std::vector<int> m_fleetGCTorpedoes;


        /* Random number generator */
        int random(int max);

        /* Object operations */
        Object& makeObject(Player& pl, ObjectKind kind);
        void releaseObject(Object* obj);
        static int32_t moveObjectTowards(Object& obj, Position toPos);

        /* Ship operations */
        void computeTorpLimit(Ship& attacker, const Ship& ship, int num_torpers, const Environment& env) const;
        void hitShipWith(Ship& sh, const Ship& firing_ship, int damage, int kill, int death_flag) const;
        void rechargeShip(Ship& ship);

        /* Player operations */
        void doPlayerGC(Player& p);

        /* Combat phases */
        void chooseEnemy(Fleet& fleet, const Environment& env, Visualizer& vis, size_t fleetNr);
        void launchFighters(const Fleet& fleet, Visualizer& vis);
        void fireTorps(const Fleet& fleet, const Environment& env, Visualizer& vis);
        void fireBeams(const Fleet& fleet, const Environment& env, Visualizer& vis);
        bool endCheck() const;
        void computeNewPosition(Fleet& fleet, const Environment& env, Visualizer& vis, size_t fleetNr);
        void doFleetGC(Fleet& fleet, const Environment& env, Visualizer& vis, size_t fleetNr);
        void fighterIntercept(Player& a, Player& b, Visualizer& vis);
        bool tryIntercept(Object& pa, Object& pb, Visualizer& vis);
        void fightersFire(const Player& player, Visualizer& vis) const;
        void findNewBase(const Player& player, Object& fighter) const;
        void moveStuff(Player& player, Visualizer& vis);

        /* Misc */
        void renderAll(Visualizer& vis) const;

        /* Debugging */
        void printCheckpoint(std::ostream& os);
    };

} } }

#endif
