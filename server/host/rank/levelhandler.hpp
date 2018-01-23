/**
  *  \file server/host/rank/levelhandler.hpp
  *  \brief Class server::host::rank::LevelHandler
  */
#ifndef C2NG_SERVER_HOST_RANK_LEVELHANDLER_HPP
#define C2NG_SERVER_HOST_RANK_LEVELHANDLER_HPP

#include "server/host/root.hpp"
#include "server/host/game.hpp"

namespace server { namespace host { namespace rank {

    /** Ranking Level Handling.
        This class contains methods to deal with player reliability/skill ratings and associated rank levels. */
    class LevelHandler {
     public:
        /** Constructor.
            \param root Service root */
        explicit LevelHandler(Root& root);

        /** Handle player turn (non-)submission.
            This updates the player's reliability.
            \param userId User Id
            \param submit true iff turn was submitted
            \param level  Replacement level (0=current player in charge of slot, 1=replacement, 2=replacement's replacement);
                          relevant only for non-submissions */
        void handlePlayerTurn(String_t userId, bool submit, uint32_t level);

        /** Handle player dropout.
            This updates the player's reliability.
            \param userId User Id
            \param game Game to work on
            \param slot Slot number */
        void handlePlayerDrop(String_t userId, Game& game, int slot);

        /** Add skill points.
            This updates skill points after the end of a game.
            \param userId User Id
            \param pts Point delta. This is normally positive, but can be negative if a game is re-judged. */
        void addPlayerRankPoints(String_t userId, int32_t pts);

        /** Check possible required rank changes.
            This will determine whether the user's new stats award them a higher or lower rank,
            and, if so, move them there and send mail.
            Call this after calling any of the other methods.
            \param userId User Id */
        void handlePlayerRankChanges(String_t userId);

     private:
        Root& m_root;
    };

} } }

#endif
