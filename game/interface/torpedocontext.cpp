/**
  *  \file game/interface/torpedocontext.cpp
  */

#include "game/interface/torpedocontext.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/error.hpp"
#include "game/spec/torpedo.hpp"
#include "game/interface/componentproperty.hpp"
#include "game/interface/weaponproperty.hpp"
#include "afl/string/format.hpp"

namespace {
    enum TorpedoDomain { ComponentPropertyDomain, WeaponPropertyDomain };

    // Mapping for torpedoes and launchers:
    const interpreter::NameTable torpedo_map[] = {
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
        { "TECH.TORPEDO",  game::interface::icpTech,      ComponentPropertyDomain, interpreter::thInt },
    };
}

game::interface::TorpedoContext::TorpedoContext(bool useLauncher, int nr, afl::base::Ref<game::spec::ShipList> shipList, afl::base::Ref<game::Root> root)
    : m_useLauncher(useLauncher),
      m_number(nr),
      m_shipList(shipList),
      m_root(root)
{
    // ex IntTorpedoContext::IntTorpedoContext, IntLauncherContext::IntLauncherContext
}

game::interface::TorpedoContext::~TorpedoContext()
{ }

// Context:
interpreter::Context::PropertyAccessor*
game::interface::TorpedoContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    // ex IntTorpedoContext::lookup, IntLauncherContext::lookup
    return lookupName(name, torpedo_map, result) ? this : 0;
}

void
game::interface::TorpedoContext::set(PropertyIndex_t index, const afl::data::Value* value)
{
    // ex IntTorpedoContext::set, IntLauncherContext::set
    // Assigments always go to the launcher, being the long-lived object.
    if (game::spec::TorpedoLauncher* launcher = m_shipList->launchers().get(m_number)) {
        switch (TorpedoDomain(torpedo_map[index].domain)) {
         case ComponentPropertyDomain:
            setComponentProperty(*launcher, ComponentProperty(torpedo_map[index].index), value, *m_shipList);
            break;
         default:
            throw interpreter::Error::notAssignable();
        }
    } else {
        throw interpreter::Error::notAssignable();
    }
}

afl::data::Value*
game::interface::TorpedoContext::get(PropertyIndex_t index)
{
    if (const game::spec::TorpedoLauncher* launcher = m_shipList->launchers().get(m_number)) {
        if (m_useLauncher) {
            return getProperty(*launcher, index);
        } else {
            return getProperty(game::spec::Torpedo(*launcher), index);
        }
    } else {
        return 0;
    }
}

bool
game::interface::TorpedoContext::next()
{
    // ex IntTorpedoContext::next, IntLauncherContext::next
    if (const game::spec::TorpedoLauncher* launcher = m_shipList->launchers().findNext(m_number)) {
        m_number = launcher->getId();
        return true;
    } else {
        return false;
    }
}

game::interface::TorpedoContext*
game::interface::TorpedoContext::clone() const
{
    // ex IntTorpedoContext::clone, IntLauncherContext::clone
    return new TorpedoContext(m_useLauncher, m_number, m_shipList, m_root);
}

game::map::Object*
game::interface::TorpedoContext::getObject()
{
    return 0;
}

void
game::interface::TorpedoContext::enumProperties(interpreter::PropertyAcceptor& acceptor) const
{
    // ex IntTorpedoContext::enumProperties, IntLauncherContext::enumProperties
    acceptor.enumTable(torpedo_map);
}

// BaseValue:
String_t
game::interface::TorpedoContext::toString(bool /*readable*/) const
{
    // ex IntTorpedoContext::toString, IntLauncherContext::toString
    return afl::string::Format(m_useLauncher ? "Launcher(%d)" : "Torpedo(%d)", m_number);
}

void
game::interface::TorpedoContext::store(interpreter::TagNode& out, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
{
    // ex IntTorpedoContext::store, IntLauncherContext::store
    out.tag = m_useLauncher ? out.Tag_Launcher : out.Tag_Torpedo;
    out.value = m_number;
}

game::interface::TorpedoContext*
game::interface::TorpedoContext::create(bool useLauncher, int nr, Session& session)
{
    game::spec::ShipList* list = session.getShipList().get();
    Root* root = session.getRoot().get();
    if (list != 0 && root != 0 && list->launchers().get(nr) != 0) {
        return new TorpedoContext(useLauncher, nr, *list, *root);
    } else {
        return 0;
    }
}

afl::data::Value*
game::interface::TorpedoContext::getProperty(const game::spec::Weapon& w, PropertyIndex_t index)
{
    // ex IntLauncherContext::get, IntTorpedoContext::get
    switch (TorpedoDomain(torpedo_map[index].domain)) {
     case ComponentPropertyDomain:
        return getComponentProperty(w, ComponentProperty(torpedo_map[index].index), *m_shipList);
     case WeaponPropertyDomain:
        return getWeaponProperty(w, WeaponProperty(torpedo_map[index].index), m_root->hostConfiguration(), m_root->hostVersion(), true);
    }
    return 0;
}
