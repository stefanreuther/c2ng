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
#include "game/interface/objectcommand.hpp"

namespace game { namespace interface {
    // FIXME: move to separate file
    enum PlanetMethod {
        ipmMark,                    // 0
        ipmUnmark,                  // 1
        ipmSetComment,              // 2
        ipmFixShip,                 // 3
        ipmRecycleShip,             // 4
        ipmBuildBase,               // 5
        ipmAutoBuild,               // 6
        ipmBuildDefense,            // 7
        ipmBuildFactories,          // 8
        ipmBuildMines,              // 9
        ipmSetColonistTax,          // 10
        ipmSetNativeTax,            // 11
        ipmSetFCode,                // 12
        ipmSetMission,              // 13
        ipmBuildBaseDefense,        // 14
        ipmSetTech,                 // 15
        ipmBuildFighters,           // 16
        ipmBuildEngines,            // 17
        ipmBuildHulls,              // 18
        ipmBuildLaunchers,          // 19
        ipmBuildBeams,              // 20
        ipmBuildTorps,              // 21
        ipmSellSupplies,            // 22
        ipmBuildShip,               // 23
        ipmCargoTransfer,           // 24
        ipmAutoTaxColonists,        // 25
        ipmAutoTaxNatives           // 26
    };
} }

namespace {
    enum PlanetDomain {
        PlanetPropertyDomain,
        BasePropertyDomain,
        OwnerPropertyDomain,
        PlanetMethodDomain
    };

    static const interpreter::NameTable planet_mapping[] = {
        // { "AUTOBUILD",                 ipmAutoBuild,         PlanetMethod,   interpreter::thProcedure },
        // { "AUTOTAXCOLONISTS",          ipmAutoTaxColonists,  PlanetMethod,   interpreter::thProcedure },
        // { "AUTOTAXNATIVES",            ipmAutoTaxNatives,    PlanetMethod,   interpreter::thProcedure },
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
        // { "BUILDBASE",                 ipmBuildBase,         PlanetMethod,   interpreter::thProcedure },
        // { "BUILDBASEDEFENSE",          ipmBuildBaseDefense,  PlanetMethod,   interpreter::thProcedure },
        // { "BUILDBEAMS",                ipmBuildBeams,        PlanetMethod,   interpreter::thProcedure },
        // { "BUILDDEFENSE",              ipmBuildDefense,      PlanetMethod,   interpreter::thProcedure },
        // { "BUILDENGINES",              ipmBuildEngines,      PlanetMethod,   interpreter::thProcedure },
        // { "BUILDFACTORIES",            ipmBuildFactories,    PlanetMethod,   interpreter::thProcedure },
        // { "BUILDFIGHTERS",             ipmBuildFighters,     PlanetMethod,   interpreter::thProcedure },
        // { "BUILDHULLS",                ipmBuildHulls,        PlanetMethod,   interpreter::thProcedure },
        // { "BUILDLAUNCHERS",            ipmBuildLaunchers,    PlanetMethod,   interpreter::thProcedure },
        // { "BUILDMINES",                ipmBuildMines,        PlanetMethod,   interpreter::thProcedure },
        // { "BUILDSHIP",                 ipmBuildShip,         PlanetMethod,   interpreter::thProcedure },
        // { "BUILDTORPS",                ipmBuildTorps,        PlanetMethod,   interpreter::thProcedure },
        // { "CARGOTRANSFER",             ipmCargoTransfer,     PlanetMethod,   interpreter::thProcedure },
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
        { "DEFENSE.BASE.WANT",         game::interface::ippBaseDefenseWanted, PlanetPropertyDomain, interpreter::thInt },
        { "DEFENSE.MAX",               game::interface::ippDefenseMax,        PlanetPropertyDomain, interpreter::thInt },
        { "DEFENSE.WANT",              game::interface::ippDefenseWanted,     PlanetPropertyDomain, interpreter::thInt },
        { "DENSITY.D",                 game::interface::ippDensityD,          PlanetPropertyDomain, interpreter::thInt },
        { "DENSITY.M",                 game::interface::ippDensityM,          PlanetPropertyDomain, interpreter::thInt },
        { "DENSITY.N",                 game::interface::ippDensityN,          PlanetPropertyDomain, interpreter::thInt },
        { "DENSITY.T",                 game::interface::ippDensityT,          PlanetPropertyDomain, interpreter::thInt },
        { "FACTORIES",                 game::interface::ippFactories,         PlanetPropertyDomain, interpreter::thInt },
        { "FACTORIES.MAX",             game::interface::ippFactoriesMax,      PlanetPropertyDomain, interpreter::thInt },
        { "FACTORIES.WANT",            game::interface::ippFactoriesWanted,   PlanetPropertyDomain, interpreter::thInt },
        { "FCODE",                     game::interface::ippFCode,             PlanetPropertyDomain, interpreter::thString },
        { "FIGHTERS",                  game::interface::ibpBaseFighters,      BasePropertyDomain,   interpreter::thInt },
        { "FIGHTERS.MAX",              game::interface::ibpBaseFightersMax,   BasePropertyDomain,   interpreter::thInt },
        // { "FIXSHIP",                   ipmFixShip,           PlanetMethod,   interpreter::thProcedure },
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
        { "MINED.D",                   game::interface::ippMinedD,            PlanetPropertyDomain, interpreter::thInt },
        { "MINED.M",                   game::interface::ippMinedM,            PlanetPropertyDomain, interpreter::thInt },
        { "MINED.N",                   game::interface::ippMinedN,            PlanetPropertyDomain, interpreter::thInt },
        { "MINED.STR",                 game::interface::ippMinedStr,          PlanetPropertyDomain, interpreter::thString},
        { "MINED.T",                   game::interface::ippMinedT,            PlanetPropertyDomain, interpreter::thInt },
        { "MINES",                     game::interface::ippMines,             PlanetPropertyDomain, interpreter::thInt },
        { "MINES.MAX",                 game::interface::ippMinesMax,          PlanetPropertyDomain, interpreter::thInt },
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
        // { "RECYCLESHIP",               ipmRecycleShip,       PlanetMethod,   interpreter::thProcedure },
        { "SCORE",                     game::interface::ippScore,             PlanetPropertyDomain, interpreter::thArray },
        // { "SELLSUPPLIES",              ipmSellSupplies,      PlanetMethod,   interpreter::thProcedure },
        // { "SETCOLONISTTAX",            ipmSetColonistTax,    PlanetMethod,   interpreter::thProcedure },
        // { "SETCOMMENT",                ipmSetComment,        PlanetMethod,   interpreter::thProcedure },
        // { "SETFCODE",                  ipmSetFCode,          PlanetMethod,   interpreter::thProcedure },
        // { "SETMISSION",                ipmSetMission,        PlanetMethod,   interpreter::thProcedure },
        // { "SETNATIVETAX",              ipmSetNativeTax,      PlanetMethod,   interpreter::thProcedure },
        // { "SETTECH",                   ipmSetTech,           PlanetMethod,   interpreter::thProcedure },
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

    const game::interface::ObjectCommand::Function_t PLANET_METHODS[] = {
        game::interface::IFObjMark,                  // 0
        game::interface::IFObjUnmark,                // 1
        // IFPlanetSetComment,         // 2
        // IFBaseFixShip,              // 3
        // IFBaseRecycleShip,          // 4
        // IFPlanetBuildBase,          // 5
        // IFPlanetAutoBuild,          // 6
        // IFPlanetBuildDefense,       // 7
        // IFPlanetBuildFactories,     // 8
        // IFPlanetBuildMines,         // 9
        // IFPlanetSetColonistTax,     // 10
        // IFPlanetSetNativeTax,       // 11
        // IFPlanetSetFCode,           // 12
        // IFBaseSetMission,           // 13
        // IFPlanetBuildBaseDefense,   // 14
        // IFBaseSetTech,              // 15
        // IFBaseBuildFighters,        // 16
        // IFBaseBuildEngines,         // 17
        // IFBaseBuildHulls,           // 18
        // IFBaseBuildLaunchers,       // 19
        // IFBaseBuildBeams,           // 20
        // IFBaseBuildTorps,           // 21
        // IFPlanetSellSupplies,       // 22
        // IFBaseBuildShip,            // 23
        // IFPlanetCargoTransfer,      // 24
        // IFPlanetAutoTaxColonists,   // 25
        // IFPlanetAutoTaxNatives,     // 26
    };

}

game::interface::PlanetContext::PlanetContext(int id,
                                              Session& session,
                                              afl::base::Ptr<Root> root,
                                              afl::base::Ptr<Game> game)
    : m_id(id),
      m_session(session),
      m_root(root),
      m_game(game)
{
    // ex IntPlanetContext::IntPlanetContext (sort-of)
}

game::interface::PlanetContext::~PlanetContext()
{ }

// Context:
bool
game::interface::PlanetContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    if (name.startsWith("PLANET.")) {
        return lookupPlanetProperty(afl::data::NameQuery(name, 7), m_session.world(), result);
    } else {
        return lookupPlanetProperty(name, m_session.world(), result);
    }
}

void
game::interface::PlanetContext::set(PropertyIndex_t index, afl::data::Value* value)
{
    // ex IntPlanetContext::set
    if (game::map::Planet* pl = getObject()) {
        if (index < NUM_PLANET_PROPERTIES) {
            // Builtin property
            switch (PlanetDomain(planet_mapping[index].domain)) {
             case PlanetPropertyDomain:
        //     setPlanetProperty(*pl, IntPlanetProperty(planet_mapping[index].index), value);
        //     break;
             case BasePropertyDomain:
        //     setBaseProperty(*pl, IntBaseProperty(planet_mapping[index].index), value);
        //     break;
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
            if (Root* root = m_root.get()) {
                switch (PlanetDomain(planet_mapping[index].domain)) {
                 case PlanetPropertyDomain:
                    return getPlanetProperty(*pl, PlanetProperty(planet_mapping[index].index),
                                             m_session.translator(),
                                             root->hostVersion(),
                                             root->hostConfiguration(),
                                             m_session.interface(),
                                             m_game);
                 case BasePropertyDomain:
                    if (const game::spec::ShipList* list = m_session.getShipList().get()) {
                        return getBaseProperty(*pl, BaseProperty(planet_mapping[index].index),
                                               m_session.translator(),
                                               root->hostConfiguration(),
                                               *list,
                                               m_session.interface(),
                                               m_game);
                    } else {
                        return 0;
                    }
                 case OwnerPropertyDomain:
                    if (const Game* game = m_game.get()) {
                        int owner;
                        if (pl->getOwner(owner)) {
                            return getPlayerProperty(owner, PlayerProperty(planet_mapping[index].index), root->playerList(), *game, root->hostConfiguration());
                        } else {
                            return 0;
                        }
                    } else {
                        return 0;
                    }
                 case PlanetMethodDomain:
                    return new ObjectCommand(m_session, *pl, PLANET_METHODS[planet_mapping[index].index]);
                }
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
    if (Game* game = m_game.get()) {
        if (int id = game::map::AnyPlanetType(game->currentTurn().universe()).findNextIndex(m_id)) {
            m_id = id;
            return true;
        }
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
    if (Game* game = m_game.get()) {
        return game->currentTurn().universe().planets().get(m_id);
    } else {
        return 0;
    }
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
game::interface::PlanetContext::store(interpreter::TagNode& out, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext* /*ctx*/) const
{
    // ex IntPlanetContext::store
    out.tag = out.Tag_Planet;
    out.value = m_id;
}

