/**
  *  \file game/interface/simobjectcontext.cpp
  *  \brief Class game::interface::SimObjectContext
  */

#include "game/interface/simobjectcontext.hpp"

#include "afl/base/countof.hpp"
#include "game/interface/playerproperty.hpp"
#include "game/sim/ability.hpp"
#include "game/spec/hull.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/error.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/savecontext.hpp"
#include "interpreter/typehint.hpp"
#include "interpreter/values.hpp"

using afl::base::Ref;
using game::sim::Object;
using game::sim::Planet;
using game::sim::Session;
using game::sim::Setup;
using game::sim::Ship;
using game::spec::Beam;
using game::spec::Engine;
using game::spec::Hull;
using game::spec::ShipList;
using game::spec::TorpedoLauncher;
using interpreter::Arguments;
using interpreter::Context;
using interpreter::Error;
using interpreter::SaveContext;
using interpreter::TagNode;
using interpreter::makeBooleanValue;
using interpreter::makeIntegerValue;
using interpreter::makeStringValue;

namespace {
    /*
     *  Property definitions
     */

    enum SimObjectProperty {
        isoAggressiveness,
        isoAuxAmmo,
        isoAuxCount,
        isoAuxId,
        isoAuxName,
        isoBaseAmmoStorage,
        isoBaseBeamTech,
        isoBaseDefense,
        isoBaseFlag,
        isoBaseTorpedoTech,
        isoBeamCount,
        isoBeamId,
        isoBeamName,
        isoCloaked,
        isoCrew,
        isoDamage,
        isoDeactivated,
        isoDefense,
        isoEngineId,
        isoEngineName,
        isoFCode,
        isoFighterBays,
        isoFighterCount,
        isoFlags,
        isoFlakCompensation,
        isoFlakRating,
        isoHasFunction,
        isoHullId,
        isoHullName,
        isoId,
        isoInterceptId,
        isoLevel,
        isoMass,
        isoName,
        isoOwnerId,
        isoOwnerName,
        isoShield,
        isoTorpedoCount,
        isoTorpedoId,
        isoTorpedoLaunchers,
        isoTorpedoName,
        isoType
    };

    enum SimObjectDomain {
        SimObjectPropertyDomain
    };

    static const interpreter::NameTable SIM_MAPPING[] = {
        { "AGGRESSIVENESS",    isoAggressiveness,    SimObjectPropertyDomain, interpreter::thInt },
        { "AUX",               isoAuxName,           SimObjectPropertyDomain, interpreter::thString },
        { "AUX$",              isoAuxId,             SimObjectPropertyDomain, interpreter::thInt },
        { "AUX.AMMO",          isoAuxAmmo,           SimObjectPropertyDomain, interpreter::thInt },
        { "AUX.COUNT",         isoAuxCount,          SimObjectPropertyDomain, interpreter::thInt },
        { "BASE.YESNO",        isoBaseFlag,          SimObjectPropertyDomain, interpreter::thBool },
        { "BEAM",              isoBeamName,          SimObjectPropertyDomain, interpreter::thString },
        { "BEAM$",             isoBeamId,            SimObjectPropertyDomain, interpreter::thInt },
        { "BEAM.COUNT",        isoBeamCount,         SimObjectPropertyDomain, interpreter::thInt },
        { "CLOAKED",           isoCloaked,           SimObjectPropertyDomain, interpreter::thBool },
        { "CREW",              isoCrew,              SimObjectPropertyDomain, interpreter::thInt },
        { "DAMAGE",            isoDamage,            SimObjectPropertyDomain, interpreter::thInt },
        { "DEACTIVATED",       isoDeactivated,       SimObjectPropertyDomain, interpreter::thBool },
        { "DEFENSE",           isoDefense,           SimObjectPropertyDomain, interpreter::thInt },
        { "DEFENSE.BASE",      isoBaseDefense,       SimObjectPropertyDomain, interpreter::thInt  },
        { "ENGINE",            isoEngineName,        SimObjectPropertyDomain, interpreter::thString },
        { "ENGINE$",           isoEngineId,          SimObjectPropertyDomain, interpreter::thInt },
        { "FCODE",             isoFCode,             SimObjectPropertyDomain, interpreter::thString },
        { "FIGHTER.BAYS",      isoFighterBays,       SimObjectPropertyDomain, interpreter::thInt },
        { "FIGHTER.COUNT",     isoFighterCount,      SimObjectPropertyDomain, interpreter::thInt },
        { "FLAGS",             isoFlags,             SimObjectPropertyDomain, interpreter::thInt },
        { "HASFUNCTION",       isoHasFunction,       SimObjectPropertyDomain, interpreter::thFunction },
        { "HULL",              isoHullName,          SimObjectPropertyDomain, interpreter::thString },
        { "HULL$",             isoHullId,            SimObjectPropertyDomain, interpreter::thInt },
        { "ID",                isoId,                SimObjectPropertyDomain, interpreter::thInt },
        { "LEVEL",             isoLevel,             SimObjectPropertyDomain, interpreter::thInt },
        { "MASS",              isoMass,              SimObjectPropertyDomain, interpreter::thInt },
        { "MISSION.INTERCEPT", isoInterceptId,       SimObjectPropertyDomain, interpreter::thInt },
        { "NAME",              isoName,              SimObjectPropertyDomain, interpreter::thString },
        { "OWNER",             isoOwnerName,         SimObjectPropertyDomain, interpreter::thString },
        { "OWNER$",            isoOwnerId,           SimObjectPropertyDomain, interpreter::thInt },
        { "RATING.C",          isoFlakCompensation,  SimObjectPropertyDomain, interpreter::thInt },
        { "RATING.R",          isoFlakRating,        SimObjectPropertyDomain, interpreter::thInt },
        { "SHIELD",            isoShield,            SimObjectPropertyDomain, interpreter::thInt },
        { "STORAGE.AMMO",      isoBaseAmmoStorage,   SimObjectPropertyDomain, interpreter::thFunction },
        { "TECH.BEAM",         isoBaseBeamTech,      SimObjectPropertyDomain, interpreter::thInt },
        { "TECH.TORPEDO",      isoBaseTorpedoTech,   SimObjectPropertyDomain, interpreter::thInt },
        { "TORP",              isoTorpedoName,       SimObjectPropertyDomain, interpreter::thString },
        { "TORP$",             isoTorpedoId,         SimObjectPropertyDomain, interpreter::thInt },
        { "TORP.COUNT",        isoTorpedoCount,      SimObjectPropertyDomain, interpreter::thInt },
        { "TORP.LCOUNT",       isoTorpedoLaunchers,  SimObjectPropertyDomain, interpreter::thInt },
        { "TYPE",              isoType,              SimObjectPropertyDomain, interpreter::thString },
    };


    /*
     *  Ability definitions
     */

    struct AbilityMap {
        const char* name;
        game::sim::Ability ability;
    };
    const AbilityMap ABILITY_MAP[] = {
        { "PLANETIMMUNITY",      game::sim::PlanetImmunityAbility },
        { "FULLWEAPONRY",        game::sim::FullWeaponryAbility },
        { "COMMANDER",           game::sim::CommanderAbility },
        { "TRIPLEBEAMKILL",      game::sim::TripleBeamKillAbility },
        { "DOUBLEBEAMCHARGE",    game::sim::DoubleBeamChargeAbility },
        { "DOUBLETORPEDOCHARGE", game::sim::DoubleTorpedoChargeAbility },
        { "ELUSIVE",             game::sim::ElusiveAbility },
        { "SQUADRON",            game::sim::SquadronAbility },
        { "SHIELDGENERATOR",     game::sim::ShieldGeneratorAbility },
        { "CLOAKEDBAYS",         game::sim::CloakedBaysAbility },
    };

    const AbilityMap* findAbility(const String_t& arg)
    {
        for (size_t i = 0; i < countof(ABILITY_MAP); ++i) {
            if (afl::string::strCaseCompare(ABILITY_MAP[i].name, arg) == 0) {
                return &ABILITY_MAP[i];
            }
        }
        return 0;
    }
}


/*
 *  HasFunctionValue - implementation of HasFunction()
 */

class game::interface::SimObjectContext::HasFunctionValue : public interpreter::IndexableValue {
 public:
    HasFunctionValue(const Ref<Session>& session, const Ref<const Root>& root, const Ref<const ShipList>& shipList, Setup::Slot_t slot)
        : m_session(session), m_root(root), m_shipList(shipList), m_slot(slot)
        { }
    virtual afl::data::Value* get(Arguments& args)
        {
            /* @q HasFunction(func:Str, Optional mode:Int):Bool (Simulation Participant Property)
               Check whether the ship has the given hull function.

               Supported values for %func are:
               - CloakedBays
               - Commander
               - DoubleBeamCharge
               - DoubleTorpedoCharge
               - Elusive
               - FullWeaponry
               - PlanetImmunity
               - ShieldGenerator
               - Squadron
               - TripleBeamKill
               For values outside this set, returns EMPTY (future compatibility).

               With %mode=0 (default), returns the effective presence of the function.
               With %mode=1, check the default/implied presence, e.g. <tt>HasFunction("Commander",1)</tt> will return True
               if the ship's hull has the Commander ability.

               @since PCC2 2.41.5, PCC2 2.1 */
            args.checkArgumentCount(1, 2);

            // 'func' parameter
            String_t abilityName;
            if (!interpreter::checkStringArg(abilityName, args.getNext())) {
                return 0;
            }

            // 'mode' parameter
            int32_t mode = 0;
            interpreter::checkIntegerArg(mode, args.getNext(), 0, 1);

            // Resolve ability
            const AbilityMap* ability = findAbility(abilityName);
            if (ability == 0) {
                return 0;
            }

            // Resolve object
            Object* obj = m_session->setup().getObject(m_slot);
            if (obj == 0) {
                return 0;
            }

            // Generate result
            switch (mode) {
             case 0:
                // Effective
                return makeBooleanValue(obj->hasAbility(ability->ability, m_session->configuration(), *m_shipList, m_root->hostConfiguration()));
             case 1:
                // Implied
                return makeBooleanValue(obj->hasImpliedAbility(ability->ability, m_session->configuration(), *m_shipList, m_root->hostConfiguration()));
             default:
                return 0;
            }
        }
    virtual void set(Arguments& args, const afl::data::Value* value)
        { rejectSet(args, value); }
    virtual size_t getDimension(size_t /*which*/) const
        { return 0; }
    virtual Context* makeFirstContext()
        { return rejectFirstContext(); }
    virtual HasFunctionValue* clone() const
        { return new HasFunctionValue(m_session, m_root, m_shipList, m_slot); }
    virtual String_t toString(bool /*readable*/) const
        { return "#<HasFunction>"; }
    virtual void store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const
        { rejectStore(out, aux, ctx); }
 private:
    Ref<Session> m_session;
    Ref<const Root> m_root;
    Ref<const ShipList> m_shipList;
    Setup::Slot_t m_slot;
};


/*
 *  AmmoStorageValue - Implementation of Storage.Ammo
 */

class game::interface::SimObjectContext::AmmoStorageValue : public interpreter::IndexableValue {
 public:
    AmmoStorageValue(const Ref<Session>& session)
        : m_session(session)
        { }
    virtual afl::data::Value* get(Arguments& args)
        {
            /* @q Storage.Ammo:Int() (Simulation Participant Property)
               Valid for planets: array containing starbase ammo storage.
               At index 1..10, contains the number of torpedoes.
               At index 11, contains the number of fighters.

               EMPTY for ships.

               @since PCC2 2.41.5, PCC2 2.1 */
            args.checkArgumentCount(1);

            // Resolve index
            int32_t idx;
            if (!interpreter::checkIntegerArg(idx, args.getNext(), 1, Planet::NUM_TORPEDO_TYPES+1)) {
                return 0;
            }

            // Resolve planet
            const Planet* p = m_session->setup().getPlanet();
            if (p == 0) {
                return 0;
            }

            // Generate result
            const int32_t result = (idx > Planet::NUM_TORPEDO_TYPES ? p->getNumBaseFighters() : p->getNumBaseTorpedoes(idx));
            return makeIntegerValue(result);
        }
    virtual void set(Arguments& args, const afl::data::Value* value)
        { rejectSet(args, value); }
    virtual size_t getDimension(size_t which) const
        { return which == 0 ? 1 : Planet::NUM_TORPEDO_TYPES + 2; }
    virtual Context* makeFirstContext()
        { return rejectFirstContext(); }
    virtual AmmoStorageValue* clone() const
        { return new AmmoStorageValue(m_session); }
    virtual String_t toString(bool /*readable*/) const
        { return "#<Storage.Ammo>"; }
    virtual void store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const
        { rejectStore(out, aux, ctx); }
 private:
    Ref<Session> m_session;
};


/*
 *  Main Entry Point
 */

game::interface::SimObjectContext::SimObjectContext(const afl::base::Ref<game::sim::Session>& session,
                                                    const afl::base::Ref<const Root>& root,
                                                    const afl::base::Ref<const ShipList>& shipList,
                                                    Setup::Slot_t slot,
                                                    afl::string::Translator& tx)
    : m_session(session),
      m_root(root),
      m_shipList(shipList),
      m_slot(slot),
      m_translator(tx)
{ }

game::interface::SimObjectContext::~SimObjectContext()
{ }

// Context:
interpreter::Context::PropertyAccessor*
game::interface::SimObjectContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    if (interpreter::lookupName(name, SIM_MAPPING, result)) {
        return this;
    } else {
        return 0;
    }
}

bool
game::interface::SimObjectContext::next()
{
    if (m_slot+1 < m_session->setup().getNumObjects()) {
        ++m_slot;
        return true;
    } else {
        return false;
    }
}

game::interface::SimObjectContext*
game::interface::SimObjectContext::clone() const
{
    return new SimObjectContext(m_session, m_root, m_shipList, m_slot, m_translator);
}

game::sim::Object*
game::interface::SimObjectContext::getObject()
{
    return m_session->setup().getObject(m_slot);
}

void
game::interface::SimObjectContext::enumProperties(interpreter::PropertyAcceptor& acceptor) const
{
    acceptor.enumTable(SIM_MAPPING);
}

// BaseValue:
String_t
game::interface::SimObjectContext::toString(bool /*readable*/) const
{
    return "#<sim-object>";
}

void
game::interface::SimObjectContext::store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}

// PropertyAccessor:
void
game::interface::SimObjectContext::set(PropertyIndex_t /*index*/, const afl::data::Value* /*value*/)
{
    throw Error::notAssignable();
}

afl::data::Value*
game::interface::SimObjectContext::get(PropertyIndex_t index)
{
    switch (SimObjectProperty(SIM_MAPPING[index].index)) {
     case isoAggressiveness:
        /* @q Aggressiveness:Int (Simulation Participant Property)
           Valid for ships: aggressiveness.
           - -1: kill mission
           - 0: not aggressive
           - 1..12: primary enemy
           - 13: no fuel
           EMPTY for planets.

           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Ship* p = getShip()) {
            return makeIntegerValue(p->getAggressiveness());
        } else {
            return 0;
        }

     case isoAuxAmmo:
        /* @q Aux.Ammo:Int (Simulation Participant Property)
           Valid for ships: number of fighters/torpedoes
           EMPTY for planets.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Ship* p = getShip()) {
            return makeIntegerValue(p->getAmmo());
        } else {
            return 0;
        }

     case isoAuxCount:
        /* @q Aux.Count:Int (Simulation Participant Property)
           Valid for ships: number of fighter bays/torpedo launchers.
           EMPTY for planets.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Ship* p = getShip()) {
            if (p->getNumBays() > 0) {
                return makeIntegerValue(p->getNumBays());
            } else {
                return makeIntegerValue(p->getNumLaunchers());
            }
        } else {
            return 0;
        }

     case isoAuxId:
        /* @q Aux$:Int (Simulation Participant Property)
           Valid for ships: type of secondary weapon.
           - 1..10 for torpedoes
           - 11 for fighters
           For planets and otherwise, EMPTY.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Ship* p = getShip()) {
            if (p->getNumBays() > 0) {
                return makeIntegerValue(m_shipList->launchers().size() + 1);
            } else if (p->getNumLaunchers() > 0) {
                return makeIntegerValue(p->getTorpedoType());
            } else {
                return 0;
            }
        } else {
            return 0;
        }

     case isoAuxName:
        /* @q Aux:Str (Simulation Participant Property)
           Secondary weapon type, full name.
           Either a torpedo system name, "Fighters", or EMPTY.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Ship* p = getShip()) {
            if (p->getNumBays() > 0) {
                return makeStringValue("Fighters");
            } else {
                if (const TorpedoLauncher* tl = m_shipList->launchers().get(p->getTorpedoType())) {
                    return makeStringValue(tl->getName(m_shipList->componentNamer()));
                } else {
                    return 0;
                }
            }
        } else {
            return 0;
        }

     case isoBaseAmmoStorage:
        // Documented in AmmoStorageValue
        if (getPlanet() != 0) {
            return new AmmoStorageValue(m_session);
        } else {
            return 0;
        }

     case isoBaseBeamTech:
        /* @q Tech.Beam:Int (Simulation Participant Property)
           Valid for planets: beam tech level.
           Beam tech level defines presence of a base; it is 0 if the planet has no base.
           EMPTY for ships.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Planet* p = getPlanet()) {
            return makeIntegerValue(p->getBaseBeamTech());
        } else {
            return 0;
        }

     case isoBaseDefense:
        /* @q Defense.Base:Int (Simulation Participant Property)
           Valid for planets: number of starbase defense posts.
           EMPTY for ships.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Planet* p = getPlanet()) {
            return makeIntegerValue(p->getBaseDefense());
        } else {
            return 0;
        }

     case isoBaseFlag:
        /* @q Base.YesNo:Bool (Simulation Participant Property)
           True if this object is a planet that has a starbase.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Planet* p = getPlanet()) {
            return makeBooleanValue(p->hasBase());
        } else {
            return makeBooleanValue(0);
        }

     case isoBaseTorpedoTech:
        /* @q Tech.Torpedo:Int (Simulation Participant Property)
           Valid for planets: torpedo tech level.
           EMPTY for ships.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Planet* p = getPlanet()) {
            return makeIntegerValue(p->getBaseTorpedoTech());
        } else {
            return 0;
        }

     case isoBeamCount:
        /* @q Beam.Count:Int (Simulation Participant Property)
           Valid for ships: number of beams.
           EMPTY for planets.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Ship* p = getShip()) {
            return makeIntegerValue(p->getNumBeams());
        } else {
            return 0;
        }

     case isoBeamId:
        /* @q Beam$:Int (Simulation Participant Property)
           Valid for ships: beam type.
           EMPTY for planets.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Ship* p = getShip()) {
            return makeIntegerValue(p->getBeamType());
        } else {
            return 0;
        }

     case isoBeamName:
        /* @q Beam:Int (Simulation Participant Property)
           Valid for ships: beam type, full name.
           EMPTY for planets.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Ship* p = getShip()) {
            if (const Beam* b = m_shipList->beams().get(p->getBeamType())) {
                return makeStringValue(b->getName(m_shipList->componentNamer()));
            } else {
                return 0;
            }
        } else {
            return 0;
        }

     case isoCloaked:
        /* @q Cloaked:Bool (Simulation Participant Property)
           Valid for ships: true if ships is cloaded.
           EMPTY for planets.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Ship* p = getShip()) {
            return makeBooleanValue((p->getFlags() & Object::fl_Cloaked) != 0);
        } else {
            return 0;
        }

     case isoCrew:
        /* @q Crew:Int (Simulation Participant Property)
           Valid for ships: current crew size.
           EMPTY for planets.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Ship* p = getShip()) {
            return makeIntegerValue(p->getCrew());
        } else {
            return 0;
        }

     case isoDamage:
        /* @q Damage:Int (Simulation Participant Property)
           Damage level in percent.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Object* p = getObject()) {
            return makeIntegerValue(p->getDamage());
        } else {
            return 0;
        }

     case isoDeactivated:
        /* @q Deactivated:Bool (Simulation Participant Property)
           True if this unit is deactivated.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Object* p = getObject()) {
            return makeBooleanValue((p->getFlags() & Object::fl_Deactivated) != 0);
        } else {
            return 0;
        }

     case isoDefense:
        /* @q Defense:Int (Simulation Participant Property)
           Valid for planets: number of defense posts.
           EMPTY for ships.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Planet* p = getPlanet()) {
            return makeIntegerValue(p->getDefense());
        } else {
            return 0;
        }

     case isoEngineId:
        /* @q Engine$:Int (Simulation Participant Property)
           Valid for ships: type of engine.
           EMPTY for planets.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Ship* p = getShip()) {
            return makeIntegerValue(p->getEngineType());
        } else {
            return 0;
        }

     case isoEngineName:
        /* @q Engine:Str (Simulation Participant Property)
           Valid for ships: type of engine, full name.
           EMPTY for planets.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Ship* p = getShip()) {
            if (const Engine* e = m_shipList->engines().get(p->getEngineType())) {
                return makeStringValue(e->getName(m_shipList->componentNamer()));
            } else {
                return 0;
            }
        } else {
            return 0;
        }

     case isoFCode:
        /* @q FCode:Str (Simulation Participant Property)
           Friendly code.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Object* p = getObject()) {
            return makeStringValue(p->getFriendlyCode());
        } else {
            return 0;
        }

     case isoFighterBays:
        /* @q Fighter.Bays:Int (Simulation Participant Property)
           Valid for ships: number of fighter bays.
           EMPTY for planets.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Ship* p = getShip()) {
            return makeIntegerValue(p->getNumBays());
        } else {
            return 0;
        }

     case isoFighterCount:
        /* @q Fighter.Count:Int (Simulation Participant Property)
           Valid for ships: number of fighters.
           EMPTY for planets.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Ship* p = getShip()) {
            if (p->getNumBays() > 0) {
                return makeIntegerValue(p->getAmmo());
            } else {
                return makeIntegerValue(0);
            }
        } else if (const Planet* p = getPlanet()) {
            return makeIntegerValue(p->getNumBaseFighters());
        } else {
            return 0;
        }

     case isoFlags:
        /* @q Flags:Int (Simulation Participant Property)
           Assorted flags.
           Contains bits with hull functions, friendly-code randomisation settings, and others.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Object* p = getObject()) {
            return makeIntegerValue(p->getFlags());
        } else {
            return 0;
        }

     case isoFlakCompensation:
        /* @q Rating.C:Int (Simulation Participant Property)
           FLAK compensation rating override.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Object* p = getObject()) {
            return makeIntegerValue(p->getFlakCompensationOverride());
        } else {
            return 0;
        }

     case isoFlakRating:
        /* @q Rating.R:Int (Simulation Participant Property)
           FLAK targeting rating override.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Object* p = getObject()) {
            return makeIntegerValue(p->getFlakRatingOverride());
        } else {
            return 0;
        }

     case isoHasFunction:
        // Documented in HasFunctionValue
        return new HasFunctionValue(m_session, m_root, m_shipList, m_slot);

     case isoHullId:
        /* @q Hull$:Int (Simulation Participant Property)
           Valid for ships: hull number.
           0 for custom ships.
           EMPTY for planets.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Ship* p = getShip()) {
            return makeIntegerValue(p->getHullType());
        } else {
            return 0;
        }

     case isoHullName:
        /* @q Hull:Str (Simulation Participant Property)
           Valid for ships: hull name.
           EMPTY for custom ships and planets.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Ship* p = getShip()) {
            if (const Hull* h = m_shipList->hulls().get(p->getHullType())) {
                return makeStringValue(h->getName(m_shipList->componentNamer()));
            } else {
                return 0;
            }
        } else {
            return 0;
        }

     case isoId:
        /* @q Id:Int (Simulation Participant Property)
           Unit Id.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Object* p = getObject()) {
            return makeIntegerValue(p->getId());
        } else {
            return 0;
        }

     case isoInterceptId:
        /* @q Mission.Intercept:Int (Simulation Participant Property)
           Valid for ships: intercept target Id.
           EMPTY for planets.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Ship* p = getShip()) {
            return makeIntegerValue(p->getInterceptId());
        } else {
            return 0;
        }

     case isoLevel:
        /* @q Level:Int (Simulation Participant Property)
           Experience level.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Object* p = getObject()) {
            return makeIntegerValue(p->getExperienceLevel());
        } else {
            return 0;
        }

     case isoMass:
        /* @q Mass:Int (Simulation Participant Property)
           Valid for ships: mass.
           EMPTY for planets.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Ship* p = getShip()) {
            return makeIntegerValue(p->getMass());
        } else {
            return 0;
        }

     case isoName:
        /* @q Name:Str (Simulation Participant Property)
           Ship or planet name.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Object* p = getObject()) {
            return makeStringValue(p->getName());
        } else {
            return 0;
        }

     case isoOwnerId:
        /* @q Owner$:Int (Simulation Participant Property)
           Unit owner number.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Object* p = getObject()) {
            return makeIntegerValue(p->getOwner());
        } else {
            return 0;
        }

     case isoOwnerName:
        /* @q Owner:Str (Simulation Participant Property)
           Unit owner short name.
           @since PCC2 2.41.5, PCC2 2.1
           @see Race.Short (Player Property) */
        if (const Object* p = getObject()) {
            if (const Player* pl = m_root->playerList().get(p->getOwner())) {
                return makeStringValue(pl->getName(Player::ShortName, m_translator));
            } else {
                return 0;
            }
        } else {
            return 0;
        }

     case isoShield:
        /* @q Shield:Int (Simulation Participant Property)
           Shield level in percent.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Object* p = getObject()) {
            return makeIntegerValue(p->getShield());
        } else {
            return 0;
        }

     case isoTorpedoCount:
        /* @q Torp.Count:Int (Simulation Participant Property)
           Valid for ships: number of torpedies.
           EMPTY for planets.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Ship* p = getShip()) {
            if (p->getNumLaunchers() > 0) {
                return makeIntegerValue(p->getAmmo());
            } else {
                return makeIntegerValue(0);
            }
        } else {
            return 0;
        }

     case isoTorpedoId:
        /* @q Torp$:Int (Simulation Participant Property)
           Valid for ships: torpedo type.
           EMPTY for planets.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Ship* p = getShip()) {
            return makeIntegerValue(p->getTorpedoType());
        } else {
            return 0;
        }

     case isoTorpedoLaunchers:
        /* @q Torp.LCount:Int (Simulation Participant Property)
           Valid for ships: number of torpedo launchers.
           EMPTY for planets.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Ship* p = getShip()) {
            return makeIntegerValue(p->getNumLaunchers());
        } else {
            return 0;
        }

     case isoTorpedoName:
        /* @q Torp:Str (Simulation Participant Property)
           Valid for ships: torpedo system name.
           EMPTY for planets.
           @since PCC2 2.41.5, PCC2 2.1 */
        if (const Ship* p = getShip()) {
            if (const TorpedoLauncher* tl = m_shipList->launchers().get(p->getTorpedoType())) {
                return makeStringValue(tl->getName(m_shipList->componentNamer()));
            } else {
                return 0;
            }
        } else {
            return 0;
        }

     case isoType:
        /* @q Type:Str (Simulation Participant Property)
           Type of participant, one of "Ship" or "Planet".
           @since PCC2 2.41.5, PCC2 2.1 */
        if (getShip() != 0) {
            return makeStringValue("Ship");
        } else if (getPlanet() != 0) {
            return makeStringValue("Planet");
        } else {
            return 0;
        }
    }
    return 0;
}

game::sim::Planet*
game::interface::SimObjectContext::getPlanet()
{
    return dynamic_cast<Planet*>(getObject());
}

game::sim::Ship*
game::interface::SimObjectContext::getShip()
{
    return dynamic_cast<Ship*>(getObject());
}
