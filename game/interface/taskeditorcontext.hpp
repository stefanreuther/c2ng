/**
  *  \file game/interface/taskeditorcontext.hpp
  *  \brief Class game::interface::TaskEditorContext
  */
#ifndef C2NG_GAME_INTERFACE_TASKEDITORCONTEXT_HPP
#define C2NG_GAME_INTERFACE_TASKEDITORCONTEXT_HPP

#include "afl/base/ptr.hpp"
#include "afl/data/value.hpp"
#include "game/session.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/singlecontext.hpp"
#include "interpreter/taskeditor.hpp"

namespace game { namespace interface {

    /** Task editor property identifier. */
    enum TaskEditorProperty {
        iteLines,                 // Lines : Str()
        iteCursor,                // Cursor : Int
        itePC,                    // Current : Int
        iteIsInSubroutine,        // Current.Active : Bool
        itePredictedCloakFuel,    // Predicted.Fuel.Cloak : Int
        itePredictedFCode,        // Predicted.FCode : Str
        itePredictedFuel,         // Predicted.Fuel : Int
        itePredictedMission,      // Predicted.Mission$ : Int
        itePredictedMovementFuel, // Predicted.Fuel.Move : Int
        itePredictedPositionX,    // Predicted.Loc.X : Int
        itePredictedPositionY,    // Predicted.Loc.Y : Int
        itePredictedSpeed,        // Predicted.Speed$ : Int
        iteTypeStr,               // Type : Str
        iteTypeInt,               // Type$ : Int
        iteObjectId               // Id : Int
    };

    /** Task editor method identifier. */
    enum TaskEditorMethod {
        itmAdd,                   // Add at cursor
        itmAddMovement,           // Add movement command
        itmConfirmMessage,        // Confirm this task's message
        itmInsert,                // Insert at position
        itmDelete                 // Delete range
    };


    const int imc_SetSpeed        = 1;   ///< Flag for insertMovementCommand: add SetSpeed command for optimum warp
    const int imc_AcceptDuplicate = 2;   ///< Flag for insertMovementCommand: force adding command even if it's a duplicate


    /** Task editor context: publish properties of an interpreter::TaskEditor.

        Note: this wraps a Ptr<TaskEditor>.
        For most code, this could be a Ref<> instead, and all code assumes it is never null.
        However, we need it nullable to be able to call releaseAutoTaskEditor() in the destructor. */
    class TaskEditorContext : public interpreter::SingleContext, public interpreter::Context::PropertyAccessor {
     public:
        /** Constructor.
            \param edit TaskEditor to publish. Must not be null.
            \param session Game session */
        TaskEditorContext(afl::base::Ptr<interpreter::TaskEditor> edit, game::Session& session);

        /** Destructor. */
        ~TaskEditorContext();

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual void set(PropertyIndex_t index, const afl::data::Value* value);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual TaskEditorContext* clone() const;
        virtual afl::base::Deletable* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor) const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        /** Create TaskEditorContext for an auto-task.
            Create the auto-task if needed.
            \param session Session
            \param k       Type of auto-task
            \param id      Object Id
            \return newly-allocated TaskEditorContext; null if no auto-task can be created
            \see Session::getAutoTaskEditor() */
        static TaskEditorContext* create(game::Session& session, interpreter::Process::ProcessKind k, Id_t id);

     private:
        afl::base::Ptr<interpreter::TaskEditor> m_edit;
        game::Session& m_session;
    };


    /** Get property of a TaskEditor.
        \param edit    TaskEditor
        \param prop    Property
        \param session Session
        \return newly-allocated value */
    afl::data::Value* getTaskEditorProperty(const afl::base::Ptr<interpreter::TaskEditor>& edit, TaskEditorProperty prop, game::Session& session);

    /** Set property of a TaskEditor.
        \param edit    TaskEditor
        \param prop    Property
        \param value   Value
        \throw interpreter::Error if property cannot be set or value is out of range */
    void setTaskEditorProperty(interpreter::TaskEditor& edit, TaskEditorProperty prop, const afl::data::Value* value);

    /** Call method on TaskEditor.
        \param edit    TaskEditor
        \param m       Method Id
        \param session Session
        \param args    Parameters */
    void callTaskEditorMethod(interpreter::TaskEditor& edit, TaskEditorMethod m, Session& session, interpreter::Arguments& args);

    /** Insert a movement command into a ship auto task.
        \param edit     TaskEditor, must be editing a ship task.
        \param verb     Verb to use
        \param pt       Target point
        \param flags    Flags (imc_XXX)
        \param session  Session (for predicting the ship's status) */
    void insertMovementCommand(interpreter::TaskEditor& edit, String_t verb, game::map::Point pt, int flags, Session& session);

} }

#endif
