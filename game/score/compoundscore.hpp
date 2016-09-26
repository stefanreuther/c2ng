/**
  *  \file game/score/compoundscore.hpp
  */
#ifndef C2NG_GAME_SCORE_COMPOUNDSCORE_HPP
#define C2NG_GAME_SCORE_COMPOUNDSCORE_HPP

#include "game/score/scoreid.hpp"
#include "game/score/turnscore.hpp"
#include "game/playerset.hpp"

namespace game { namespace score {

    class TurnScoreList;

    class CompoundScore {
     public:
        enum DefaultScore {
            TotalShips,
            TimScore
        };
        
        typedef TurnScore::Value_t Value_t;

        CompoundScore();

        CompoundScore(const TurnScoreList& list, ScoreId_t id, int factor);

        CompoundScore(const TurnScoreList& list, DefaultScore kind);

        void add(const TurnScoreList& list, ScoreId_t id, int factor);

        Value_t get(const TurnScore& turn, int player) const;

        Value_t get(const TurnScore& turn, PlayerSet_t players) const;

        Value_t get(const TurnScoreList& list, int turnNr, int player) const;

        Value_t get(const TurnScoreList& list, int turnNr, PlayerSet_t players) const;

     private:
        // FIXME: WScore also has a name.
        static const size_t MAX = 4;
        bool m_valid;
        size_t m_numParts : 8;
        TurnScore::Slot_t m_slot[MAX];
        int m_factor[MAX];
    };
 
} }

#endif
