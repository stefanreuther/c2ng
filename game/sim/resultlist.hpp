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
#include "util/numberformatter.hpp"
#include "afl/string/translator.hpp"

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
        /** Formatted version of a ClassResult. */
        struct ClassInfo {
            String_t label;
            int32_t weight;
            PlayerArray<int> ownedUnits;
            bool hasSample;

            ClassInfo()
                : label(), weight(), ownedUnits(), hasSample()
                { }
        };

        /** Formatted version of a UnitResult. */
        struct UnitInfo {
            /** Type of range information. */
            enum Type {
                Damage,
                Shield,
                DefenseLost,
                NumBaseFightersLost,
                MinFightersAboard,
                Crew,
                NumFightersLost,
                NumFightersRemaining,
                NumTorpedoesFired,
                NumTorpedoesRemaining,
                NumTorpedoHits
            };
            static const size_t MAX_TYPE = static_cast<size_t>(NumTorpedoHits);

            /** Range information item. */
            struct Item {
                Type type;                ///< Type.
                int32_t min;              ///< Minimum value.
                int32_t max;              ///< Maximum value.
                double average;           ///< Average.
                bool hasMinSample;        ///< true if a sample battle for minimum value exists.
                bool hasMaxSample;        ///< true if a sample battle for maximum value exists.
                Item(Type type, int32_t min, int32_t max, double average, bool hasMinSample, bool hasMaxSample)
                    : type(type), min(min), max(max), average(average), hasMinSample(hasMinSample), hasMaxSample(hasMaxSample)
                    { }
            };

            int32_t numFightsWon;         ///< Number of fights won.
            int32_t numFights;            ///< Number of fights taken part in.
            int32_t numCaptures;          ///< Number of fights unit got captured in.
            int32_t cumulativeWeight;     ///< Cumulative weight; number to divide numFights etc. by to get percentages.
            bool hasAbsoluteCounts;       ///< true if numFights etc. are actual counts; false if they are only relative to cumulativeWeight.
            std::vector<Item> info;       ///< Range information.
            UnitInfo()
                : numFightsWon(0), numFights(0), numCaptures(0), cumulativeWeight(1), hasAbsoluteCounts(false), info()
                { }
        };

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

        /** Describe class result.
            \param index Index [0,getNumClassResults())
            \param fmt  Number formatter
            \return Information; empty/default if index out of range. */
        ClassInfo describeClassResult(size_t index, const util::NumberFormatter& fmt) const;

        /** Describe unit result.
            \param index Index [0,getNumUnitResults())
            \param setup Simulation setup
            \return Information; empty/default if index out of range. */
        UnitInfo describeUnitResult(size_t index, const Setup& setup) const;

        /** Get sample battle.
            \param index Unit index
            \param type  Type of battle
            \param max   true to return maximum specimen, false to return minimum
            \return Sample battle; 0 if none */
        afl::base::Ptr<game::vcr::Database> getUnitSampleBattle(size_t index, UnitInfo::Type type, bool max) const;

        /** Get number of battles fought.
            \return Number of addResult() invocations */
        size_t getNumBattles() const;

        /** Get class result index of last result added.
            This can be used to highlight the class result in a user interface, but has no other meaning.
            \return Index */
        size_t getLastClassResultIndex() const;

     private:
        int32_t m_totalWeight;            ///< Total weight. This is the value to which the battles are "normalized". ex total_weight. FIXME: name
        int32_t m_cumulativeWeight;       ///< Sum of weights of all fights. ex cumulative_weight. FIXME: name
        int m_numBattles;                 ///< Total number of battles so far. ex battle_count.
        size_t m_lastClassResultIndex;    ///< Last class result index.

        typedef afl::container::PtrVector<UnitResult> UnitResults_t;
        typedef afl::container::PtrVector<ClassResult> ClassResults_t;

        UnitResults_t  m_unitResults;     ///< Per-unit results for each unit. ex unit_results.
        ClassResults_t m_classResults;    ///< Per-class results for each class. ex class_results.

        size_t updateClassResultSortOrder(size_t change_index);
        UnitInfo::Item packItem(UnitInfo::Type type, const UnitResult::Item& item) const;
    };

    /** Get human-readable string representation of a UnitInfo::Type.
        \param type value
        \param tx Translator
        \return string */
    String_t toString(ResultList::UnitInfo::Type type, afl::string::Translator& tx);

} }

#endif
