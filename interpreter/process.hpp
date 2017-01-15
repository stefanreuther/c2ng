/**
  *  \file interpreter/process.hpp
  */
#ifndef C2NG_INTERPRETER_PROCESS_HPP
#define C2NG_INTERPRETER_PROCESS_HPP

#include "interpreter/contextprovider.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/namemap.hpp"
#include "interpreter/error.hpp"
#include "afl/base/signal.hpp"

namespace interpreter {

    class World;

    class Process : public ContextProvider {
        // ex IntExecutionContext
        // friend class IntVMSaveContext;
        // friend class IntVMLoadContext;
     public:
        typedef BytecodeObject::PC_t PC_t;
        typedef afl::data::Segment Segment_t;
        typedef afl::data::NameMap NameMap_t;

        /** Stack frame of an executing process. */
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
        /** Exception handler. */
        struct ExceptionHandler {
            size_t frameSP;
            size_t contextSP;
            size_t valueSP;
            PC_t pc;
        };
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
    
        Process(World& world, String_t name, uint32_t processId);
        ~Process();

        // Frames:
        Frame& pushFrame(BCORef_t bco, bool wantResult);
        void popFrame();
        size_t getNumActiveFrames() const;
        Frame* getOutermostFrame();
        const Frame* getFrame(size_t nr) const;

        // Exceptions:
        void pushExceptionHandler(PC_t pc);
        void pushExceptionHandler(PC_t pc, size_t frameSP, size_t contextSP, size_t valueSP);
        void popExceptionHandler();

        // Contexts:
        void pushNewContext(Context* ctx);
        void pushContextsFrom(afl::container::PtrVector<Context>& ctxs);
        void markContextTOS();
        bool setContextTOS(size_t n);
        void popContext();
        const afl::container::PtrVector<Context>& getContexts() const;
        size_t getContextTOS() const
            { return m_contextTOS; }

        // Value stack:
        void pushNewValue(afl::data::Value* v);
        void dropValue();
        afl::data::Value* getResult();
        size_t getStackSize() const;
        const Segment_t& getValueStack() const
            { return m_valueStack; }
        Segment_t& getValueStack()
            { return m_valueStack; }

        // Attributes:
        void            setState(State ps);
        State           getState() const;
        void            setProcessGroupId(uint32_t pgid);
        uint32_t        getProcessGroupId() const;
        uint32_t        getProcessId() const;
        void            setName(String_t name);
        String_t        getName() const;
        void            setPriority(int pri);
        int             getPriority() const;
        const Error&    getError() const;
        void            setProcessKind(ProcessKind k);
        ProcessKind     getProcessKind() const;

        // Execution:
        void addTraceTo(Error& err);
        void run();
        bool runTemporary();
        void executeInstruction();
        void suspendForUI();

        // ContextProvider:
        virtual Context* lookup(const afl::data::NameQuery& q, Context::PropertyIndex_t& index);

        // Inspection/Manipulation:
        bool setVariable(String_t name, afl::data::Value* value);
        afl::data::Value* getVariable(String_t name);
        game::map::Object* getCurrentObject() const;
        game::map::Object* getInvokingObject() const;
        void handleFunctionCall(BCORef_t bco, Segment_t& args, bool wantResult);

        bool handleLoad(String_t name);

        // void setNotificationMessage(IntNotificationMessage* msg);
        // IntNotificationMessage* getNotificationMessage() const;

        // bool checkForBreak();
        // static void setNewGlobalContext(IntContext* p);
        // static void setBreakHandler(bool check_break());
        void handleException(String_t e, String_t trace);
        const afl::container::PtrVector<ExceptionHandler>& getExceptions() const
            { return m_exceptions; }

        Context* makeFrameContext(size_t level);

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
        afl::container::PtrVector<ExceptionHandler> m_exceptions;

        /** Value stack. */
        Segment_t m_valueStack;

        // /** Global context template. \see setNewGlobalContext */
        // static std::auto_ptr<IntContext> global_context;

        // /** Notification message associated with this process. */
        // IntNotificationMessage* notification_message;

        /** Process kind. This is used for labelling and finding the process;
            it has no effect upon its execution. Think of this as an extension
            of the process name. (PCC 1.x actually used process names to find
            auto tasks, but this fails here since we do i18n.) */
        ProcessKind m_processKind;

        /** Context top-of-stack. Registers the number of contexts that were on the
            process's context stack when it was created. This is used to identify
            the process it was invoked from. */
        size_t m_contextTOS;

        /** Process group Id. */
        uint32_t m_processGroupId;

        /** Process Id. */
        uint32_t m_processId;
    };

}

#endif
