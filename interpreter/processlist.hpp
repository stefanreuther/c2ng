/**
  *  \file interpreter/processlist.hpp
  *  \brief Class interpreter::ProcessList
  */
#ifndef C2NG_INTERPRETER_PROCESSLIST_HPP
#define C2NG_INTERPRETER_PROCESSLIST_HPP

#include "afl/base/signal.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/string.hpp"

namespace interpreter {

    class Process;
    class World;

    /** Process list.
        Manages a list of processes and handles interactions between them.

        Processes are run in process groups.
        When several processes are put in the same process group, one is picked (startProcessGroup()) and run.
        The next process from the same process group runs when the previous one completes.
        Process completion means the process terminated successfully or unsuccessfully, or suspended.
        For this to work, <b>external process state changes should only be made through ProcessList</b>.
        Changes made on the process itself (other than those it does on itself while it is executing) may cause that trigger be missed and the process group get stuck.

        Processes may wait for UI.
        To avoid that another process kicks in, this will defer the whole process group.
        However, UI may start new processes in new process groups (recursive processes).

        c2ng change: PCC1 and PCC2 do not have process groups.
        Instead, they runs all processes one after the other.
        This works because they never have recursive processes.
        Synchronisation with user interface is rather whacky in PCC1/PCC2.
        PCC2 has temporary processes to execute expressions and immediately return a result.
        c2ng does no longer support that. */
    class ProcessList {
     public:
        typedef afl::container::PtrVector<Process> Vector_t;

        /** Make new, empty ProcessList. */
        ProcessList();

        /** Destructor. */
        ~ProcessList();

        /** Create a regular process.
            The process is created in state Suspended.
            You have to
            - fill it (by pushing some frames)
            - resume it (resumeProcess(p, allocateProcessGroup()))
            - run it (run())

            \param world parent World
            \param name Name of process
            \return Reference to blank process object */
        Process& create(World& world, String_t name);

        /** Allocate a process group Id.
            \return new process group Id */
        uint32_t allocateProcessGroup();

        /** Start a process group.
            If the process still has processes, selects one of them. Call run() to actually run it.
            If the process group has no more processes, declares it finished using sig_processGroupFinish.

            Use for starting freshly-created process groups.

            Call
            - startProcessGroup()
            - run()

            \param pgid process group Id */
        void startProcessGroup(uint32_t pgid);

        /** Move process into a process group.
            If the process is waiting, moves it into the given process group and marks it for eventual execution.
            If other processes are in the same process group, moves those into the process group as well.
            The given process and all the other processes will run when the leader of the process group finishes execution.

            Use when a recursive process (inner) causes its invoker (outer) to continue, but continues itself.
            In this case, call
            - joinProcess(innerProcess, outerPGID) to merge innerProcess into the outer process group
            - continueProcess(outerProcess) to continue the outer process and therefore the outer process group
            - run()

            \param proc process
            \param pgid process group Id */
        void joinProcess(Process& proc, uint32_t pgid);

        /** Join process groups.
            Moves all processes from the old group into the new one,
            signaling termination of the old one.

            \param oldGroup Old process group
            \param newGroup New process group */
        void joinProcessGroup(uint32_t oldGroup, uint32_t newGroup);

        /** Resume a process.
            Places the process into the given process group and marks it for eventual execution.

            Use for resuming suspended processes on user request.

            Call
            - resumeProcess() to place the process in the process group
            - startProcessGroup()
            - run()

            \param proc process
            \param pgid process group Id */
        void resumeProcess(Process& proc, uint32_t pgid);

        /** Resume all suspended processes and place them in the given process group.
            Run the process group (startProcessGroup(), run()) to run them.

            Use for resuming suspended processes on user request,

            \param pgid process group Id */
        void resumeSuspendedProcesses(uint32_t pgid);

        /** Terminate a process.
            Marks the process terminated. Call removeTerminatedProcesses() to actually remove the object.
            If there is another process in this process' process group, selects the next process; call run() to actually run it.

            Use for terminating a process on user request.

            \param proc process */
        void terminateProcess(Process& proc);

        /** Continue a process.
            If the process is waiting, selects it for execution. Call run() to actually run it.

            Use for continuing a process that voluntarily entered state Waiting if the action that caused it to wait succeeded.
            That process will already be in a process group, possibly having other processes behind it.

            Call
            - continueProcess()
            - run()

            \param proc process */
        void continueProcess(Process& proc);

        /** Continue a process with an error.
            If the process is waiting, selects it for execution. Call run() to actually run it.

            Use for continuing a process that voluntarily entered state Waiting if the action that caused it to wait failed.
            That process will already be in a process group, possibly having other processes behind it.

            Call
            - continueProcessWithFailure()
            - run()

            \param proc process
            \param error Error message */
        void continueProcessWithFailure(Process& proc, String_t error);

        /** Run selected processes.
            Runs as many processes as it possibly can, in priority order:
            - processes started with startProcessGroup()
            - processes that got selected because their predecessor in their process group terminated */
        void run();

        /** Terminate all processes.
            Marks all processes terminated, excluding frozen ones. Call removeTerminatedProcesses() to actually remove the objects.

            Use for terminating processes on user request. */
        void terminateAllProcesses();

        /** Remove all terminated processes (zombie reaper).
            This will actually cause the Process objects to be destroyed. */
        void removeTerminatedProcesses();

        /** Handle a priority change.
            Call this after a change to a process' priority.
            If this causes the process's location in the process list to change, move it accordingly.
            \param proc process */
        void handlePriorityChange(const Process& proc);

        /** Get process, given an object.
            Locates a process, given a process kind and invoking object.
            \param obj Invoking object
            \param kind Process kind
            \return process if any, 0 if none */
        Process* findProcessByObject(const afl::base::Deletable* obj, uint8_t kind) const;

        /** Find process, given a process Id.
            Locates a process with the given Id.
            \param processId process id
            \return process if any, 0 if none */
        Process* findProcessById(uint32_t processId) const;

        /** Get process list.
            Use with care. */
        const Vector_t& getProcessList() const;

        /** Signal: process group finished.
            Called whenever a process group tries to run but has no more processes.
            This means all processes have completed.
            \param pgid process group Id */
        afl::base::Signal<void(uint32_t)> sig_processGroupFinish;

        /** Signal: process changed state in a relevant way.
            This is a semi ad-hoc mechanism to drive tue UI's "process here" marker.

            Called with willDelete=false after the process ran.
            Called with willDelete=true before the process is deleted (after termination).

            \param proc       Process
            \param willDelete Whether process will be deleted */
        afl::base::Signal<void(const Process&, bool)> sig_processStateChange;

     private:
        /** Allocate a process Id. */
        uint32_t allocateProcessId();

        Process* findRunningProcess() const;

        /** Process list. */
        Vector_t m_processes;

        /** Counter for new process group Ids. */
        uint32_t m_processGroupId;

        /** Counter for new process Ids. */
        uint32_t m_processId;

        /** Marker for recursive invocation. */
        bool m_running;
    };

}

#endif
