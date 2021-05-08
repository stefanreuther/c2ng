/**
  *  \file game/interface/hullcontext.cpp
  */

#include "game/interface/hullcontext.hpp"
#include "afl/string/format.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/typehint.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/error.hpp"
#include "game/interface/hullproperty.hpp"
#include "game/interface/componentproperty.hpp"

namespace {

    enum HullDomain { HullPropertyDomain, ComponentPropertyDomain };


    // FIXME: PCC 1.x items currently not implemented here:
    //   Hull (same as Name)
    //   Hull$ (same as Id)
    //   Hull.Short (same as Name.Short)
    //   Hull.Special (same as Special)
    //   Hull.Special.Str (same as Special.Str)
    const interpreter::NameTable HULL_MAPPING[] = {
        { "BEAM.MAX",         game::interface::ihpMaxBeams,         HullPropertyDomain,      interpreter::thInt },
        { "CARGO.MAX",        game::interface::ihpMaxCargo,         HullPropertyDomain,      interpreter::thInt },
        { "CARGO.MAXFUEL",    game::interface::ihpMaxFuel,          HullPropertyDomain,      interpreter::thInt },
        { "COST.D",           game::interface::icpCostD,            ComponentPropertyDomain, interpreter::thInt },
        { "COST.M",           game::interface::icpCostM,            ComponentPropertyDomain, interpreter::thInt },
        { "COST.MC",          game::interface::icpCostMC,           ComponentPropertyDomain, interpreter::thInt },
        { "COST.STR",         game::interface::icpCostStr,          ComponentPropertyDomain, interpreter::thString },
        { "COST.T",           game::interface::icpCostT,            ComponentPropertyDomain, interpreter::thInt },
        { "CREW.NORMAL",      game::interface::ihpMaxCrew,          HullPropertyDomain,      interpreter::thInt },
        { "ENGINE.COUNT",     game::interface::ihpNumEngines,       HullPropertyDomain,      interpreter::thInt },
        { "FIGHTER.BAYS",     game::interface::ihpNumFighterBays,   HullPropertyDomain,      interpreter::thInt },
        { "ID",               game::interface::icpId,               ComponentPropertyDomain, interpreter::thInt },
        { "IMAGE",            game::interface::ihpImage,            HullPropertyDomain,      interpreter::thInt },
        { "IMAGE$",           game::interface::ihpImage2,           HullPropertyDomain,      interpreter::thInt },
        { "MASS",             game::interface::icpMass,             ComponentPropertyDomain, interpreter::thInt },
        { "NAME",             game::interface::icpName,             ComponentPropertyDomain, interpreter::thString },
        { "NAME.SHORT",       game::interface::icpNameShort,        ComponentPropertyDomain, interpreter::thString },
        { "SPECIAL",          game::interface::ihpSpecial,          HullPropertyDomain,      interpreter::thString },
        { "TECH",             game::interface::icpTech,             ComponentPropertyDomain, interpreter::thInt },
        { "TECH.HULL",        game::interface::icpTech,             ComponentPropertyDomain, interpreter::thInt },
        { "TORP.LMAX",        game::interface::ihpMaxTorpLaunchers, HullPropertyDomain,      interpreter::thInt },
    };
}

game::interface::HullContext::HullContext(int nr, afl::base::Ref<game::spec::ShipList> shipList, afl::base::Ref<game::Root> root)
    : m_number(nr),
      m_shipList(shipList), m_root(root)
{
    // ex IntHullContext::IntHullContext
}

game::interface::HullContext::~HullContext()
{ }

// Context:
game::interface::HullContext*
game::interface::HullContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    // ex IntHullContext::lookup
    return lookupName(name, HULL_MAPPING, result) ? this : 0;
}

void
game::interface::HullContext::set(PropertyIndex_t index, const afl::data::Value* value)
{
    if (game::spec::Hull* hull = m_shipList->hulls().get(m_number)) {
        switch (HullDomain(HULL_MAPPING[index].domain)) {
         case HullPropertyDomain:
            setHullProperty(*hull, HullProperty(HULL_MAPPING[index].index), value, *m_shipList);
            break;
         default:
            throw interpreter::Error::notAssignable();
        }
    } else {
        throw interpreter::Error::notAssignable();
    }
}

afl::data::Value*
game::interface::HullContext::get(PropertyIndex_t index)
{
    if (const game::spec::Hull* hull = m_shipList->hulls().get(m_number)) {
        switch (HullDomain(HULL_MAPPING[index].domain)) {
         case HullPropertyDomain:
            return getHullProperty(*hull, HullProperty(HULL_MAPPING[index].index), *m_shipList, m_root->hostConfiguration());
         case ComponentPropertyDomain:
            return getComponentProperty(*hull, ComponentProperty(HULL_MAPPING[index].index), *m_shipList);
        }
    }
    return 0;
}

bool
game::interface::HullContext::next()
{
    // ex IntHullContext::next
    // ex shipint.pas:CHullspecContext.Next
    if (const game::spec::Hull* h = m_shipList->hulls().findNext(m_number)) {
        m_number = h->getId();
        return true;
    } else {
        return false;
    }
}

game::interface::HullContext*
game::interface::HullContext::clone() const
{
    return new HullContext(m_number, m_shipList, m_root);
}

game::map::Object*
game::interface::HullContext::getObject()
{
    // ex IntHullContext::getObject
    return 0;
}

void
game::interface::HullContext::enumProperties(interpreter::PropertyAcceptor& acceptor)
{
    // ex IntHullContext::enumProperties
    acceptor.enumTable(HULL_MAPPING);
}

// BaseValue:
String_t
game::interface::HullContext::toString(bool /*readable*/) const
{
    // ex IntHullContext::toString
    return afl::string::Format("Hull(%d)", m_number);
}

void
game::interface::HullContext::store(interpreter::TagNode& out, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
{
    // ex IntHullContext::store
    out.tag = out.Tag_Hull;
    out.value = m_number;
}

game::interface::HullContext*
game::interface::HullContext::create(int nr, Session& session)
{
    // ex shipint.pas:CreateHullspecContext
    // FIXME: this refuses creating a HullContext for nonexistant hulls.
    // Nu has discontinuous hull Ids. Should we allow creating them?
    game::spec::ShipList* list = session.getShipList().get();
    Root* root = session.getRoot().get();
    if (list != 0 && root != 0 && list->hulls().get(nr) != 0) {
        return new HullContext(nr, *list, *root);
    }
    return 0;
}
