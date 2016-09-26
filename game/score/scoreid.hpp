/**
  *  \file game/score/scoreid.hpp
  */
#ifndef C2NG_GAME_SCORE_SCOREID_HPP
#define C2NG_GAME_SCORE_SCOREID_HPP

#include "afl/base/types.hpp"

// --- Type 51 (Player Score) [PHost 3.4d / 4.0+] ---
//  +0  50 BYTEs   Name; player-readable description of this score.
// +50     WORD    Identifier
//                  1      Per-game Score. This is intended to be used by
//                         the "main" scoring program in use for this game
//                         to report "the score".
//                  2      Build points (PAL / PBP)
//                  3      Minefields allowed
//                  4      Minefields laid
//                  5..99  reserved
//                  100+   available
//                  1000   (c2nu) Military Score
//                  1001   (c2nu) Inventory Score
// +52     WORD    Turns-over-limit required to win, -1 if not known.
// +54     DWORD   Win limit, -1 if none
// +58  11 DWORDs  Scores for each player. -1 if not known.

namespace game { namespace score {

    typedef int16_t ScoreId_t;

    const ScoreId_t ScoreId_Planets      = -1;
    const ScoreId_t ScoreId_Capital      = -2;
    const ScoreId_t ScoreId_Freighters   = -3;
    const ScoreId_t ScoreId_Bases        = -4;

    const ScoreId_t ScoreId_Score        = 1;
    const ScoreId_t ScoreId_BuildPoints  = 2;
    const ScoreId_t ScoreId_MinesAllowed = 3;
    const ScoreId_t ScoreId_MinesLaid    = 4;

} }

#endif
