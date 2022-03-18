/**
  *  \file game/score/turnscore.hpp
  *  \brief Class game::score::TurnScore
  */
#ifndef C2NG_GAME_SCORE_TURNSCORE_HPP
#define C2NG_GAME_SCORE_TURNSCORE_HPP

#include <vector>
#include "game/timestamp.hpp"
#include "afl/base/inlineoptional.hpp"
#include "afl/base/types.hpp"

namespace game { namespace score {

    /** Score file record.
        Contains one turn's scores, as a two-dimensional array (see operator())
        mapping player numbers and score indexes to values.
        The indexes are variable according to the TurnScoreList's schema. */
    class TurnScore {
     public:
        /** Slot identifier. */
        typedef size_t Slot_t;

        /** Score value.
            Scores can be unknown. */
        typedef afl::base::InlineOptional<int32_t,-1> Value_t;

        /** Constructor.
            \param turnNumber Turn number
            \param time       Timestamp */
        TurnScore(int turnNumber, Timestamp time);

        /** Destructor. */
        ~TurnScore();

        /** Get turn number.
            \return turn number */
        int getTurnNumber() const;

        /** Get timestamp.
            \return time stamp */
        const Timestamp& getTimestamp() const;

        /** Set value.
            If the slot/player are not valid, the call is ignored.
            \param slot Slot identifier
            \param player Player number [1,MAX_PLAYERS]
            \param value New score value */
        void set(Slot_t slot, int player, Value_t value);

        /** Get value.
            \param slot Slot identifier
            \param player Player number [1,MAX_PLAYERS]
            \return Score. Unknown if slot/player are out of range, or this entry has never been set */
        Value_t get(Slot_t slot, int player) const;

     private:
        /** Turn number. */
        int m_turnNumber;

        /** Timestamp. */
        Timestamp m_timestamp;

        /** Values.
            This array is maintained as an array of slices of size MAX_PLAYERS. */
        std::vector<Value_t> m_values;
    };

} }

#endif
