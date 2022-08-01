/**
  *  \file interpreter/mutexfunctions.cpp
  *  \brief Mutex Functions
  */

#include "interpreter/mutexfunctions.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/error.hpp"
#include "interpreter/mutexcontext.hpp"
#include "interpreter/process.hpp"
#include "interpreter/simplefunction.hpp"
#include "interpreter/structuretypedata.hpp"
#include "interpreter/structurevalue.hpp"
#include "interpreter/structurevaluedata.hpp"
#include "interpreter/values.hpp"
#include "interpreter/world.hpp"

namespace interpreter { namespace {

    /** Implementation of Lock().
        This must be implemented separately using the full interface (not the simplified one)
        because it needs an execution context. */
    class LockFunction : public CallableValue {
     public:
        explicit LockFunction(World& world);
        ~LockFunction();

        // CallableValue:
        virtual void call(Process& proc, afl::data::Segment& args, bool want_result);
        virtual bool isProcedureCall() const;
        virtual int32_t getDimension(int32_t which) const;
        virtual Context* makeFirstContext();
        virtual LockFunction* clone() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const;

     private:
        World& m_world;
    };

    /*
     *  Helper for parsing Lock() args
     */

    struct LockArgs {
        String_t name;
        String_t note;
    };
    void parseLockArgs(Arguments& args, LockArgs& out)
    {
        args.checkArgumentCount(1, 2);
        if (!checkStringArg(out.name, args.getNext()) || out.name.empty()) {
            throw Error("Expecting lock name");
        }
        checkStringArg(out.note, args.getNext());
    }

    /*
     *  Helper for parsing GetLockInfo() args
     */

    struct GetLockInfoArgs {
        String_t name;
        int32_t option;
        GetLockInfoArgs()
            : name(), option()
            { }
    };
    bool parseGetLockInfoArgs(Arguments& args, GetLockInfoArgs& out)
    {
        args.checkArgumentCount(1, 2);
        if (!checkStringArg(out.name, args.getNext())) {
            return false;
        }
        checkIntegerArg(out.option, args.getNext(), 0, 2);
        return true;
    }

} }


/****************************** LockFunction *****************************/

interpreter::LockFunction::LockFunction(World& world)
    : m_world(world)
{ }

interpreter::LockFunction::~LockFunction()
{ }

// CallableValue:
void
interpreter::LockFunction::call(Process& proc, afl::data::Segment& args, bool want_result)
{
    // ex IntLock::call
    // ex ccexec.pas:op_LOCK_func
    /* @q Lock(name:Str, Optional hint:Str):Any (Function)
       Acquire a lock.

       When auto-tasks control parts of the game,
       it must be made sure that two tasks do not accidentally stomp on each other's feet,
       or that you don't accidentally change something the auto-task controls.
       To do that, auto-tasks can acquire locks, which are honored by the user interface.
       If the user tries to do something which an auto-task claims for itself,
       a warning message is displayed, citing the auto-task name and the %hint
       given by the %Lock invocation.

       Locks are acquired using the %Lock function in combination with the %With statement:
       | With Lock(name) Do
       |   % protected code here
       | EndWith
       The <tt>With Lock</tt> statement acquires the lock.
       The lock is held by the current process until the %With statement terminates,
       usually at the %EndWith.

       A lock is uniquely identified by a name.
       No two processes can have a lock at the same time.
       If a process tries to acquire a blocked lock, this fails with an error.

       The following lock names are known by PCC, and honored by the user interface:
       <table>
        <tr><td width="10">pNNN.tax</td>
            <td>Taxation. Controls the tax change commands (natives/colonists).</td></tr>
        <tr><td width="10">pNNN.struct</td>
            <td>Structures (mines/factories/defense). Controls the structure building commands.</td></tr>
        <tr><td width="10">sNNN.waypoint</td>
            <td>Waypoint. Controls the ship's waypoint. Setting an Intercept order is considered a waypoint change.
                Note that locking the waypoint on a fleet member can not always be enforced.</td></tr>
       </table>
       The names are case-insensitive. "NNN" stands for the unit Id (e.g. "p15.tax").

       <b>Note:</b> A lock does <em>not</em> block particular operations.
       Even if someone has the tax lock, the {SetColonistTax} command will still work.
       The lock is intended as a hint for user-interface commands to display a warning,
       but not to block anything.

       <b>Note 2:</b> Although %Lock formally is a function, using it in other ways than a
       <tt>With Lock</tt> statement is not supported; it may work or not, it's not guaranteed.
       The return value cannot meaningfully be used.

       @see GetLockInfo
       @since PCC2 1.99.17, PCC 1.1.2, PCC2 2.40.1 */
    Arguments a(args, 0, args.size());
    LockArgs parsed;
    parseLockArgs(a, parsed);

    MutexContext* result = new MutexContext(afl::string::strUCase(parsed.name), parsed.note);
    if (want_result) {
        proc.pushNewValue(result);
    } else {
        delete result;
    }
}

bool
interpreter::LockFunction::isProcedureCall() const
{
    // ex IntLock::isProcedureCall
    return false;
}

int32_t
interpreter::LockFunction::getDimension(int32_t /*which*/) const
{
    // ex IntLock::getDimension
    return 0;
}

interpreter::Context*
interpreter::LockFunction::makeFirstContext()
{
    // ex IntLock::makeFirstContext
    return rejectFirstContext();
}

interpreter::LockFunction*
interpreter::LockFunction::clone() const
{
    // ex IntLock::clone
    return new LockFunction(m_world);
}

// BaseValue:
String_t
interpreter::LockFunction::toString(bool /*readable*/) const
{
    // ex IntLock::toString
    return "Lock";
}

void
interpreter::LockFunction::store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const
{
    // ex IntLock::store
    rejectStore(out, aux, ctx);
}

/************************** GetLockInfoFunction **************************/

namespace interpreter { namespace {

    afl::data::Value* IFGetLockInfo(World& world, Arguments& args)
    {
        // ex IFGetLockInfo
        /* @q GetLockInfo(name:Str, Optional type:Int):Any (Function)
           Get lock information.
           Checks whether there is an active lock named %name, and return information about it.

           - type=0 (default): return true if there is a lock, false if there isn't
           - type=1: return the name of the process owning the lock, EMPTY if there is no lock
           - type=2: return the information text (%hint parameter for {Lock()}), EMPTY if there is no lock

           @see Lock()
           @since PCC2 1.99.17, PCC2 2.40.1 */

        // Parse args
        GetLockInfoArgs parsed;
        if (!parseGetLockInfoArgs(args, parsed)) {
            return 0;
        }

        // Action
        MutexList::Mutex* mtx = /*m_*/world.mutexList().query(afl::string::strUCase(parsed.name));
        switch (parsed.option) {
         case 0:
            return makeBooleanValue(mtx != 0);
         case 1:
            if (mtx != 0) {
                if (const Process* p = mtx->getOwner()) {
                    return makeStringValue(p->getName());
                }
            }
            break;
         case 2:
            if (mtx != 0) {
                return makeStringValue(mtx->getNote());
            }
            break;
        }
        return 0;
    }

} }

/**************************** Dummy Functions ****************************/

namespace interpreter { namespace {

    afl::data::Value* IFDummyLock(Arguments& args)
    {
        // Parse args
        LockArgs parsed;
        parseLockArgs(args, parsed);

        // Return empty structure (satisfies intended use in 'With')
        return new StructureValue(*new StructureValueData(*new StructureTypeData()));
    }

    afl::data::Value* IFDummyGetLockInfo(Arguments& args)
    {
        // Parse args
        GetLockInfoArgs parsed;
        if (!parseGetLockInfoArgs(args, parsed)) {
            return 0;
        }

        // Return correct type saying "no lock"
        if (parsed.option == 0) {
            return makeBooleanValue(false);
        } else {
            return 0;
        }
    }

    void defineFunction(BytecodeObject& bco, const char* name, afl::data::Value* (*fcn)(interpreter::Arguments&))
    {
        interpreter::SimpleFunction<void> funcValue(fcn);
        bco.addPushLiteral(&funcValue);
        bco.addInstruction(Opcode::maPop, Opcode::sLocal, bco.addLocalVariable(name));
    }

} }

/*
 *  Public Entry Points
 */

void
interpreter::registerMutexFunctions(World& world)
{
    world.setNewGlobalValue("LOCK", new LockFunction(world));
    world.setNewGlobalValue("GETLOCKINFO", new interpreter::SimpleFunction<World&>(world, IFGetLockInfo));
}

void
interpreter::registerDummyMutexFunctions(BytecodeObject& bco)
{
    defineFunction(bco, "LOCK", IFDummyLock);
    defineFunction(bco, "GETLOCKINFO", IFDummyGetLockInfo);
}
