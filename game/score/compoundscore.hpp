/**
  *  \file game/score/compoundscore.hpp
  *  \brief Class game::score::CompoundScore
  */
#ifndef C2NG_GAME_SCORE_COMPOUNDSCORE_HPP
#define C2NG_GAME_SCORE_COMPOUNDSCORE_HPP

#include "game/score/scoreid.hpp"
#include "game/score/turnscore.hpp"
#include "game/playerset.hpp"

namespace game { namespace score {

    class TurnScoreList;

    /** Compound score.
        This class provides a front end to query a TurnScoreList or TurnScore object.
        In particular, it allows building compound scores, such as "Total Ships" being the sum of ScoreId_Freighters and ScoreId_Capital,
        and it allows querying scores for player sets (i.e. teams).

        As a general rule, a compound score query produces a valid result if all columns (Score Ids) are present in the TurnScoreList schema,
        and at least some of them have a known value.
        That is, "Total Ships" can be computed if the score file has the ScoreId_Freighters and ScoreId_Capital columns,
        even if the ScoreId_Capital for a particular player is not known,
        or if the score for a team is being computed and one team member's scores are not known.

        CompoundScore has a maximum limit of components and goes into an invalid state when that limit is exceeded.
        In invalid state, it answers all queries with "unknown".
        As of 20170409, that component limit is 4.

        Since scores are physically indexed by slot numbers, and a TurnScoreList manages the mapping from score Ids to slot numbers,
        the TurnScoreList object used with calls to a CompoundScore instance must always be the same. */
    class CompoundScore {
     public:
        /** Default score.
            Use these with the constructor to build default scores. */
        enum DefaultScore {
            TotalShips,           /**< Total Ships score (sum of freighters and capital ships). */
            TimScore              /**< Tim-Score (one point per freighter, 10 for capital/planets, 120 for bases). */
        };

        /** Score value.
            Scores can be unknown. */
        typedef TurnScore::Value_t Value_t;

        /** Default constructor.
            Builds a score with no components.
            Use add() to add some. */
        CompoundScore();

        /** Construct single-slot score.
            This is a shortcut for default-constructing a CompoundScore and then adding a single component.
            \param list TurnScoreList containing score definitions
            \param id   Score Id (NOT slot!)
            \param factor Scale factor */
        CompoundScore(const TurnScoreList& list, ScoreId_t id, int factor);

        /** Construct default score.
            This is a shortcut for default-constructing a CompoundScore and then adding components.
            \param list TurnScoreList containing score definitions
            \param kind Kind of score */
        CompoundScore(const TurnScoreList& list, DefaultScore kind);

        /** Add a score component.
            \param list TurnScoreList containing score definitions
            \param id Score Id (NOT slot!)
            \param factor Scale factor */
        void add(const TurnScoreList& list, ScoreId_t id, int factor);

        /** Get score from turn, single player.
            \param turn TurnScore object
            \param player Player number
            \return score */
        Value_t get(const TurnScore& turn, int player) const;

        /** Get score from turn, player list.
            \param turn TurnScore object
            \param players Player set
            \return score */
        Value_t get(const TurnScore& turn, PlayerSet_t players) const;

        /** Get score from turn, single player.
            \param turn TurnScoreList object
            \param turnNr Turn number
            \param player Player number
            \return score; unknown if the requested turn does not exist */
        Value_t get(const TurnScoreList& list, int turnNr, int player) const;

        /** Get score from turn, player list.
            \param turn TurnScoreList object
            \param turnNr Turn number
            \param players Player set
            \return score; unknown if the requested turn does not exist */
        Value_t get(const TurnScoreList& list, int turnNr, PlayerSet_t players) const;

     private:
        // FIXME: the origin class, WScore, also managed a name.

        /** Maximum number of components. */
        static const size_t MAX = 4;

        /** Validity flag.
            A CompoundScore is invalid if an unknown ScoreId_t is added, or the component limit was exceeded. */
        bool m_valid;

        /** Number of components. */
        size_t m_numParts : 8;

        /** Slot numbers for each component. */
        TurnScore::Slot_t m_slot[MAX];

        /** Scale factors for each component. */
        int m_factor[MAX];
    };

} }

#endif
