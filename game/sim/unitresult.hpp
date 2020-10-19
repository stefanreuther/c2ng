/**
  *  \file game/sim/unitresult.hpp
  *  \brief Class game::sim::UnitResult
  */
#ifndef C2NG_GAME_SIM_UNITRESULT_HPP
#define C2NG_GAME_SIM_UNITRESULT_HPP

#include "afl/base/types.hpp"
#include "game/sim/result.hpp"
#include "game/vcr/statistic.hpp"

namespace game { namespace sim {

    class Planet;
    class Ship;

    /** Overall simulation result for a single unit.
        Contains statistics counters for that unit.
        The values are stored relative to the current battle's total_battle_weight which is NOT stored in this object. */
    class UnitResult {
     public:
        /** Statistics counter.
            Counts minimum, maximum and total (for average computation). */
        struct Item {
            int32_t  min;
            int32_t  max;
            int32_t  totalScaled;            // ex total_scaled
            Database_t minSpecimen;          // ex min_specimen
            Database_t maxSpecimen;          // ex max_specimen

            /** Make blank result. */
            Item();

            /** Make inverted result.
                Make result that would have been obtained if each call to add(x,w) would have been replaced by add(subtract_from-x,w).
                \param [in] orig          original object
                \param [in] subtract_from value to subtract this from
                \param [in] scale         total scale (sum of all w.this_battle_weight used in making this result) */
            Item(const Item& orig, int32_t subtract_from, int32_t scale);
        };


        /** Default constructor. */
        UnitResult();

        int getNumFightsWon() const;
        int getNumFights() const;
        int getNumCaptures() const;
        const Item& getNumTorpedoesFired() const;
        const Item& getNumFightersLost() const;
        const Item& getDamage() const;
        const Item& getShield() const;
        const Item& getCrewLeftOrDefenseLost() const;
        const Item& getNumTorpedoHits() const;
        const Item& getMinFightersAboard() const;

        /** Change weight of this unit result.
            Assuming the result so far was obtained using \c old_weight, adjusts all counters such that the result is appropriate for \c new_weight.
            \param oldWeight Old cumulative weight.
            \param newWeight New cumulative weight. */
        void changeWeight(int32_t oldWeight, int32_t newWeight);

        /** Add unit result from ship.
            The first call must have res.this_battle_index=0, subsequent calls must have res.this_battle_index!=0.
            \param oldShip [in] Original ship
            \param newShip [in] Ship at end of battle
            \param stat    [in] Extra statistic from VCR
            \param res     [in] Battle result record (needed for this_battle_index, this_battle_weight) */
        void addResult(const Ship& oldShip, const Ship& newShip, const game::vcr::Statistic& stat, const Result& res);

        /** Add unit result from planet.
            The first call must have res.this_battle_index=0, subsequent calls must have res.this_battle_index!=0.
            \param oldPlanet [in] Original planet
            \param newPlanet [in] Planet at end of battle
            \param stat      [in] Extra statistic from VCR
            \param res       [in] Battle result record (needed for this_battle_index, this_battle_weight) */
        void addResult(const Planet& oldPlanet, const Planet& newPlanet, const game::vcr::Statistic& stat, const Result& res);

     private:
        int m_numFightsWon;                  ///< Number of times this ship survived. ex won.
        int m_numFights;                     ///< Number of times this ship fought. ex fought.
        int m_numCaptures;                   ///< Number of times this ship got captured (survived with different owner). ex captured.
        Item m_numTorpedoesFired;            ///< Number of torpedoes fired (ships and planets). ex torps_fired.
        Item m_numFightersLost;              ///< Number of fighters lost (ships and planets). ex fighters_lost.
        Item m_damage;                       ///< Damage at end (ships and planets). ex damage_taken.
        Item m_shield;                       ///< Shield at end (ships and planets). ex shields_left.
        Item m_crewLeftOrDefenseLost;        ///< Crew left (ships) or defense lost (planets). ex crew_left_or_defense_lost.
        Item m_numTorpedoHits;               ///< Torps hit (ships and planets). ex torps_hit.
        Item m_minFightersAboard;            ///< Minimum fighters on unit at any one time (ships and planets). ex min_fighters_aboard.

        static void add(Item& it, int32_t value, const Result& w);
        static void changeWeight(Item& it, int32_t oldWeight, int32_t newWeight);
    };

} }

#endif
