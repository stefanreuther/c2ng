/**
  *  \file game/actions/basebuildexecutor.hpp
  *  \brief Interface game::actions::BaseBuildExecutor
  */
#ifndef C2NG_GAME_ACTIONS_BASEBUILDEXECUTOR_HPP
#define C2NG_GAME_ACTIONS_BASEBUILDEXECUTOR_HPP

#include "afl/base/deletable.hpp"
#include "game/types.hpp"

namespace game { namespace actions {

    /** Abstract starbase build action executor.
        Derived classes receive requests from starbase build actions to
        produce computations or perform the actual build. */
    class BaseBuildExecutor : public afl::base::Deletable {
     public:
        /** Change tech level.
            \param area which tech level to change
            \param value new value */
        virtual void setBaseTechLevel(TechLevel area, int value) = 0;

        /** Change component storage.
            This modifies the hull, engine, beam, or torpedo launcher storage.
            \param area area
            \param index index (truehull slot, engine number, beam number, torpedo number)
            \param value new number of components
            \param free additional number of (previously present) components consumed by action at no cost */
        virtual void setBaseStorage(TechLevel area, int index, int value, int free) = 0;

        /** Account for a foreign hull. Intended for price computations involving hulls the player
            can not build. Such a change cannot be committed.
            \param number hull slot [1,NUM_HULLS]
            \param count  number of hulls to build
            \param free   additional number of (previously present) hulls consumed by action at no cost */
        virtual void accountHull(int number, int count, int free) = 0;

        /** Account fighter bays.
            Just for generating cost summaries.
            \param count Number of fighter bays */
        virtual void accountFighterBay(int count) = 0;
    };

} }

#endif
