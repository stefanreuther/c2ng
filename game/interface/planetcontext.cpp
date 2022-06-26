/**
  *  \file game/interface/planetcontext.cpp
  */

#include "game/interface/planetcontext.hpp"
#include "afl/base/countof.hpp"
#include "game/map/anyplanettype.hpp"
#include "interpreter/nametable.hpp"
#include "game/interface/baseproperty.hpp"
#include "game/interface/planetproperty.hpp"
#include "interpreter/typehint.hpp"
#include "game/turn.hpp"
#include "interpreter/error.hpp"
#include "interpreter/values.hpp"
#include "afl/string/format.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "game/interface/playerproperty.hpp"
#include "game/interface/planetmethod.hpp"
#include "interpreter/procedurevalue.hpp"

namespace {
    enum PlanetDomain {
        PlanetPropertyDomain,
        BasePropertyDomain,
        OwnerPropertyDomain,
        PlanetMethodDomain
    };

    static const interpreter::NameTable planet_mapping[] = {
        { "AUTOBUILD",                 game::interface::ipmAutoBuild,         PlanetMethodDomain,   interpreter::thProcedure },
        { "AUTOTAXCOLONISTS",          game::interface::ipmAutoTaxColonists,  PlanetMethodDomain,   interpreter::thProcedure },
        { "AUTOTAXNATIVES",            game::interface::ipmAutoTaxNatives,    PlanetMethodDomain,   interpreter::thProcedure },
        { "BASE",                      game::interface::ippBaseStr,           PlanetPropertyDomain, interpreter::thString },
        { "BASE.BUILDING",             game::interface::ippBaseBuildFlag,     PlanetPropertyDomain, interpreter::thBool },
        { "BASE.YESNO",                game::interface::ippBaseFlag,          PlanetPropertyDomain, interpreter::thBool },
        { "BUILD",                     game::interface::ibpBuildHullName,     BasePropertyDomain,   interpreter::thString },
        { "BUILD.BEAM$",               game::interface::ibpBuildBeam,         BasePropertyDomain,   interpreter::thInt },
        { "BUILD.BEAM.COUNT",          game::interface::ibpBuildBeamCount,    BasePropertyDomain,   interpreter::thInt },
        { "BUILD.ENGINE$",             game::interface::ibpBuildEngine,       BasePropertyDomain,   interpreter::thInt },
        { "BUILD.HULL$",               game::interface::ibpBuildHull,         BasePropertyDomain,   interpreter::thInt },
        { "BUILD.QPOS",                game::interface::ibpBuildQueuePos,     BasePropertyDomain,   interpreter::thInt },
        { "BUILD.SHORT",               game::interface::ibpBuildHullShort,    BasePropertyDomain,   interpreter::thInt },
        { "BUILD.TORP$",               game::interface::ibpBuildTorp,         BasePropertyDomain,   interpreter::thInt },
        { "BUILD.TORP.COUNT",          game::interface::ibpBuildTorpCount,    BasePropertyDomain,   interpreter::thInt },
        { "BUILD.YESNO",               game::interface::ibpBuildFlag,         BasePropertyDomain,   interpreter::thBool },
        { "BUILDBASE",                 game::interface::ipmBuildBase,         PlanetMethodDomain,   interpreter::thProcedure },
        { "BUILDBASEDEFENSE",          game::interface::ipmBuildBaseDefense,  PlanetMethodDomain,   interpreter::thProcedure },
        { "BUILDBEAMS",                game::interface::ipmBuildBeams,        PlanetMethodDomain,   interpreter::thProcedure },
        { "BUILDDEFENSE",              game::interface::ipmBuildDefense,      PlanetMethodDomain,   interpreter::thProcedure },
        { "BUILDENGINES",              game::interface::ipmBuildEngines,      PlanetMethodDomain,   interpreter::thProcedure },
        { "BUILDFACTORIES",            game::interface::ipmBuildFactories,    PlanetMethodDomain,   interpreter::thProcedure },
        { "BUILDFIGHTERS",             game::interface::ipmBuildFighters,     PlanetMethodDomain,   interpreter::thProcedure },
        { "BUILDHULLS",                game::interface::ipmBuildHulls,        PlanetMethodDomain,   interpreter::thProcedure },
        { "BUILDLAUNCHERS",            game::interface::ipmBuildLaunchers,    PlanetMethodDomain,   interpreter::thProcedure },
        { "BUILDMINES",                game::interface::ipmBuildMines,        PlanetMethodDomain,   interpreter::thProcedure },
        { "BUILDSHIP",                 game::interface::ipmBuildShip,         PlanetMethodDomain,   interpreter::thProcedure },
        { "BUILDTORPS",                game::interface::ipmBuildTorps,        PlanetMethodDomain,   interpreter::thProcedure },
        { "CARGOTRANSFER",             game::interface::ipmCargoTransfer,     PlanetMethodDomain,   interpreter::thProcedure },
        { "COLONISTS",                 game::interface::ippColonists,         PlanetPropertyDomain, interpreter::thInt },
        { "COLONISTS.CHANGE",          game::interface::ippColonistChangeStr, PlanetPropertyDomain, interpreter::thString },
        { "COLONISTS.CHANGE$",         game::interface::ippColonistChange,    PlanetPropertyDomain, interpreter::thInt },
        { "COLONISTS.HAPPY",           game::interface::ippColonistHappyStr,  PlanetPropertyDomain, interpreter::thString },
        { "COLONISTS.HAPPY$",          game::interface::ippColonistHappy,     PlanetPropertyDomain, interpreter::thInt },
        { "COLONISTS.SUPPORTED",       game::interface::ippColonistSupported, PlanetPropertyDomain, interpreter::thInt },
        { "COLONISTS.TAX",             game::interface::ippColonistTax,       PlanetPropertyDomain, interpreter::thInt },
        { "COLONISTS.TAX.INCOME",      game::interface::ippColonistTaxIncome, PlanetPropertyDomain, interpreter::thInt },
        { "DAMAGE",                    game::interface::ibpBaseDamage,        BasePropertyDomain,   interpreter::thInt },
        { "DEFENSE",                   game::interface::ippDefense,           PlanetPropertyDomain, interpreter::thInt },
        { "DEFENSE.BASE",              game::interface::ibpBaseDefense,       BasePropertyDomain,   interpreter::thInt },
        { "DEFENSE.BASE.MAX",          game::interface::ibpBaseDefenseMax,    BasePropertyDomain,   interpreter::thInt },
        { "DEFENSE.BASE.SPEED",        game::interface::ippBaseDefenseSpeed,  PlanetPropertyDomain, interpreter::thInt },
        { "DEFENSE.BASE.WANT",         game::interface::ippBaseDefenseWanted, PlanetPropertyDomain, interpreter::thInt },
        { "DEFENSE.MAX",               game::interface::ippDefenseMax,        PlanetPropertyDomain, interpreter::thInt },
        { "DEFENSE.SPEED",             game::interface::ippDefenseSpeed,      PlanetPropertyDomain, interpreter::thInt },
        { "DEFENSE.WANT",              game::interface::ippDefenseWanted,     PlanetPropertyDomain, interpreter::thInt },
        { "DENSITY.D",                 game::interface::ippDensityD,          PlanetPropertyDomain, interpreter::thInt },
        { "DENSITY.M",                 game::interface::ippDensityM,          PlanetPropertyDomain, interpreter::thInt },
        { "DENSITY.N",                 game::interface::ippDensityN,          PlanetPropertyDomain, interpreter::thInt },
        { "DENSITY.T",                 game::interface::ippDensityT,          PlanetPropertyDomain, interpreter::thInt },
        { "FACTORIES",                 game::interface::ippFactories,         PlanetPropertyDomain, interpreter::thInt },
        { "FACTORIES.MAX",             game::interface::ippFactoriesMax,      PlanetPropertyDomain, interpreter::thInt },
        { "FACTORIES.SPEED",           game::interface::ippFactoriesSpeed,    PlanetPropertyDomain, interpreter::thInt },
        { "FACTORIES.WANT",            game::interface::ippFactoriesWanted,   PlanetPropertyDomain, interpreter::thInt },
        { "FCODE",                     game::interface::ippFCode,             PlanetPropertyDomain, interpreter::thString },
        { "FIGHTERS",                  game::interface::ibpBaseFighters,      BasePropertyDomain,   interpreter::thInt },
        { "FIGHTERS.MAX",              game::interface::ibpBaseFightersMax,   BasePropertyDomain,   interpreter::thInt },
        { "FIXSHIP",                   game::interface::ipmFixShip,           PlanetMethodDomain,   interpreter::thProcedure },
        { "GROUND.D",                  game::interface::ippGroundD,           PlanetPropertyDomain, interpreter::thInt },
        { "GROUND.M",                  game::interface::ippGroundM,           PlanetPropertyDomain, interpreter::thInt },
        { "GROUND.N",                  game::interface::ippGroundN,           PlanetPropertyDomain, interpreter::thInt },
        { "GROUND.T",                  game::interface::ippGroundT,           PlanetPropertyDomain, interpreter::thInt },
        { "ID",                        game::interface::ippId,                PlanetPropertyDomain, interpreter::thInt },
        { "INDUSTRY",                  game::interface::ippIndustry,          PlanetPropertyDomain, interpreter::thString },
        { "INDUSTRY$",                 game::interface::ippIndustryCode,      PlanetPropertyDomain, interpreter::thInt },
        { "LEVEL",                     game::interface::ippLevel,             PlanetPropertyDomain, interpreter::thInt },
        { "LOC.X",                     game::interface::ippLocX,              PlanetPropertyDomain, interpreter::thInt },
        { "LOC.Y",                     game::interface::ippLocY,              PlanetPropertyDomain, interpreter::thInt },
        { "MARK",                      game::interface::ipmMark,              PlanetMethodDomain,   interpreter::thProcedure },
        { "MARKED",                    game::interface::ippMarked,            PlanetPropertyDomain, interpreter::thBool },
        { "MESSAGES",                  game::interface::ippMessages,          PlanetPropertyDomain, interpreter::thArray },
        { "MINED.D",                   game::interface::ippMinedD,            PlanetPropertyDomain, interpreter::thInt },
        { "MINED.M",                   game::interface::ippMinedM,            PlanetPropertyDomain, interpreter::thInt },
        { "MINED.N",                   game::interface::ippMinedN,            PlanetPropertyDomain, interpreter::thInt },
        { "MINED.STR",                 game::interface::ippMinedStr,          PlanetPropertyDomain, interpreter::thString},
        { "MINED.T",                   game::interface::ippMinedT,            PlanetPropertyDomain, interpreter::thInt },
        { "MINES",                     game::interface::ippMines,             PlanetPropertyDomain, interpreter::thInt },
        { "MINES.MAX",                 game::interface::ippMinesMax,          PlanetPropertyDomain, interpreter::thInt },
        { "MINES.SPEED",               game::interface::ippMinesSpeed,        PlanetPropertyDomain, interpreter::thInt },
        { "MINES.WANT",                game::interface::ippMinesWanted,       PlanetPropertyDomain, interpreter::thInt },
        { "MISSION",                   game::interface::ibpMissionName,       BasePropertyDomain,   interpreter::thString },
        { "MISSION$",                  game::interface::ibpMission,           BasePropertyDomain,   interpreter::thInt },
        { "MONEY",                     game::interface::ippMoney,             PlanetPropertyDomain, interpreter::thInt },
        { "NAME",                      game::interface::ippName,              PlanetPropertyDomain, interpreter::thString },
        { "NATIVES",                   game::interface::ippNatives,           PlanetPropertyDomain, interpreter::thString },
        { "NATIVES.CHANGE",            game::interface::ippNativeChangeStr,   PlanetPropertyDomain, interpreter::thString },
        { "NATIVES.CHANGE$",           game::interface::ippNativeChange,      PlanetPropertyDomain, interpreter::thInt },
        { "NATIVES.GOV",               game::interface::ippNativeGov,         PlanetPropertyDomain, interpreter::thString },
        { "NATIVES.GOV$",              game::interface::ippNativeGovCode,     PlanetPropertyDomain, interpreter::thInt },
        { "NATIVES.HAPPY",             game::interface::ippNativeHappyStr,    PlanetPropertyDomain, interpreter::thString },
        { "NATIVES.HAPPY$",            game::interface::ippNativeHappy,       PlanetPropertyDomain, interpreter::thInt },
        { "NATIVES.RACE",              game::interface::ippNativeRace,        PlanetPropertyDomain, interpreter::thString },
        { "NATIVES.RACE$",             game::interface::ippNativeRaceCode,    PlanetPropertyDomain, interpreter::thInt },
        { "NATIVES.TAX",               game::interface::ippNativeTax,         PlanetPropertyDomain, interpreter::thInt },
        { "NATIVES.TAX.BASE",          game::interface::ippNativeTaxBase,     PlanetPropertyDomain, interpreter::thInt },
        { "NATIVES.TAX.INCOME",        game::interface::ippNativeTaxIncome,   PlanetPropertyDomain, interpreter::thInt },
        { "NATIVES.TAX.MAX",           game::interface::ippNativeTaxMax,      PlanetPropertyDomain, interpreter::thInt },
        { "ORBIT",                     game::interface::ippOrbitingShips,     PlanetPropertyDomain, interpreter::thInt },
        { "ORBIT.ENEMY",               game::interface::ippOrbitingEnemies,   PlanetPropertyDomain, interpreter::thInt },
        { "ORBIT.OWN",                 game::interface::ippOrbitingOwn,       PlanetPropertyDomain, interpreter::thInt },
        { "OWNER",                     game::interface::iplShortName,         OwnerPropertyDomain,  interpreter::thString },
        { "OWNER$",                    game::interface::iplId,                OwnerPropertyDomain,  interpreter::thInt },
        { "OWNER.ADJ",                 game::interface::iplAdjName,           OwnerPropertyDomain,  interpreter::thString },
        { "PLAYED",                    game::interface::ippPlayed,            PlanetPropertyDomain, interpreter::thBool },
        { "RECYCLESHIP",               game::interface::ipmRecycleShip,       PlanetMethodDomain,   interpreter::thProcedure },
        { "REF",                       game::interface::ippReference,         PlanetPropertyDomain, interpreter::thNone },
        { "SCORE",                     game::interface::ippScore,             PlanetPropertyDomain, interpreter::thArray },
        { "SELLSUPPLIES",              game::interface::ipmSellSupplies,      PlanetMethodDomain,   interpreter::thProcedure },
        { "SETCOLONISTTAX",            game::interface::ipmSetColonistTax,    PlanetMethodDomain,   interpreter::thProcedure },
        { "SETCOMMENT",                game::interface::ipmSetComment,        PlanetMethodDomain,   interpreter::thProcedure },
        { "SETFCODE",                  game::interface::ipmSetFCode,          PlanetMethodDomain,   interpreter::thProcedure },
        { "SETMISSION",                game::interface::ipmSetMission,        PlanetMethodDomain,   interpreter::thProcedure },
        { "SETNATIVETAX",              game::interface::ipmSetNativeTax,      PlanetMethodDomain,   interpreter::thProcedure },
        { "SETTECH",                   game::interface::ipmSetTech,           PlanetMethodDomain,   interpreter::thProcedure },
        { "SHIPYARD",                  game::interface::ibpShipyardStr,       BasePropertyDomain,   interpreter::thString },
        { "SHIPYARD.ACTION",           game::interface::ibpShipyardAction,    BasePropertyDomain,   interpreter::thString },
        { "SHIPYARD.ID",               game::interface::ibpShipyardId,        BasePropertyDomain,   interpreter::thInt },
        { "SHIPYARD.NAME",             game::interface::ibpShipyardName,      BasePropertyDomain,   interpreter::thString },
        { "STORAGE.AMMO",              game::interface::ibpAmmoStorage,       BasePropertyDomain,   interpreter::thArray },
        { "STORAGE.BEAMS",             game::interface::ibpBeamStorage,       BasePropertyDomain,   interpreter::thArray },
        { "STORAGE.ENGINES",           game::interface::ibpEngineStorage,     BasePropertyDomain,   interpreter::thArray },
        { "STORAGE.HULLS",             game::interface::ibpHullStorage,       BasePropertyDomain,   interpreter::thArray },
        { "STORAGE.LAUNCHERS",         game::interface::ibpLauncherStorage,   BasePropertyDomain,   interpreter::thArray },
        { "SUPPLIES",                  game::interface::ippSupplies,          PlanetPropertyDomain, interpreter::thInt },
        { "TASK",                      game::interface::ippTask,              PlanetPropertyDomain, interpreter::thBool },
        { "TASK.BASE",                 game::interface::ippTaskBase,          PlanetPropertyDomain, interpreter::thBool },
        { "TECH.BEAM",                 game::interface::ibpBeamTech,          BasePropertyDomain,   interpreter::thInt },
        { "TECH.ENGINE",               game::interface::ibpEngineTech,        BasePropertyDomain,   interpreter::thInt },
        { "TECH.HULL",                 game::interface::ibpHullTech,          BasePropertyDomain,   interpreter::thInt },
        { "TECH.TORPEDO",              game::interface::ibpTorpedoTech,       BasePropertyDomain,   interpreter::thInt },
        { "TEMP",                      game::interface::ippTempStr,           PlanetPropertyDomain, interpreter::thString },
        { "TEMP$",                     game::interface::ippTemp,              PlanetPropertyDomain, interpreter::thInt },
        { "TURN.COLONISTS",            game::interface::ippColonistTime,      PlanetPropertyDomain, interpreter::thInt },
        { "TURN.MINERALS",             game::interface::ippMineralTime,       PlanetPropertyDomain, interpreter::thInt },
        { "TURN.MONEY",                game::interface::ippCashTime,          PlanetPropertyDomain, interpreter::thInt },
        { "TURN.NATIVES",              game::interface::ippNativeTime,        PlanetPropertyDomain, interpreter::thInt },
        { "TYPE",                      game::interface::ippTypeStr,           PlanetPropertyDomain, interpreter::thString },
        { "TYPE.SHORT",                game::interface::ippTypeChar,          PlanetPropertyDomain, interpreter::thString },
        { "UNMARK",                    game::interface::ipmUnmark,            PlanetMethodDomain,   interpreter::thProcedure },
    };

    const size_t NUM_PLANET_PROPERTIES = countof(planet_mapping);

    bool lookupPlanetProperty(const afl::data::NameQuery& q, interpreter::World& world, interpreter::Context::PropertyIndex_t& result)
    {
        // Check user-defined properties
        afl::data::NameMap::Index_t ix = world.planetPropertyNames().getIndexByName(q);
        if (ix != afl::data::NameMap::nil) {
            result = ix + NUM_PLANET_PROPERTIES;
            return true;
        }

        // Check predefined properties
        return interpreter::lookupName(q, planet_mapping, result);
    }

    class PlanetMethodValue : public interpreter::ProcedureValue {
     public:
        PlanetMethodValue(int id,
                          game::Session& session,
                          game::interface::PlanetMethod ipm,
                          afl::base::Ref<game::Root> root,
                          afl::base::Ref<game::Game> game,
                          afl::base::Ref<game::Turn> turn)
            : m_id(id),
              m_session(session),
              m_method(ipm),
              m_root(root),
              m_game(game),
              m_turn(turn)
            { }

        // ProcedureValue:
        virtual void call(interpreter::Process& proc, interpreter::Arguments& a)
            {
                if (game::map::Planet* pl = m_turn->universe().planets().get(m_id)) {
                    game::interface::callPlanetMethod(*pl, m_method, a, proc, m_session, m_game->mapConfiguration(), *m_turn, *m_root);
                }
            }

        virtual PlanetMethodValue* clone() const
            { return new PlanetMethodValue(m_id, m_session, m_method, m_root, m_game, m_turn); }

     private:
        game::Id_t m_id;
        game::Session& m_session;
        game::interface::PlanetMethod m_method;
        afl::base::Ref<game::Root> m_root;
        afl::base::Ref<game::Game> m_game;
        afl::base::Ref<game::Turn> m_turn;
    };
}

game::interface::PlanetContext::PlanetContext(int id,
                                              Session& session,
                                              afl::base::Ref<Root> root,
                                              afl::base::Ref<Game> game)
    : m_id(id),
      m_session(session),
      m_root(root),
      m_game(game)
{
    // ex IntPlanetContext::IntPlanetContext (sort-of)
    // FIXME: ShipContext takes and keeps a ship list. Should we do the same?
}

game::interface::PlanetContext::~PlanetContext()
{ }

// Context:
interpreter::Context::PropertyAccessor*
game::interface::PlanetContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    // ex planint.pas:CPlanetContext.ResolveValue
    if (name.startsWith("PLANET.")) {
        return lookupPlanetProperty(afl::data::NameQuery(name, 7), m_session.world(), result) ? this : 0;
    } else {
        return lookupPlanetProperty(name, m_session.world(), result) ? this : 0;
    }
}

void
game::interface::PlanetContext::set(PropertyIndex_t index, const afl::data::Value* value)
{
    // ex IntPlanetContext::set
    if (game::map::Planet* pl = getObject()) {
        if (index < NUM_PLANET_PROPERTIES) {
            // Builtin property
            switch (PlanetDomain(planet_mapping[index].domain)) {
             case PlanetPropertyDomain:
                setPlanetProperty(*pl, PlanetProperty(planet_mapping[index].index), value, *m_root);
                break;
             case BasePropertyDomain:
                setBaseProperty(*pl, BaseProperty(planet_mapping[index].index), value);
                break;
             case OwnerPropertyDomain:
             case PlanetMethodDomain:
                throw interpreter::Error::notAssignable();
            }
        } else {
            // User property
            if (afl::data::Segment* seg = m_session.world().planetProperties().create(m_id)) {
                seg->set(index - NUM_PLANET_PROPERTIES, value);
            }
            pl->markDirty();
        }
    } else {
        // Nonexistant ships will still have a Planet object, so this branch is only taken for out-of-range Ids
        throw interpreter::Error::notAssignable();
    }
}

afl::data::Value*
game::interface::PlanetContext::get(PropertyIndex_t index)
{
    // ex IntPlanetContext::get
    if (game::map::Planet* pl = getObject()) {
        if (index < NUM_PLANET_PROPERTIES) {
            // Builtin property
            int owner;
            switch (PlanetDomain(planet_mapping[index].domain)) {
             case PlanetPropertyDomain:
                return getPlanetProperty(*pl, PlanetProperty(planet_mapping[index].index),
                                         m_session,
                                         m_root,
                                         m_game);
             case BasePropertyDomain:
                return getBaseProperty(*pl, BaseProperty(planet_mapping[index].index),
                                       m_session.translator(),
                                       m_root->hostConfiguration(),
                                       m_session.getShipList(),
                                       &m_game->currentTurn());
             case OwnerPropertyDomain:
                if (pl->getOwner(owner)) {
                    return getPlayerProperty(owner, PlayerProperty(planet_mapping[index].index), m_root->playerList(), *m_game, m_root->hostConfiguration());
                } else {
                    return 0;
                }
             case PlanetMethodDomain:
                return new PlanetMethodValue(pl->getId(), m_session, PlanetMethod(planet_mapping[index].index), m_root, m_game, m_game->currentTurn());
            }
            return 0;
        } else {
            // User property
            return afl::data::Value::cloneOf(m_session.world().planetProperties().get(m_id, index - NUM_PLANET_PROPERTIES));
        }
    } else {
        // Nonexistant ships will still have a Planet object, so this branch is only taken for out-of-range Ids
        return 0;
    }
}

bool
game::interface::PlanetContext::next()
{
    // ex planint.pas:CPlanetContext.Next
    if (Id_t id = game::map::AnyPlanetType(m_game->currentTurn().universe().planets()).findNextIndex(m_id)) {
        m_id = id;
        return true;
    }
    return false;
}

game::interface::PlanetContext*
game::interface::PlanetContext::clone() const
{
    return new PlanetContext(*this);
}

game::map::Planet*
game::interface::PlanetContext::getObject()
{
    return m_game->currentTurn().universe().planets().get(m_id);
}

void
game::interface::PlanetContext::enumProperties(interpreter::PropertyAcceptor& acceptor)
{
    // ex IntPlanetContext::enumProperties
    acceptor.enumNames(m_session.world().planetPropertyNames());
    acceptor.enumTable(planet_mapping);
}

// BaseValue:
String_t
game::interface::PlanetContext::toString(bool /*readable*/) const
{
    // ex IntPlanetContext::toString
    return afl::string::Format("Planet(%d)", m_id);
}

void
game::interface::PlanetContext::store(interpreter::TagNode& out, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
{
    // ex IntPlanetContext::store
    out.tag = out.Tag_Planet;
    out.value = m_id;
}

game::interface::PlanetContext*
game::interface::PlanetContext::create(int id, Session& session)
{
    // ex planint.pas:CPlanetContext.Load (sort-of), planint.pas:CreatePlanetContext
    Game* game = session.getGame().get();
    Root* root = session.getRoot().get();
    if (game != 0 && root != 0 && game->currentTurn().universe().planets().get(id) != 0) {
        return new PlanetContext(id, session, *root, *game);
    } else {
        return 0;
    }
}
