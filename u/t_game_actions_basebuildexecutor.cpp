/**
  *  \file u/t_game_actions_basebuildexecutor.cpp
  *  \brief Test for game::actions::BaseBuildExecutor
  */

#include "game/actions/basebuildexecutor.hpp"

#include "t_game_actions.hpp"

/** Interface test. */
void
TestGameActionsBaseBuildExecutor::testInterface()
{
    class Tester : public game::actions::BaseBuildExecutor {
     public:
        virtual void setBaseTechLevel(game::TechLevel /*area*/, int /*value*/)
            { }
        virtual void setBaseStorage(game::TechLevel /*area*/, int /*index*/, int /*value*/, int /*free*/)
            { }
        virtual void accountHull(int /*number*/, int /*count*/, int /*free*/)
            { }
    };
    Tester t;
}

