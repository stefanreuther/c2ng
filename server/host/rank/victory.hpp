/**
  *  \file server/host/rank/victory.hpp
  *  \brief Victory Recognition and Ranking
  */
#ifndef C2NG_SERVER_HOST_RANK_VICTORY_HPP
#define C2NG_SERVER_HOST_RANK_VICTORY_HPP

#include "server/host/game.hpp"
#include "server/host/root.hpp"

namespace server { namespace host { namespace rank {

    /** Check victory condition.
        Call this for a running game, to determine whether it ends according to a victory condition.

        \param root    Service root
        \param gameDir Game directory in host filer
        \param game    Game object

        \retval false Game continues, no changes made
        \retval true  Game ends. Ranks have been saved into database. Caller must mark game finished,
                      send mail, and process ranking system (computeGameRankings). */
    bool checkVictory(Root& root, String_t gameDir, Game& game);

    /** Check victory for forced game end.
        If a game is forcibly terminated, this tries to make up a rank list.
        \param game Game object */
    void checkForcedGameEnd(Game& game);

    /** Compute rank points after a game end.
        Call this after checkVictory() ended a game, or after checkForcedGameEnd().
        It will compute new points, and generate appropriate promotions.
        \param root Service root
        \param game Game object */
    void computeGameRankings(Root& root, Game& game);

} } }

#endif
