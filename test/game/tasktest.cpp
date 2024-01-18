/**
  *  \file test/game/tasktest.cpp
  *  \brief Test for game::Task
  */

#include "game/task.hpp"
#include "afl/test/testrunner.hpp"

/** Test makeResultTask(). */
AFL_TEST("game.Task:makeResultTask", a)
{
    bool result = false;
    std::auto_ptr<game::StatusTask_t> t = game::makeResultTask(result);
    t->call(true);
    a.check("", result);
}

/** Test makeConfirmationTask(). */
AFL_TEST("game.Task:makeConfirmationTask", a)
{
    bool result = false;
    std::auto_ptr<game::Task_t> t = game::makeConfirmationTask(true, game::makeResultTask(result));
    t->call();
    a.check("", result);
}

/** Test makeConfirmationTask(), "then" branch. */
AFL_TEST("game.Task:makeConfirmationTask:then", a)
{
    bool branch1 = false, branch2 = false;
    std::auto_ptr<game::StatusTask_t> t = game::makeConditionalTask(
        game::makeConfirmationTask(true, game::makeResultTask(branch1)),
        game::makeConfirmationTask(true, game::makeResultTask(branch2))
        );
    t->call(true);
    a.check("01. then", branch1);
    a.check("02. else", !branch2);
}

/** Test makeConfirmationTask(), "otherwise" branch. */
AFL_TEST("game.Task:makeConfirmationTask:else", a)
{
    bool branch1 = false, branch2 = false;
    std::auto_ptr<game::StatusTask_t> t = game::makeConditionalTask(
        game::makeConfirmationTask(true, game::makeResultTask(branch1)),
        game::makeConfirmationTask(true, game::makeResultTask(branch2))
        );
    t->call(false);
    a.check("01. then", !branch1);
    a.check("02. else", branch2);
}
