/**
  *  \file game/sim/classresult.hpp
  *  \brief Class game::sim::ClassResult
  */
#ifndef C2NG_GAME_SIM_CLASSRESULT_HPP
#define C2NG_GAME_SIM_CLASSRESULT_HPP

#include "game/playerarray.hpp"
#include "afl/base/types.hpp"
#include "game/sim/result.hpp"

namespace game { namespace sim {

    class Setup;

    /** Result summary grouped by result.
        Classes are defined by having similar result sets; this object contains information about one such class.
        Similarity is so far defined by identical numbers of surviving ships. */
    class ClassResult {
     public:
        /** Constructor.
            Builds a new ClassResult from simulator output.
            \param newState  [in] Simulator output
            \param result    [in] Simulator meta-information */
        ClassResult(const Setup& newState, const Result& result);
        ~ClassResult();

        /** Get class description.
            \return class (units owned for each player) */
        const PlayerArray<int>& getClass() const;

        /** Get weight.
            The weight is either a plain occurence count, or a weighed sum of occurences, depending on the setup and options.
            This class' probability is its weight relative to the sum of all weights.
            \return weight */
        int32_t getWeight() const;

        /** Get sample battle.
            \return last battle that produced this result */
        Database_t getSampleBattle() const;

        void changeWeight(int32_t oldWeight, int32_t newWeight);

        /** Check same class.
            \param other Other result
            \return both results are of the same class */
        bool isSameClass(const ClassResult& other) const;

        /** Add new result of same class.
            Updates statistics counter accordingly.
            \param other Other (newer) result
            \pre isSameClass(other) */
        void addSameClassResult(const ClassResult& other);

     private:
        PlayerArray<int> m_ownedUnits;        ///< Number of units owned by this player; identifies this result class. ex owned_units.
        int32_t m_weight;                     ///< Total weight of this result. ex cumulative_weight.
        Database_t m_sampleBattle;            ///< An example battle. ex sample_battle.
    };

} }

#endif
