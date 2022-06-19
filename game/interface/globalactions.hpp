/**
  *  \file game/interface/globalactions.hpp
  *  \brief Class game::interface::GlobalActions
  */
#ifndef C2NG_GAME_INTERFACE_GLOBALACTIONS_HPP
#define C2NG_GAME_INTERFACE_GLOBALACTIONS_HPP

#include "afl/bits/smallset.hpp"
#include "afl/container/ptrvector.hpp"
#include "game/ref/list.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/world.hpp"

namespace game { namespace interface {

    /** Global actions.
        Manages a list of actions and allows generating code for them.

        A global action is defined as a set of three CallableValues, defined in script or native code, see addAction() for details.

        The user interface provides a way to pick parameters (flags) and choose an action.
        The action is than compiled into bytecode on a case-by-case basis, and executes in a process.
        This allows using the entire user-interface and I/O synchronisation for the action. */
    class GlobalActions {
     public:
        struct Action;

        /** Flags. */
        enum Flag {
            ExcludeNumericFriendlyCodes,       ///< Do not process objects with numeric friendly codes. Handled at runtime.
            ExcludeSpecialFriendlyCodes,       ///< Do not process objects with special friendly codes. Handled at runtime.
            ExcludeUnmarkedObjects,            ///< Do not process unmarked objects. Handled at runtime.
            ExcludeShips,                      ///< Do not process ships. Handled at code-generation time.
            ExcludePlanets,                    ///< Do not process planets. Handled at code-generation time.
            OverrideLocks                      ///< Override locks.
        };
        typedef afl::bits::SmallSet<Flag> Flags_t;

        /** Constructor. */
        GlobalActions();

        /** Destructor. */
        ~GlobalActions();

        /** Add a new global action.
            @param prepareFunction  Preparation function.
                                    Must be a function defined as "Function prepare()".
                                    Must return a state (to proceed) or empty (to cancel).
                                    For example, this could be a function to ask for a friendly code, and return that, or empty.
            @param execSub          Execution subroutine.
                                    Must be defined as "Sub exec(obj,state)", called with
                                    - obj: object being worked on
                                    - state: state as returned by prepare()
                                    For example, could be a subroutine that does "Call obj->SetFCode state".
                                    The function is allowed to throw, which will be accounted by the generated code and not cause the iteration to abort.
            @param resultSub        Result subroutine.
                                    Must be defined as "Sub result(state,globalState)", called with
                                    - state: state as returned by prepare(), updated by exec()
                                    - globalState: global state provided by framework, a structure with attributes
                                      - NumSuccess: number of objects processed
                                      - NumErrors: total number of objects
                                      - Error: last error message
                                    The function will typically show some sort of summary message.
            @return Index assigned to this action */
        size_t addAction(const interpreter::CallableValue& prepareFunction,
                         const interpreter::CallableValue& execSub,
                         const interpreter::CallableValue& resultSub);

        /** Get action by index.
            @param idx Index
            @return Pointer to action on success, null if index is out of range */
        const Action* getActionByIndex(size_t idx) const;

        /** Generate code for global action (iterating over all objects).
            @param a      Action obtained with getActionByIndex(), should not be null
            @param world  Interpreter World
            @param flags  Flags that limit the accessed objects
            @return newly-allocated bytecode object */
        interpreter::BCORef_t compileGlobalAction(const Action* a, interpreter::World& world, Flags_t flags) const;

        /** Generate code for action iterating over a list of objects.
            @param a      Action obtained with getActionByIndex(), should not be null
            @param list   Object list
            @param world  Interpreter World
            @param flags  Flags that limit the accessed objects
            @return newly-allocated bytecode object */
        interpreter::BCORef_t compileListAction(const Action* a, const game::ref::List& list, interpreter::World& world, Flags_t flags) const;

     private:
        afl::container::PtrVector<Action> m_actions;
    };

} }

#endif
