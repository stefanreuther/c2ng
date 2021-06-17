/**
  *  \file game/vcr/score.hpp
  *  \brief Class game::vcr::Score
  */
#ifndef C2NG_GAME_VCR_SCORE_HPP
#define C2NG_GAME_VCR_SCORE_HPP

#include "afl/base/types.hpp"
#include "util/range.hpp"

namespace game { namespace vcr {

    /** Scores for a fight.
        Units accumulate scores during fights.
        This object can be used to track these scores for a unit. */
    class Score {
     public:
        typedef util::Range<int32_t> Range_t;

        /** Constructor.
            Make a blank object. */
        Score();

        /** Add build point range.
            Build points can differ for aggressor and opponent, so we may only be able to give a range.
            Build points are scaled by factor 1000 to eliminate rounding errors.
            \param r Range of points x 1000, should not be empty */
        void addBuildMillipoints(Range_t r);

        /** Add experience points.
            \param points Experience points */
        void addExperience(Range_t points);

        /** Add destroyed tons.
            \param tons Destroyed tons (=weight of opponent) */
        void addTonsDestroyed(Range_t tons);

        /** Get build points.
            \return Build points range x 1000 */
        Range_t getBuildMillipoints() const;

        /** Get experience points.
            \return Experience points range */
        Range_t getExperience() const;

        /** Get destroyed tons.
            \return Destroyed tons range */
        Range_t getTonsDestroyed() const;

     private:
        Range_t m_buildMillipoints;
        Range_t m_experience;
        Range_t m_tonsDestroyed;
    };

} }

#endif
