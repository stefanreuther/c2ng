/**
 *  \file game/interface/objectcommand.cpp
 */

#include "game/interface/objectcommand.hpp"
#include "interpreter/arguments.hpp"

game::interface::ObjectCommand::ObjectCommand(game::Session& session, game::map::Object& obj, Function_t func)
    : m_session(session),
      m_object(obj),
      m_game(session.getGame()), // see "Lifetime" in class description
      m_function(func)
{
    // ex IntObjectProcedureValue::IntObjectProcedureValue
}

game::interface::ObjectCommand::~ObjectCommand()
{ }

// CallableValue:
void
game::interface::ObjectCommand::call(interpreter::Process& proc, interpreter::Arguments& args)
{
    // ex IntObjectProcedureValue::call
    m_function(m_session, m_object, proc, args);
}

game::interface::ObjectCommand*
game::interface::ObjectCommand::clone() const
{
    // IntObjectProcedureValue::clone
    return new ObjectCommand(m_session, m_object, m_function);
}

void
game::interface::IFObjMark(game::Session& /*session*/, game::map::Object& obj, interpreter::Process& /*proc*/, interpreter::Arguments& args)
{
    IFObjMark(obj, args);
}

void
game::interface::IFObjMark(game::map::Object& obj, interpreter::Arguments& args)
{
    // ex int/if/objif.h:IFObjMark
    // ex shipint.pas:Ship_Mark
    // ex planint.pas:Planet_Mark
    /* @q Mark Optional flag:Bool (Planet Command, Ship Command, Ufo Command, Storm Command, Minefield Command)
       Mark object.
       Marks the current object.
       If the flag is specified as False, unmarks it instead.
       @since PCC2 1.99.9, PCC 1.0.5, PCC2 2.40.1
       @diff This command is also available for ufos, ion storms, and minefields since PCC2 1.99.13.
       Older versions and PCC 1.x only allow it for ships and planets.
       @see Unmark */
    args.checkArgumentCount(0, 1);

    bool state = true;
    interpreter::checkBooleanArg(state, args.getNext());
    obj.setIsMarked(state);
}

void
game::interface::IFObjUnmark(game::Session& /*session*/, game::map::Object& obj, interpreter::Process& /*proc*/, interpreter::Arguments& args)
{
    // ex shipint.pas;Ship_Unmark
    // ex planint.pas:Planet_Unmark
    IFObjUnmark(obj, args);
}

void
game::interface::IFObjUnmark(game::map::Object& obj, interpreter::Arguments& args)
{
    // ex int/if/objif.h:IFObjUnmark
    /* @q Unmark (Planet Command, Ship Command, Ufo Command, Storm Command, Minefield Command)
       Unmark object.
       Unmarks the current object.
       @since PCC2 1.99.9, PCC 1.0.5, PCC2 2.40.1
       @diff This command is also available for ufos, ion storms, and minefields since PCC2 1.99.13.
       Older versions and PCC 1.x only allow it for ships and planets.
       @see Mark */
    args.checkArgumentCount(0);
    obj.setIsMarked(false);
}
