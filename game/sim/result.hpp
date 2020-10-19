/**
  *  \file game/sim/result.hpp
  *  \brief Class game::sim::Result
  */
#ifndef C2NG_GAME_SIM_RESULT_HPP
#define C2NG_GAME_SIM_RESULT_HPP

#include "afl/base/ptr.hpp"
#include "afl/base/types.hpp"
#include "game/sim/configuration.hpp"
#include "game/vcr/database.hpp"

namespace game { namespace sim {

    typedef afl::base::Ptr<game::vcr::Database> Database_t;

    /** Result of a single simulation.
        In addition to a set of battles and an updated status,
        the simulator can return a weight.
        This is exclusively used in seed control mode.
        For example, when doing left/right balancing using Balance360k,
        the fight with bonus has a probability of 59%; the unmodified fight appears with 41%.
        In addition, appearance of this bonus increases series length from 110 to 220.

        - driver calls init to initialize parameters from configuration and set this_battle_index.
        - simulator updates this_battle_weight, total_battle_weight, series_length
          with information from the series. */
    // ex GSimBattleResult
    class Result {
     public:
        /** Weight of this battle for statistics purposes.
            Filled in by simulator.
            Called 'BattleWeight' in ccsim.pas. */
        int32_t this_battle_weight;

        /** Total weight of a series.
            Filled in by simulator.

            This field must be constant for one set of options;
            it is mostly used for detecting and fixing when it happens to be NOT constant to avoid generating too bad garbage.
            If everything works correctly, this field is redundant.

            Called 'Divider' in ccsim.pas. */
        int32_t total_battle_weight;

        /** Length of a series.
            This is the number of possibly-different battles that can appear.
            Filled in by simulator, must be constant for one set of options.

            Called 'Modulus' in ccsim.pas. */
        int32_t series_length;

        /** Index of this battle, 0-based.
            Filled in by driver code.
            Used by simulator to determine where in a non-equal set we are. */
        int32_t this_battle_index;

        /** Actual battle. Filled in by simulator. */
        Database_t battles;

        /** Constructor. */
        // FIXME: merge with init?
        Result()
            : this_battle_weight(1), total_battle_weight(1), series_length(1), this_battle_index(0), battles()
            { }

        /** Initialize.
            \param config            Simulation configuration
            \param this_battle_index Index of the battle to be simulated, 0-based. */
        void init(const Configuration& config, int this_battle_index);

        /** Add a series of a given length.
            Returns the position in the series.
            For example, "addSeries(2)" says that we double the length of a series because there are two cases to handle,
            and returns 0 or 1 saying in which case we are:
            during the first series (e.g. this_battle_index = [0,109]), the result will be 0;
            during the second series (e.g. this_battle_index = [110,219]), the result will be 1.

            \param length Length of new series

            \return position */
        int addSeries(int length);

        /** Adjust weight.
            Force this battle's weight to the given value.
            Use this to make the counters comparable to another Result with the specified weight.
            \param new_weight New weight */
        void changeWeightTo(int32_t new_weight);
    };

} }

#endif
