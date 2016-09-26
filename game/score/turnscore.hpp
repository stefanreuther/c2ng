/**
  *  \file game/score/turnscore.hpp
  */
#ifndef C2NG_GAME_SCORE_TURNSCORE_HPP
#define C2NG_GAME_SCORE_TURNSCORE_HPP

#include <vector>
#include "game/timestamp.hpp"
#include "afl/base/inlineoptional.hpp"
#include "afl/base/types.hpp"

namespace game { namespace score {

    // /** Score file record. Contains one turn's scores, as a two-dimensional array (see operator()).
    // The indexes are variable according to the GStatFile's schema. */
    class TurnScore {
     public:
        typedef size_t Slot_t;

        typedef afl::base::InlineOptional<int32_t,-1> Value_t;

        TurnScore(int turn, Timestamp time);

        ~TurnScore();

        int getTurnNumber() const;

        const Timestamp& getTimestamp() const;

        void set(Slot_t slot, int player, Value_t value);

        Value_t get(Slot_t slot, int player) const;

     private:
        int m_turnNumber;
        Timestamp m_timestamp;
        std::vector<Value_t> m_values;
    };

} }

#endif
