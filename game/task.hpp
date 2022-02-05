/**
  *  \file game/task.hpp
  *  \brief Tasks
  *
  *  In general, all game operations are allowed to block for I/O or computation.
  *  However, some operations may call back into the user interface.
  *  To not block the game thread for such operations, we implement them as a task.
  *
  *  Each such operation consists of a list of tasks, built using a chain
  *
  *       someTask(nextTask(nextTask(finalTask())))
  *
  *  Each elementary task knows its successor, and calls it as its last operation.
  *  It can at any time stop executing and cause some external trigger (=UI) to resume at this very place
  *  using a mechanism not specified here.
  *
  *  The invoking module must stash away the created task and invoke call() on it.
  *  The final task will notify the invoking module, which destroys the entire list.
  *
  *  Tasks must not throw.
  *
  *  Destroying the list before the task completes must always be supported to cancel a task.
  *  Programs that do not support interactivity can use
  *
  *      someTask(...)->call();
  *
  *  which will execute the list, and cancel it if it requires interactivity.
  */
#ifndef C2NG_GAME_TASK_HPP
#define C2NG_GAME_TASK_HPP

#include <memory>
#include "afl/base/closure.hpp"

namespace game {

    /** Generic type for a pending task.
        Must not throw. */
    typedef afl::base::Closure<void()> Task_t;

    /** Task that receives an I/O status.
        Must not throw. */
    typedef afl::base::Closure<void(bool)> StatusTask_t;

    /** Create a task that confirms an operation.
        @param flag Result to report
        @param then Task to receive status
        @return newly-allocated task */
    std::auto_ptr<Task_t> makeConfirmationTask(bool flag, std::auto_ptr<StatusTask_t> then);

    /** Create a task that stashes away a result.
        @param [out] result Result to store
        @return newly-allocated task */
    std::auto_ptr<StatusTask_t> makeResultTask(bool& result);

    /** Create a conditional task.
        @param then      Task to execute when task is invoked with a success result
        @param otherwise Task to execute when task is invoked with a failure result
        @return newly-allocated task */
    std::auto_ptr<StatusTask_t> makeConditionalTask(std::auto_ptr<Task_t> then, std::auto_ptr<Task_t> otherwise);

}

#endif
