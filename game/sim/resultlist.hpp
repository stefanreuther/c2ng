/**
  *  \file game/sim/resultlist.hpp
  *  \brief Class game::sim::ResultList
  */
#ifndef C2NG_GAME_SIM_RESULTLIST_HPP
#define C2NG_GAME_SIM_RESULTLIST_HPP

#include "afl/base/memory.hpp"
#include "afl/base/types.hpp"
#include "afl/container/ptrvector.hpp"
#include "game/sim/classresult.hpp"
#include "game/sim/unitresult.hpp"
#include "game/vcr/statistic.hpp"

namespace game { namespace sim {

    class Setup;
    class Result;
    
    /** Summary of a sequence of simulations.
        Contains statistics for each unit (UnitResult)
        as well as different result classes including example battles (ClassResult).

        Every battle can have a weight.
        For example, with Host's left/right balancing,
        one particular setup with one particular seed has a 59-vs-41% weight of occuring.
        We do not simulate these 59+41 battles, but instead only simulate two and adjust their weights accordingly.
        For memory efficiency, only the ResultList stores the effective total weight,
        the UnitResult::Item's do not know the value internally. */
    class ResultList {
     public:
        /** Make blank ResultList. */
        ResultList();

        /** Destructor. */
        ~ResultList();

        /** Incorporate result into this object.

            The first call must have res.this_battle_index=0, subsequent calls must have res.this_battle_index!=0.

            Both states (oldState, newState) must have the same structure (same number of ships, planets).

            \param oldState [in] Simulator input
            \param newState [in] Simulator output
            \param stats    [in] VCR statistics provided; array must parallel the Setup::getObject().
            \param result   [in] Result meta-information provided by simulator */
        void addResult(const Setup& oldState, const Setup& newState, afl::base::Memory<const game::vcr::Statistic> stats, Result result);

        /** Get cumulative weight.
            This is the sum of all weights of all simulated battles.
            \return cumulative weight */
        int32_t getCumulativeWeight() const;

        /** Get total weight to which weights are normalized.
            \return weight */
        int32_t getTotalWeight() const;

        /** Get number of result classes.
            \return Number of result classes */
        size_t getNumClassResults() const;

        /** Get number of unit results.
            \return Number of unit results */
        size_t getNumUnitResults() const;

        /** Get class result.
            \param index Index [0,getNumClassResults())
            \return Class result; 0 if index out of range */
        const ClassResult* getClassResult(size_t index) const;

        /** Get unit result.
            \param index Index [0,getNumUnitResults()), same as index to Setup::getObject()
            \return Class result; 0 if index out of range */
        const UnitResult* getUnitResult(size_t index) const;

        /** Get number of battles fought.
            \return Number of addResult() invocations */
        size_t getNumBattles() const;

     private:
        int32_t m_totalWeight;            ///< Total weight. This is the value to which the battles are "normalized". ex total_weight. FIXME: name
        int32_t m_cumulativeWeight;       ///< Sum of weights of all fights. ex cumulative_weight. FIXME: name
        int m_numBattles;                ///< Total number of battles so far. ex battle_count.

        typedef afl::container::PtrVector<UnitResult> UnitResults_t;
        typedef afl::container::PtrVector<ClassResult> ClassResults_t;
    
        UnitResults_t  m_unitResults;     ///< Per-unit results for each unit. ex unit_results.
        ClassResults_t m_classResults;    ///< Per-class results for each class. ex class_results.

        void updateClassResultSortOrder(size_t change_index);
    };

} }

#endif
