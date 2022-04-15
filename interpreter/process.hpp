/**
  *  \file interpreter/process.hpp
  *  \brief Class interpreter::Process
  */
#ifndef C2NG_INTERPRETER_PROCESS_HPP
#define C2NG_INTERPRETER_PROCESS_HPP

#include <memory>
#include "afl/base/closure.hpp"
#include "afl/base/deletable.hpp"
#include "afl/base/signal.hpp"
#include "afl/data/namemap.hpp"
#include "afl/data/segment.hpp"
#include "afl/string/translator.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/contextreceiver.hpp"
#include "interpreter/error.hpp"
#include "interpreter/staticcontext.hpp"

namespace interpreter {

    class World;

    /** Process.
        A process executes script commands.
        It holds all required state, namely:
        - identifying information (name, Id, etc.)
        - active stack frames with their local variables
        - active contexts
        - active exception handlers

        Execution of multiple processes is coordinated by ProcessList which also creates Process objects.

        To execute script code, create a Process object (e.g. using ProcessList::create()),
        push a frame (pushFrame()), and execute it (e.g. using ProcessList::run() or run()).

        Unless the process is executing code that refers to transient state (e.g. GUI objects),
        it can be suspended to disk (see SaveContext) and later reloaded.
        This is mainly used for auto-tasks.

        c2ng no longer includes the ability to execute temporary processes (runTemporary). */
    class Process : public StaticContext, public ContextReceiver {
        // ex IntExecutionContext
     public:
        typedef BytecodeObject::PC_t PC_t;
        typedef afl::data::Segment Segment_t;
        typedef afl::data::NameMap NameMap_t;

        /** Stack frame of an executing process.
            Each stack frame describes an executing bytecode object.
            If the process has no more stackframes, it terminates (state Ended). */
        struct Frame {
            BCORef_t bco;           /**< Bytecode object executing in this frame. */
            PC_t pc;                /**< Next instruction to execute. */
            Segment_t localValues;  /**< Local values (parameters, local variables). */
            NameMap_t localNames;   /**< Local names (parameters, local variables). */
            size_t contextSP;       /**< Top of context stack when this frame was opened. */
            size_t exceptionSP;     /**< Top of exception stack when this frame was opened. */
            size_t frameSP;         /**< Own index. */
            bool wantResult;        /**< Set if caller wants a result on the stack. That is, when this frame is removed, an additional value must be pushed to the value stack. */

            Frame(BCORef_t bco);
            ~Frame();
        };

        /** Exception handler.
            An exception handler is activated by discarding values on the stacks
            to the sizes as described in the handler, and jumping to the given program counter. */
        struct ExceptionHandler {
            size_t frameSP;         /**< Size of frame stack. \see getNumActiveFrames() */
            size_t contextSP;       /**< Size of context stack. \see getContexts() */
            size_t valueSP;         /**< Size of value stack. \see getStackSize() */
            PC_t pc;                /**< Program counter of exception handler. */
        };

        /** Process state. */
        enum State {
            Suspended,          ///< Process is not running.
            Frozen,             ///< Process is frozen and must not be modified/run. For example, it's being edited by the auto-task editor.
            Runnable,           ///< Process wants to run in a process group.
            Running,            ///< Process is currently running in a process group.
            Waiting,            ///< Process is waiting (for UI, data, etc.). This also blocks all other processes in the process group.
            Ended,              ///< Process ran to end, successfully. If it can produce a result, it did so.
            Terminated,         ///< Process terminated using "End" statement. It did not produce a result.
            Failed              ///< Process failed using "Abort" statement or other error. It did not produce a result.
        };

        enum ProcessKind {
            pkDefault,
            pkShipTask,
            pkPlanetTask,
            pkBaseTask
        };

        /** Finalizer.
            A process can have one finalizer.
            The finalizer's job is to report the process status to some observer.
            The driver (ProcessList) needs to call finalize() to run the finalizer at an appropriate place.

            Finalizers are not persisted in any way,
            their job is to report a status back to someone who started the process on the UI.
            If the process suspends, the finalizer will be called to report the suspension.

            FIXME: we will need an API for observers to be able to watch and interrupt processes
            (for the user Ctrl+C feature and possibly debugging).
            That API will probably replace Finalizer. */
        class Finalizer : public afl::base::Deletable {
         public:
            /** Perform finalisation for this process.
                \param p Process */
            virtual void finalizeProcess(Process& p) = 0;
        };

        /** Freezer.
            If a process is in state Frozen, the Freezer links to the component that froze it.
            So far, this is just a tag interface. */
        class Freezer {
         protected:
            virtual ~Freezer()
                { }
        };

        /** Task to execute while a process suspends.
            @see suspend() */
        typedef afl::base::Closure<void()> Task_t;


        /** Create process.
            Normally, processes are created by ProcessList::create().
            \param world     World
            \param name      Process name
            \param processId Process Id */
        Process(World& world, String_t name, uint32_t processId);

        /** Destructor. */
        ~Process();


        /*
         *  Frames
         */

        /** Push new frame (subroutine call).
            The frame is initialized with the current process status, representing an initiated call, and need not normally be modified.
            \param bco         BytecodeObject to execute in the frame
            \param wantResult  true: frame should produce a result (push onto value stack); false: no result
            \return Handle to frame. Deserialisation will update it with deserialized values. */
        Frame& pushFrame(BCORef_t bco, bool wantResult);

        /** Pop frame (subroutine return).
            This will clean up context/exception stacks,
            and update the value stack depending on the frame's wantResult and the BCO's isProcedure flag
            (add/keep/discard one value). */
        void popFrame();

        /** Get number of active frames.
            \return Number of active frames. Process can execute as long as this is nonzero. */
        size_t getNumActiveFrames() const;

        /** Get outermost frame.
            This frame represents the process's invoking command or script;
            for an Auto-Task, this is the task's text.
            \return frame; null if nonexistant */
        Frame* getOutermostFrame();

        /** Get frame by index.
            For inspection and serialisation.
            \param nr Frame number [0,getNumActiveFrames()), 0=outermost
            \return frame; 0 if nr out of range */
        const Frame* getFrame(size_t nr) const;

        /** Create Context for frame.
            This is intended for deserialisation.
            \param nr Frame number [0,getNumActiveFrames()), 0=outermost
            \return Newly-allocated context; null of nr out of range */
        Context* makeFrameContext(size_t nr) const;


        /*
         *  Exceptions
         */

        /** Push exception handler (catch).
            If an exception occurs, the current stack situation is restored and the program counter set to the given value.
            \param pc Program Counter of exception handler */
        void pushExceptionHandler(PC_t pc);

        /** Push exception handler (deserialisation).
            If an exception occurs, the specified situation (stacks, program counter) is assumed.
            \param pc        Program Counter of exception handler
            \param frameSP   Number of active frames
            \param contextSP Number of active contexts
            \param valueSP   Number of values on value stack */
        void pushExceptionHandler(PC_t pc, size_t frameSP, size_t contextSP, size_t valueSP);

        /** Pop exception handler (uncatch).
            Undoes the previous pushExceptionHandler().
            The previous exception handler becomes active again.
            \throw Error if no exception handler is active */
        void popExceptionHandler();

        /** Access exception handlers.
            For serialisation.
            \return Vector of exception handlers */
        const afl::container::PtrVector<ExceptionHandler>& getExceptionHandlers() const;


        /*
         *  Contexts
         */

        /** Push a new context.
            The new context will be placed on top of the stack and be the first
            to be considered for variable lookups.
            \param ctx Context. Must not be null. Ownership taken over by Process. */
        void pushNewContext(Context* ctx);

        /** Push new contexts from list.
            Appends the given contexts to the context stack.
            The last element of \c ctxs will be the new topmost context.
            \param ctxs Contexts. Vector will be emptied/nulled; ownership taken over by Process. */
        void pushContextsFrom(afl::container::PtrVector<Context>& ctxs);

        /** Mark top-of-context-stack.
            Call after setting up the process, before starting it, to define the situation invoking the process.
            This is used to find the invoking object,
            even if the process temporarily activates a different object's context.
            \see getInvokingObject() */
        void markContextTOS();

        /** Set top-of-context-stack.
            Use for deserialisation.
            \param n New value
            \return true on success; false if value out of range
            \see markContextTOS() */
        bool setContextTOS(size_t n);

        /** Get top-of-context-stack.
            \return Value
            \see markContextTOS() */
        size_t getContextTOS() const;

        /** Pop context.
            Cancels a previous pushNewContext(). */
        void popContext();

        /** Access list of contexts.
            For serialisation.
            \return Vector of contexts */
        const afl::container::PtrVector<Context>& getContexts() const;


        /*
         *  Value stack
         */

        /** Push new value.
            \param v Value. Can be null, ownership taken over by process */
        void pushNewValue(afl::data::Value* v);

        /** Drop topmost value. */
        void dropValue();

        /** Get process result (top of value stack).
            \return value, can be null, owned by Process */
        const afl::data::Value* getResult() const;

        /** Get size of stack.
            \return Number of values on stack. */
        size_t getStackSize() const;

        /** Access value stack.
            For serialisation.
            \return Value stack */
        const Segment_t& getValueStack() const;

        /** Access value stack.
            For deserialisation.
            \return Value stack */
        Segment_t& getValueStack();


        /*
         *  Attributes
         */

        /** Set process status.
            \param ps New status */
        void setState(State ps);

        /** Get process status.
            \return process status */
        State getState() const;

        /** Set process group Id.
            See ProcessList for a discussion of process groups.
            Process does not interpret the process group Id in any way.
            \param pgid Process group Id */
        void setProcessGroupId(uint32_t pgid);

        /** Get process group Id.
            \return Process group Id */
        uint32_t getProcessGroupId() const;

        /** Get process Id.
            The process Id is assigned when the process is created,
            and serves identification purposes.
            Process does not interpret the process Id in any way.
            \return Process Id */
        uint32_t getProcessId() const;

        /** Set process name.
            The process name serves identification purposes and is not interpreted in any way.
            The name is normally set during construction, this function is used for deserialisation.
            \param name New name */
        void setName(String_t name);

        /** Get process name.
            \return name */
        String_t getName() const;

        /** Set priority.
            The priority is used by ProcessList to order processes for execution.
            Process does not interpret the priority in any way.
            \param pri New priority
            \see ProcessList::handlePriorityChange */
        void setPriority(int pri);

        /** Get priority.
            \return priority */
        int getPriority() const;

        /** Get last error message.
            If the process produced an error (uncaught exception, state Failed), that can be retrieved here.
            \return error */
        const Error& getError() const;

        /** Set process kind.
            The process kind serves identification purposes and is not interpreted in any way.
            \param k Kind */
        void setProcessKind(ProcessKind k);

        /** Get process kind.
            \return kind */
        ProcessKind getProcessKind() const;

        /** Freeze process (set state from Suspended to Frozen).
            A component can freeze a process to manipulate its interior.
            Only one component can do that to a process.
            The process is put in state Frozen in which it cannot execute.

            \param p Freezer. Serves as identification of the invoking component; must live sufficiently long

            \throw Error if process already frozen or cannot currently freeze */
        void freeze(Freezer& p);

        /** Unfreeze process (set state from Frozen to Suspended). */
        void unfreeze();

        /** Get freezer.
            \return component that called freeze(); null if none */
        Freezer* getFreezer() const;

        /** Access world.
            \return world */
        World& world() const;


        /*
         *  Execution
         */

        /** Add current position to an error trace.
            \param err [in/out] Trace added here */
        void addTraceTo(Error& err) const;

        /** Run process (set state to Running).
            Returns when the process leaves state Running, that is:
            - Ended
            - Terminated
            - Failed
            - Suspended
            - Waiting */
        void run();

        /** Execute a single instruction.
            Does not catch errors; caller needs to do that. */
        void executeInstruction();

        /** Suspend this process to perform UI operations (set state to Waiting). */
        void suspendForUI();

        /** Suspend this process to execute a task.
            This sets the status to Waiting and executes the task.
            The task needs to schedule an external event which resumes the task
            (using ProcessList::continueProcess or ProcessList::continueProcessWithFailure).

            suspend() needs to be the last call in a command implementation.
            Likewise, continueProcess() must be the last call in the task implementation
            because it causes the task to be deleted.

            If the process is destroyed in the meantime, the task will be deleted
            and must make sure to not resume the process.

            The task's destructor therefore must not change the process' state.

            \param task Task. Can be null in which case the external event must be scheduled separately */
        void suspend(std::auto_ptr<Task_t> task);

        /** Look up value.
            This is an interface method of StaticContext.
            \param [in]  q      Name query
            \param [out] index  On success, property index
            \return non-null PropertyAccessor if found, null on failure. */
        Context::PropertyAccessor* lookup(const afl::data::NameQuery& q, Context::PropertyIndex_t& index);


        /*
         *  Inspection / Manipulation
         */

        /** Set variable in this process.
            This function is intended to be used by implementations of commands which set variables by name, e.g. "UI.RESULT".
            The variable is set in the topmost context that defines it.
            \param name Name
            \param value Value; will be cloned
            \retval true Assignment succeeded
            \retval false Assignment failed (variable not defined or context refuses to accept value) */
        bool setVariable(String_t name, afl::data::Value* value);

        /** Get variable from this process.
            The variable is fetched from the topmost context that defines it.
            \param name Name
            \return Value, owned by process. Null if variable is not defined. */
        afl::data::Value* getVariable(String_t name);

        /** Get game object this process is working on.
            Returns the object from the innermost context that has one.
            Use this for command implementations that implicitly use that object's context.
            \return Object, can be null. Owned by the Context/World/... */
        game::map::Object* getCurrentObject() const;

        /** Get game object this process was invoked from.
            Returns the object from an outer context as defined by markContextTOS().
            Use this for labeling the process for the user.
            \return Object, can be null. Owned by the Context/World/... */
        game::map::Object* getInvokingObject() const;

        /** Handle user subroutine invocation.
            This is the implementation of SubroutineValue::call().
            \param bco Called bytecode object
            \param args Arguments provided by used
            \param wantResult True if a result is required */
        void handleFunctionCall(BCORef_t bco, Segment_t& args, bool wantResult);

        /** Handle "Load" command.
            Loads and compiles the file, and pushes an appropriate frame.
            \param name   File name given by user
            \param origin Origin to set to the given code (see BytecodeObject::setOrigin)
            \retval true File loaded and compiled successfully
            \retval false File not found
            \throw Error compilation error */
        bool handleLoad(String_t name, const String_t& origin);

        /** Handle exception.
            Transfers control to the most recently pushed exception handler,
            if any, and have it process exception e.
            If there is no active handler, stops the bytecode process.
            \param e Error text
            \param trace Trace */
        void handleException(String_t e, String_t trace);

        /*
         *  Finalizer
         */

        /** Set finalizer.
            \param p Newly-allocated finalizer or null. Will be owned by process. */
        void setNewFinalizer(Finalizer* p);

        /** Call and discard finalizer. */
        void finalize();


        /** Signal: invalidate observers.
            Called before the process starts executing.
            All pointers obtained previously may become invalid. */
        afl::base::Signal<void()> sig_invalidate;

     private:
        class FrameContext;

        void checkStack(uint32_t required);
        void handleInvalidOpcode();
        void handleDim(Segment_t& values, NameMap_t& names, uint16_t index);
        void handleEvalStatement(uint16_t nargs);
        void handleEvalExpression();
        void handleAddHook();
        void handleNewArray(uint16_t ndim);
        void handleResizeArray(uint16_t ndim);
        void handleMakeList(uint16_t nelems);
        void handleNewHash();
        void handleBind(uint16_t nargs);
        bool handleDecrement();
        afl::data::Value* getReferencedValue(const Opcode& op);

        void logProcessState(const char* why);

        World& m_world;

        /** Status. */
        State m_state;

        /** Process name. */
        String_t m_processName;

        /** Priority. */
        int m_processPriority;

        /** Error message if terminated by error. */
        Error m_processError;

        /** All active stack frames. */
        afl::container::PtrVector<Frame> m_frames;

        /** All active contexts for name lookup. */
        afl::container::PtrVector<Context> m_contexts;

        /** All active exception handling contexts. */
        afl::container::PtrVector<ExceptionHandler> m_exceptionHandlers;

        /** Value stack. */
        Segment_t m_valueStack;

        /** Process kind. This is used for labelling and finding the process;
            it has no effect upon its execution. Think of this as an extension
            of the process name. (PCC 1.x actually used process names to find
            auto tasks, but this fails here since we do i18n.) */
        ProcessKind m_processKind;

        /** Context top-of-stack. Registers the number of contexts that were on the
            process's context stack when it was created. This is used to identify
            the object it was invoked from. */
        size_t m_contextTOS;

        /** Process group Id. */
        uint32_t m_processGroupId;

        /** Process Id. */
        const uint32_t m_processId;

        /** Freezer.
            If non-null, the task is in state Frozen, and the pointee is responsible for unfreezing it. */
        Freezer* m_pFreezer;

        /** Finalizer, if non-null. */
        std::auto_ptr<Finalizer> m_finalizer;

        /** Task being executed if process is in status Waiting.
            Can be null. */
        std::auto_ptr<Task_t> m_task;
    };

    /** Format Process::State to string.
        \param st State
        \param tx Translator
        \return string representation of st */
    String_t toString(Process::State st, afl::string::Translator& tx);

}

#endif
