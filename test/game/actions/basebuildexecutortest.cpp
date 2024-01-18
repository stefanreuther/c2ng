/**
  *  \file test/game/actions/basebuildexecutortest.cpp
  *  \brief Test for game::actions::BaseBuildExecutor
  */

#include "game/actions/basebuildexecutor.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.actions.BaseBuildExecutor")
{
    class Tester : public game::actions::BaseBuildExecutor {
     public:
        virtual void setBaseTechLevel(game::TechLevel /*area*/, int /*value*/)
            { }
        virtual void setBaseStorage(game::TechLevel /*area*/, int /*index*/, int /*value*/, int /*free*/)
            { }
        virtual void accountHull(int /*number*/, int /*count*/, int /*free*/)
            { }
        virtual void accountFighterBay(int /*count*/)
            { }
    };
    Tester t;
}
