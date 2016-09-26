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
game::interface::ObjectCommand::call(interpreter::Process& proc, afl::data::Segment& args, bool wantResult)
{
    // ex IntObjectProcedureValue::call
    interpreter::Arguments a(args, 0, args.size());
    m_function(m_session, m_object, proc, a);
    if (wantResult) {
        proc.pushNewValue(0);
    }
}

bool
game::interface::ObjectCommand::isProcedureCall()
{
    // ex IntObjectProcedureValue::isProcedureCall
    return true;
}

int32_t
game::interface::ObjectCommand::getDimension(int32_t /*which*/)
{
    // ex IntObjectProcedureValue::getDimension
    return 0;
}

interpreter::Context*
game::interface::ObjectCommand::makeFirstContext()
{
    // ex IntObjectProcedureValue::makeFirstContext
    return 0;
}
game::interface::ObjectCommand*
game::interface::ObjectCommand::clone() const
{
    // IntObjectProcedureValue::clone
    return new ObjectCommand(m_session, m_object, m_function);
}

// BaseValue:
String_t
game::interface::ObjectCommand::toString(bool /*readable*/) const
{
    // ex IntObjectProcedureValue::toString
    return "#<obj-procedure>";
}

void
game::interface::ObjectCommand::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext* /*ctx*/) const
{
    // ex IntObjectProcedureValue::store
    throw interpreter::Error::notSerializable();
}

// /** Implementation of the "Mark" command.
//     Syntax: 'Mark [flag]' */
void
game::interface::IFObjMark(game::Session& /*session*/, game::map::Object& obj, interpreter::Process& /*proc*/, interpreter::Arguments& args)
{
    // ex int/if/objif.h:IFObjMark
    /* @q Mark Optional flag:Bool (Planet Command, Ship Command, Ufo Command, Storm Command, Minefield Command)
       Mark object.
       Marks the current object.
       If the flag is specified as False, unmarks it instead.
       @since PCC2 1.99.9, PCC 1.0.5
       @diff This command is also available for ufos, ion storms, and minefields since PCC2 1.99.13.
       Older versions and PCC 1.x only allow it for ships and planets.
       @see Unmark */
    args.checkArgumentCount(0, 1);

    bool state = true;
    interpreter::checkBooleanArg(state, args.getNext());
    obj.setIsMarked(state);
}

// /** Implementation of the "Unmark" command.
//     Syntax: 'Unmark' */
void
game::interface::IFObjUnmark(game::Session& /*session*/, game::map::Object& obj, interpreter::Process& /*proc*/, interpreter::Arguments& args)
{
    // ex int/if/objif.h:IFObjUnmark
    /* @q Unmark (Planet Command, Ship Command, Ufo Command, Storm Command, Minefield Command)
       Unmark object.
       Unmarks the current object.
       @since PCC2 1.99.9, PCC 1.0.5
       @diff This command is also available for ufos, ion storms, and minefields since PCC2 1.99.13.
       Older versions and PCC 1.x only allow it for ships and planets.
       @see Mark */
    args.checkArgumentCount(0);
    obj.setIsMarked(false);
}
