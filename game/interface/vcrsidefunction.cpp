/**
  *  \file game/interface/vcrsidefunction.cpp
  *  \brief Class game::interface::VcrSideFunction
  */

#include "game/interface/vcrsidefunction.hpp"
#include "interpreter/arguments.hpp"

game::interface::VcrSideFunction::VcrSideFunction(size_t battleNumber,
                                                  afl::string::Translator& tx,
                                                  const afl::base::Ref<const Root>& root,
                                                  const afl::base::Ptr<game::vcr::Database>& battles,
                                                  const afl::base::Ref<const game::spec::ShipList>& shipList)
    : m_battleNumber(battleNumber),
      m_translator(tx),
      m_root(root),
      m_battles(battles),
      m_shipList(shipList)
{ }

// IndexableValue:
game::interface::VcrSideContext*
game::interface::VcrSideFunction::get(interpreter::Arguments& args)
{
    // Parse argument
    size_t i;
    args.checkArgumentCount(1);
    if (!interpreter::checkIndexArg(i, args.getNext(), 1, getNumObjects())) {
        return 0;
    }

    // OK, build result. Note that the user indexes are 1-based!
    return new VcrSideContext(m_battleNumber, i, m_translator, m_root, m_battles, m_shipList);
}

void
game::interface::VcrSideFunction::set(interpreter::Arguments& args, const afl::data::Value* value)
{
    // ex IntVcrSideArray::set
    rejectSet(args, value);
}

// CallableValue:
size_t
game::interface::VcrSideFunction::getDimension(size_t which) const
{
    if (which == 0) {
        return 1;
    } else {
        return getNumObjects() + 1;
    }
}

game::interface::VcrSideContext*
game::interface::VcrSideFunction::makeFirstContext()
{
    if (getNumObjects() > 0) {
        return new VcrSideContext(m_battleNumber, 0, m_translator, m_root, m_battles, m_shipList);
    } else {
        return 0;
    }
}

game::interface::VcrSideFunction*
game::interface::VcrSideFunction::clone() const
{
    // ex IntVcrSideArray::clone
    return new VcrSideFunction(m_battleNumber, m_translator, m_root, m_battles, m_shipList);
}

// BaseValue:
String_t
game::interface::VcrSideFunction::toString(bool /*readable*/) const
{
    // ex IntVcrSideArray::toString
    return "#<array>";
}

void
game::interface::VcrSideFunction::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    // ex IntVcrSideArray::store
    rejectStore(out, aux, ctx);
}

size_t
game::interface::VcrSideFunction::getNumObjects() const
{
    if (game::vcr::Database* db = m_battles.get()) {
        if (game::vcr::Battle* battle = db->getBattle(m_battleNumber)) {
            return battle->getNumObjects();
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}
