/**
  *  \file game/interface/friendlycodefunction.cpp
  */

#include "game/interface/friendlycodefunction.hpp"
#include "interpreter/error.hpp"
#include "interpreter/values.hpp"
#include "interpreter/arguments.hpp"

game::interface::FriendlyCodeFunction::FriendlyCodeFunction(Session& session)
    : IndexableValue(),
      m_session(session)
{ }

// IndexableValue:
game::interface::FriendlyCodeContext*
game::interface::FriendlyCodeFunction::get(interpreter::Arguments& args)
{
    /* @q FCode(sid:Int):Obj (Function, Context)
       Access friendly code properties.
       Use as
       | ForEach FCode Do ...
       or
       | With FCode(fc) Do ...

       @see int:index:group:friendlycodeproperty|Friendly Code Properties
       @since PCC2 2.40.1 */
    args.checkArgumentCount(1);

    String_t fc;
    if (!interpreter::checkStringArg(fc, args.getNext())) {
        return 0;
    }

    Root* root = m_session.getRoot().get();
    game::spec::ShipList* shipList = m_session.getShipList().get();
    if (root == 0 || shipList == 0) {
        return 0;
    }

    size_t slot;
    if (!shipList->friendlyCodes().getIndexByName(fc, slot)) {
        return 0;
    }

    return new FriendlyCodeContext(slot, *root, *shipList);
}

void
game::interface::FriendlyCodeFunction::set(interpreter::Arguments& /*args*/, afl::data::Value* /*value*/)
{
    throw interpreter::Error::notAssignable();
}

// CallableValue:
int32_t
game::interface::FriendlyCodeFunction::getDimension(int32_t /*which*/) const
{
    return 0;
}

game::interface::FriendlyCodeContext*
game::interface::FriendlyCodeFunction::makeFirstContext()
{
    Root* root = m_session.getRoot().get();
    game::spec::ShipList* shipList = m_session.getShipList().get();
    if (root != 0 && shipList != 0 && shipList->friendlyCodes().at(0) != 0) {
        return new FriendlyCodeContext(0, *root, *shipList);
    } else {
        return 0;
    }
}

game::interface::FriendlyCodeFunction*
game::interface::FriendlyCodeFunction::clone() const
{
    return new FriendlyCodeFunction(m_session);
}

// BaseValue:
String_t
game::interface::FriendlyCodeFunction::toString(bool /*readable*/) const
{
    return "#<array:FCode>";
}

void
game::interface::FriendlyCodeFunction::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
{
    throw interpreter::Error::notSerializable();
}
