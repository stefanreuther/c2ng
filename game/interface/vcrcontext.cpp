/**
  *  \file game/interface/vcrcontext.cpp
  */

#include "game/interface/vcrcontext.hpp"
#include "interpreter/nametable.hpp"
#include "game/interface/vcrproperty.hpp"
#include "game/interface/vcrsideproperty.hpp"
#include "interpreter/typehint.hpp"
#include "afl/string/format.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "game/game.hpp"

namespace {
    enum VcrDomain {
        VcrPropertyDomain,
        LeftPropertyDomain,
        RightPropertyDomain
    };

    const interpreter::NameTable vcr_mapping[] = {
        { "ALGORITHM",                game::interface::ivpAlgorithm,     VcrPropertyDomain,   interpreter::thString },
        { "CAPABILITIES",             game::interface::ivpFlags,         VcrPropertyDomain,   interpreter::thInt },
        { "LEFT",                     game::interface::ivsNameFull,      LeftPropertyDomain,  interpreter::thString },
        { "LEFT.AUX",                 game::interface::ivsAuxName,       LeftPropertyDomain,  interpreter::thString },
        { "LEFT.AUX$",                game::interface::ivsAuxId,         LeftPropertyDomain,  interpreter::thInt },
        { "LEFT.AUX.AMMO",            game::interface::ivsAuxAmmo,       LeftPropertyDomain,  interpreter::thInt },
        { "LEFT.AUX.COUNT",           game::interface::ivsAuxCount,      LeftPropertyDomain,  interpreter::thInt },
        { "LEFT.AUX.SHORT",           game::interface::ivsAuxShort,      LeftPropertyDomain,  interpreter::thString },
        { "LEFT.BEAM",                game::interface::ivsBeamName,      LeftPropertyDomain,  interpreter::thString },
        { "LEFT.BEAM$",               game::interface::ivsBeamId,        LeftPropertyDomain,  interpreter::thInt },
        { "LEFT.BEAM.COUNT",          game::interface::ivsBeamCount,     LeftPropertyDomain,  interpreter::thInt },
        { "LEFT.BEAM.SHORT",          game::interface::ivsBeamShort,     LeftPropertyDomain,  interpreter::thString },
        { "LEFT.CREW",                game::interface::ivsCrew,          LeftPropertyDomain,  interpreter::thInt },
        { "LEFT.CREW$",               game::interface::ivsCrewRaw,       LeftPropertyDomain,  interpreter::thInt },
        { "LEFT.DAMAGE",              game::interface::ivsDamage,        LeftPropertyDomain,  interpreter::thInt },
        { "LEFT.FIGHTER.BAYS",        game::interface::ivsFighterBays,   LeftPropertyDomain,  interpreter::thInt },
        { "LEFT.FIGHTER.COUNT",       game::interface::ivsFighterCount,  LeftPropertyDomain,  interpreter::thInt },
        { "LEFT.HULL",                game::interface::ivsHullName,      LeftPropertyDomain,  interpreter::thString },
        { "LEFT.HULL$",               game::interface::ivsHullId,        LeftPropertyDomain,  interpreter::thInt },
        { "LEFT.ID",                  game::interface::ivsId,            LeftPropertyDomain,  interpreter::thInt },
        { "LEFT.IMAGE",               game::interface::ivsImage,         LeftPropertyDomain,  interpreter::thInt },
        { "LEFT.LEVEL",               game::interface::ivsLevel,         LeftPropertyDomain,  interpreter::thInt },
        { "LEFT.MASS",                game::interface::ivsMass,          LeftPropertyDomain,  interpreter::thInt },
        { "LEFT.NAME",                game::interface::ivsName,          LeftPropertyDomain,  interpreter::thString },
        { "LEFT.OWNER",               game::interface::ivsOwnerShort,    LeftPropertyDomain,  interpreter::thString },
        { "LEFT.OWNER$",              game::interface::ivsOwnerId,       LeftPropertyDomain,  interpreter::thInt },
        { "LEFT.OWNER.ADJ",           game::interface::ivsOwnerAdj,      LeftPropertyDomain,  interpreter::thString },
        { "LEFT.SHIELD",              game::interface::ivsShield,        LeftPropertyDomain,  interpreter::thInt },
        { "LEFT.STATUS",              game::interface::ivsStatus,        LeftPropertyDomain,  interpreter::thString },
        { "LEFT.STATUS$",             game::interface::ivsStatusRaw,     LeftPropertyDomain,  interpreter::thString },
        { "LEFT.TORP",                game::interface::ivsTorpName,      LeftPropertyDomain,  interpreter::thString },
        { "LEFT.TORP$",               game::interface::ivsTorpId,        LeftPropertyDomain,  interpreter::thInt },
        { "LEFT.TORP.COUNT",          game::interface::ivsTorpCount,     LeftPropertyDomain,  interpreter::thInt },
        { "LEFT.TORP.LCOUNT",         game::interface::ivsTorpLCount,    LeftPropertyDomain,  interpreter::thInt },
        { "LEFT.TORP.SHORT",          game::interface::ivsTorpShort,     LeftPropertyDomain,  interpreter::thString },
        { "LEFT.TYPE",                game::interface::ivsType,          LeftPropertyDomain,  interpreter::thString },
        { "LEFT.TYPE.SHORT",          game::interface::ivsTypeShort,     LeftPropertyDomain,  interpreter::thString },
        { "MAGIC",                    game::interface::ivpMagic,         VcrPropertyDomain,   interpreter::thInt },
        { "NUMUNITS",                 game::interface::ivpNumUnits,      VcrPropertyDomain,   interpreter::thInt },
        { "RIGHT",                    game::interface::ivsNameFull,      RightPropertyDomain, interpreter::thString },
        { "RIGHT.AUX",                game::interface::ivsAuxName,       RightPropertyDomain, interpreter::thString },
        { "RIGHT.AUX$",               game::interface::ivsAuxId,         RightPropertyDomain, interpreter::thInt },
        { "RIGHT.AUX.AMMO",           game::interface::ivsAuxAmmo,       RightPropertyDomain, interpreter::thInt },
        { "RIGHT.AUX.COUNT",          game::interface::ivsAuxCount,      RightPropertyDomain, interpreter::thInt },
        { "RIGHT.AUX.SHORT",          game::interface::ivsAuxShort,      RightPropertyDomain, interpreter::thString },
        { "RIGHT.BEAM",               game::interface::ivsBeamName,      RightPropertyDomain, interpreter::thString },
        { "RIGHT.BEAM$",              game::interface::ivsBeamId,        RightPropertyDomain, interpreter::thInt },
        { "RIGHT.BEAM.COUNT",         game::interface::ivsBeamCount,     RightPropertyDomain, interpreter::thInt },
        { "RIGHT.BEAM.SHORT",         game::interface::ivsBeamShort,     RightPropertyDomain, interpreter::thString },
        { "RIGHT.CREW",               game::interface::ivsCrew,          RightPropertyDomain, interpreter::thInt },
        { "RIGHT.CREW$",              game::interface::ivsCrewRaw,       RightPropertyDomain, interpreter::thInt },
        { "RIGHT.DAMAGE",             game::interface::ivsDamage,        RightPropertyDomain, interpreter::thInt },
        { "RIGHT.FIGHTER.BAYS",       game::interface::ivsFighterBays,   RightPropertyDomain, interpreter::thInt },
        { "RIGHT.FIGHTER.COUNT",      game::interface::ivsFighterCount,  RightPropertyDomain, interpreter::thInt },
        { "RIGHT.HULL",               game::interface::ivsHullName,      RightPropertyDomain, interpreter::thString },
        { "RIGHT.HULL$",              game::interface::ivsHullId,        RightPropertyDomain, interpreter::thInt },
        { "RIGHT.ID",                 game::interface::ivsId,            RightPropertyDomain, interpreter::thInt },
        { "RIGHT.IMAGE",              game::interface::ivsImage,         RightPropertyDomain, interpreter::thInt },
        { "RIGHT.LEVEL",              game::interface::ivsLevel,         RightPropertyDomain, interpreter::thInt },
        { "RIGHT.MASS",               game::interface::ivsMass,          RightPropertyDomain, interpreter::thInt },
        { "RIGHT.NAME",               game::interface::ivsName,          RightPropertyDomain, interpreter::thString },
        { "RIGHT.OWNER",              game::interface::ivsOwnerShort,    RightPropertyDomain, interpreter::thString },
        { "RIGHT.OWNER$",             game::interface::ivsOwnerId,       RightPropertyDomain, interpreter::thInt },
        { "RIGHT.OWNER.ADJ",          game::interface::ivsOwnerAdj,      RightPropertyDomain, interpreter::thString },
        { "RIGHT.SHIELD",             game::interface::ivsShield,        RightPropertyDomain, interpreter::thInt },
        { "RIGHT.STATUS",             game::interface::ivsStatus,        RightPropertyDomain, interpreter::thString },
        { "RIGHT.STATUS$",            game::interface::ivsStatusRaw,     RightPropertyDomain, interpreter::thString },
        { "RIGHT.TORP",               game::interface::ivsTorpName,      RightPropertyDomain, interpreter::thString },
        { "RIGHT.TORP$",              game::interface::ivsTorpId,        RightPropertyDomain, interpreter::thInt },
        { "RIGHT.TORP.COUNT",         game::interface::ivsTorpCount,     RightPropertyDomain, interpreter::thInt },
        { "RIGHT.TORP.LCOUNT",        game::interface::ivsTorpLCount,    RightPropertyDomain, interpreter::thInt },
        { "RIGHT.TORP.SHORT",         game::interface::ivsTorpShort,     RightPropertyDomain, interpreter::thString },
        { "RIGHT.TYPE",               game::interface::ivsType,          RightPropertyDomain, interpreter::thString },
        { "RIGHT.TYPE.SHORT",         game::interface::ivsTypeShort,     RightPropertyDomain, interpreter::thString },
        { "SEED",                     game::interface::ivpSeed,          VcrPropertyDomain,   interpreter::thInt },
        { "TYPE$",                    game::interface::ivpType,          VcrPropertyDomain,   interpreter::thInt },
        { "UNIT",                     game::interface::ivpUnits,         VcrPropertyDomain,   interpreter::thArray },
    };
}

game::interface::VcrContext::VcrContext(size_t battleNumber,
                                        Session& session,
                                        afl::base::Ref<Root> root,     // for PlayerList
                                        afl::base::Ref<Turn> turn,     // for Turn
                                        afl::base::Ref<game::spec::ShipList> shipList)
    : m_battleNumber(battleNumber),
      m_session(session),
      m_root(root),
      m_turn(turn),
      m_shipList(shipList)
{
    // ex IntVcrContext::IntVcrContext
}
game::interface::VcrContext::~VcrContext()
{ }

// Context:
interpreter::Context::PropertyAccessor*
game::interface::VcrContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    // ex IntVcrContext::lookup
    return lookupName(name, vcr_mapping, result) ? this : 0;
}

afl::data::Value*
game::interface::VcrContext::get(PropertyIndex_t index)
{
    // ex IntVcrContext::get
    // ex values.pas:CVcrContext.ResolveValue
    if (game::vcr::Battle* battle = getBattle()) {
        switch (VcrDomain(vcr_mapping[index].domain)) {
         case VcrPropertyDomain:
            return getVcrProperty(m_battleNumber, VcrProperty(vcr_mapping[index].index), m_session, m_root, m_turn, m_shipList);
            
         case LeftPropertyDomain:
            return getVcrSideProperty(*battle, 0, VcrSideProperty(vcr_mapping[index].index),
                                      m_session.translator(),
                                      *m_shipList,
                                      m_root->hostConfiguration(),
                                      m_root->playerList());

         case RightPropertyDomain:
            return getVcrSideProperty(*battle, 1, VcrSideProperty(vcr_mapping[index].index),
                                      m_session.translator(),
                                      *m_shipList,
                                      m_root->hostConfiguration(),
                                      m_root->playerList());
        }
    }
    return 0;
}

bool
game::interface::VcrContext::next()
{
    // ex IntVcrContext::next
    if (game::vcr::Database* db = m_turn->getBattles().get()) {
        if (m_battleNumber + 1 < db->getNumBattles()) {
            ++m_battleNumber;
            return true;
        }
    }
    return false;
}

game::interface::VcrContext*
game::interface::VcrContext::clone() const
{
    return new VcrContext(m_battleNumber, m_session, m_root, m_turn, m_shipList);
}

game::map::Object*
game::interface::VcrContext::getObject()
{
    return 0;
}

void
game::interface::VcrContext::enumProperties(interpreter::PropertyAcceptor& acceptor)
{
    // ex IntVcrContext::enumProperties
    acceptor.enumTable(vcr_mapping);
}

// BaseValue:
String_t
game::interface::VcrContext::toString(bool /*readable*/) const
{
    // ex IntVcrContext::toString
    return afl::string::Format("Vcr(%d)", m_battleNumber);
}

void
game::interface::VcrContext::store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
{
    // ex IntVcrContext::store
    throw interpreter::Error::notAssignable();
}

game::vcr::Battle*
game::interface::VcrContext::getBattle() const
{
    if (game::vcr::Database* db = m_turn->getBattles().get()) {
        return db->getBattle(m_battleNumber);
    } else {
        return 0;
    }
}

game::interface::VcrContext*
game::interface::VcrContext::create(size_t battleNumber, Session& session)
{
    // Check major objects
    Root* r = session.getRoot().get();
    Game* g = session.getGame().get();
    game::spec::ShipList* s = session.getShipList().get();
    if (r == 0 || g == 0 || s == 0) {
        return 0;
    }

    // Check presence of battle
    Turn& t = g->currentTurn();
    game::vcr::Database* db = t.getBattles().get();
    if (db == 0 || db->getBattle(battleNumber) == 0) {
        return 0;
    }

    // OK
    return new VcrContext(battleNumber, session, *r, t, *s);
}
