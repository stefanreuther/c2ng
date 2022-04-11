/**
  *  \file game/interface/labelvector.hpp
  *  \brief Class game::interface::LabelVector
  */
#ifndef C2NG_GAME_INTERFACE_LABELVECTOR_HPP
#define C2NG_GAME_INTERFACE_LABELVECTOR_HPP

#include "afl/base/ref.hpp"
#include "afl/string/string.hpp"
#include "game/map/objecttype.hpp"
#include "game/types.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/world.hpp"
#include "util/vector.hpp"

namespace game { namespace interface {

    /** Object labels.
        Labels are computed using using the script interpreter, using an expression.
        This class provides the means to manage them for one object type.

        Each object's label has one of the states
        - clean (current value is known and valid)
        - dirty (object has changed, value needs to be recomputed)
        - updating (update is ongoing)
        - updating+dirty (normally, object changes are ignored while a label is being updated;
          however, it can be forced, for example, if the expression changes)

        Basic operation:
        - define expression using setExpression()
        - in Universe::sig_preUpdate, call checkObjects(), this sets labels to status dirty
        - when hasDirtyLabels() is set, use compileUpdater(), this sets labels to status updating
        - run the produced code, this sets labels using updateLabel()
        - finally, call finishUpdate() to revert them to clean (no longer updating). */
    class LabelVector {
     public:
        /** Constructor.
            Make an empty vector. */
        LabelVector();

        /** Destructor. */
        ~LabelVector();

        /** Clear all label state.
            Does not clear the expression and error state. */
        void clear();

        /** Get label for object by Id.
            @param id Id
            @return label */
        String_t getLabel(Id_t id) const;

        /** Check for changed objects and mark their labels dirty.
            Use as response to Universe::sig_preUpdate to mark objects.
            After this call,
            - call hasDirtyLabels() to check whether anything was marked dirty.
            - call compileUpdater() to make the updater code
            @param ty Object type */
        void checkObjects(game::map::ObjectType& ty);

        /** Forcibly mark object labels dirty.
            Use as response to Universe::sig_preUpdate to mark objects.
            After this call,
            - call hasDirtyLabels() to check whether anything was marked dirty.
            - call compileUpdater() to make the updater code
            @param ty Object type */
        void markObjects(game::map::ObjectType& ty);

        /** Forcibly mark everything clean.
            This reverts the marks done by checkObjects/markObjects and therefore discards updates. */
        void markClean();

        /** Update label.
            Marks the label finished updating.
            @param id       Object Id
            @param success  true if recomputation succeeded, false on error
            @param value    On success, new label; on error, error message */
        void updateLabel(Id_t id, bool success, String_t value);

        /** Check for dirty labels.
            If this returns true, use compileUpdater() to generate code to update it.
            @return true if there are dirty labels */
        bool hasDirtyLabels() const;

        /** Check for labels being updated.
            @return true if there are labels being updated
            @see compileUpdater() */
        bool hasUpdatingLabels() const;

        /** Check for changed labels.
            @return true Labels have been changed after last call to markLabelsUnchanged(). */
        bool hasChangedLabels() const;

        /** Reset labels-changed status. */
        void markLabelsUnchanged();

        /** Set expression.
            @param expr  Expression provided by user
            @param world World (for CompilationContext) */
        void setExpression(String_t expr, interpreter::World& world);

        /** Get expression.
            @return last successfully-set expression */
        String_t getExpression() const;

        /** Generate code to update labels.
            This moves all labels from the "need update" (dirty) state to the "being updated" state,
            thereby clearing hasDirtyLabels(), setting hasUpdatingLabels().

            Caller must arrange for the created code to run in a temporary process
            and then call finishUpdate().

            @param [out] bco             Bytecode object
            @param [in]  arrayFunction   Function to obtain object (e.g. "PLANET")
            @param [in]  updateFunction  Function to be called as "updateFunction(ID, VALUE, SUCCESS)",
                                         must be forwarded to updateLabel().

            @return Number of objects being updated */
        int compileUpdater(interpreter::BytecodeObject& bco, const interpreter::CallableValue& arrayFunction, const interpreter::CallableValue& updateFunction);

        /** Finish update.
            Clears the hasUpdatingLabels() status.
            Call this after possible changes to the universe have been processed by checkObjects().

            After this call, use hasDirtyLabels() to check whether there's more work to do. */
        void finishUpdate();

        /** Check error status.
            It is an error if execution of the given expression failed for all updates, and did not succeed for any single one.
            This most likely refers to an error on a name, for example.
            If it not an error if execution of the expression fails some of the time, e.g. divide-by-zero for some,
            because in this case, the problem depends on the data, not the expression.

            In addition, it is an error if the expression failed to compile.

            @return error status */
        bool hasError() const;

        /** Get last error.
            @return error
            @pre hasError() */
        String_t getLastError() const;

        /** Clear error status.
            @post !hasError() */
        void clearErrorStatus();

     private:
        util::Vector<String_t,Id_t> m_labelValues;
        util::Vector<uint8_t,Id_t> m_labelStatus;

        // Overall status
        bool m_hasDirtyLabels;
        bool m_hasUpdatingLabels;
        bool m_hasChangedLabels;

        // Recomputation status
        bool m_hasSuccess;
        bool m_hasError;
        String_t m_lastError;

        // Expression
        enum ExpressionState {
            ExpressionEmpty,
            ExpressionCompiled,
            ExpressionError
        };
        ExpressionState m_expressionState;              // always valid
        String_t m_expression;                          // always valid
        String_t m_expressionError;                     // set if ExpressionError
        interpreter::BCOPtr_t m_compiledExpression;     // set if ExpressionCompiled, otherwise null

        void setLabel(Id_t id, const String_t& value);
    };

} }

#endif
