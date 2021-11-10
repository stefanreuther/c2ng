/**
  *  \file game/interface/taskeditorcontext.hpp
  */
#ifndef C2NG_GAME_INTERFACE_TASKEDITORCONTEXT_HPP
#define C2NG_GAME_INTERFACE_TASKEDITORCONTEXT_HPP

#include "afl/base/ptr.hpp"
#include "afl/data/value.hpp"
#include "interpreter/singlecontext.hpp"
#include "interpreter/taskeditor.hpp"
#include "game/session.hpp"
#include "interpreter/arguments.hpp"

namespace game { namespace interface {

    /** Task editor property identifier. */
    enum TaskEditorProperty {
        iteLines,               // Lines : Str()
        iteCursor,              // Cursor : Int
        itePC,                  // Current : Int
        iteIsInSubroutine,      // Current.Active : Bool
        iteTypeStr,             // Type : Str
        iteTypeInt,             // Type$ : Int
        iteObjectId             // Id : Int
    };

    /** Task editor method identifier. */
    enum TaskEditorMethod {
        itmAdd,                 // Add at cursor
        itmInsert,              // Insert at position
        itmDelete               // Delete range
    };


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
        virtual game::map::Object* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

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
    void setTaskEditorProperty(const afl::base::Ptr<interpreter::TaskEditor>& edit, TaskEditorProperty prop, const afl::data::Value* value);


    void callTaskEditorMethod(interpreter::TaskEditor& edit, TaskEditorMethod m, interpreter::Arguments& args);



} }

#endif
