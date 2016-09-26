/**
  *  \file game/vcr/score.hpp
  *  \brief Class game::vcr::Score
  */
#ifndef C2NG_GAME_VCR_SCORE_HPP
#define C2NG_GAME_VCR_SCORE_HPP

#include "afl/base/types.hpp"

namespace game { namespace vcr {

    /** Scores for a fight.
        Units accumulate scores during fights.
        This object can be used to track these scores for a unit. */
    class Score {
     public:
        /** Constructor.
            Make a blank object. */
        Score();

        /** Add build point range.
            Build points can differ for aggressor and opponent, so we may only be able to give a range.
            Build points are scaled by factor 1000 to eliminate rounding errors.
            \param min Minimum build points x 1000
            \param max Maximum build points x 1000 */
        void addBuildMillipoints(int32_t min, int32_t max);

        /** Add experience points.
            \param points Experience points */
        void addExperience(int32_t points);

        /** Add destroyed tons.
            \param tons Destroyed tons (=weight of opponent) */
        void addTonsDestroyed(int32_t tons);

        /** Get minimum build points.
            \return Minimum build points x 1000 */
        int getBuildMillipointsMin() const;

        /** Get maximum build points.
            \return Maximum build points x 1000 */
        int getBuildMillipointsMax() const;

        /** Get experience points.
            \return Experience points */
        int getExperience() const;

        /** Get destroyed tons.
            \return Destroyed tons */
        int getTonsDestroyed() const;

     private:
        int32_t m_buildMillipointsMin;
        int32_t m_buildMillipointsMax;
        int32_t m_experience;
        int32_t m_tonsDestroyed;
    };

} }

#endif
