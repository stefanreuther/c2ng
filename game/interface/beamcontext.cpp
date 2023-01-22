/**
  *  \file game/interface/beamcontext.cpp
  */

#include "game/interface/beamcontext.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/typehint.hpp"
#include "afl/string/format.hpp"
#include "game/spec/beam.hpp"
#include "game/interface/componentproperty.hpp"
#include "game/interface/weaponproperty.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/error.hpp"

namespace {
    enum BeamDomain { ComponentPropertyDomain, WeaponPropertyDomain };

    // Mapping for beams:
    static const interpreter::NameTable BEAM_MAP[] = {
        { "COST.D",        game::interface::icpCostD,     ComponentPropertyDomain, interpreter::thInt },
        { "COST.M",        game::interface::icpCostM,     ComponentPropertyDomain, interpreter::thInt },
        { "COST.MC",       game::interface::icpCostMC,    ComponentPropertyDomain, interpreter::thInt },
        { "COST.STR",      game::interface::icpCostStr,   ComponentPropertyDomain, interpreter::thString },
        { "COST.T",        game::interface::icpCostT,     ComponentPropertyDomain, interpreter::thInt },
        { "DAMAGE",        game::interface::iwpDamage,    WeaponPropertyDomain,    interpreter::thInt },
        { "ID",            game::interface::icpId,        ComponentPropertyDomain, interpreter::thInt },
        { "KILL",          game::interface::iwpKill,      WeaponPropertyDomain,    interpreter::thInt },
        { "MASS",          game::interface::icpMass,      ComponentPropertyDomain, interpreter::thInt },
        { "NAME",          game::interface::icpName,      ComponentPropertyDomain, interpreter::thString },
        { "NAME.SHORT",    game::interface::icpNameShort, ComponentPropertyDomain, interpreter::thString },
        { "TECH",          game::interface::icpTech,      ComponentPropertyDomain, interpreter::thInt },
        { "TECH.BEAM",     game::interface::icpTech,      ComponentPropertyDomain, interpreter::thInt },
    };

}

game::interface::BeamContext::BeamContext(int nr, afl::base::Ref<game::spec::ShipList> shipList, afl::base::Ref<game::Root> root)
    : m_number(nr),
      m_shipList(shipList),
      m_root(root)
{
    // ex IntBeamContext::IntBeamContext
}

game::interface::BeamContext::~BeamContext()
{ }

// Context:
interpreter::Context::PropertyAccessor*
game::interface::BeamContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    // ex IntBeamContext::lookup
    return lookupName(name, BEAM_MAP, result) ? this : 0;
}

void
game::interface::BeamContext::set(PropertyIndex_t index, const afl::data::Value* value)
{
    if (game::spec::Beam* b = m_shipList->beams().get(m_number)) {
        switch (BeamDomain(BEAM_MAP[index].domain)) {
         case ComponentPropertyDomain:
            setComponentProperty(*b, ComponentProperty(BEAM_MAP[index].index), value, *m_shipList);
            break;
         default:
            throw interpreter::Error::notAssignable();
        }
    } else {
        throw interpreter::Error::notAssignable();
    }
}

afl::data::Value*
game::interface::BeamContext::get(PropertyIndex_t index)
{
    if (const game::spec::Beam* b = m_shipList->beams().get(m_number)) {
        switch (BeamDomain(BEAM_MAP[index].domain)) {
         case ComponentPropertyDomain:
            return getComponentProperty(*b, ComponentProperty(BEAM_MAP[index].index), *m_shipList);
         case WeaponPropertyDomain:
            return getWeaponProperty(*b, WeaponProperty(BEAM_MAP[index].index), m_root->hostConfiguration(), m_root->hostVersion(), false);
        }
    }
    return 0;
}

bool
game::interface::BeamContext::next()
{
    if (game::spec::Beam* b = m_shipList->beams().findNext(m_number)) {
        m_number = b->getId();
        return true;
    } else {
        return false;
    }
}

game::interface::BeamContext*
game::interface::BeamContext::clone() const
{
    // ex IntBeamContext::clone
    return new BeamContext(m_number, m_shipList, m_root);
}

afl::base::Deletable*
game::interface::BeamContext::getObject()
{
    // ex IntBeamContext::getObject
    return 0;
}

void
game::interface::BeamContext::enumProperties(interpreter::PropertyAcceptor& acceptor) const
{
    acceptor.enumTable(BEAM_MAP);
}

// BaseValue:
String_t
game::interface::BeamContext::toString(bool /*readable*/) const
{
    // ex IntBeamContext::toString
    return afl::string::Format("Beam(%d)", m_number);
}

void
game::interface::BeamContext::store(interpreter::TagNode& out, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
{
    out.tag = out.Tag_Beam;
    out.value = m_number;
}

game::interface::BeamContext*
game::interface::BeamContext::create(int nr, Session& session)
{
    game::spec::ShipList* list = session.getShipList().get();
    Root* root = session.getRoot().get();
    if (list != 0 && root != 0 && list->beams().get(nr) != 0) {
        return new BeamContext(nr, *list, *root);
    }
    return 0;
}
