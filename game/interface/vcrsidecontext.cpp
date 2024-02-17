/**
  *  \file game/interface/vcrsidecontext.cpp
  *  \brief Class game::interface::VcrSideContext
  */

#include "game/interface/vcrsidecontext.hpp"
#include "game/game.hpp"
#include "game/interface/vcrsideproperty.hpp"
#include "game/vcr/battle.hpp"
#include "game/vcr/database.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/typehint.hpp"
#include "interpreter/values.hpp"

using interpreter::makeSizeValue;

namespace {
    enum VcrSideDomain {
        SidePropertyDomain,
        SideIdDomain
    };

    const interpreter::NameTable side_mapping[] = {
        { "AUX",                    game::interface::ivsAuxName,         SidePropertyDomain,  interpreter::thString },
        { "AUX$",                   game::interface::ivsAuxId,           SidePropertyDomain,  interpreter::thInt },
        { "AUX.AMMO",               game::interface::ivsAuxAmmo,         SidePropertyDomain,  interpreter::thInt },
        { "AUX.COUNT",              game::interface::ivsAuxCount,        SidePropertyDomain,  interpreter::thInt },
        { "AUX.SHORT",              game::interface::ivsAuxShort,        SidePropertyDomain,  interpreter::thString },
        { "BEAM",                   game::interface::ivsBeamName,        SidePropertyDomain,  interpreter::thString },
        { "BEAM$",                  game::interface::ivsBeamId,          SidePropertyDomain,  interpreter::thInt },
        { "BEAM.COUNT",             game::interface::ivsBeamCount,       SidePropertyDomain,  interpreter::thInt },
        { "BEAM.SHORT",             game::interface::ivsBeamShort,       SidePropertyDomain,  interpreter::thString },
        { "CONFIG.BEAMCHARGERATE",  game::interface::ivsBeamChargeRate,  SidePropertyDomain,  interpreter::thInt },
        { "CONFIG.BEAMKILLRATE",    game::interface::ivsBeamKillRate,    SidePropertyDomain,  interpreter::thInt },
        { "CONFIG.CREWDEFENSERATE", game::interface::ivsCrewDefenseRate, SidePropertyDomain,  interpreter::thInt },
        { "CONFIG.TORPCHARGERATE",  game::interface::ivsTorpChargeRate,  SidePropertyDomain,  interpreter::thInt },
        { "CONFIG.TORPMISSRATE",    game::interface::ivsTorpMissRate,    SidePropertyDomain,  interpreter::thInt },
        { "CREW",                   game::interface::ivsCrew,            SidePropertyDomain,  interpreter::thInt },
        { "CREW$",                  game::interface::ivsCrewRaw,         SidePropertyDomain,  interpreter::thInt },
        { "DAMAGE",                 game::interface::ivsDamage,          SidePropertyDomain,  interpreter::thInt },
        { "FIGHTER.BAYS",           game::interface::ivsFighterBays,     SidePropertyDomain,  interpreter::thInt },
        { "FIGHTER.COUNT",          game::interface::ivsFighterCount,    SidePropertyDomain,  interpreter::thInt },
        { "HULL",                   game::interface::ivsHullName,        SidePropertyDomain,  interpreter::thString },
        { "HULL$",                  game::interface::ivsHullId,          SidePropertyDomain,  interpreter::thInt },
        { "ID",                     game::interface::ivsId,              SidePropertyDomain,  interpreter::thInt },
        { "IMAGE",                  game::interface::ivsImage,           SidePropertyDomain,  interpreter::thInt },
        { "INDEX",                  0,                                   SideIdDomain,        interpreter::thInt },
        { "ISPLANET",               game::interface::ivsIsPlanet,        SidePropertyDomain,  interpreter::thBool },
        { "LEVEL",                  game::interface::ivsLevel,           SidePropertyDomain,  interpreter::thInt },
        { "MASS",                   game::interface::ivsMass,            SidePropertyDomain,  interpreter::thInt },
        { "NAME",                   game::interface::ivsName,            SidePropertyDomain,  interpreter::thString },
        { "NAME.FULL",              game::interface::ivsNameFull,        SidePropertyDomain,  interpreter::thString },
        { "OWNER",                  game::interface::ivsOwnerShort,      SidePropertyDomain,  interpreter::thString },
        { "OWNER$",                 game::interface::ivsOwnerId,         SidePropertyDomain,  interpreter::thInt },
        { "OWNER.ADJ",              game::interface::ivsOwnerAdj,        SidePropertyDomain,  interpreter::thString },
        { "ROLE",                   game::interface::ivsRole,            SidePropertyDomain,  interpreter::thString },
        { "SHIELD",                 game::interface::ivsShield,          SidePropertyDomain,  interpreter::thInt },
        { "STATUS",                 game::interface::ivsStatus,          SidePropertyDomain,  interpreter::thString },
        { "STATUS$",                game::interface::ivsStatusRaw,       SidePropertyDomain,  interpreter::thInt },
        { "TORP",                   game::interface::ivsTorpName,        SidePropertyDomain,  interpreter::thString },
        { "TORP$",                  game::interface::ivsTorpId,          SidePropertyDomain,  interpreter::thInt },
        { "TORP.COUNT",             game::interface::ivsTorpCount,       SidePropertyDomain,  interpreter::thInt },
        { "TORP.LCOUNT",            game::interface::ivsTorpLCount,      SidePropertyDomain,  interpreter::thInt },
        { "TORP.SHORT",             game::interface::ivsTorpShort,       SidePropertyDomain,  interpreter::thString },
        { "TYPE",                   game::interface::ivsType,            SidePropertyDomain,  interpreter::thString },
        { "TYPE.SHORT",             game::interface::ivsTypeShort,       SidePropertyDomain,  interpreter::thString },
    };
}

game::interface::VcrSideContext::VcrSideContext(size_t battleNumber,
                                                size_t side,
                                                afl::string::Translator& tx,
                                                const afl::base::Ref<const Root>& root,
                                                const afl::base::Ptr<game::vcr::Database>& battles,
                                                const afl::base::Ref<const game::spec::ShipList>& shipList)
    : m_battleNumber(battleNumber),
      m_side(side),
      m_translator(tx),
      m_root(root),
      m_battles(battles),
      m_shipList(shipList)
{
    // ex IntVcrSideContext::IntVcrSideContext
}

game::interface::VcrSideContext::~VcrSideContext()
{ }

// Context:
interpreter::Context::PropertyAccessor*
game::interface::VcrSideContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    // ex IntVcrSideContext::lookup
    return lookupName(name, side_mapping, result) ? this : 0;
}

afl::data::Value*
game::interface::VcrSideContext::get(PropertyIndex_t index)
{
    // ex IntVcrSideContext::get
    switch (VcrSideDomain(side_mapping[index].domain)) {
     case SidePropertyDomain:
        if (game::vcr::Battle* battle = getBattle()) {
            return getVcrSideProperty(*battle,
                                      m_side,
                                      VcrSideProperty(side_mapping[index].index),
                                      m_translator,
                                      *m_shipList,
                                      m_root->hostConfiguration(),
                                      m_root->playerList());
        }
        return 0;

     case SideIdDomain:
        /* @q Index:Int (Combat Participant Property)
           Position of this unit in the fight.
           This is the index into the fight's {Unit (Combat Property)|Unit} array. */
        return makeSizeValue(m_side + 1);
    }
    return 0;
}

bool
game::interface::VcrSideContext::next()
{
    // ex IntVcrSideContext::next
    if (game::vcr::Battle* battle = getBattle()) {
        if (m_side + 1 < battle->getNumObjects()) {
            ++m_side;
            return true;
        }
    }
    return false;
}

game::interface::VcrSideContext*
game::interface::VcrSideContext::clone() const
{
    // ex IntVcrSideContext::clone
    return new VcrSideContext(m_battleNumber, m_side, m_translator, m_root, m_battles, m_shipList);
}

afl::base::Deletable*
game::interface::VcrSideContext::getObject()
{
    return 0;
}

void
game::interface::VcrSideContext::enumProperties(interpreter::PropertyAcceptor& acceptor) const
{
    acceptor.enumTable(side_mapping);
}

// BaseValue:
String_t
game::interface::VcrSideContext::toString(bool /*readable*/) const
{
    // ex IntVcrSideContext::toString
    return "#<vcr-object>";
}

void
game::interface::VcrSideContext::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    // ex IntVcrSideContext::store
    rejectStore(out, aux, ctx);
}

game::vcr::Battle*
game::interface::VcrSideContext::getBattle() const
{
    if (game::vcr::Database* db = m_battles.get()) {
        return db->getBattle(m_battleNumber);
    } else {
        return 0;
    }
}
