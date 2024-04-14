/**
  *  \file game/interface/taskeditorcontext.cpp
  *  \brief Class game::interface::TaskEditorContext
  */

#include "game/interface/taskeditorcontext.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "afl/string/format.hpp"
#include "game/actions/preconditions.hpp"
#include "game/game.hpp"
#include "game/interface/basetaskpredictor.hpp"
#include "game/interface/shiptaskpredictor.hpp"
#include "game/limits.hpp"
#include "game/map/fleet.hpp"
#include "game/map/object.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/root.hpp"
#include "game/spec/basichullfunction.hpp"
#include "game/spec/shiplist.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/procedurevalue.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/values.hpp"

using afl::string::Format;
using game::Game;
using game::Root;
using game::interface::BaseTaskPredictor;
using game::interface::ShipTaskPredictor;
using game::map::Universe;
using game::spec::BasicHullFunction;
using game::spec::ShipList;
using interpreter::Error;
using interpreter::Process;
using interpreter::TaskEditor;
using interpreter::checkFlagArg;
using interpreter::checkIntegerArg;
using interpreter::checkStringArg;
using interpreter::makeBooleanValue;
using interpreter::makeIntegerValue;
using interpreter::makeOptionalIntegerValue;
using interpreter::makeOptionalStringValue;
using interpreter::makeStringValue;

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
                if (!interpreter::checkIndexArg(index, args.getNext(), 0, m_edit->getNumInstructions())) {
                    return 0;
                }
                return makeStringValue((*m_edit)[index]);
            }
        virtual void set(interpreter::Arguments& args, const afl::data::Value* value)
            {
                size_t index;
                args.checkArgumentCount(1);
                if (!interpreter::checkIndexArg(index, args.getNext(), 0, m_edit->getNumInstructions())) {
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
            { return rejectFirstContext(); }

        // BaseValue:
        virtual String_t toString(bool /*readable*/) const
            { return "#<array>"; }
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
            { rejectStore(out, aux, ctx); }

        // Value:
        virtual TaskEditorLinesProperty* clone() const
            { return new TaskEditorLinesProperty(m_edit, m_session); }

     private:
        afl::base::Ptr<TaskEditor> m_edit;  // must be Ptr<> to allow releaseAutoTaskEditor()
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
            { game::interface::callTaskEditorMethod(*m_edit, m_method, m_session, args); }
        virtual ProcedureValue* clone() const
            { return new TaskEditorClosure(m_edit, m_method, m_session); }
     private:
        afl::base::Ptr<TaskEditor> m_edit;  // must be Ptr<> to allow releaseAutoTaskEditor()
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

    /* @q AddMovement verb:Str, x:Int, y:Int, Optional flags:Str (Auto Task Command)
       Add a movement command to the current auto-task at cursor position.
       The auto-task needs to be a ship task.

       Flags can be:
       - "s": set speed command if required
       - "d": accept duplicates

       @since PCC2 2.40.12 */
    void IFTaskEditor_AddMovement(TaskEditor& edit, interpreter::Arguments& args, game::Session& session)
    {
        // ex WShipAutoTaskSelection::insertWaypointCommand
        args.checkArgumentCount(3, 4);

        // Mandatory args
        String_t verb;
        int x, y;
        if (!checkStringArg(verb, args.getNext())
            || !checkIntegerArg(x, args.getNext(), 0, game::MAX_NUMBER)
            || !checkIntegerArg(y, args.getNext(), 0, game::MAX_NUMBER))
        {
            return;
        }

        // Optional args
        int flags = 0;
        checkFlagArg(flags, 0, args.getNext(), "SD");
        static_assert(game::interface::imc_SetSpeed == 1, "SetSpeed");
        static_assert(game::interface::imc_AcceptDuplicate == 2, "AcceptDuplicate");
        game::interface::insertMovementCommand(edit, verb, game::map::Point(x, y), flags, session);
    }

    /* @q ConfirmMessage (Auto Task Command)
       Confirm the task's notification message.
       This will cause the task to continue executing when the user stops editing it.

       @since PCC2 2.40.12 */
    void IFTaskEditor_ConfirmMessage(TaskEditor& edit, interpreter::Arguments& args, game::Session& session)
    {
        // ex WTaskMessageTile::onConfirm
        args.checkArgumentCount(0);

        game::interface::NotificationStore& notif = session.notifications();
        notif.confirmMessage(notif.findMessageByProcessId(edit.process().getProcessId()), true);
        if (game::map::Object* obj = dynamic_cast<game::map::Object*>(edit.process().getInvokingObject())) {
            obj->markDirty();
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
            interpreter::checkIndexArg(index, pos, 0, edit.getNumInstructions()+1);
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
        if (!interpreter::checkIndexArg(index, args.getNext(), 0, edit.getNumInstructions()+1)) {
            return;
        }

        // Count: unrestricted, will be limited, defaults to 1
        size_t count = 1;
        interpreter::checkIndexArg(count, args.getNext(), 0, size_t(-1));
        count = std::min(count, edit.getNumInstructions() - index);

        // Do it
        edit.replace(index, count, afl::base::Nothing, TaskEditor::DefaultCursor, TaskEditor::DefaultPC);
    }

    /*
     *  Implementation of Predicted.XXX properties
     */

    enum PredictedValue {
        pvFriendlyCode,
        pvMission,
        pvMovementFuel,
        pvCloakFuel,
        pvRemainingFuel,
        pvWarpFactor,
        pvPositionX,
        pvPositionY
    };

    afl::data::Value* getPredictedValue(TaskEditor& edit, game::Session& session, PredictedValue which)
    {
        const Game* g = session.getGame().get();
        const Root* r = session.getRoot().get();
        const ShipList* sl = session.getShipList().get();
        if (g == 0 || r == 0 || sl == 0) {
            // Missing environment: cannot predict
            return 0;
        } else if (const game::map::Ship* sh = dynamic_cast<const game::map::Ship*>(edit.process().getInvokingObject())) {
            // Ship prediction
            ShipTaskPredictor pred(g->currentTurn().universe(), sh->getId(), g->shipScores(), *sl, g->mapConfiguration(), r->hostConfiguration(), r->hostVersion(), r->registrationKey());
            pred.predictTask(edit, edit.getCursor());
            switch (which) {
             case pvFriendlyCode:   return makeStringValue(pred.getFriendlyCode());
             case pvMission:        return makeIntegerValue(pred.getMission());
             case pvMovementFuel:   return makeIntegerValue(pred.getMovementFuel());
             case pvCloakFuel:      return makeIntegerValue(pred.getCloakFuel());
             case pvRemainingFuel:  return makeIntegerValue(pred.getRemainingFuel());
             case pvWarpFactor:     return makeIntegerValue(pred.getWarpFactor());
             case pvPositionX:      return makeIntegerValue(pred.getPosition().getX());
             case pvPositionY:      return makeIntegerValue(pred.getPosition().getY());
            }
            return 0;
        } else if (const game::map::Planet* pl = dynamic_cast<const game::map::Planet*>(edit.process().getInvokingObject())) {
            // Planet prediction
            BaseTaskPredictor pred(*pl, g->currentTurn().universe(), *sl, r->hostConfiguration());
            pred.predictTask(edit, edit.getCursor());
            switch (which) {
             case pvFriendlyCode:   return makeOptionalStringValue(pred.planet().getFriendlyCode());
             case pvMission:        return makeOptionalIntegerValue(pred.planet().getBaseMission());
             case pvMovementFuel:   return 0;
             case pvCloakFuel:      return 0;
             case pvRemainingFuel:  return 0;
             case pvWarpFactor:     return 0;
             case pvPositionX:      return 0;
             case pvPositionY:      return 0;
            }
            return 0;
        } else {
            // Wrong type
            return 0;
        }
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
        { "ADDMOVEMENT",     game::interface::itmAddMovement,    TaskEditorMethodDomain,   interpreter::thProcedure },
        { "CONFIRMMESSAGE",  game::interface::itmConfirmMessage, TaskEditorMethodDomain,   interpreter::thProcedure },

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

        /* @q Predicted.FCode:Str (Auto Task Property)
           Predicted friendly code at current position.
           Considers all previous "SetFCode" commands.
           @since PCC2 2.40.12 */
        { "PREDICTED.FCODE", game::interface::itePredictedFCode, TaskEditorPropertyDomain, interpreter::thString },

        /* @q Predicted.Fuel:Int (Auto Task Property)
           Predicted remaining fuel on ship at current position in auto task.
           Considers all previous commands.
           EMPTY if the current task is not a ship task.
           @since PCC2 2.40.12 */
        { "PREDICTED.FUEL",  game::interface::itePredictedCloakFuel, TaskEditorPropertyDomain, interpreter::thInt },

        /* @q Predicted.Fuel.Cloak:Int (Auto Task Property)
           Fuel used for cloaking by ship at current position in auto task.
           Considers all previous commands.
           EMPTY if the current task is not a ship task.
           @since PCC2 2.40.12 */
        { "PREDICTED.FUEL.CLOAK", game::interface::itePredictedCloakFuel, TaskEditorPropertyDomain, interpreter::thInt },

        /* @q Predicted.Fuel.Move:Int (Auto Task Property)
           Fuel used for movement by ship at current position in auto task.
           Considers all previous commands.
           EMPTY if the current task is not a ship task.
           @since PCC2 2.40.12 */
        { "PREDICTED.FUEL.MOVE", game::interface::itePredictedMovementFuel, TaskEditorPropertyDomain, interpreter::thInt },

        /* @q Predicted.Loc.X:Int (Auto Task Property)
           Predicted X location of ship at current position in auto task.
           Considers all previous commands.
           EMPTY if the current task is not a ship task.
           @since PCC2 2.40.12 */
        { "PREDICTED.LOC.X",  game::interface::itePredictedPositionX, TaskEditorPropertyDomain, interpreter::thInt },

        /* @q Predicted.Loc.Y:Int (Auto Task Property)
           Predicted Y location of ship at current position in auto task.
           Considers all previous commands.
           EMPTY if the current task is not a ship task.
           @since PCC2 2.40.12 */
        { "PREDICTED.LOC.Y",  game::interface::itePredictedPositionY, TaskEditorPropertyDomain, interpreter::thInt },

        /* @q Predicted.Mission$:Int (Auto Task Property)
           Predicted mission number of ship or starbase at current position in auto task.
           Considers all previous commands.
           @since PCC2 2.40.12 */
        { "PREDICTED.MISSION$", game::interface::itePredictedMission, TaskEditorPropertyDomain, interpreter::thInt },

        /* @q Predicted.Speed$:Int (Auto Task Property)
           Predicted speed of ship at current position in auto task.
           Considers all previous commands.
           EMPTY if the current task is not a ship task.
           @since PCC2 2.40.12 */
        { "PREDICTED.SPEED$", game::interface::itePredictedSpeed, TaskEditorPropertyDomain, interpreter::thInt },

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
{
    afl::except::checkAssertion(edit.get() != 0, "edit != 0");
}

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
        return setTaskEditorProperty(*m_edit, TaskEditorProperty(TASKEDITOR_MAP[index].index), value);
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

afl::base::Deletable*
game::interface::TaskEditorContext::getObject()
{
    return 0;
}

void
game::interface::TaskEditorContext::enumProperties(interpreter::PropertyAcceptor& acceptor) const
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
game::interface::TaskEditorContext::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
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
    if (edit.get() != 0) {
        switch (prop) {
         case iteLines:
            return new TaskEditorLinesProperty(edit, session);

         case iteCursor:
            return makeIntegerValue(limitRange(edit->getCursor()));

         case itePC:
            return makeIntegerValue(limitRange(edit->getPC()));

         case iteIsInSubroutine:
            return makeBooleanValue(edit->isInSubroutineCall());

         case itePredictedFCode:
            return getPredictedValue(*edit, session, pvFriendlyCode);

         case itePredictedCloakFuel:
            return getPredictedValue(*edit, session, pvCloakFuel);

         case itePredictedFuel:
            return getPredictedValue(*edit, session, pvRemainingFuel);

         case itePredictedMission:
            return getPredictedValue(*edit, session, pvMission);

         case itePredictedMovementFuel:
            return getPredictedValue(*edit, session, pvMovementFuel);

         case itePredictedPositionX:
            return getPredictedValue(*edit, session, pvPositionX);

         case itePredictedPositionY:
            return getPredictedValue(*edit, session, pvPositionY);

         case itePredictedSpeed:
            return getPredictedValue(*edit, session, pvWarpFactor);

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
            if (const game::map::Object* p = dynamic_cast<const game::map::Object*>(edit->process().getInvokingObject())) {
                return makeIntegerValue(p->getId());
            } else {
                return 0;
            }
        }
    }
    return 0;
}

void
game::interface::setTaskEditorProperty(interpreter::TaskEditor& edit, TaskEditorProperty prop, const afl::data::Value* value)
{
    size_t n;
    switch (prop) {
     case iteCursor:
        if (interpreter::checkIndexArg(n, value, 0, edit.getNumInstructions() + 1)) {
            edit.setCursor(n);
        }
        break;

     case itePC:
        if (interpreter::checkIndexArg(n, value, 0, edit.getNumInstructions())) {
            edit.setPC(n);
        }
        break;

     case iteIsInSubroutine:
     case iteLines:
     case itePredictedCloakFuel:
     case itePredictedFCode:
     case itePredictedFuel:
     case itePredictedMission:
     case itePredictedMovementFuel:
     case itePredictedPositionX:
     case itePredictedPositionY:
     case itePredictedSpeed:
     case iteTypeStr:
     case iteTypeInt:
     case iteObjectId:
        throw Error::notAssignable();
    }
}

void
game::interface::callTaskEditorMethod(interpreter::TaskEditor& edit, TaskEditorMethod m, Session& session, interpreter::Arguments& args)
{
    switch (m) {
     case itmAdd:
        IFTaskEditor_Add(edit, args);
        break;
     case itmAddMovement:
        IFTaskEditor_AddMovement(edit, args, session);
        break;
     case itmConfirmMessage:
        IFTaskEditor_ConfirmMessage(edit, args, session);
        break;
     case itmInsert:
        IFTaskEditor_Insert(edit, args);
        break;
     case itmDelete:
        IFTaskEditor_Delete(edit, args);
        break;
    }
}

void
game::interface::insertMovementCommand(interpreter::TaskEditor& edit, String_t verb, game::map::Point pt, int flags, Session& session)
{
    // ex WShipAutoTaskSelection::insertWaypointCommand
    const bool wantSetSpeed = (flags & imc_SetSpeed) != 0;
    const bool wantDuplicate = (flags & imc_AcceptDuplicate) != 0;

    // We need a ship to work
    game::map::Ship* sh = dynamic_cast<game::map::Ship*>(edit.process().getInvokingObject());
    if (sh == 0) {
        throw Error("Not a ship auto-task");
    }

    // Ship prediction to find current state
    const Root& r = game::actions::mustHaveRoot(session);
    Game& g = game::actions::mustHaveGame(session);
    Universe& u = g.currentTurn().universe();
    const ShipList& shipList = game::actions::mustHaveShipList(session);
    ShipTaskPredictor pred(u, sh->getId(), g.shipScores(), shipList, g.mapConfiguration(), r.hostConfiguration(), r.hostVersion(), r.registrationKey());
    pred.predictTask(edit, edit.getCursor());
    if (!wantDuplicate && pred.getPosition() == pt) {
        return;
    }

    // Collect commands so we add them all at once
    afl::data::StringList_t commands;

    // Set speed if desired
    if (wantSetSpeed) {
        // Do it
        const int32_t dist2 = g.mapConfiguration().getSquaredDistance(pred.getPosition(), pt);
        const bool shipCanJump = sh->hasSpecialFunction(BasicHullFunction::Hyperdrive, g.shipScores(), shipList, r.hostConfiguration());
        if (shipCanJump && r.hostVersion().isExactHyperjumpDistance2(dist2)) {
            /* Looks like a hyperjump, so make one. This code is not in the regular
               auto-warp function, but is's very convenient for planning double-jumps. */
            int speed = std::max(2, game::map::Fleet(u, *sh).getMaxEfficientWarp(shipList));
            if (pred.getWarpFactor() < speed) {
                commands.push_back(Format("SetSpeed %d", speed));
            }
            commands.push_back(Format("SetFCode \"HYP\"   %% %s", session.translator()("hyperjump")));
        } else {
            /* Not a hyperjump */
            if (shipCanJump && pred.getFriendlyCode() == "HYP" && !r.hostVersion().isExactHyperjumpDistance2(dist2)) {
                commands.push_back(Format("SetFCode %s   %% %s",
                                          interpreter::quoteString(shipList.friendlyCodes().generateRandomCode(session.rng(), r.hostVersion())),
                                          session.translator()("cancel hyperjump")));
            }

            /* Optimize speed */
            if (pred.getPosition() != pt) {
                int n = getOptimumWarp(u, sh->getId(), pred.getPosition(), pt, g.shipScores(), shipList, g.mapConfiguration(), r);
                if (n != 0 && n != pred.getWarpFactor()) {
                    commands.push_back(Format("SetSpeed %d", n));
                }
            }
        }
    }

    // Finally, add the waypoint command
    String_t command = Format("%s %d, %d", verb, pt.getX(), pt.getY());
    validateCommand(command);

    String_t comment = u.findLocationName(pt, Universe::NameGravity | Universe::NameNoSpace, g.mapConfiguration(), r.hostConfiguration(), r.hostVersion(), session.translator());
    if (!comment.empty()) {
        command += "   % ";
        command += comment;
    }
    commands.push_back(command);

    edit.replace(edit.getCursor(), 0, commands, TaskEditor::PlaceCursorAfter, TaskEditor::DefaultPC);
}
