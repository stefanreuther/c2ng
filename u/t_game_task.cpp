/**
  *  \file u/t_game_task.cpp
  *  \brief Test for game::Task
  */

#include "game/task.hpp"

#include "t_game.hpp"

/** Test makeResultTask(). */
void
TestGameTask::testMakeResultTask()
{
    bool result = false;
    std::auto_ptr<game::StatusTask_t> t = game::makeResultTask(result);
    t->call(true);
    TS_ASSERT(result);
}

/** Test makeConfirmationTask(). */
void
TestGameTask::testMakeConfirmationTask()
{
    bool result = false;
    std::auto_ptr<game::Task_t> t = game::makeConfirmationTask(true, game::makeResultTask(result));
    t->call();
    TS_ASSERT(result);
}

/** Test makeConfirmationTask(), "then" branch. */
void
TestGameTask::testMakeConditionalTask1()
{
    bool branch1 = false, branch2 = false;
    std::auto_ptr<game::StatusTask_t> t = game::makeConditionalTask(
        game::makeConfirmationTask(true, game::makeResultTask(branch1)),
        game::makeConfirmationTask(true, game::makeResultTask(branch2))
        );
    t->call(true);
    TS_ASSERT(branch1);
    TS_ASSERT(!branch2);
}

/** Test makeConfirmationTask(), "otherwise" branch. */
void
TestGameTask::testMakeConditionalTask2()
{
    bool branch1 = false, branch2 = false;
    std::auto_ptr<game::StatusTask_t> t = game::makeConditionalTask(
        game::makeConfirmationTask(true, game::makeResultTask(branch1)),
        game::makeConfirmationTask(true, game::makeResultTask(branch2))
        );
    t->call(false);
    TS_ASSERT(!branch1);
    TS_ASSERT(branch2);
}

