/**
  *  \file interpreter/process.cpp
  */

#include <cassert>
#include <stdexcept>
#include "interpreter/process.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/format.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/arrayvalue.hpp"
#include "interpreter/binaryexecution.hpp"
#include "interpreter/closure.hpp"
#include "interpreter/compilationcontext.hpp"
#include "interpreter/defaultstatementcompilationcontext.hpp"
#include "interpreter/expr/parser.hpp"
#include "interpreter/filecommandsource.hpp"
#include "interpreter/hashvalue.hpp"
#include "interpreter/memorycommandsource.hpp"
#include "interpreter/optimizer.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/statementcompilationcontext.hpp"
#include "interpreter/statementcompiler.hpp"
#include "interpreter/structuretype.hpp"
#include "interpreter/structurevalue.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "interpreter/ternaryexecution.hpp"
#include "interpreter/tokenizer.hpp"
#include "interpreter/unaryexecution.hpp"
#include "interpreter/values.hpp"
#include "interpreter/world.hpp"
#include "util/translation.hpp"
#include "afl/except/fileproblemexception.hpp"

namespace {

    const char LOG_NAME[] = "interpreter.process";

// std::auto_ptr<IntContext> IntExecutionContext::global_context;
//
// static bool
// noBreak()
// {
//     return false;
// }
//
// static bool (*check_break)() = noBreak;
//
//
    void validateCalledObject(bool isProcedure, uint8_t minor)
    {
        using interpreter::Error;
        using interpreter::Opcode;
        if ((minor & Opcode::miIMRefuseFunctions) != 0 && !isProcedure)
            throw Error::typeError(Error::ExpectProcedure);
        if ((minor & Opcode::miIMRefuseProcedures) != 0 && isProcedure)
            throw Error::typeError(Error::ExpectIndexable);
    }
}

/***************************** Process::Frame ****************************/

/** Construct stack frame.
    \param bco Byte code object associated with this frame (=code to execute) */
interpreter::Process::Frame::Frame(BCORef_t bco)
    : bco(bco),
      pc(0),
      localNames(bco->getLocalNames()),
      contextSP(0),
      exceptionSP(0),
      frameSP(0),
      wantResult(false)
{ }

/** Destroy stack frame. */
interpreter::Process::Frame::~Frame()
{ }


/************************* Process::FrameContext *************************/

/** Local variables for an execution frame.
    Provides access to the local variables of an executing stack frame. */
class interpreter::Process::FrameContext : public interpreter::SingleContext {
 public:
    FrameContext(Process::Frame& frame)
        : m_frame(frame)
        { }
    ~FrameContext()
        { }

    // Context:
    virtual FrameContext* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
        {
            NameMap_t::Index_t i = m_frame.localNames.getIndexByName(name);
            if (i != NameMap_t::nil) {
                result = i;
                return this;
            } else {
                return 0;
            }
        }
    virtual void set(PropertyIndex_t index, afl::data::Value* value)
        {
            // ex IntExecutionFrameContext::set
            m_frame.localValues.set(index, value);
        }
    virtual afl::data::Value* get(PropertyIndex_t index)
        {
            // ex IntExecutionFrameContext::get
            return afl::data::Value::cloneOf(m_frame.localValues[index]);
        }
    virtual game::map::Object* getObject()
        { return 0; }
    virtual void enumProperties(PropertyAcceptor& acceptor)
        { acceptor.enumNames(m_frame.localNames); }

    // BaseValue:
    virtual String_t toString(bool /*readable*/) const
        { return "#<stack-frame>"; }
    virtual void store(TagNode& out, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, SaveContext& /*ctx*/) const
        {
            out.tag   = TagNode::Tag_Frame;
            out.value = m_frame.frameSP;
        }

    // Value:
    virtual FrameContext* clone() const
        { return new FrameContext(m_frame); }

 private:
    Process::Frame& m_frame;
};




/******************************** Process ********************************/



/** Constructor. */
interpreter::Process::Process(World& world, String_t name, uint32_t processId)
    : m_world(world),
      m_state(Suspended),
      m_processName(name),
      m_processPriority(50),
      m_processError(String_t()),
      // notification_message(0),
      m_processKind(pkDefault),
      m_processGroupId(0),
      m_processId(processId)
{
    // ex IntExecutionContext::IntExecutionContext
    const afl::container::PtrVector<Context>& globalContexts = world.globalContexts();
    for (size_t i = 0, n = globalContexts.size(); i < n; ++i) {
        if (Context* ctx = globalContexts[i]) {
            pushNewContext(ctx->clone());
        }
    }
    m_contextTOS = m_contexts.size();
}

/** Destructor. */
interpreter::Process::~Process()
{
    // ex IntExecutionContext::~IntExecutionContext

    // FIXME: port this
    // // Termination of a process deletes its associated message.
    // // Use setNotificationMessage which implements the whole protocol.
    // setNotificationMessage(0);

    // Disown all my mutexes
    m_world.mutexList().disownLocksByProcess(this);

    sig_invalidate.raise();
}

// /** Push new frame. This frame will execute util termination, then execution
//     will resume at the current frame. */
interpreter::Process::Frame&
interpreter::Process::pushFrame(BCORef_t bco, bool wantResult)
{
    // ex IntExecutionContext::pushNewFrame
    Frame* frame = new Frame(bco);
    frame->contextSP   = m_contexts.size();
    frame->exceptionSP = m_exceptions.size();
    frame->frameSP     = m_frames.size();
    frame->wantResult  = wantResult;
    m_frames.pushBackNew(frame);
    m_contexts.pushBackNew(new FrameContext(*frame));
    return *frame;
}

// /** Pop frame. Execution resumes at previous frame. */
void
interpreter::Process::popFrame()
{
    // ex IntExecutionContext::popFrame()
    assert(m_frames.size() != 0);
    Frame* frame = m_frames.back();

    /* Clean up stacks */
    while (m_contexts.size() > frame->contextSP) {
        m_contexts.popBack();
    }
    while (m_exceptions.size() > frame->exceptionSP) {
        m_exceptions.popBack();
    }

    /* Generate result */
    if (frame->wantResult && frame->bco->isProcedure()) {
        /* Caller wants a result, but we don't have one */
        m_valueStack.pushBackNew(0);
    }
    if (!frame->wantResult && !frame->bco->isProcedure()) {
        /* Caller doesn't want a result */
        checkStack(1);
        m_valueStack.popBack();
    }

    /* Discard frame */
    m_frames.popBack();
}

size_t
interpreter::Process::getNumActiveFrames() const
{
    return m_frames.size();
}

interpreter::Process::Frame*
interpreter::Process::getOutermostFrame()
{
    // FIXME: empty?
    return m_frames[0];
}

const interpreter::Process::Frame*
interpreter::Process::getFrame(size_t nr) const
{
    return (nr < m_frames.size()
            ? m_frames[nr]
            : 0);
}


// /** Push exception handler. If program triggers an error, execution resumes
//     in the current frame at the specified program counter.
//     \param pc Program counter */
void
interpreter::Process::pushExceptionHandler(PC_t pc)
{
    // ex IntExecutionContext::pushExceptionHandler(IntBytecodeObject::pc_t pc)
    ExceptionHandler* e = new ExceptionHandler;
    e->valueSP   = m_valueStack.size();
    e->contextSP = m_contexts.size();
    e->frameSP   = m_frames.size();
    e->pc        = pc;
    m_exceptions.pushBackNew(e);
}

void
interpreter::Process::pushExceptionHandler(PC_t pc, size_t frameSP, size_t contextSP, size_t valueSP)
{
    ExceptionHandler* e = new ExceptionHandler;
    e->valueSP   = valueSP;
    e->contextSP = contextSP;
    e->frameSP   = frameSP;
    e->pc        = pc;
    m_exceptions.pushBackNew(e);
}

// /** Pop exception handler. Cancels a previous pushExceptionHandler. */
void
interpreter::Process::popExceptionHandler()
{
    // ex IntExecutionContext::popExceptionHandler()
    m_exceptions.popBack();
}

// /** Push a new context. This one will be the first one used to look up variables. */
void
interpreter::Process::pushNewContext(Context* ctx)
{
    // ex IntExecutionContext::pushNewContext
    m_contexts.pushBackNew(ctx);
}

// /** Copy contexts from a template. */
void
interpreter::Process::pushContextsFrom(afl::container::PtrVector<Context>& ctxs)
{
    // ex IntExecutionContext::pushContextsFrom
    for (size_t i = 0; i < ctxs.size(); ++i) {
        pushNewContext(ctxs.extractElement(i));
    }
}

void
interpreter::Process::markContextTOS()
{
    // ex IntExecutionContext::markContextTOS()
    m_contextTOS = m_contexts.size();
}

bool
interpreter::Process::setContextTOS(size_t n)
{
    if (n <= m_contexts.size()) {
        m_contextTOS = n;
        return true;
    } else {
        return false;
    }
}

// /** Pop context. Cancels a previous pushNewContext. */
void
interpreter::Process::popContext()
{
    // ex IntExecutionContext::popContext()
    m_contexts.popBack();
}

const afl::container::PtrVector<interpreter::Context>&
interpreter::Process::getContexts() const
{
    return m_contexts;
}

void
interpreter::Process::pushNewValue(afl::data::Value* v)
{
    m_valueStack.pushBackNew(v);
}

void
interpreter::Process::dropValue()
{
    m_valueStack.popBack();
}

afl::data::Value*
interpreter::Process::getResult()
{
    // FIXME: what if empty?
    return m_valueStack.top();
}

size_t
interpreter::Process::getStackSize() const
{
    return m_valueStack.size();
}

void
interpreter::Process::setState(State ps)
{
    // ex IntExecutionContext::setState
    m_state = ps;
}

interpreter::Process::State
interpreter::Process::getState() const
{
    // ex IntExecutionContext::getState
    return m_state;
}

void
interpreter::Process::setProcessGroupId(uint32_t pgid)
{
    m_processGroupId = pgid;
}

uint32_t
interpreter::Process::getProcessGroupId() const
{
    return m_processGroupId;
}

uint32_t
interpreter::Process::getProcessId() const
{
    return m_processId;
}

void
interpreter::Process::setName(String_t name)
{
    m_processName = name;
}

String_t
interpreter::Process::getName() const
{
    // ex IntExecutionContext::getName
    return m_processName;
}

void
interpreter::Process::setPriority(int pri)
{
    // ex IntExecutionContext::setPriority(int pri)
    m_processPriority = pri;
}

int
interpreter::Process::getPriority() const
{
    // ex IntExecutionContext::getPriority() const
    return m_processPriority;
}

const interpreter::Error&
interpreter::Process::getError() const
{
    // ex IntExecutionContext::getError() const
    return m_processError;
}

void
interpreter::Process::setProcessKind(ProcessKind k)
{
    // ex IntExecutionContext::setProcessKind
    m_processKind = k;
}

interpreter::Process::ProcessKind
interpreter::Process::getProcessKind() const
{
    // ex IntExecutionContext::getProcessKind
    return m_processKind;
}

// /** Add trace corresponding to last executed instruction to the specified IntError object. */
void
interpreter::Process::addTraceTo(Error& err)
{
    // ex IntExecutionContext::addTraceTo
    static const char*const firstformats[] = {
        N_("in file '%s', line %d"),
        N_("in file '%s'"),
        N_("in %s, file '%s', line %d"),
        N_("in %s")
    };
    static const char*const secondformats[] = {
        N_("called by file '%s', line %d"),
        N_("called by file '%s'"),
        N_("called by %s, file '%s', line %d"),
        N_("called by %s")
    };

    const char*const* formats = firstformats;

    size_t contextSP = m_contexts.size();

    for (size_t i = m_frames.size(); i > 0; --i) {
        /*  file   procedure   line
             -         -         -       -
             -         -         x       -
             -         x         -       "in procedure/function %s"
             -         x         x       "in procedure/function %s"
             x         -         -       "in file %s"
             x         -         x       "in file %s, line %d"
             x         x         -       "in procedure/function %s"
             x         x         x       "in procedure/function %s, file %s, line %d" */
        Frame* frame = m_frames[i-1];
        BCORef_t bco = m_frames[i-1]->bco;
        String_t bcoName = bco->getName();
        String_t fileName = m_world.fileSystem().getFileName(bco->getFileName());
        uint32_t lineNr = bco->getLineNumber(frame->pc - 1);

        /* Name the code location */
        if (bcoName.size() == 0) {
            /* No procedure name known. If we know file and or line, list those */
            if (fileName.size()) {
                if (lineNr) {
                    err.addTrace(afl::string::Format(_(formats[0]).c_str(), fileName, lineNr));
                } else {
                    err.addTrace(afl::string::Format(_(formats[1]).c_str(), fileName));
                }
                formats = secondformats;
            }
        } else {
            /* Procedure name known. Generate name to use for actual formatting. */
            SubroutineValue* sv = dynamic_cast<SubroutineValue*>(m_world.globalValues()[m_world.globalPropertyNames().getIndexByName(bcoName)]);
            if (sv == 0 || &bco.get() != &sv->getBytecodeObject().get()) {
                bcoName = afl::string::Format("(%s)", bcoName);
            }

            if (fileName.size() != 0 && lineNr != 0) {
                err.addTrace(afl::string::Format(_(formats[2]).c_str(), bcoName, fileName, lineNr));
            } else {
                err.addTrace(afl::string::Format(_(formats[3]).c_str(), bcoName));
            }
            formats = secondformats;
        }

        /* If we can name the innermost context inside this frame, do so.
           Although we could name all of them, name at most one to keep the backtrace concise. */
        while (contextSP > frame->contextSP) {
            --contextSP;
            String_t n = m_contexts[contextSP]->toString(true);
            if (n.size() && n[0] != '#') {
                err.addTrace(afl::string::Format(_("at %s").c_str(), n));
                break;
            }
        }
        contextSP = frame->contextSP;
    }
}

// /** Run this process. */
void
interpreter::Process::run()
{
    // ex IntExecutionContext::run()
    // unsigned counter = 0;
    logProcessState("run");

    // Notify observers.
    // This general mechanism is used to invalidate ProcessObserverContexts when their parent process runs.
    // FIXME: Can we make this more elegant?
    sig_invalidate.raise();

    m_state = Running;
    while (m_state == Running) {
        try {
            executeInstruction();
        }
        catch (Error& e) {
            handleException(e.what(), e.getTrace());
        }
        // FIXME: port this
        // catch (GError& e) {
        //     handleException(e.getScriptError(), string_t());
        // }
        catch (std::bad_alloc& e) {
            // Do not reflect this back into the script.
            throw;
        }
        catch (std::exception& e) {
            handleException(e.what(), String_t());
        }
        // FIXME: port this
        // if (++counter == 10000) {
        //     counter = 0;
        //     checkForBreak();
        // }
    }
    logProcessState("end");
}

// /** Run temporary process. Runs the given process, which is a stack
//     object of the caller (not created with createProcess). The process
//     is not permitted to suspend.

//     \retval true  The process has run till its end and finished normally.
//                   If it contained an expression, that expression's result
//                   is available on the stack (exec.getResult()).
//     \retval false The process has terminated abnormally. This happens when
//                   the process received an error, but also when it
//                   executed an 'End' statement. In this case, it has not
//                   produced a result. */
//
// c2ng comment:
//     try to avoid runTemporary(). Processes that execute in runTemporary() will not have a process group Id assigned
//     and thus escape the regular process state machine. In particular, this means they must fail if they wait.
//     For c2ng, this means they cannot call UI. In the JavaScript version, this would mean they cannot fetch data.
bool
interpreter::Process::runTemporary()
{
    // ex int/process.h:runTemporaryProcess
    while (1) {
        run();

        bool handled = false;
        switch (getState()) {
         case Suspended:
            // Tries to suspend, this is not allowed for a temporary process.
            handleException("Cannot suspend temporary process", String_t());
            handled = true;
            break;

         case Frozen:
            // Should not happen; count as error.
            return false;

         case Runnable:
         case Running:
            // Should not happen; count as error. ProcessList::run sets these to Failed.
            m_processError = "Invalid state";
            setState(Failed);
            return false;

         case Waiting:
            // The process called UI. This counts as failure.
            // We are a temporary process, the wait has nowhere to return.
            handleException("Cannot wait from temporary process", String_t());
            handled = true;
            break;

         case Ended:
            // Successful execution.
            return true;

         case Terminated:
            // Counts as failure because it did not produce a result.
            return false;

         case Failed:
            // Failure
            return false;
        }

        if (!handled) {
            // Fallback (could be the switch's default, but that would suppress the "not all values handled" warning)
            return false;
        }
    }
}


// /** Execute a single instruction. */
void
interpreter::Process::executeInstruction()
{
    // ex IntExecutionContext::executeInstruction()
    // Terminated?
    if (m_frames.size() == 0) {
        m_state = Ended;
        return;
    }

    // Find frame
    Frame& f = *m_frames.back();

    // Run off end of code?
    if (f.pc >= f.bco->getNumInstructions()) {
        popFrame();
        return;
    }

// #if 0
//     // Log it
//     for (uint32_t i = 0; i < m_valueStack.size() && i < 10; ++i)
//         console << (i ? " : " : "   ") << IntValue::toStringOf(m_valueStack.top(i), true);
//     if (m_valueStack.size() > 10)
//         console << " : ...";
//     console << "\n";
//     if (m_processName.find("Temporary") == string_t::npos) {
//         console << m_processName << ":" << f.pc << ":   " << f.bco->getDisassembly(f.pc) << "\n";
//     }
// #endif

#if 0
    m_world.logListener().write(afl::sys::LogListener::Trace, m_processName, afl::string::Format("%d>%d:\t%s", m_frames.size(), f.pc, f.bco->getDisassembly(f.pc, m_world)));
#endif

#if 0
    std::cout << String_t(afl::string::Format("%d>%d:\t%s", m_frames.size(), f.pc, f.bco->getDisassembly(f.pc, m_world))) << "\n";
#endif

    // Execute it
    const Opcode& op = (*f.bco)(f.pc++);

    // Cache m_valueStack address. This saves >3% on my computer.
    Segment_t& valueStack = this->m_valueStack;
    switch (op.major) {
     case Opcode::maPush:
        /* Push value */
        switch (op.minor) {
         case Opcode::sNamedVariable:
         {
             Context::PropertyIndex_t index;
             if (Context* ctx = lookup(f.bco->getName(op.arg), index)) {
                 valueStack.pushBackNew(ctx->get(index));
             } else {
                 throw Error::unknownIdentifier(f.bco->getName(op.arg));
             }
             break;
         }
         case Opcode::sLocal:
            valueStack.pushBack(f.localValues[op.arg]);
            break;
         case Opcode::sStatic:
            valueStack.pushBack(m_frames.front()->localValues[op.arg]);
            break;
         case Opcode::sShared:
            valueStack.pushBack(m_world.globalValues()[op.arg]);
            break;
         case Opcode::sNamedShared:
         {
             NameMap_t::Index_t index = m_world.globalPropertyNames().getIndexByName(f.bco->getName(op.arg));
             if (index != NameMap_t::nil) {
                 valueStack.pushBack(m_world.globalValues()[index]);
             } else {
                 throw Error::unknownIdentifier(f.bco->getName(op.arg));
             }
             break;
         }
         case Opcode::sLiteral:
            valueStack.pushBack(f.bco->getLiteral(op.arg));
            break;
         case Opcode::sInteger:
            valueStack.pushBackNew(makeIntegerValue(int16_t(op.arg)));
            break;
         case Opcode::sBoolean:
            valueStack.pushBackNew(makeBooleanValue(int16_t(op.arg)));
            break;
         default:
            handleInvalidOpcode();
        }
        break;

     case Opcode::maBinary:
        /* Binary operations */
        {
            checkStack(2);
            afl::data::Value* a = valueStack.top(1);
            afl::data::Value* b = valueStack.top(0);
            afl::data::Value* result = executeBinaryOperation(m_world, op.minor, a, b);
            valueStack.popBackN(2);
            valueStack.pushBackNew(result);
        }
        break;

     case Opcode::maUnary:
        /* Unary operations */
        {
            checkStack(1);
            afl::data::Value* result = executeUnaryOperation(m_world, op.minor, valueStack.top(0));
            valueStack.popBack();
            valueStack.pushBackNew(result);
        }
        break;

     case Opcode::maTernary:
        /* Ternary operations */
        {
            checkStack(3);
            afl::data::Value* a = valueStack.top(2);
            afl::data::Value* b = valueStack.top(1);
            afl::data::Value* c = valueStack.top(0);
            afl::data::Value* result = executeTernaryOperation(m_world, op.minor, a, b, c);
            valueStack.popBackN(3);
            valueStack.pushBackNew(result);
        }
        break;

     case Opcode::maJump:
        /* Jump instructions */
        if ((op.minor & Opcode::jOtherMask) != 0) {
            // Other jump
            switch (op.minor & ~Opcode::jSymbolic) {
             case Opcode::jCatch:
                // "catch"
                pushExceptionHandler(f.bco->getJumpTarget(op.minor, op.arg));
                break;
             case Opcode::jDecZero:
                // "jdz", decrement and jump if zero
                // FIXME: optimized execution: when encountering value N here, and
                // there are more than N 'jdz' instructions, jump directly without
                // generating the intermediate results.
                if (handleDecrement()) {
                    f.pc = f.bco->getJumpTarget(op.minor, op.arg);
                }
                break;
             default:
                handleInvalidOpcode();
                break;
            }
        } else {
            // regular jump
            switch (op.minor & Opcode::jAlways) {
             case Opcode::jAlways:
                // Jump always
                f.pc = f.bco->getJumpTarget(op.minor, op.arg);
                break;
             case 0:
                // Jump never
                break;
             default:
                // Jump on condition
                checkStack(1);
                int cond = getBooleanValue(valueStack.top());
                int mask = (cond < 0 ? Opcode::jIfEmpty
                            : cond > 0 ? Opcode::jIfTrue
                            : Opcode::jIfFalse);
                if ((op.minor & mask) != 0) {
                    f.pc = f.bco->getJumpTarget(op.minor, op.arg);
                }
                break;
            }
            if (op.minor & Opcode::jPopAlways) {
                checkStack(1);
                valueStack.popBack();
            }
        }
        break;

     case Opcode::maIndirect:
        /* Indirect call */
        switch (uint8_t operation = op.minor & Opcode::miIMOpMask) {
         case Opcode::miIMCall:
         case Opcode::miIMLoad:
         {
            checkStack(op.arg + 1);
            std::auto_ptr<afl::data::Value> p(valueStack.extractTop());

            /* CALLIND nargs   rr:args:R      => rr
               PUSHIND nargs   rr:args:R      => rr:result */
            if (p.get() == 0) {
                /* Dereferencing null stays null, but only for functions */
                validateCalledObject(false, op.minor);
                valueStack.popBackN(op.arg);
                if (operation == Opcode::miIMLoad)
                    valueStack.pushBackNew(0);
            } else if (CallableValue* iv = dynamic_cast<CallableValue*>(p.get())) {
                /* We can load this */
                validateCalledObject(iv->isProcedureCall(), op.minor);

                Segment_t args;
                valueStack.transferLastTo(op.arg, args);

                iv->call(*this, args, operation == Opcode::miIMLoad);
            } else {
                /* Error */
                if (op.minor & Opcode::miIMRefuseFunctions)
                    throw Error::typeError(Error::ExpectProcedure);
                else
                    throw Error::typeError(Error::ExpectIndexable);
            }
            break;
         }

         case Opcode::miIMStore:
         case Opcode::miIMPop:
            /* STOREIND nargs   rr:args:val:R  => rr:val
               POPIND nargs     rr:args:val:R  => rr */
            checkStack(op.arg + 2);
            if (IndexableValue* iv = dynamic_cast<IndexableValue*>(valueStack.top(0))) {
                /* We can assign this */
                validateCalledObject(iv->isProcedureCall(), op.minor);
                afl::data::Value* value = valueStack.top(1);
                Arguments args(valueStack, valueStack.size() - op.arg - 2, op.arg);
                iv->set(args, value);

                /* Update stack */
                if ((op.minor & Opcode::miIMOpMask) == Opcode::miIMPop) {
                    valueStack.popBackN(op.arg+2);
                } else {
                    valueStack.swapElements(valueStack.size() - op.arg - 2,
                                            valueStack,
                                            valueStack.size() - 2);
                    valueStack.popBackN(op.arg+1);
                }
            } else {
                /* Error */
                if (op.minor & Opcode::miIMRefuseFunctions)
                    throw Error::typeError(Error::ExpectProcedure);
                else
                    throw Error::typeError(Error::ExpectIndexable);
            }
            break;
        }
        break;

     case Opcode::maStack:
        /* Stack operations */
        switch (op.minor) {
         case Opcode::miStackDup:
            checkStack(op.arg+1);
            valueStack.pushBack(valueStack.top(op.arg));
            break;
         case Opcode::miStackDrop:
            checkStack(op.arg);
            valueStack.popBackN(op.arg);
            break;
         case Opcode::miStackSwap:
            checkStack(op.arg+1);
            valueStack.swapElements(valueStack.size() - op.arg - 1, valueStack, valueStack.size() - 1);
            break;
         default:
            handleInvalidOpcode();
        }
        break;

     case Opcode::maStore:
        /* Pop or store into variable */
        checkStack(1);
        switch (op.minor) {
         case Opcode::sNamedVariable:
         {
             Context::PropertyIndex_t index;
             if (Context* ctx = lookup(f.bco->getName(op.arg), index)) {
                 ctx->set(index, valueStack.top());
             } else {
                 throw Error::unknownIdentifier(f.bco->getName(op.arg));
             }
             break;
         }
         case Opcode::sLocal:
            f.localValues.set(op.arg, valueStack.top());
            break;
         case Opcode::sStatic:
            m_frames.front()->localValues.set(op.arg, valueStack.top());
            break;
         case Opcode::sShared:
            m_world.globalValues().set(op.arg, valueStack.top());
            break;
         case Opcode::sNamedShared:
         {
             NameMap_t::Index_t index = m_world.globalPropertyNames().getIndexByName(f.bco->getName(op.arg));
             if (index != NameMap_t::nil) {
                 m_world.globalValues().set(index, valueStack.top());
             } else {
                 throw Error::unknownIdentifier(f.bco->getName(op.arg));
             }
             break;
         }
         default:
            handleInvalidOpcode();
        }
        break;

     case Opcode::maPop:
        /* Pop into variable */
        checkStack(1);
        switch (op.minor) {
         case Opcode::sNamedVariable:
         {
             Context::PropertyIndex_t index;
             if (Context* ctx = lookup(f.bco->getName(op.arg), index)) {
                 ctx->set(index, valueStack.top());
                 valueStack.popBack();
             } else {
                 throw Error::unknownIdentifier(f.bco->getName(op.arg));
             }
             break;
         }
         case Opcode::sLocal:
            f.localValues.setNew(op.arg, valueStack.extractTop());
            break;
         case Opcode::sStatic:
            m_frames.front()->localValues.setNew(op.arg, valueStack.extractTop());
            break;
         case Opcode::sShared:
            m_world.globalValues().setNew(op.arg, valueStack.extractTop());
            break;
         case Opcode::sNamedShared:
         {
             NameMap_t::Index_t index = m_world.globalPropertyNames().getIndexByName(f.bco->getName(op.arg));
             if (index != NameMap_t::nil) {
                 m_world.globalValues().setNew(index, valueStack.extractTop());
             } else {
                 throw Error::unknownIdentifier(f.bco->getName(op.arg));
             }
             break;
         }
         default:
            handleInvalidOpcode();
        }
        break;

     case Opcode::maMemref:
        switch (op.minor) {
         case Opcode::miIMCall:
         case Opcode::miIMLoad:
            /* Load/Evaluate TOS.field */
            checkStack(1);
            if (!valueStack.top()) {
                /* Dereferencing null stays null, no change to stack */
            } else if (Context* cv = dynamic_cast<Context*>(valueStack.top())) {
                /* It's a context */
                Context::PropertyIndex_t index;
                if (Context* foundContext = cv->lookup(f.bco->getName(op.arg), index)) {
                    /* Load permitted */
                    afl::data::Value* v = foundContext->get(index);
                    valueStack.popBack();
                    if (op.minor == Opcode::miIMCall) {
                        delete v;
                    } else {
                        valueStack.pushBackNew(v);
                    }
                } else {
                    /* Name not found */
                    throw Error::unknownIdentifier(f.bco->getName(op.arg));
                }
            } else {
                /* Not a context */
                throw Error::typeError(Error::ExpectRecord);
            }
            break;

         case Opcode::miIMStore:
         case Opcode::miIMPop:
            /* Store/Pop into TOS.field */
            checkStack(2);
            if (Context* cv = dynamic_cast<Context*>(valueStack.top())) {
                /* It's a context */
                Context::PropertyIndex_t index;
                if (Context* foundContext = cv->lookup(f.bco->getName(op.arg), index)) {
                    /* Assignment permitted */
                    foundContext->set(index, valueStack.top(1));
                } else {
                    /* Name not found */
                    throw Error::unknownIdentifier(f.bco->getName(op.arg));
                }
            } else {
                /* Not a context */
                throw Error::typeError(Error::ExpectRecord);
            }
            valueStack.popBack();            // context
            if (op.minor == Opcode::miIMPop) {
                valueStack.popBack();        // value
            }
            break;
         default:
            handleInvalidOpcode();
        }
        break;

     case Opcode::maDim:
        /* Create variable, initialize with TOS unless it already exists. */
        switch (op.minor) {
         case Opcode::sLocal:
            handleDim(f.localValues, f.localNames, op.arg);
            break;
         case Opcode::sStatic:
            handleDim(m_frames.front()->localValues, m_frames.front()->localNames, op.arg);
            break;
         case Opcode::sShared:
            handleDim(m_world.globalValues(), m_world.globalPropertyNames(), op.arg);
            break;
         default:
            handleInvalidOpcode();
            break;
        }
        break;

     case Opcode::maSpecial:
        switch (op.minor) {
         case Opcode::miSpecialUncatch:
            /* Cancel previous catch */
            popExceptionHandler();
            break;
         case Opcode::miSpecialReturn:
            /* Stop this frame, return to caller */
            checkStack(op.arg);
            popFrame();
            break;
         case Opcode::miSpecialWith:
            /* Add TOS to context stack */
            checkStack(1);
            if (Context* cv = dynamic_cast<Context*>(valueStack.top())) {
                /* Valid context */
                valueStack.extractTop();
                pushNewContext(cv);
            } else {
                /* Not a context */
                throw Error::typeError(Error::ExpectRecord);
            }
            break;
         case Opcode::miSpecialEndWith:
            /* Cancel previous miSpecialWith */
            popContext();
            break;
         case Opcode::miSpecialFirstIndex:
            /* Start iteration */
            checkStack(1);
            if (CallableValue* iv = dynamic_cast<CallableValue*>(valueStack.top())) {
                if (Context* con = iv->makeFirstContext()) {
                    /* We have something to iterate over */
                    pushNewContext(con);
                    valueStack.popBack();
                    valueStack.pushBackNew(makeBooleanValue(1));
                } else {
                    /* Set exists but is empty */
                    valueStack.popBack();
                    valueStack.pushBackNew(0);
                }
            } else {
                /* This is not a set */
                throw Error::typeError(Error::ExpectIterable);
            }
            break;
         case Opcode::miSpecialNextIndex:
            /* Continue iteration */
            if (m_contexts.back()->next()) {
                valueStack.pushBackNew(makeBooleanValue(1));
            } else {
                popContext();
                valueStack.pushBackNew(0);
            }
            break;
         case Opcode::miSpecialEndIndex:
            /* Cancel iteration */
            popContext();
            break;
         case Opcode::miSpecialEvalStatement:
            /* Eval */
            handleEvalStatement(op.arg);
            break;
         case Opcode::miSpecialEvalExpr:
            /* Eval */
            handleEvalExpression();
            break;
         case Opcode::miSpecialDefSub:
            /* Define subroutine */
            {
                checkStack(1);
                NameMap_t::Index_t index = m_world.globalPropertyNames().addMaybe(f.bco->getName(op.arg));
                m_world.globalValues().setNew(index, valueStack.extractTop());
            }
            break;
         case Opcode::miSpecialDefShipProperty:
            m_world.shipPropertyNames().addMaybe(f.bco->getName(op.arg));
            break;
         case Opcode::miSpecialDefPlanetProperty:
            m_world.planetPropertyNames().addMaybe(f.bco->getName(op.arg));
            break;
         case Opcode::miSpecialLoad:
            checkStack(1);
            if (valueStack.top() != 0) {
                String_t name = toString(valueStack.top(), false);
                valueStack.popBack();
                if (!handleLoad(name, f.bco->getOrigin())) {
                    valueStack.pushBackNew(makeStringValue("File not found"));
                } else {
                    valueStack.pushBackNew(0);
                }
            }
            break;
         case Opcode::miSpecialPrint:
            checkStack(1);
            if (valueStack.top() != 0) {
                m_world.logListener().write(afl::sys::LogListener::Info, "script", toString(valueStack.top(), false));
            }
            valueStack.popBack();
            break;
         case Opcode::miSpecialAddHook:
            if (op.arg != 0)
                handleInvalidOpcode();
            handleAddHook();
            break;
         case Opcode::miSpecialRunHook: {
            if (op.arg != 0)
                handleInvalidOpcode();
            checkStack(1);
            std::auto_ptr<afl::data::Value> p(valueStack.extractTop());
            if (p.get() != 0) {
                String_t name = "ON " + toString(p.get(), false);
                if (CallableValue* cv = dynamic_cast<CallableValue*>(m_world.globalValues()[m_world.globalPropertyNames().getIndexByName(name)])) {
                    /* It is defined and refers to a subroutine (we ignore the invalid case where it is defind but not a subroutine */
                    Segment_t args;
                    cv->call(*this, args, false);
                }
            }
            break;
         }
         case Opcode::miSpecialThrow:
            checkStack(1);
            if (afl::data::Value* v = valueStack.top()) {
                handleException(toString(v, false), String_t());
            } else {
                handleException("Throw empty", String_t());
            }
            break;
         case Opcode::miSpecialTerminate:
            m_state = Terminated;
            break;
         case Opcode::miSpecialSuspend:
            m_state = Suspended;
            break;
         case Opcode::miSpecialNewArray:
            handleNewArray(op.arg);
            break;
         case Opcode::miSpecialMakeList:
            handleMakeList(op.arg);
            break;
         case Opcode::miSpecialNewHash:
            if (op.arg != 0)
                handleInvalidOpcode();
            handleNewHash();
            break;
         case Opcode::miSpecialInstance:
            checkStack(1);
            if (StructureType* isv = dynamic_cast<StructureType*>(valueStack.top())) {
                afl::base::Ref<StructureTypeData> isvd = isv->getType();
                valueStack.popBack();
                valueStack.pushBackNew(new StructureValue(*new StructureValueData(isvd)));
            } else {
                handleException("Invalid structure constructor", String_t());
            }
            break;
         case Opcode::miSpecialResizeArray:
            handleResizeArray(op.arg);
            break;
         case Opcode::miSpecialBind:
            handleBind(op.arg);
            break;
         case Opcode::miSpecialFirst:
            /* Start iteration */
            /* @since 2.0.7, 2.40.6 */
            checkStack(1);
            if (CallableValue* iv = dynamic_cast<CallableValue*>(valueStack.top())) {
                /* We have something to iterate over */
                Context* con = iv->makeFirstContext();
                valueStack.popBack();
                valueStack.pushBackNew(con);
            } else {
                /* This is not a set */
                throw Error::typeError(Error::ExpectIterable);
            }
            break;

         case Opcode::miSpecialNext:
            /* Continue iteration */
            /* @since 2.0.7, 2.40.6 */
            checkStack(1);
            if (Context* ctx = dynamic_cast<Context*>(valueStack.top())) {
                /* It's a context, advance */
                if (!ctx->next()) {
                    /* End of iteration, dump it */
                    valueStack.popBack();
                    valueStack.pushBackNew(0);
                }
            } else {
                /* Wrong usage, not a set */
                throw Error::typeError(Error::ExpectIterable);
            }
            break;

         default:
            handleInvalidOpcode();
        }
        break;

     case Opcode::maFusedUnary:
        if (f.pc < f.bco->getNumInstructions()) {
            afl::data::Value* a = getReferencedValue(op);
            afl::data::Value* result = executeUnaryOperation(m_world, (*f.bco)(f.pc).minor, a);
            valueStack.pushBackNew(result);
            ++f.pc;
        } else {
            handleInvalidOpcode();
        }
        break;

     case Opcode::maFusedBinary:
        if (f.pc < f.bco->getNumInstructions()) {
            checkStack(1);
            afl::data::Value* a = valueStack.top(0);
            afl::data::Value* b = getReferencedValue(op);
            afl::data::Value* result = executeBinaryOperation(m_world, (*f.bco)(f.pc).minor, a, b);
            valueStack.popBack();
            valueStack.pushBackNew(result);
            ++f.pc;
        } else {
            handleInvalidOpcode();
        }
        break;

     case Opcode::maFusedComparison:
        /* bcmp + jmp */
        if (f.pc < f.bco->getNumInstructions()) {
            checkStack(2);
            afl::data::Value* a = valueStack.top(1);
            afl::data::Value* b = valueStack.top(0);

            const Opcode& next = (*f.bco)(f.pc);
            int result = executeComparison(op.minor, a, b);
            int mask = (result < 0 ? Opcode::jIfEmpty
                        : result == 0 ? Opcode::jIfFalse
                        : Opcode::jIfTrue);
            if ((next.minor & mask) != 0) {
                // Perform the jump
                f.pc = f.bco->getJumpTarget(next.minor, next.arg);
            } else {
                // Skip the jump
                ++f.pc;
            }
            valueStack.popBackN(2);
        } else {
            handleInvalidOpcode();
        }
        break;

     case Opcode::maFusedComparison2:
        /* push + bcmp + jxxp */
        if (f.pc+1 < f.bco->getNumInstructions()) {
            checkStack(1);
            afl::data::Value* a = valueStack.top(0);
            afl::data::Value* b = getReferencedValue(op);

            int result = executeComparison((*f.bco)(f.pc).minor, a, b);
            const Opcode& next = (*f.bco)(f.pc+1);
            int mask = (result < 0 ? Opcode::jIfEmpty
                        : result == 0 ? Opcode::jIfFalse
                        : Opcode::jIfTrue);
            if ((next.minor & mask) != 0) {
                // Perform the jump
                f.pc = f.bco->getJumpTarget(next.minor, next.arg);
            } else {
                // Skip the jump
                f.pc += 2;
            }
            valueStack.popBack();
        } else {
            handleInvalidOpcode();
        }
        break;

     case Opcode::maInplaceUnary:
        /* pushloc + uinc/udec */
        if (f.pc < f.bco->getNumInstructions() && op.minor == Opcode::sLocal) {
            /* Check delta */
            int32_t delta = (*f.bco)(f.pc).minor == unInc ? +1 : -1;

            /* Operate */
            afl::data::Value* v = f.localValues.extractElement(op.arg);
            if (afl::data::IntegerValue* iv = dynamic_cast<afl::data::IntegerValue*>(v)) {
                iv->add(delta);
                ++f.pc;
            } else if (afl::data::FloatValue* fv = dynamic_cast<afl::data::FloatValue*>(v)) {
                fv->add(delta);
                ++f.pc;
            } else {
                // We cannot operate on this. Just push it.
            }
            valueStack.pushBackNew(v);
        } else {
            handleInvalidOpcode();
        }
        break;

     default:
        handleInvalidOpcode();
    }
}


// /** Suspend this process to perform UI operations. */
void
interpreter::Process::suspendForUI()
{
    // ex IntExecutionContext::suspendForUI, sort-of
    // FIXME: original code verifies that process is actually running
    // FIXME: pre-execute jumps and frame terminations to cause the process to execute prematurely
    // - be careful with frames catching UI.RESULT
    // - be careful with return values
    m_state = Waiting;
    m_world.notifyListeners();
}

// /** Perform name lookup.
//     \param name  [in] Name to look up
//     \param index [out] If found, the context's cookie for access to the name
//     \return Context containing the name, 0 if none */
interpreter::Context*
interpreter::Process::lookup(const afl::data::NameQuery& q, Context::PropertyIndex_t& index)
{
    // ex IntExecutionContext::lookup
    for (size_t i = m_contexts.size(); i > 0; --i) {
        Context* c = m_contexts[i-1];
        if (Context* fc = c->lookup(q, index)) {
            return fc;
        }
    }
    return 0;
}

// /** Set variable in this process. This consumes possible exceptions.
//     It is intended to be used by implementations of commands which set
//     variables by name; generally, those are "UI.RESULT" or
//     "CARGO.REMAINDER" which may not fail.
//     \retval true assignment succeeded
//     \retval false assignment failed */
bool
interpreter::Process::setVariable(String_t name, afl::data::Value* value)
{
    // ex IntExecutionContext::setVariable
    try {
        Context::PropertyIndex_t index;
        if (Context* ctx = lookup(name, index)) {
            ctx->set(index, value);
            return true;
        }
    }
    catch (Error&) { }
    return false;
}

// /** Get variable in this process.
//     \return variable value; Null if variable is not set. */
afl::data::Value*
interpreter::Process::getVariable(String_t name)
{
    // ex IntExecutionContext::getVariable
    Context::PropertyIndex_t index;
    if (Context* ctx = lookup(name, index)) {
        return ctx->get(index);
    } else {
        return 0;
    }
}

// /** Get object this process is currently in. */
game::map::Object*
interpreter::Process::getCurrentObject() const
{
    // ex IntExecutionContext::getCurrentObject
    for (size_t i = m_contexts.size(); i > 0; --i) {
        if (game::map::Object* obj = m_contexts[i-1]->getObject()) {
            return obj;
        }
    }
    return 0;
}

// /** Get object this process was invoked from. */
game::map::Object*
interpreter::Process::getInvokingObject() const
{
    // ex IntExecutionContext::getInvokingObject
    for (size_t i = m_contextTOS; i > 0; --i) {
        if (game::map::Object* obj = m_contexts[i-1]->getObject()) {
            return obj;
        }
    }
    return 0;
}

// /** Handle user function call instruction.
//     \param bco called function
//     \param args arguments provided by user (can be modified)
//     \param wantResult true if we expect a result, false if we don't */
void
interpreter::Process::handleFunctionCall(BCORef_t bco, Segment_t& args, bool wantResult)
{
    // ex IntExecutionContext::handleFunctionCall
    /* Verify number of arguments */
    checkArgumentCount(args.size(), bco->getMinArgs(), bco->isVarargs() ? args.size() : bco->getMaxArgs());

    /* Make a new frame executing this BCO */
    Frame& frame = pushFrame(bco, wantResult);

    /* Prepare parameters */
    size_t numArgs = std::min(args.size(), size_t(bco->getMaxArgs()));
    size_t numVarArgs = args.size() - numArgs;
    afl::base::Ptr<ArrayData> va;

    /* Copy the varargs */
    if (bco->isVarargs()) {
        va = new ArrayData();
        va->addDimension(numVarArgs);
        args.transferLastTo(numVarArgs, va->content);
    }

    /* Copy regular parameters */
    args.transferLastTo(numArgs, frame.localValues);

    /* Store the varargs */
    if (bco->isVarargs()) {
        frame.localValues.setNew(bco->getMaxArgs(), new ArrayValue(*va));
    }
}

// FIXME: port/remove
// /** Set break handler. This function is called periodically to check
//     whether the user wants to break. */
// void
// IntExecutionContext::setBreakHandler(bool check_break())
// {
//     ::check_break = check_break;
// }
//
//
// /** Handle Load instruction. Loads and compiles the file, and pushes an appropriate frame executing it.
//     \param name File name given by user
//     \retval true file loaded and compiled successfully
//     \retval false file not found
//     \throw IntError on compilation error */
bool
interpreter::Process::handleLoad(String_t name, const String_t& origin)
{
    // ex IntExecutionContext::handleLoad
    afl::base::Ptr<afl::io::Stream> file = m_world.openLoadFile(name);
    if (file.get() == 0) {
        // Failure
        return false;
    } else {
        // Make new frame
        pushFrame(m_world.compileFile(*file, origin, StatementCompiler::DEFAULT_OPTIMISATION_LEVEL), false);
        return true;
    }
}


// /** Set notification message. The message will be associated with this
//     process. In case another message was associated with the process,
//     it will be removed.
//     \param msg Message. Can be 0 to delete this process' message. */
// void
// IntExecutionContext::setNotificationMessage(IntNotificationMessage* msg)
// {
//     if (notification_message != msg) {
//         /* Detach old message. Note that oldmsg->remove will call
//            setNotificationMessage(0), so first get the message out
//            of the way. */
//         IntNotificationMessage* oldmsg = notification_message;
//         notification_message = 0;
//         if (oldmsg != 0) {
//             oldmsg->remove();
//         }
//
//         /* Attach new message */
//         notification_message = msg;
//         if (msg != 0) {
//             msg->setAssociatedProcess(this);
//         }
//     }
// }
//
// /** Get current notification message. */
// IntNotificationMessage*
// IntExecutionContext::getNotificationMessage() const
// {
//     return notification_message;
// }
//
// /** Check whether the user wants to break. */
// bool
// IntExecutionContext::checkForBreak()
// {
//     /* Check for break, and set termination flag. We cannot just discard
//        the executing frames because we're still in executeInstruction which
//        still refers to one of them. */
//     bool result = check_break();
//     if (result)
//         m_processState = psTerminated;
//     return result;
// }

// /** Check that stack contains a number of values. */
inline void
interpreter::Process::checkStack(uint32_t required)
{
    // ex IntExecutionContext::checkStack
    if (m_valueStack.size() < required)
        throw Error::internalError("stack error");
}

// /** Handle invalid opcode. */
void
interpreter::Process::handleInvalidOpcode()
{
    // ex IntExecutionContext::handleInvalidOpcode()
    throw Error::internalError("invalid opcode");
}

// /** Handle exception. Transfers control to the most recently pushed exception handler,
//     if any, and have it process exception e. If there is no active handler, stops
//     the bytecode process. */
void
interpreter::Process::handleException(String_t e, String_t trace)
{
    // ex IntExecutionContext::handleException
    if (m_exceptions.size()) {
        /* There is a user-specified exception handler */
        ExceptionHandler* eh = m_exceptions.back();

        /* Unwind stacks */
        while (m_valueStack.size() > eh->valueSP) {
            m_valueStack.popBack();
        }
        while (m_contexts.size() > eh->contextSP) {
            m_contexts.popBack();
        }
        while (m_frames.size() > eh->frameSP) {
            m_frames.popBack();
        }

        /* Push exception value for user program */
        m_valueStack.pushBackNew(makeStringValue(e));

        /* Change program counter */
        m_frames.back()->pc = eh->pc;

        /* Pop exception frame */
        m_exceptions.popBack();

        /* We may have been called from a suspended process, so make us runnable again */
        m_state = Running;
    } else {
        /* No user-specified exception handler, so reflect error to caller */
        m_processError = Error(e);
        if (trace.size())
            m_processError.addTrace(trace);
        addTraceTo(m_processError);
        m_state = Failed;
    }
}

// /** Make stack frame context by Id. This is used to deserialize stack
//     frame contexts. It only works while actually deserializing a process.
//     \param id Id read from file
//     \return IntExecutionFrameContext object; 0 if those are not allowed here */
interpreter::Context*
interpreter::Process::makeFrameContext(size_t level)
{
    if (level < m_frames.size()) {
        return new FrameContext(*m_frames[level]);
    } else {
        return 0;
    }
}

// /** Handle Dim instruction.
//     \param values data segment affected by this instruction
//     \param names name list affected by this instruction
//     \param index index into our name table */
void
interpreter::Process::handleDim(Segment_t& values, NameMap_t& names, uint16_t index)
{
    // ex IntExecutionContext::handleDim
    /* Check stack */
    checkStack(1);

    /* Check name. Must not be blank. */
    const String_t name = m_frames.back()->bco->getName(index);
    if (name.size() == 0)
        handleInvalidOpcode();

    /* Does it already exist? */
    NameMap_t::Index_t existing = names.getIndexByName(name);
    if (existing == NameMap_t::nil) {
        /* No, add it */
        values.set(names.add(name), m_valueStack.top());
    }

    m_valueStack.popBack();
}

// /** Handle "sevals" statement. Compiles the arguments into a new temporary BCO
//     and pushes a frame executing that.
//     \param nargs Number of arguments given (=number of script lines) */
void
interpreter::Process::handleEvalStatement(uint16_t nargs)
{
    // ex IntExecutionContext::handleEvalStatement
    /* Verify stack */
    checkStack(nargs);

    /* Build command source */
    MemoryCommandSource mcs;
    for (size_t i = 0; i < nargs; ++i) {
        mcs.addLine(toString(m_valueStack.top(nargs-i-1), false));
    }

    /* Drop args */
    m_valueStack.popBackN(nargs);

    /* Prepare compilation */
    DefaultStatementCompilationContext scc(m_world);
    if (nargs == 1) {
        /* One-liner */
        scc.withFlag(scc.LocalContext)
            .withFlag(scc.ExpressionsAreStatements)
            .withFlag(scc.RefuseBlocks)
            .withFlag(scc.LinearExecution)
            .withContextProvider(this);
    } else {
        /* Multi-line block */
        scc.withFlag(scc.LocalContext)
            .withFlag(scc.ExpressionsAreStatements)
            .withFlag(scc.LinearExecution)
            .withContextProvider(0);
    }

    /* Compile */
    BCORef_t bco = *new BytecodeObject();
    StatementCompiler sc(mcs);
    sc.compileList(*bco, scc);
    sc.finishBCO(*bco, scc);
    bco->setName("Eval");

    /* Execute */
    pushFrame(bco, false);
}

// /** Handle "sevalx" instruction. Compiles the stack top into a new BCO,
//     and pushes a frame executing that and returning a single result. */
void
interpreter::Process::handleEvalExpression()
{
    // ex IntExecutionContext::handleEvalExpression
    /* Verify stack */
    checkStack(1);

    /* Eval(0) stays 0 */
    if (m_valueStack.top() == 0)
        return;

    /* Compile */
    Tokenizer tok(toString(m_valueStack.top(), false));
    std::auto_ptr<interpreter::expr::Node> expr(interpreter::expr::Parser(tok).parse());
    if (tok.getCurrentToken() != tok.tEnd) {
        throw Error::garbageAtEnd(true);
    }
    m_valueStack.popBack();

    BCORef_t bco = *new BytecodeObject();
    expr->compileValue(*bco, CompilationContext(m_world));
    bco->setIsProcedure(false);
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialReturn, 1);
    optimize(m_world, *bco, 1);
    bco->relocate();
    bco->setName("Eval");

    /* Execute */
    pushFrame(bco, true);
}

// /** Handle "saddhook" instruction. */
void
interpreter::Process::handleAddHook()
{
    // ex IntExecutionContext::handleAddHook
    /* Verify stack */
    checkStack(2);
    if (m_valueStack.top() == 0 || m_valueStack.top(1) == 0) {
        m_valueStack.popBackN(2);
        return;
    }

    /* Verify code */
    SubroutineValue* sv = dynamic_cast<SubroutineValue*>(m_valueStack.top());
    if (!sv) {
        throw Error::typeError(Error::ExpectProcedure);
    }

    /* Verify name */
    String_t hookName = "ON " + toString(m_valueStack.top(1), false);
    NameMap_t::Index_t pos = m_world.globalPropertyNames().addMaybe(hookName);
    BCOPtr_t hook; // FIXME: can we make this use BCORef?
    if (m_world.globalValues()[pos] == 0) {
        /* Create it */
        hook = new BytecodeObject();
        hook->setIsProcedure(true);
        hook->setName(hookName);
        m_world.globalValues().setNew(pos, new SubroutineValue(*hook));
    } else {
        /* Use existing */
        SubroutineValue* sv = dynamic_cast<SubroutineValue*>(m_world.globalValues()[pos]);
        if (!sv) {
            throw Error::typeError(Error::ExpectProcedure);
        }
        hook = sv->getBytecodeObject().asPtr();
    }

    /* Add it */
    hook->addPushLiteral(sv);
    hook->addInstruction(Opcode::maIndirect, Opcode::miIMCall, 0);

    /* Clean up stack */
    m_valueStack.popBackN(2);
}

/** Handle "snewarray" instruction. */
void
interpreter::Process::handleNewArray(uint16_t ndim)
{
    // ex IntExecutionContext::handleNewArray
    /* Check preconditions */
    if (ndim == 0)
        handleInvalidOpcode();
    checkStack(ndim);

    /* Create array object */
    afl::base::Ref<ArrayData> ad = *new ArrayData();
    for (size_t i = 0; i < ndim; ++i) {
        afl::data::ScalarValue* iv = dynamic_cast<afl::data::ScalarValue*>(m_valueStack.top(ndim - i - 1));
        if (!iv)
            throw Error::typeError(Error::ExpectInteger);
        if (!ad->addDimension(iv->getValue()))
            throw Error::rangeError();
    }
    m_valueStack.popBackN(ndim);
    m_valueStack.pushBackNew(new ArrayValue(ad));
}

// /** Handle "sresizearray" instruction. */
void
interpreter::Process::handleResizeArray(uint16_t ndim)
{
    // ex IntExecutionContext::handleResizeArray
    /* Check preconditions */
    if (ndim == 0)
        handleInvalidOpcode();
    checkStack(ndim+1);

    /* Create dummy array object */
    ArrayData ad;
    for (size_t i = 0; i < ndim; ++i) {
        afl::data::ScalarValue* iv = dynamic_cast<afl::data::ScalarValue*>(m_valueStack.top(ndim - i - 1));
        if (!iv)
            throw Error::typeError(Error::ExpectInteger);
        if (!ad.addDimension(iv->getValue()))
            throw Error::rangeError();
    }
    m_valueStack.popBackN(ndim);

    /* Fetch the array object */
    ArrayValue* a = dynamic_cast<ArrayValue*>(m_valueStack.top());
    if (!a) {
        throw Error::typeError(Error::ExpectArray);
    }
    afl::base::Ref<ArrayData> origAD = a->getData();

    /* Modify it */
    origAD->resize(ad);
    m_valueStack.popBack();
}

// /** Handle "smakelist" instruction. */
void
interpreter::Process::handleMakeList(uint16_t nelems)
{
    // ex IntExecutionContext::handleMakeList
    /* Check preconditions */
    checkStack(nelems);

    /* Create array object */
    afl::base::Ref<ArrayData> ad = *new ArrayData();
    if (!ad->addDimension(nelems))
        throw Error::rangeError();

    /* Populate it */
    m_valueStack.transferLastTo(nelems, ad->content);
    m_valueStack.pushBackNew(new ArrayValue(ad));
}

/** Handle "snewhash" instruction. */
void
interpreter::Process::handleNewHash()
{
    // IntExecutionContext::handleNewHash
    m_valueStack.pushBackNew(new HashValue(afl::data::Hash::create()));
}

// /** Handle "sbind" instruction. */
void
interpreter::Process::handleBind(uint16_t nargs)
{
    // ex IntExecutionContext::handleBind(uint16_t nargs)
    /* Check preconditions.
       Note that "sbind 0" is an expensive nop; let's allow it in case it someday allows something clever. */
    checkStack(nargs + 1);

    /* Build the closure */
    std::auto_ptr<Closure> c(new Closure());
    if (CallableValue* func = dynamic_cast<CallableValue*>(m_valueStack.top())) {
        /* OK */
        c->setNewFunction(func);
        m_valueStack.extractTop();
    } else {
        /* Error */
        throw Error::typeError(Error::ExpectCallable);
    }
    c->addNewArgumentsFrom(m_valueStack, nargs);

    /* Return */
    m_valueStack.pushBackNew(c.release());
}

// /** Handle decrement.
//     \return true iff result is zero */
bool
interpreter::Process::handleDecrement()
{
    // ex IntExecutionContext::handleDecrement()
    checkStack(1);
    afl::data::Value* v = m_valueStack.top();
    if (afl::data::IntegerValue* iv = dynamic_cast<afl::data::IntegerValue*>(v)) {
        iv->add(-1);
        return iv->getValue() == 0;
    } else if (afl::data::FloatValue* fv = dynamic_cast<afl::data::FloatValue*>(v)) {
        fv->add(-1);
        return fv->getValue() == 0;
    } else {
        throw Error::typeError(Error::ExpectNumeric);
    }
}

/** Get value referenced by an instruction. */
afl::data::Value*
interpreter::Process::getReferencedValue(const Opcode& op)
{
    // ex IntExecutionContext::getReferencedValue
    switch (op.minor) {
     case Opcode::sLocal:
        return m_frames.back()->localValues[op.arg];
     case Opcode::sStatic:
        return m_frames.front()->localValues[op.arg];
     case Opcode::sShared:
        return m_world.globalValues()[op.arg];
     case Opcode::sNamedShared:
     {
         NameMap_t::Index_t index = m_world.globalPropertyNames().getIndexByName(m_frames.back()->bco->getName(op.arg));
         if (index != NameMap_t::nil) {
             return m_world.globalValues()[index];
         } else {
             throw Error::unknownIdentifier(m_frames.back()->bco->getName(op.arg));
         }
     }
     case Opcode::sLiteral:
        return m_frames.back()->bco->getLiteral(op.arg);
     default:
        handleInvalidOpcode();
        return 0;
    }
}

void
interpreter::Process::logProcessState(const char* why)
{
    const char* state = "?";
    switch (m_state) {
     case Suspended:  state = "Suspended"; break;
     case Frozen:     state = "Frozen"; break;
     case Runnable:   state = "Runnable"; break;
     case Running:    state = "Running"; break;
     case Waiting:    state = "Waiting"; break;
     case Ended:      state = "Ended"; break;
     case Terminated: state = "Terminated"; break;
     case Failed:     state = "Failed"; break;
    }

    // Write a brief message:
    //   run 17@33 Running 'UI.Foo'
    m_world.logListener().write(afl::sys::LogListener::Trace, LOG_NAME,
                                afl::string::Format("%s %d@%d %s, '%s'") << why << m_processId << m_processGroupId << state << m_processName);
}
