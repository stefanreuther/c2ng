/**
  *  \file game/interface/enginecontext.cpp
  */

#include "game/interface/enginecontext.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/typehint.hpp"
#include "game/interface/componentproperty.hpp"
#include "game/interface/engineproperty.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "afl/string/format.hpp"
#include "interpreter/error.hpp"

namespace {
    enum EngineDomain { ComponentPropertyDomain, EnginePropertyDomain };

    // Mapping for engines:
    const interpreter::NameTable ENGINE_MAP[] = {
        { "COST.D",        game::interface::icpCostD,         ComponentPropertyDomain, interpreter::thInt },
        { "COST.M",        game::interface::icpCostM,         ComponentPropertyDomain, interpreter::thInt },
        { "COST.MC",       game::interface::icpCostMC,        ComponentPropertyDomain, interpreter::thInt },
        { "COST.STR",      game::interface::icpCostStr,       ComponentPropertyDomain, interpreter::thString },
        { "COST.T",        game::interface::icpCostT,         ComponentPropertyDomain, interpreter::thInt },
        { "FUELFACTOR",    game::interface::iepFuelFactor,    EnginePropertyDomain,    interpreter::thArray },
        { "ID",            game::interface::icpId,            ComponentPropertyDomain, interpreter::thInt },
        { "NAME",          game::interface::icpName,          ComponentPropertyDomain, interpreter::thString },
        { "NAME.SHORT",    game::interface::icpNameShort,     ComponentPropertyDomain, interpreter::thString },
        { "SPEED$",        game::interface::iepEfficientWarp, EnginePropertyDomain,    interpreter::thInt },
        { "TECH",          game::interface::icpTech,          ComponentPropertyDomain, interpreter::thInt },
        { "TECH.ENGINE",   game::interface::icpTech,          ComponentPropertyDomain, interpreter::thInt },
    };
}

game::interface::EngineContext::EngineContext(int nr, afl::base::Ref<game::spec::ShipList> shipList)
    : m_number(nr),
      m_shipList(shipList)
{
    // ex IntEngineContext::IntEngineContext
}

game::interface::EngineContext::~EngineContext()
{ }

// Context:
interpreter::Context::PropertyAccessor*
game::interface::EngineContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    // ex IntEngineContext::lookup
    return lookupName(name, ENGINE_MAP, result) ? this : 0;
}

void
game::interface::EngineContext::set(PropertyIndex_t index, const afl::data::Value* value)
{
    if (game::spec::Engine* e = m_shipList->engines().get(m_number)) {
        switch (EngineDomain(ENGINE_MAP[index].domain)) {
         case ComponentPropertyDomain:
            setComponentProperty(*e, ComponentProperty(ENGINE_MAP[index].index), value, *m_shipList);
            break;
         case EnginePropertyDomain:
            setEngineProperty(*e, EngineProperty(ENGINE_MAP[index].index), value, *m_shipList);
            break;
        }
    } else {
        throw interpreter::Error::notAssignable();
    }
}

afl::data::Value*
game::interface::EngineContext::get(PropertyIndex_t index)
{
    // ex IntEngineContext::get
    if (const game::spec::Engine* e = m_shipList->engines().get(m_number)) {
        switch (EngineDomain(ENGINE_MAP[index].domain)) {
         case ComponentPropertyDomain:
            return getComponentProperty(*e, ComponentProperty(ENGINE_MAP[index].index), *m_shipList);
         case EnginePropertyDomain:
            return getEngineProperty(*e, EngineProperty(ENGINE_MAP[index].index));
        }
    }
    return 0;
}

bool
game::interface::EngineContext::next()
{
    // ex IntEngineContext::next
    if (game::spec::Engine* e = m_shipList->engines().findNext(m_number)) {
        m_number = e->getId();
        return true;
    } else {
        return false;
    }
}

game::interface::EngineContext*
game::interface::EngineContext::clone() const
{
    // ex IntEngineContext::clone
    return new EngineContext(m_number, m_shipList);
}

game::map::Object*
game::interface::EngineContext::getObject()
{
    // ex IntEngineContext::getObject
    return 0;
}

void
game::interface::EngineContext::enumProperties(interpreter::PropertyAcceptor& acceptor) const
{
    // ex IntEngineContext::enumProperties
    acceptor.enumTable(ENGINE_MAP);
}

// BaseValue:
String_t
game::interface::EngineContext::toString(bool /*readable*/) const
{
    // ex IntEngineContext::toString
    return afl::string::Format("Engine(%d)", m_number);
}

void
game::interface::EngineContext::store(interpreter::TagNode& out, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
{
    // ex IntEngineContext::store
    out.tag = out.Tag_Engine;
    out.value = m_number;
}

game::interface::EngineContext*
game::interface::EngineContext::create(int nr, Session& session)
{
    game::spec::ShipList* list = session.getShipList().get();
    if (list != 0 && list->engines().get(nr) != 0) {
        return new EngineContext(nr, *list);
    } else {
        return 0;
    }
}
