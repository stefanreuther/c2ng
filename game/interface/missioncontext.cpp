/**
  *  \file game/interface/missioncontext.cpp
  */

#include "game/interface/missioncontext.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/typehint.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "game/interface/missionproperty.hpp"

namespace {
    enum MissionDomain {
        MissionPropertyDomain
    };

    const interpreter::NameTable MISSION_MAPPING[] = {
        { "COMMAND",         game::interface::impSetCommand,          MissionPropertyDomain, interpreter::thString },
        { "CONDITION",       game::interface::impConditionExpression, MissionPropertyDomain, interpreter::thString },
        { "FLAGS",           game::interface::impFlags,               MissionPropertyDomain, interpreter::thString },
        { "INTERCEPT.FLAGS", game::interface::impInterceptFlags,      MissionPropertyDomain, interpreter::thString },
        { "INTERCEPT.NAME",  game::interface::impInterceptName,       MissionPropertyDomain, interpreter::thString },
        { "INTERCEPT.TYPE",  game::interface::impInterceptType,       MissionPropertyDomain, interpreter::thString },
        { "KEY",             game::interface::impHotkey,              MissionPropertyDomain, interpreter::thString },
        { "LABEL",           game::interface::impLabelExpression,     MissionPropertyDomain, interpreter::thString },
        { "NAME",            game::interface::impName,                MissionPropertyDomain, interpreter::thString },
        { "NAME.SHORT",      game::interface::impShortName,           MissionPropertyDomain, interpreter::thString },
        { "NUMBER",          game::interface::impNumber,              MissionPropertyDomain, interpreter::thInt },
        { "RACE$",           game::interface::impRaces,               MissionPropertyDomain, interpreter::thInt },
        { "TOW.FLAGS",       game::interface::impTowFlags,            MissionPropertyDomain, interpreter::thString },
        { "TOW.NAME",        game::interface::impTowName,             MissionPropertyDomain, interpreter::thString },
        { "TOW.TYPE",        game::interface::impTowType,             MissionPropertyDomain, interpreter::thString },
        { "WARNING",         game::interface::impWarningExpression,   MissionPropertyDomain, interpreter::thString },
    };
}

game::interface::MissionContext::MissionContext(size_t slot,
                                                afl::base::Ref<game::spec::ShipList> shipList)
    : m_slot(slot),
      m_shipList(shipList)
{ }

game::interface::MissionContext::~MissionContext()
{ }

// Context:
interpreter::Context::PropertyAccessor*
game::interface::MissionContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    return lookupName(name, MISSION_MAPPING, result) ? this : 0;
}

afl::data::Value*
game::interface::MissionContext::get(PropertyIndex_t index)
{
    const game::spec::Mission* msn = m_shipList->missions().at(m_slot);
    if (msn != 0) {
        return getMissionProperty(*msn, MissionProperty(MISSION_MAPPING[index].index));
    } else {
        return 0;
    }
}

bool
game::interface::MissionContext::next()
{
    size_t n = m_slot+1;
    if (m_shipList->missions().at(n) != 0) {
        m_slot = n;
        return true;
    } else {
        return false;
    }
}

game::interface::MissionContext*
game::interface::MissionContext::clone() const
{
    return new MissionContext(m_slot, m_shipList);
}

game::map::Object*
game::interface::MissionContext::getObject()
{
    return 0;
}

void
game::interface::MissionContext::enumProperties(interpreter::PropertyAcceptor& acceptor)
{
    acceptor.enumTable(MISSION_MAPPING);
}

// BaseValue:
String_t
game::interface::MissionContext::toString(bool /*readable*/) const
{
    return "#<mission>";
}

void
game::interface::MissionContext::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}
