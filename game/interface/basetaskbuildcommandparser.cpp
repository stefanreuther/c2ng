/**
  *  \file game/interface/basetaskbuildcommandparser.cpp
  */

#include "game/interface/basetaskbuildcommandparser.hpp"
#include "game/interface/planetmethod.hpp"

game::interface::BaseTaskBuildCommandParser::BaseTaskBuildCommandParser(const game::spec::ShipList& shipList)
    : m_shipList(shipList),
      m_verb(),
      m_order()
{ }

bool
game::interface::BaseTaskBuildCommandParser::predictInstruction(const String_t& name, interpreter::Arguments& args)
{
    // ex WBaseTaskBuildCommandParser::predictInstruction
    if (name == "BUILDSHIP" || name == "ENQUEUESHIP") {
        // parseBuildShipCommand may produce a partial, invalid order when it returns false!
        ShipBuildOrder newOrder;
        if (parseBuildShipCommand(args, newOrder, m_shipList)) {
            m_order = newOrder;
            m_verb = name;
        }
    }
    return true;
}

const String_t&
game::interface::BaseTaskBuildCommandParser::getVerb() const
{
    // ex WBaseTaskBuildCommandParser::getVerb
    return m_verb;
}

const game::ShipBuildOrder&
game::interface::BaseTaskBuildCommandParser::getOrder() const
{
    // ex WBaseTaskBuildCommandParser::getOrder
    return m_order;
}
