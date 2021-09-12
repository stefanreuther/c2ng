/**
  *  \file game/interface/taskeditorcontext.cpp
  */

#include "game/interface/taskeditorcontext.hpp"
#include "interpreter/values.hpp"
#include "game/map/object.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/error.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/procedurevalue.hpp"
#include "afl/data/stringlist.hpp"

using interpreter::Error;
using interpreter::Process;
using interpreter::TaskEditor;
using interpreter::makeBooleanValue;
using interpreter::makeIntegerValue;
using interpreter::makeStringValue;
using interpreter::checkIntegerArg;
using interpreter::checkStringArg;

namespace {
    /*
     *  Conversion utilities
     */
    int32_t limitRange(size_t n)
    {
        if (n >= 0x7FFFFFFF) {
            return 0x7FFFFFFF;
        } else {
            return static_cast<int32_t>(n);
        }
    }

    bool checkIndexArg(size_t& out, const afl::data::Value* v, size_t limit)
    {
        // Parse value
        int32_t i;
        if (!checkIntegerArg(i, v)) {
            return false;
        }

        // Check range
        if (i < 0 || i >= limitRange(limit)) {
            throw Error::rangeError();
        }

        out = static_cast<size_t>(i);
        return true;
    }

    void validateCommand(const String_t& str)
    {
        if (!TaskEditor::isValidCommand(str)) {
            throw Error("This is not a valid auto task command");
        }
    }

    void addCommandListArg(afl::data::StringList_t& list, const afl::data::Value* p)
    {
        if (p != 0) {
            String_t s = interpreter::toString(p, false);
            validateCommand(s);
            list.push_back(s);
        }
    }

    void checkCommandListArg(afl::data::StringList_t& list, interpreter::Arguments& args)
    {
        while (args.getNumArgs() > 0) {
            afl::data::Value* p = args.getNext();
            if (interpreter::IndexableValue* iv = dynamic_cast<interpreter::IndexableValue*>(p)) {
                // It's an array
                if (iv->getDimension(0) != 1) {
                    throw Error::typeError(Error::ExpectArray);
                }
                for (int32_t i = 0, n = iv->getDimension(1); i < n; ++i) {
                    // Construct "(i)" arguments
                    afl::data::Segment argSeg;
                    argSeg.pushBackInteger(i);
                    interpreter::Arguments args(argSeg, 0, 1);

                    // Fetch value. This may throw.
                    std::auto_ptr<afl::data::Value> value(iv->get(args));

                    // Add
                    addCommandListArg(list, value.get());
                }
            } else {
                // Not an array, just stringify
                addCommandListArg(list, p);
            }
        }
    }


    /*
     *  Implementation of Lines()
     */

    class TaskEditorLinesProperty : public interpreter::IndexableValue {
     public:
        TaskEditorLinesProperty(const afl::base::Ptr<TaskEditor>& edit, game::Session& session)
            : m_edit(edit),
              m_session(session)
            { }
        ~TaskEditorLinesProperty()
            { m_session.releaseAutoTaskEditor(m_edit); }

        // IndexableValue:
        virtual afl::data::Value* get(interpreter::Arguments& args)
            {
                size_t index;
                args.checkArgumentCount(1);
                if (!checkIndexArg(index, args.getNext(), m_edit->getNumInstructions())) {
                    return 0;
                }
                return makeStringValue((*m_edit)[index]);
            }
        virtual void set(interpreter::Arguments& args, afl::data::Value* value)
            {
                size_t index;
                args.checkArgumentCount(1);
                if (!checkIndexArg(index, args.getNext(), m_edit->getNumInstructions())) {
                    return;
                }

                String_t stringValue;
                checkStringArg(stringValue, value);
                validateCommand(stringValue);

                m_edit->replace(index, 1, TaskEditor::Commands_t::fromSingleObject(stringValue), TaskEditor::DefaultCursor, TaskEditor::DefaultPC);
            }

        // CallableValue:
        virtual int32_t getDimension(int32_t which) const
            { return which == 0 ? 1 : limitRange(m_edit->getNumInstructions()); }
        virtual interpreter::Context* makeFirstContext()
            { throw Error::typeError(Error::ExpectIterable); }

        // BaseValue:
        virtual String_t toString(bool /*readable*/) const
            { return "#<array>"; }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
            { throw Error::notSerializable(); }

        // Value:
        virtual TaskEditorLinesProperty* clone() const
            { return new TaskEditorLinesProperty(m_edit, m_session); }

     private:
        afl::base::Ptr<TaskEditor> m_edit;
        game::Session& m_session;
    };


    /*
     *  TaskEditor Method
     */

    class TaskEditorClosure : public interpreter::ProcedureValue {
     public:
        TaskEditorClosure(const afl::base::Ptr<TaskEditor>& edit, game::interface::TaskEditorMethod m, game::Session& session)
            : m_edit(edit), m_method(m), m_session(session)
            { }
        ~TaskEditorClosure()
            { m_session.releaseAutoTaskEditor(m_edit); }
        virtual void call(Process& /*proc*/, interpreter::Arguments& args)
            { game::interface::callTaskEditorMethod(*m_edit, m_method, args); }
        virtual ProcedureValue* clone() const
            { return new TaskEditorClosure(m_edit, m_method, m_session); }
     private:
        afl::base::Ptr<TaskEditor> m_edit;
        game::interface::TaskEditorMethod m_method;
        game::Session& m_session;
    };


    /*
     *  Methods
     */

    /* @q Add command:Str... (Auto Task Command)
       Add the given commands to the current auto-task at cursor position.

       The commands can be either strings or an array of strings.

       @since PCC2 2.40.7 */
    void IFTaskEditor_Add(TaskEditor& edit, interpreter::Arguments& args)
    {
        args.checkArgumentCountAtLeast(1);

        afl::data::StringList_t lines;
        checkCommandListArg(lines, args);

        if (!lines.empty()) {
            edit.replace(edit.getCursor(), 0, lines, TaskEditor::PlaceCursorAfter, TaskEditor::DefaultPC);
        }
    }

    /* @q Insert pos:Any, command:Str... (Auto Task Command)
       Insert the given commands to the current auto-task at the given position.

       The position can be:
       - "next": make the commands execute next (insert at {Current (Auto Task Property)|Current})
       - "end": add commands at the end of the task, before a possible {Restart} command
       - a 0-based index: insert before the given position

       The commands can be either strings or an array of strings.

       @since PCC2 2.40.7 */
    void IFTaskEditor_Insert(TaskEditor& edit, interpreter::Arguments& args)
    {
        args.checkArgumentCountAtLeast(2);

        // Position
        afl::data::Value* pos = args.getNext();
        if (pos == 0) {
            return;
        }

        // Commands
        afl::data::StringList_t lines;
        checkCommandListArg(lines, args);

        // Do it
        String_t posStr = interpreter::toString(pos, false);
        if (afl::string::strCaseCompare(posStr, "next") == 0) {
            if (!lines.empty()) {
                edit.addAsCurrent(lines);
            }
        } else if (afl::string::strCaseCompare(posStr, "end") == 0) {
            if (!lines.empty()) {
                edit.addAtEnd(lines);
            }
        } else {
            size_t index = 0;
            checkIndexArg(index, pos, edit.getNumInstructions()+1);
            if (!lines.empty()) {
                edit.replace(index, 0, lines, TaskEditor::DefaultCursor, TaskEditor::DefaultPC);
            }
        }
    }

    /* @q Delete index:Int, Optional count:Int (Auto Task Command)
       Delete lines from the auto-task.
       The index parameter is the 0-based position of the line to delete.
       The count parameter specifies the number of lines to delete; if left out, one line is deleted.
       @since PCC2 2.40.7 */
    void IFTaskEditor_Delete(TaskEditor& edit, interpreter::Arguments& args)
    {
        args.checkArgumentCount(1, 2);

        // Index: [0, getNumInstructions()]
        size_t index = 0;
        if (!checkIndexArg(index, args.getNext(), edit.getNumInstructions()+1)) {
            return;
        }

        // Count: unrestricted, will be limited, defaults to 1
        size_t count = 1;
        checkIndexArg(count, args.getNext(), size_t(-1));
        count = std::min(count, edit.getNumInstructions() - index);

        // Do it
        edit.replace(index, count, afl::base::Nothing, TaskEditor::DefaultCursor, TaskEditor::DefaultPC);
    }



    /*
     *  Property Mapping
     */
    enum TaskEditorDomain {
        TaskEditorPropertyDomain,
        TaskEditorMethodDomain
    };

    const interpreter::NameTable TASKEDITOR_MAP[] = {
        { "ADD",             game::interface::itmAdd,            TaskEditorMethodDomain,   interpreter::thProcedure },
        /* @q Current:Int (Auto Task Property)
           Index of the current line in the auto-task.
           The index is 0-based, possible values range from 0 to Dim(Lines)-1.
           Can be assigned to change the next line to execute.
           @assignable
           @since PCC2 2.40.7 */
        { "CURRENT",         game::interface::itePC,             TaskEditorPropertyDomain, interpreter::thInt },

        /* @q Current.Active:Bool (Auto Task Property)
           Status of the current line in the auto-task.
           If true, the line has already begun executing.
           If false, the line has not yet started executing.
           @since PCC2 2.40.7 */
        { "CURRENT.ACTIVE",  game::interface::iteIsInSubroutine, TaskEditorPropertyDomain, interpreter::thBool },

        /* @q Cursor:Int (Auto Task Property)
           Cursor position.
           The index is 0-based, possible values range from 0 to Dim(Lines).
           Can be assigned to change the cursor position.
           @assignable
           @since PCC2 2.40.7 */
        { "CURSOR",          game::interface::iteCursor,         TaskEditorPropertyDomain, interpreter::thInt },

        { "DELETE",          game::interface::itmDelete,         TaskEditorMethodDomain,   interpreter::thProcedure },
        /* @q Id:Int (Auto Task Property)
           Id of the object this auto-task is for.
           @since PCC2 2.40.7 */
        { "ID",              game::interface::iteObjectId,       TaskEditorPropertyDomain, interpreter::thInt },

        { "INSERT",          game::interface::itmInsert,         TaskEditorMethodDomain,   interpreter::thProcedure },
        /* @q Lines:Str() (Auto Task Property)
           Commands in this auto-task.
           Elements in this array can be read and written.
           @assignable
           @since PCC2 2.40.7 */
        { "LINES",           game::interface::iteLines,          TaskEditorPropertyDomain, interpreter::thArray },

        /* @q Type:Str (Auto Task Property)
           Type of the object this auto-task is for.
           Possible values are "ship", "planet", "base".
           @since PCC2 2.40.7 */
        { "TYPE",            game::interface::iteTypeStr,        TaskEditorPropertyDomain, interpreter::thString },

        /* @q Type$:Int (Auto Task Property)
           Type of the object this auto-task is for, as integer.
           Possible values are 1=ship, 2=planet, 3=base.
           @since PCC2 2.40.7 */
        { "TYPE$",           game::interface::iteTypeInt,        TaskEditorPropertyDomain, interpreter::thInt },
    };

}


/*
 *  Class TaskEditorContext
 */

game::interface::TaskEditorContext::TaskEditorContext(afl::base::Ptr<interpreter::TaskEditor> edit, game::Session& session)
    : SingleContext(),
      m_edit(edit),
      m_session(session)
{ }

game::interface::TaskEditorContext::~TaskEditorContext()
{
    m_session.releaseAutoTaskEditor(m_edit);
}

// Context:
interpreter::Context::PropertyAccessor*
game::interface::TaskEditorContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    return lookupName(name, TASKEDITOR_MAP, result) ? this : 0;
}

void
game::interface::TaskEditorContext::set(PropertyIndex_t index, const afl::data::Value* value)
{
    switch (TaskEditorDomain(TASKEDITOR_MAP[index].domain)) {
     case TaskEditorPropertyDomain:
        return setTaskEditorProperty(m_edit, TaskEditorProperty(TASKEDITOR_MAP[index].index), value);
     case TaskEditorMethodDomain:
        throw interpreter::Error::notAssignable();
    }
}

afl::data::Value*
game::interface::TaskEditorContext::get(PropertyIndex_t index)
{
    switch (TaskEditorDomain(TASKEDITOR_MAP[index].domain)) {
     case TaskEditorPropertyDomain:
        return getTaskEditorProperty(m_edit, TaskEditorProperty(TASKEDITOR_MAP[index].index), m_session);
     case TaskEditorMethodDomain:
        return new TaskEditorClosure(m_edit, TaskEditorMethod(TASKEDITOR_MAP[index].index), m_session);
    }
    return 0;
}

game::interface::TaskEditorContext*
game::interface::TaskEditorContext::clone() const
{
    return new TaskEditorContext(m_edit, m_session);
}

game::map::Object*
game::interface::TaskEditorContext::getObject()
{
    return 0;
}

void
game::interface::TaskEditorContext::enumProperties(interpreter::PropertyAcceptor& acceptor)
{
    acceptor.enumTable(TASKEDITOR_MAP);
}

// BaseValue:
String_t
game::interface::TaskEditorContext::toString(bool /*readable*/) const
{
    return "#<task>";
}

void
game::interface::TaskEditorContext::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
{
    throw interpreter::Error::notSerializable();
}

game::interface::TaskEditorContext*
game::interface::TaskEditorContext::create(game::Session& session, interpreter::Process::ProcessKind k, Id_t id)
{
    afl::base::Ptr<TaskEditor> edit = session.getAutoTaskEditor(id, k, true);
    if (edit.get() != 0) {
        return new TaskEditorContext(edit, session);
    } else {
        return 0;
    }
}



/*
 *  Global Functions
 */

afl::data::Value*
game::interface::getTaskEditorProperty(const afl::base::Ptr<interpreter::TaskEditor>& edit, TaskEditorProperty prop, game::Session& session)
{
    switch (prop) {
     case iteLines:
        return new TaskEditorLinesProperty(edit, session);

     case iteCursor:
        return makeIntegerValue(limitRange(edit->getCursor()));

     case itePC:
        return makeIntegerValue(limitRange(edit->getPC()));

     case iteIsInSubroutine:
        return makeBooleanValue(edit->isInSubroutineCall());

     case iteTypeStr:
        switch (edit->process().getProcessKind()) {
         case Process::pkDefault:    break;
         case Process::pkShipTask:   return makeStringValue("ship");
         case Process::pkPlanetTask: return makeStringValue("planet");
         case Process::pkBaseTask:   return makeStringValue("base");
        }
        return 0;

     case iteTypeInt:
        switch (edit->process().getProcessKind()) {
         case Process::pkDefault:    break;
         case Process::pkShipTask:   return makeIntegerValue(1);
         case Process::pkPlanetTask: return makeIntegerValue(2);
         case Process::pkBaseTask:   return makeIntegerValue(3);
        }
        return 0;

     case iteObjectId:
        if (const game::map::Object* p = edit->process().getInvokingObject()) {
            return makeIntegerValue(p->getId());
        } else {
            return 0;
        }
    }
    return 0;
}

void
game::interface::setTaskEditorProperty(const afl::base::Ptr<interpreter::TaskEditor>& edit, TaskEditorProperty prop, const afl::data::Value* value)
{
    size_t n;
    switch (prop) {
     case iteCursor:
        if (checkIndexArg(n, value, edit->getNumInstructions() + 1)) {
            edit->setCursor(n);
        }
        break;

     case itePC:
        if (checkIndexArg(n, value, edit->getNumInstructions())) {
            edit->setPC(n);
        }
        break;

     case iteIsInSubroutine:
     case iteLines:
     case iteTypeStr:
     case iteTypeInt:
     case iteObjectId:
        throw Error::notAssignable();
    }
}

void
game::interface::callTaskEditorMethod(interpreter::TaskEditor& edit, TaskEditorMethod m, interpreter::Arguments& args)
{
    switch (m) {
     case itmAdd:
        IFTaskEditor_Add(edit, args);
        break;
     case itmInsert:
        IFTaskEditor_Insert(edit, args);
        break;
     case itmDelete:
        IFTaskEditor_Delete(edit, args);
        break;
    }
}
