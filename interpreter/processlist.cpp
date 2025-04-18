/**
  *  \file interpreter/processlist.cpp
  *  \brief Class interpreter::ProcessList
  *
  *  In c2ng, all processes eventually appear in a process list,
  *  where they are scheduled according to their priority, and can wait for various conditions.
  *  In particular, processes can wait for UI, where the UI calls new processes (scripted dialog).
  *  To control execution, processes are grouped into process groups.
  *  This architecture serves as a blueprint for the JavaScript version that may need to wait more often,
  *  and where the runtime just doesn't allow waiting within the interpreter's execution stream.
  *
  *  PCC2 had temporary processes that were used whenever the PCC2 core
  *  needs to evaluate an expression and wishes immediate feedback.
  *  Such processes were not allowed to suspend (miSpecialSuspend).
  *  They could still call UI because that stops the entire execution.
  *  We don't want that anymore; all potential expression feedback needs to be coded as a script
  *  (e.g. filtering a mission-list for valid missions).
  *
  *  For the same purpose as temporary processes, PCC1 had process-less expression evaluation.
  *  This does not support user-defined functions; we therefore do not support that either.
  */

#include "interpreter/processlist.hpp"
#include "interpreter/process.hpp"
#include "interpreter/world.hpp"

using interpreter::Process;

namespace {
    uint32_t allocateId(uint32_t& var)
    {
        ++var;
        if (var == 0) {
            ++var;
        }
        return var;
    }

    bool isTerminatedState(Process::State st)
    {
        bool result = false;
        switch (st) {
         case Process::Ended:
         case Process::Terminated:
         case Process::Failed:
            result = true;
            break;
         case Process::Suspended:
         case Process::Frozen:
         case Process::Runnable:
         case Process::Running:
         case Process::Waiting:
            result = false;
            break;
        }
        return result;
    }
}

// Make new, empty ProcessList
interpreter::ProcessList::ProcessList()
    : m_processes(),
      m_processGroupId(0),
      m_processId(0),
      m_running(false)
{ }

// Destructor.
interpreter::ProcessList::~ProcessList()
{ }

// Create a regular process.
interpreter::Process&
interpreter::ProcessList::create(World& world, String_t name)
{
    // ex int/process.h:createProcess
    Process& proc = *m_processes.pushBackNew(new Process(world, name, allocateProcessId()));
    handlePriorityChange(proc);
    return proc;
}

// Allocate a process group Id.
uint32_t
interpreter::ProcessList::allocateProcessGroup()
{
    return allocateId(m_processGroupId);
}

// Start a process group.
void
interpreter::ProcessList::startProcessGroup(uint32_t pgid)
{
    bool ok = false;
    for (size_t i = 0; i != m_processes.size(); ++i) {
        Process& proc = *m_processes[i];
        if (proc.getState() == Process::Runnable && proc.getProcessGroupId() == pgid) {
            proc.setState(Process::Running);
            ok = true;
            break;
        }
    }
    if (!ok) {
        // Finalize entire process group
        for (size_t i = 0; i != m_processes.size(); ++i) {
            Process& proc = *m_processes[i];
            if (proc.getProcessGroupId() == pgid) {
                proc.finalize();
            }
        }

        // Tell caller
        sig_processGroupFinish.raise(pgid);
    }
}

// Terminate a process group.
void
interpreter::ProcessList::terminateProcessGroup(uint32_t pgid)
{
    // This is very simple:
    // - unlike terminateProcess(), do not try to proceed the next process in the group.
    //   This would make the operation nominally O(n^2).
    // - unlike terminateAllProcesses(), do not preserve Frozen process.
    //   A process is only in a process group if it is executing, in which case it is not Frozen.
    for (Vector_t::iterator i = m_processes.begin(); i != m_processes.end(); ++i) {
        if ((*i)->getProcessGroupId() == pgid) {
            (*i)->setState(Process::Terminated);
        }
    }

    // Tell observers that this process group is gone.
    // Easiest way is try to start it, so it notices that it's complete.
    startProcessGroup(pgid);
}

// Move process into a process group.
void
interpreter::ProcessList::joinProcess(Process& proc, uint32_t pgid)
{
    bool rename = false;
    switch (proc.getState()) {
     case Process::Suspended:
     case Process::Frozen:
        // It's not actually waiting, and we would wreak havoc trying to unblock it.
        break;

     case Process::Runnable:
     case Process::Running:
     case Process::Waiting:
        // OK
        rename = true;
        proc.setState(Process::Runnable);
        break;

     case Process::Ended:
     case Process::Terminated:
     case Process::Failed:
        // It's not actually waiting.
        // This should not normally happen as we're about to continue this process.
        rename = true;
        break;
    }

    // If we successfully made it runnable, move it into the target PG.
    // It may bring other processes with it.
    if (rename) {
        joinProcessGroup(proc.getProcessGroupId(), pgid);
    }
}

// Join process groups.
void
interpreter::ProcessList::joinProcessGroup(uint32_t oldGroup, uint32_t newGroup)
{
    for (size_t i = 0; i != m_processes.size(); ++i) {
        Process& p = *m_processes[i];
        if (p.getProcessGroupId() == oldGroup) {
            p.setProcessGroupId(newGroup);
        }
    }

    // oldGroup no longer exists, signal its termination
    sig_processGroupFinish.raise(oldGroup);
}

// Resume a process.
void
interpreter::ProcessList::resumeProcess(Process& proc, uint32_t pgid)
{
    switch (proc.getState()) {
     case Process::Suspended:
     case Process::Runnable:
        // Mark for resume. This could change a process' process group.
        proc.setState(Process::Runnable);
        proc.setProcessGroupId(pgid);
        break;

     case Process::Frozen:
     case Process::Running:
     case Process::Waiting:
     case Process::Ended:
     case Process::Terminated:
     case Process::Failed:
        // Cannot mark these for resume
        break;
    }
}

// Resume all suspended processes and place them in the given process group.
void
interpreter::ProcessList::resumeSuspendedProcesses(uint32_t pgid)
{
    // ex runRunnableProcesses, sort-of
    for (size_t i = 0, n = m_processes.size(); i != n; ++i) {
        Process& p = *m_processes[i];
        if (p.getState() == Process::Suspended) {
            p.setState(Process::Runnable);
            p.setProcessGroupId(pgid);
        }
    }
}

// Terminate a process.
void
interpreter::ProcessList::terminateProcess(Process& proc)
{
    switch (proc.getState()) {
     case Process::Suspended:
     case Process::Frozen:
     case Process::Runnable:
        // No cleanup needed. Just do it.
        // In case of a frozen process, we confirm the request; it could be the same one who debugs it.
        proc.setState(Process::Terminated);
        break;

     case Process::Running:
     case Process::Waiting:
        // Must proceed this process' process group.
        proc.setState(Process::Terminated);
        startProcessGroup(proc.getProcessGroupId());
        break;

     case Process::Ended:
     case Process::Terminated:
     case Process::Failed:
        // It's already finished; the one who did that hopefully ran the process group.
        proc.setState(Process::Terminated);
        break;
    }
}

// Continue a process.
void
interpreter::ProcessList::continueProcess(Process& proc)
{
    switch (proc.getState()) {
     case Process::Suspended:
     case Process::Frozen:
     case Process::Runnable:
     case Process::Running:
        // It's not actually waiting.
        break;

     case Process::Waiting:
        // OK
        proc.setState(Process::Running);
        break;

     case Process::Ended:
     case Process::Terminated:
     case Process::Failed:
        // It's not actually waiting.
        break;
    }
}

void
interpreter::ProcessList::continueProcessWithFailure(Process& proc, String_t error)
{
    switch (proc.getState()) {
     case Process::Suspended:
     case Process::Frozen:
     case Process::Runnable:
     case Process::Running:
        // It's not actually waiting.
        break;

     case Process::Waiting:
        // OK
        proc.setState(Process::Running);
        proc.handleException(error, String_t());
        if (proc.getState() != Process::Running) {
            startProcessGroup(proc.getProcessGroupId());
        }
        break;

     case Process::Ended:
     case Process::Terminated:
     case Process::Failed:
        // It's not actually waiting.
        break;
    }
}


// Run selected processes.
void
interpreter::ProcessList::run(Process::Observer* pObserver)
{
    // ex int/process.h:runRunnableProcesses, sort-of
    // ex ccexec.pas:RunRunnableProcesses, sort-of
    // We must avoid being called recursively, i.e. if a process causes ProcessList::run to be called again.
    if (!m_running) {
        m_running = true;
        try {
            while (Process* proc = findRunningProcess()) {
                proc->run(pObserver);
                sig_processStateChange.raise(*proc, false);

                bool handled = false;
                switch (proc->getState()) {
                 case Process::Suspended:
                    // Voluntary suspend. Start another one from this process group.
                    startProcessGroup(proc->getProcessGroupId());
                    handled = true;
                    break;

                 case Process::Frozen:
                    // Someone froze it. Hope they will un-thaw it.
                    // This normally should not happen, and if this process is restarted, it will most likely run in a new process group.
                    // Thus, continue this group.
                    startProcessGroup(proc->getProcessGroupId());
                    handled = true;
                    break;

                 case Process::Runnable:
                 case Process::Running:
                    // run() should not exit with a process in this state.
                    // Mark it failed and proceed with the process group.
                    proc->setState(Process::Failed);
                    startProcessGroup(proc->getProcessGroupId());
                    handled = true;
                    break;

                 case Process::Waiting:
                    // Process waits. Someone will wake it.
                    handled = true;
                    break;

                 case Process::Ended:
                 case Process::Terminated:
                    // Process ended. Start another one from this process group.
                    handled = true;
                    startProcessGroup(proc->getProcessGroupId());
                    break;

                 case Process::Failed:
                    // Process failed. Log and Start another one from this process group.
                    handled = true;
                    proc->world().logError(afl::sys::LogListener::Error, proc->getError());
                    startProcessGroup(proc->getProcessGroupId());
                    break;
                }

                if (!handled) {
                    // Fallback (could be the switch's default, but that would suppress the "not all values handled" warning)
                    proc->setState(Process::Failed);
                    startProcessGroup(proc->getProcessGroupId());
                }
            }
            m_running = false;
        }
        catch (...) {
            m_running = false;
            throw;
        }
    }
}

// Terminate all processes.
void
interpreter::ProcessList::terminateAllProcesses()
{
    // ex int/process.h:terminateAllProcesses
    for (Vector_t::iterator i = m_processes.begin(); i != m_processes.end(); ++i) {
        if ((*i)->getState() != Process::Frozen) {
            (*i)->setState(Process::Terminated);
        }
    }
}

// Remove all terminated processes (zombie reaper).
void
interpreter::ProcessList::removeTerminatedProcesses()
{
    // ex int/process.h:killTerminatedProcesses
    // ex ccexec.pas:KillTerminatedProcesses

    // Do not garbage-collect while running; this might be a resumption handler of a process about to suspend
    // (i.e. process does suspend(), task does continueProcessWithFailure(), run(), removeTerminatedProcesses().)
    if (!m_running) {
        // Select processes and remove them one-by-one.
        // Efficiency-wise, this is the same as run(), i.e. a O(n**2) algorithm.
        // We originally ran through this list once, moving the terminated processes to the end, deleting them all at once in O(n).
        // That fails if a terminating process causes other processes to terminate, and this function being entered recursively.
        // This happens when a process dies that has a TaskEditorContext on stack, e.g. user entering 'AutoTask(1,Id)' at a console.
        // (The alternative would have been to make this function reentrancy-save in a similar way as run().)
        while (1) {
            bool found = false;
            for (Vector_t::iterator i = m_processes.begin(); i != m_processes.end(); ++i) {
                Process* p = *i;
                if (isTerminatedState(p->getState())) {
                    sig_processStateChange.raise(*p, true);
                    m_processes.erase(i);
                    found = true;
                    break;
                }
            }
            if (!found) {
                break;
            }
        }
    }
}

// Handle a priority change.
void
interpreter::ProcessList::handlePriorityChange(const Process& proc)
{
    // ex int/process.h:handlePriorityChange
    // ex ccexec.pas:EnqueueProcess, Reschedule
    /* FIXME: check the interaction of this routine and runRunnableProcesses.
       This will change the list runRunnableProcesses is iterating. */

    /* Locate the process. Search backward, because a common case is
       that the last process changes its priority, as a result of
       being loaded or created. */
    size_t pos = m_processes.size();
    while (pos > 0 && &proc != m_processes[pos-1]) {
        --pos;
    }
    if (pos == 0) {
        // Process not found. This should not happen normally.
        return;
    }
    --pos;

    if (pos > 0 && proc.getPriority() < m_processes[pos-1]->getPriority()) {
        // Low value (high priority), move process to front
        do {
            m_processes.swapElements(pos, pos-1);
            --pos;
        } while (pos > 0 && proc.getPriority() < m_processes[pos-1]->getPriority());
    }
    if (pos < m_processes.size()-1 && proc.getPriority() > m_processes[pos+1]->getPriority()) {
        /* High value (low priority), move process to back.
           Note ">" vs. ">=" in the conditions: we move the process
           only if it's out of place, but when we move it, we move it
           to the end of its priority. */
        do {
            m_processes.swapElements(pos, pos+1);
            ++pos;
        } while (pos < m_processes.size()-1 && proc.getPriority() >= m_processes[pos+1]->getPriority());
    }
}

// Get process, given an object.
interpreter::Process*
interpreter::ProcessList::findProcessByObject(const afl::base::Deletable* obj, uint8_t kind) const
{
    // ex int/process.h:getProcessByObject
    for (Vector_t::const_iterator i = m_processes.begin(); i != m_processes.end(); ++i) {
        if ((*i)->getProcessKind() == kind && (*i)->getInvokingObject() == obj) {
            return *i;
        }
    }
    return 0;
}

// Get process, given a process Id.
interpreter::Process*
interpreter::ProcessList::findProcessById(uint32_t processId) const
{
    for (Vector_t::const_iterator i = m_processes.begin(); i != m_processes.end(); ++i) {
        if ((*i)->getProcessId() == processId) {
            return *i;
        }
    }
    return 0;
}

// Get process list.
const interpreter::ProcessList::Vector_t&
interpreter::ProcessList::getProcessList() const
{
    // ex int/process.h:getProcessList
    return m_processes;
}

// Allocate a process group Id.
inline uint32_t
interpreter::ProcessList::allocateProcessId()
{
    return allocateId(m_processId);
}

interpreter::Process*
interpreter::ProcessList::findRunningProcess() const
{
    for (Vector_t::const_iterator i = m_processes.begin(); i != m_processes.end(); ++i) {
        if ((*i)->getState() == Process::Running) {
            return *i;
        }
    }
    return 0;
}
