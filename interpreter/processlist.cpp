/**
  *  \file interpreter/processlist.cpp
  *  \brief Class interpreter::ProcessList
  *
  *  Original PCC2 comment:
  *
  *  We distinguish between two kinds of processes, regular processes
  *  and temporary processes.
  *
  *  Regular processes are added to the process list normally, and can
  *  do everything they might want to do, including suspending. Such
  *  processes are created using createProcess. The user then sets up
  *  the process and runs it using runRunnableProcesses,
  *  killTerminatedProcesses
  *
  *  Temporary processes are used whenever the PCC2 core needs to
  *  evaluate an expression, and wishes immediate feedback. Such
  *  processes are not allowed to suspend; when they try to, they
  *  receive a (catchable) error. Such processes are created by simply
  *  creating a IntExecutionContext object on the stack, setting it
  *  up, and and giving it to runTemporaryProcess. If that returns
  *  true, the process has produced its result as requested, otherwise
  *  it didn't. The process never appears in the process list, and is
  *  killed automatically when the IntExecutionContext object goes out
  *  of scope. This is the equivalent to PCC 1.x's simple process-less
  *  expression evaluation. PCC2 doesn't support process-less
  *  evaluation because it supports user-defined functions.
  */

#include "interpreter/processlist.hpp"
#include "interpreter/process.hpp"

// Make new, empty ProcessList
interpreter::ProcessList::ProcessList()
    : m_processes(),
      m_processGroupId(0),
      m_processId(0)
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
    return ++m_processGroupId;
}

// Allocate a process group Id.
uint32_t
interpreter::ProcessList::allocateProcessId()
{
    return ++m_processId;
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
        sig_processGroupFinish.raise(pgid);
    }
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
        uint32_t oldGroup = proc.getProcessGroupId();
        for (size_t i = 0; i != m_processes.size(); ++i) {
            Process& p = *m_processes[i];
            if (p.getProcessGroupId() == oldGroup) {
                p.setProcessGroupId(pgid);
            }
        }
    }
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
interpreter::ProcessList::run()
{
    // ex int/process.h:runRunnableProcesses, sort-of
    while (Process* proc = findRunningProcess()) {
        proc->run();

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
         case Process::Failed:
            // Process ended in one way or another. Start another one from this process group.
            handled = true;
            startProcessGroup(proc->getProcessGroupId());
            break;
        }

        if (!handled) {
            // Fallback (could be the switch's default, but that would suppress the "not all values handled" warning)
            proc->setState(Process::Failed);
            startProcessGroup(proc->getProcessGroupId());
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
    size_t i = 0, o = 0;
    while (i < m_processes.size()) {
        switch (m_processes[i]->getState()) {
         case Process::Ended:
         case Process::Terminated:
         case Process::Failed:
            // Killing a process invoked from an object marks that object dirty,
            // in order to remove the "there's a process here" marker.
            if (game::map::Object* obj = m_processes[i]->getInvokingObject()) {
                obj->markDirty();
            }
            ++i;
            break;
         case Process::Suspended:
         case Process::Frozen:
         case Process::Runnable:
         case Process::Running:
         case Process::Waiting:
            m_processes.swapElements(i, o);
            ++i;
            ++o;
            break;
        }
    }
    while (o < m_processes.size()) {
        m_processes.popBack();
    }
}

// FIXME: retire
// // /** Check whether there's any runnable process. */
// bool
// interpreter::ProcessList::hasAnyRunnableProcess() const
// {
//     // ex int/process.h:hasAnyRunnableProcess
//     for (Vector_t::const_iterator i = m_processes.begin(); i != m_processes.end(); ++i) {
//         if ((*i)->getState() == Process::psRunnable) {
//             return true;
//         }
//     }
//     return false;
// }

// Handle a priority change.
void
interpreter::ProcessList::handlePriorityChange(Process& proc)
{
    // ex int/process.h:handlePriorityChange
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
interpreter::ProcessList::getProcessByObject(const game::map::Object* obj, uint8_t kind) const
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
interpreter::ProcessList::getProcessById(uint32_t processId) const
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
