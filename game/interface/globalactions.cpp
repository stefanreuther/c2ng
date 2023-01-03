/**
  *  \file game/interface/globalactions.cpp
  *  \brief Class game::interface::GlobalActions
  */

#include <memory>
#include "game/interface/globalactions.hpp"
#include "afl/data/integervalue.hpp"
#include "interpreter/mutexfunctions.hpp"
#include "interpreter/optimizer.hpp"
#include "interpreter/structuretype.hpp"
#include "interpreter/structuretypedata.hpp"
#include "interpreter/subroutinevalue.hpp"

using game::interface::GlobalActions;
using interpreter::BytecodeObject;
using interpreter::CallableValue;
using interpreter::Opcode;
using interpreter::SubroutineValue;

/*
 *  @change We compile the general Global Action flow into a single script process.
 *  In contrast, PCC2 controls the entire flow from C++ code.
 *
 *  We wish to allow actions implemented in scripts, and use the UI and I/O synchronisation provided for scripts.
 *  The downside is that we cannot easily spawn new script processes from the global action,
 *  e.g. run an auto-task command on many units.
 *
 *  General operation:
 *
 *  Each action has a LocalState (provided by the action's prepareFunction()),
 *  and a GlobalState (structure provided by framework, with attributes NumErrors, NumSuccess, Error).
 *
 *  The execSub is wrapped into a subroutine that checks preconditions (actionSub) and handles errors.
 *  We then create either a loop over the Ship/Planet arrays, or iterate over a list and generate code for each matching object.
 */

namespace {
    const int OPTIMIZATION_LEVEL = 2;

    /*
     *  Macros (partially shared with SearchQuery)
     */

    /** Top half of a 'Try xxx' instruction.
        @param[out] bco Target BCO
        @return catchLabel parameter for endTry() */
    BytecodeObject::Label_t startTry(interpreter::BytecodeObject& bco)
    {
        BytecodeObject::Label_t catchLabel = bco.makeLabel();
        bco.addJump(Opcode::jCatch, catchLabel);
        return catchLabel;
    }

    /** Bottom half of a 'Try xxx' instruction.
        @param[out] bco Target BCO
        @param[in] catchLabel result of startTry() */
    void endTry(interpreter::BytecodeObject& bco, BytecodeObject::Label_t catchLabel)
    {
        BytecodeObject::Label_t endLabel = bco.makeLabel();
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialUncatch, 0);
        bco.addJump(Opcode::jAlways, endLabel);
        bco.addLabel(catchLabel);
        bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
        bco.addLabel(endLabel);
    }

    /** Generate code to load an attribute of an object (obj->name).
        @param[out] bco  Target BCO
        @param[in]  obj  Object attribute variable
        @param[in]  name Attribute name */
    void loadAttribute(interpreter::BytecodeObject& bco, uint16_t obj, const char* name)
    {
        bco.addInstruction(Opcode::maPush, Opcode::sLocal, obj);
        bco.addInstruction(Opcode::maMemref, Opcode::miIMLoad, bco.addName(name));
    }

    /** Generate code to pop an attribute of an object (obj->name).
        @param[out] bco  Target BCO
        @param[in]  obj  Object attribute variable
        @param[in]  name Attribute name */
    void popAttribute(interpreter::BytecodeObject& bco, uint16_t obj, const char* name)
    {
        bco.addInstruction(Opcode::maPush, Opcode::sLocal, obj);
        bco.addInstruction(Opcode::maMemref, Opcode::miIMPop, bco.addName(name));
    }

    /** Compile conditional exit.
        @param[out] bco         Target BCO
        @param[in]  continueIf  Conditions (jIfEmpty, etc.) when to proceed */
    void exitUnless(interpreter::BytecodeObject& bco, uint8_t continueIf)
    {
        BytecodeObject::Label_t skipLabel = bco.makeLabel();
        bco.addJump(continueIf | Opcode::jPopAlways, skipLabel);
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialReturn, 0);
        bco.addLabel(skipLabel);
    }

    /** Finish a BCO.
        @param world  World
        @param bco    BytecodeObject */
    void finish(interpreter::World& world, BytecodeObject& bco)
    {
        if (OPTIMIZATION_LEVEL > 0) {
            optimize(world, bco, OPTIMIZATION_LEVEL);
        }
        if (OPTIMIZATION_LEVEL >= 0) {
            bco.relocate();
        }
    }


    /*
     *  Global Action compilation
     */

    // GlobalState structure offsets
    const char*const GS_NumSuccess = "NUMSUCCESS";
    const char*const GS_NumErrors  = "NUMERRORS";
    const char*const GS_Error      = "ERROR";

    /** Create action wrapper.
        Wraps an action's execSub into a sub that checks preconditions, handles errors, and does basic accounting.
        @param world   Interpreter world
        @param execSub Action's exec subroutine
        @param flags   Flags
        @return Newly-allocated BCO */
    interpreter::BCORef_t compileAction(interpreter::World& world, const CallableValue& execSub, GlobalActions::Flags_t flags)
    {
        // ex WGlobalActionParameter::maybeProcess (sort-of), globact.pas:MaybeExec (sort-of)
        // Parameters
        interpreter::BCORef_t bco = BytecodeObject::create(true);
        uint16_t obj         = bco->addArgument("OBJ", false);
        uint16_t localState  = bco->addArgument("LOCALSTATE", false);
        uint16_t globalState = bco->addArgument("GLOBALSTATE", false);

        // Check preconditions
        // Failure when checking preconditions is not fatal
        const BytecodeObject::Label_t precondCatch = startTry(*bco);

        // Reject unmarked, if requested: "Try If Not obj->Marked Then Return"
        if (flags.contains(GlobalActions::ExcludeUnmarkedObjects)) {
            loadAttribute(*bco, obj, "MARKED");
            exitUnless(*bco, Opcode::jIfEmpty | Opcode::jIfTrue);
        }

        // Reject numeric friendly codes, if requested: "Try If Not IsEmpty(obj->FCode) Then Return"
        if (flags.contains(GlobalActions::ExcludeNumericFriendlyCodes)) {
            loadAttribute(*bco, obj, "FCODE");
            bco->addInstruction(Opcode::maUnary, interpreter::unVal, 0);
            exitUnless(*bco, Opcode::jIfEmpty);
        }

        // Reject special friendly codes, if requested: "Try If Not IsSpecialFCode(obj->FCode) Then Return"
        if (flags.contains(GlobalActions::ExcludeSpecialFriendlyCodes)) {
            loadAttribute(*bco, obj, "FCODE");
            bco->addInstruction(Opcode::maPush, Opcode::sNamedShared, bco->addName("ISSPECIALFCODE"));
            bco->addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 1);
            exitUnless(*bco, Opcode::jIfEmpty | Opcode::jIfFalse);
        }

        endTry(*bco, precondCatch);

        // Implementation
        const BytecodeObject::Label_t execCatch = startTry(*bco);

        // Call implementation: "execSub Obj, LocalState"
        bco->addInstruction(Opcode::maPush, Opcode::sLocal, obj);
        bco->addInstruction(Opcode::maPush, Opcode::sLocal, localState);
        bco->addPushLiteral(&execSub);
        bco->addInstruction(Opcode::maIndirect, Opcode::miIMCall, 2);

        // Count success: "GlobalState->NumSuccess := GlobalState->NumSuccess+1"
        loadAttribute(*bco, globalState, GS_NumSuccess);
        bco->addInstruction(Opcode::maUnary, interpreter::unInc, 0);
        popAttribute(*bco, globalState, GS_NumSuccess);

        // Error handling
        const BytecodeObject::Label_t execSkip = bco->makeLabel();
        bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialUncatch, 0);
        bco->addJump(Opcode::jAlways, execSkip);
        bco->addLabel(execCatch);

        // Save error: "GlobalState->Error := System.Err" (without actually creating a System.Err)
        popAttribute(*bco, globalState, GS_Error);

        // Count errors: "GlobalState->NumErrors := GlobalState->NumErrors+1"
        loadAttribute(*bco, globalState, GS_NumErrors);
        bco->addInstruction(Opcode::maUnary, interpreter::unInc, 0);
        popAttribute(*bco, globalState, GS_NumErrors);

        // Finish
        bco->addLabel(execSkip);
        finish(world, *bco);
        return bco;
    }

    /** Compile prelude (set up a global action process).
        @param [out] bco BytecodeObject
        @return Address of "GlobalState" local variable */
    uint16_t compilePrelude(BytecodeObject& bco, GlobalActions::Flags_t flags)
    {
        // If locks are being overridden, inject dummy functions
        if (flags.contains(GlobalActions::OverrideLocks)) {
            interpreter::registerDummyMutexFunctions(bco);
        }

        // Create structure
        afl::base::Ref<interpreter::StructureTypeData> stData(*new interpreter::StructureTypeData());
        stData->names().add(GS_NumSuccess);
        stData->names().add(GS_NumErrors);
        stData->names().add(GS_Error);
        interpreter::StructureType stValue(stData);

        // Create and initialize variable
        //   Local GlobalState = struct()
        uint16_t lv = bco.addLocalVariable("GLOBALSTATE");
        bco.addPushLiteral(&stValue);
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialInstance, 0);
        bco.addInstruction(Opcode::maPop, Opcode::sLocal, lv);
        //   GlobalState->NumSuccess := 0
        bco.addInstruction(Opcode::maPush, Opcode::sInteger, 0);
        bco.addInstruction(Opcode::maPush, Opcode::sLocal, lv);
        bco.addInstruction(Opcode::maMemref, Opcode::miIMPop, bco.addName(GS_NumSuccess));
        //   GlobalState->NumErrors := 0
        bco.addInstruction(Opcode::maPush, Opcode::sInteger, 0);
        bco.addInstruction(Opcode::maPush, Opcode::sLocal, lv);
        bco.addInstruction(Opcode::maMemref, Opcode::miIMPop, bco.addName(GS_NumErrors));

        return lv;
    }

    /** Compile preparation step.
        @param [out] bco              BytecodeObject
        @param [in]  prepareFunction  Action's preparation function */
    uint16_t compilePreparation(BytecodeObject& bco, const CallableValue& prepareFunction)
    {
        // Call preparation function
        //   Local LocalState = prepareFunction()
        uint16_t lv = bco.addLocalVariable("LOCALSTATE");
        bco.addPushLiteral(&prepareFunction);
        bco.addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 0);
        bco.addInstruction(Opcode::maStore, Opcode::sLocal, lv);
        // Skip execution if user canceled
        //   If IsEmpty(LocalState) Then Return
        exitUnless(bco, Opcode::jIfTrue | Opcode::jIfFalse);
        return lv;
    }

    /** Compile result step.
        @param [out] bco         BytecodeObject
        @param [in]  resultSub   Action's result subroutine
        @param [in]  localState  Address of LocalState variable
        @param [in]  globalState Address of GlobalState variable */
    void compileResult(BytecodeObject& bco, const CallableValue& resultSub, uint16_t localState, uint16_t globalState)
    {
        // Call finalisation function
        //   resultSub(LocalState, GlobalState)
        bco.addInstruction(Opcode::maPush, Opcode::sLocal, localState);
        bco.addInstruction(Opcode::maPush, Opcode::sLocal, globalState);
        bco.addPushLiteral(&resultSub);
        bco.addInstruction(Opcode::maIndirect, Opcode::miIMCall, 2);
    }

    /** Compile iteration over an array.
        This is used for compileGlobalAction.
        @param [out] bco         BytecodeObject
        @param [in]  actionSub   Action subroutine; result of compileAction()
        @param [in]  arrayName   Array to iterate over
        @param [in]  localState  Address of LocalState variable
        @param [in]  globalState Address of GlobalState variable */
    void compileIteration(BytecodeObject& bco, const afl::data::Value& actionSub, const char* arrayName, uint16_t localState, uint16_t globalState)
    {
        // ForEach <arrayName> As <A> Do actionSub <A>, <localState>, <globalState>
        const BytecodeObject::Label_t again = bco.makeLabel();
        const BytecodeObject::Label_t end   = bco.makeLabel();

        bco.addInstruction(Opcode::maPush, Opcode::sNamedShared, bco.addName(arrayName));    // array
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialFirst, 0);                    // obj
        bco.addLabel(again);
        bco.addJump(Opcode::jIfFalse | Opcode::jIfEmpty, end);
        bco.addInstruction(Opcode::maStack, Opcode::miStackDup, 0);                          // obj:obj
        bco.addInstruction(Opcode::maPush, Opcode::sLocal, localState);                      // obj:obj:localState
        bco.addInstruction(Opcode::maPush, Opcode::sLocal, globalState);                     // obj:obj:localState:globalState
        bco.addPushLiteral(&actionSub);                                                      // obj:obj:localState:globalState:actionSub
        bco.addInstruction(Opcode::maIndirect, Opcode::miIMCall, 3);                         // obj
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialNext, 0);                     // obj
        bco.addJump(Opcode::jAlways, again);
        bco.addLabel(end);
        bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
    }

    /** Compile action for a single object.
        This is used for compileListAction.
        @param [out] bco         BytecodeObject
        @param [in]  actionSub   Action subroutine; result of compileAction()
        @param [in]  arrayName   Array to iterate over
        @param [in]  id          Index into array (object Id)
        @param [in]  localState  Address of LocalState variable
        @param [in]  globalState Address of GlobalState variable */
    void compileSingleObject(BytecodeObject& bco, const afl::data::Value& actionSub, const char* arrayName, int32_t id, uint16_t localState, uint16_t globalState)
    {
        // actionSub <arrayName>(<Id>), <localState>, <globalState>
        const afl::data::IntegerValue idValue(id);
        bco.addPushLiteral(&idValue);                                                        // id
        bco.addInstruction(Opcode::maPush, Opcode::sNamedShared, bco.addName(arrayName));    // id:array
        bco.addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 1);                         // array(id)
        bco.addInstruction(Opcode::maPush, Opcode::sLocal, localState);                      // array(id):localState
        bco.addInstruction(Opcode::maPush, Opcode::sLocal, globalState);                     // array(id):localState:globalState
        bco.addPushLiteral(&actionSub);                                                      // array(id):localState:globalState:actionSub
        bco.addInstruction(Opcode::maIndirect, Opcode::miIMCall, 3);                         // []
    }
}


/*
 *  Action
 */

struct game::interface::GlobalActions::Action {
    std::auto_ptr<CallableValue> prepareFunction;
    std::auto_ptr<CallableValue> execSub;
    std::auto_ptr<CallableValue> resultSub;
};


/*
 *  GlobalActions
 */

// Constructor.
game::interface::GlobalActions::GlobalActions()
    : m_actions()
{ }

// Destructor.
game::interface::GlobalActions::~GlobalActions()
{ }

// Add a new global action.
size_t
game::interface::GlobalActions::addAction(const interpreter::CallableValue& prepareFunction,
                                          const interpreter::CallableValue& execSub,
                                          const interpreter::CallableValue& resultSub)
{
    std::auto_ptr<Action> a(new Action());
    a->prepareFunction.reset(prepareFunction.clone());
    a->execSub.reset(execSub.clone());
    a->resultSub.reset(resultSub.clone());
    m_actions.pushBackNew(a.release());
    return m_actions.size()-1;
}

// Get action by index.
const game::interface::GlobalActions::Action*
game::interface::GlobalActions::getActionByIndex(size_t idx) const
{
    if (idx < m_actions.size()) {
        return m_actions[idx];
    } else {
        return 0;
    }
}

// Generate code for global action (iterating over all objects).
interpreter::BCORef_t
game::interface::GlobalActions::compileGlobalAction(const Action* a, interpreter::World& world, Flags_t flags) const
{
    // ex WGlobalActionParameter::execute (part), globact.pas:Action (part)
    interpreter::BCORef_t result = BytecodeObject::create(true);
    if (a != 0) {
        // Setup
        const uint16_t globalState = compilePrelude(*result, flags);
        const uint16_t localState = compilePreparation(*result, *a->prepareFunction);
        const SubroutineValue actionSub(compileAction(world, *a->execSub, flags));

        // Iterate over arrays
        if (!flags.contains(ExcludeShips)) {
            compileIteration(*result, actionSub, "SHIP", localState, globalState);
        }
        if (!flags.contains(ExcludePlanets)) {
            compileIteration(*result, actionSub, "PLANET", localState, globalState);
        }

        // Finish
        compileResult(*result, *a->resultSub, localState, globalState);
        finish(world, *result);
    }
    return result;
}

// Generate code for action iterating over a list of objects.
interpreter::BCORef_t
game::interface::GlobalActions::compileListAction(const Action* a, const game::ref::List& list, interpreter::World& world, Flags_t flags) const
{
    // ex WGlobalActionParameter::execute (part), globact.pas:Action (part)
    interpreter::BCORef_t result = BytecodeObject::create(true);
    if (a != 0) {
        // Setup
        const uint16_t globalState = compilePrelude(*result, flags);
        const uint16_t localState = compilePreparation(*result, *a->prepareFunction);
        const SubroutineValue actionSub(compileAction(world, *a->execSub, flags));

        // Iterate over list
        for (size_t i = 0, n = list.size(); i < n; ++i) {
            Reference r = list[i];
            switch (r.getType()) {
             case Reference::Ship:
                if (!flags.contains(ExcludeShips)) {
                    compileSingleObject(*result, actionSub, "SHIP", r.getId(), localState, globalState);
                }
                break;

             case Reference::Planet:
             case Reference::Starbase:
                if (!flags.contains(ExcludePlanets)) {
                    compileSingleObject(*result, actionSub, "PLANET", r.getId(), localState, globalState);
                }
                break;

             case Reference::Null:
             case Reference::Special:
             case Reference::Player:
             case Reference::MapLocation:
             case Reference::IonStorm:
             case Reference::Minefield:
             case Reference::Ufo:
             case Reference::Hull:
             case Reference::Engine:
             case Reference::Beam:
             case Reference::Torpedo:
                break;
            }
        }

        // Finish
        compileResult(*result, *a->resultSub, localState, globalState);
        finish(world, *result);
    }
    return result;
}
