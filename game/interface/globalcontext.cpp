/**
  *  \file game/interface/globalcontext.cpp
  */

#include "game/interface/globalcontext.hpp"
#include "game/interface/globalproperty.hpp"
#include "afl/base/countof.hpp"
#include "interpreter/nametable.hpp"
#include "interpreter/typehint.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/error.hpp"
#include "game/game.hpp"
#include "game/interface/playerproperty.hpp"
#include "game/root.hpp"
#include "game/interface/userinterfaceproperty.hpp"
#include "game/interface/userinterfacepropertyaccessor.hpp"

namespace {
    enum GlobalDomain {
        GlobalPropertyDomain,
        MyPlayerPropertyDomain,
        UIPropertyDomain
    };

    static const interpreter::NameTable global_mapping[] = {
        /* @q Chart.X:Int (Global Property), Chart.Y:Int (Global Property)
           Current position in the starchart.
           - on the starchart (and player screen): center of starchart.
           Values can be assigned to modify the current position.
           - on control screens: center of scanner (=location of current unit).
           Cannot be modified.
           @diff PCC 1.x always used the "on the starchart" interpretation, i.e. these values
           give a starchart position that may not have anything to do with the current unit's
           position the user actually sees.
           @assignable
           @see UI.GotoChart
           @since PCC2 1.99.10, PCC 1.0.16 */
        { "CHART.X",               game::interface::iuiChartX,            UIPropertyDomain,       interpreter::thInt },
        { "CHART.Y",               game::interface::iuiChartY,            UIPropertyDomain,       interpreter::thInt },
        { "MY.BASES",              game::interface::iplScoreBases,        MyPlayerPropertyDomain, interpreter::thInt },
        { "MY.INMSGS",             game::interface::igpMyInMsgs,          GlobalPropertyDomain,   interpreter::thInt },
        // { "MY.OUTMSGS",            game::interface::igpMyOutMsgs,         GlobalPropertyDomain,   interpreter::thInt },
        { "MY.PBPS",               game::interface::iplPBPs,              MyPlayerPropertyDomain, interpreter::thInt },
        { "MY.PLANETS",            game::interface::iplScorePlanets,      MyPlayerPropertyDomain, interpreter::thInt },
        { "MY.RACE",               game::interface::iplShortName,         MyPlayerPropertyDomain, interpreter::thString },
        { "MY.RACE$",              game::interface::iplId,                MyPlayerPropertyDomain, interpreter::thInt },
        { "MY.RACE.ADJ",           game::interface::iplAdjName,           MyPlayerPropertyDomain, interpreter::thString },
        { "MY.RACE.FULL",          game::interface::iplFullName,          MyPlayerPropertyDomain, interpreter::thString },
        { "MY.RACE.ID",            game::interface::iplRaceId,            MyPlayerPropertyDomain, interpreter::thInt },
        { "MY.RACE.MISSION",       game::interface::iplMission,           MyPlayerPropertyDomain, interpreter::thInt },
        { "MY.SCORE",              game::interface::iplScore,             MyPlayerPropertyDomain, interpreter::thInt },
        { "MY.SHIPS",              game::interface::iplScoreShips,        MyPlayerPropertyDomain, interpreter::thInt },
        { "MY.SHIPS.CAPITAL",      game::interface::iplScoreCapital,      MyPlayerPropertyDomain, interpreter::thInt },
        { "MY.SHIPS.FREIGHTERS",   game::interface::iplScoreFreighters,   MyPlayerPropertyDomain, interpreter::thInt },
        { "MY.TEAM",               game::interface::iplTeam,              MyPlayerPropertyDomain, interpreter::thInt },
        { "MY.VCRS",               game::interface::igpMyVCRs,            GlobalPropertyDomain,   interpreter::thInt },
        { "SELECTION.LAYER",       game::interface::igpSelectionLayer,    GlobalPropertyDomain,   interpreter::thInt },
        { "SHIPS.CAPITAL",         game::interface::iplTotalCapital,      MyPlayerPropertyDomain, interpreter::thInt },  // Implemented as player properties because
        { "SHIPS.FREIGHTERS",      game::interface::iplTotalFreighters,   MyPlayerPropertyDomain, interpreter::thInt },  // that has easier access to a 'TGen'. A better
        { "SHIPS.TOTAL",           game::interface::iplTotalShips,        MyPlayerPropertyDomain, interpreter::thInt },  // way may be to implement it using GStatFile.
        // SYSTEM.EGG
        // SYSTEM.ERR -> global variable (localizable!)
        { "SYSTEM.GAMEDIRECTORY",  game::interface::igpGameDirectory,     GlobalPropertyDomain,   interpreter::thString },
        { "SYSTEM.GAMETYPE",       game::interface::igpRegSharewareText,  GlobalPropertyDomain,   interpreter::thString },
        { "SYSTEM.GAMETYPE$",      game::interface::igpRegSharewareFlag,  GlobalPropertyDomain,   interpreter::thInt },
        // SYSTEM.GUI -> conif, guiif
        { "SYSTEM.HOST",           game::interface::igpSystemHost,        GlobalPropertyDomain,   interpreter::thString },
        { "SYSTEM.HOST$",          game::interface::igpSystemHostCode,    GlobalPropertyDomain,   interpreter::thInt },
        { "SYSTEM.HOSTVERSION",    game::interface::igpSystemHostVersion, GlobalPropertyDomain,   interpreter::thString },
        { "SYSTEM.LANGUAGE",       game::interface::igpSystemLanguage,    GlobalPropertyDomain,   interpreter::thString },
        { "SYSTEM.LOCAL",          game::interface::igpFileFormatLocal,   GlobalPropertyDomain,   interpreter::thInt },
        { "SYSTEM.PROGRAM",        game::interface::igpSystemProgram,     GlobalPropertyDomain,   interpreter::thString },
        { "SYSTEM.RANDOMSEED",     game::interface::igpRandomSeed,        GlobalPropertyDomain,   interpreter::thInt },
        { "SYSTEM.REGSTR1",        game::interface::igpRegStr1,           GlobalPropertyDomain,   interpreter::thString },
        { "SYSTEM.REGSTR2",        game::interface::igpRegStr2,           GlobalPropertyDomain,   interpreter::thString },
        { "SYSTEM.REMOTE",         game::interface::igpFileFormatRemote,  GlobalPropertyDomain,   interpreter::thInt },
        { "SYSTEM.ROOTDIRECTORY",  game::interface::igpRootDirectory,     GlobalPropertyDomain,   interpreter::thString },
        /* @q System.Sim:Bool (Global Property)
           True if the combat simulator is currently in use, otherwise false.
           @since PCC2 1.99.10, PCC 1.0.9 */
        { "SYSTEM.SIM",            game::interface::iuiSimFlag,           UIPropertyDomain,       interpreter::thBool },
        { "SYSTEM.VERSION",        game::interface::igpSystemVersion,     GlobalPropertyDomain,   interpreter::thString },
        { "SYSTEM.VERSION$",       game::interface::igpSystemVersionCode, GlobalPropertyDomain,   interpreter::thInt },
        { "TURN",                  game::interface::igpTurnNumber,        GlobalPropertyDomain,   interpreter::thInt },
        { "TURN.DATE",             game::interface::igpTurnDate,          GlobalPropertyDomain,   interpreter::thString },
        { "TURN.ISNEW",            game::interface::igpTurnIsNew,         GlobalPropertyDomain,   interpreter::thBool },
        { "TURN.TIME",             game::interface::igpTurnTime,          GlobalPropertyDomain,   interpreter::thString },
        /* @q UI.Iterator:Iterator (Global Property)
           Iterator controlling current screen or dialog.
           EMPTY if the current screen has no iterator.
           @since PCC2 2.40 */
        { "UI.ITERATOR",           game::interface::iuiIterator,          UIPropertyDomain,       interpreter::thNone },
        /* @q UI.Screen:Int (Global Property)
           Number of current screen. See {UI.GotoScreen} for a list.
           0 if no control screen is active.
           @see UI.GotoScreen
           @since PCC2 1.99.10, PCC 1.0.14 */
        { "UI.SCREEN",             game::interface::iuiScreenNumber,      UIPropertyDomain,       interpreter::thInt },
        /* @q UI.X:Int (Global Property), UI.Y:Int (Global Property)
           Scanner position.
           - on the starchart (and player screen): same as {Chart.X}, {Chart.Y}.
           - on control screens: position of the scanner. Changing the values moves the scanner.
           @assignable
           @since PCC2 1.99.10, PCC 1.0.14 */
        { "UI.X",                  game::interface::iuiScanX,             UIPropertyDomain,       interpreter::thInt },
        { "UI.Y",                  game::interface::iuiScanY,             UIPropertyDomain,       interpreter::thInt },
    };

    const size_t NUM_GLOBAL_PROPERTIES = countof(global_mapping);

    bool lookupGlobalProperty(const afl::data::NameQuery& name, interpreter::Context::PropertyIndex_t& result, game::Session& session)
    {
        // ex int/if/globalif.cc:lookupGlobalProperty
        afl::data::NameMap::Index_t ix = session.world().globalPropertyNames().getIndexByName(name);
        if (ix != afl::data::NameMap::nil) {
            result = ix + NUM_GLOBAL_PROPERTIES;
            return true;
        }

        return lookupName(name, global_mapping, result);
    }
}

game::interface::GlobalContext::GlobalContext(Session& session)
    : m_session(session)
{ }

game::interface::GlobalContext::~GlobalContext()
{ }

// Context:
game::interface::GlobalContext*
game::interface::GlobalContext::lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
{
    // ex IntGlobalContext::lookup
    if (name.startsWith("GLOBAL.")) {
        return lookupGlobalProperty(afl::data::NameQuery(name, 7), result, m_session) ? this : 0;
    } else {
        return lookupGlobalProperty(name, result, m_session) ? this : 0;
    }
}

void
game::interface::GlobalContext::set(PropertyIndex_t index, afl::data::Value* value)
{
    // ex IntGlobalContext::set
    if (index >= NUM_GLOBAL_PROPERTIES) {
        // User variable
        m_session.world().globalValues().set(index - NUM_GLOBAL_PROPERTIES, value);
    } else {
        // Global property
        switch (GlobalDomain(global_mapping[index].domain)) {
         case GlobalPropertyDomain:
            setGlobalProperty(GlobalProperty(global_mapping[index].index), m_session, value);
            break;
         case UIPropertyDomain:
            m_session.uiPropertyStack().set(UserInterfaceProperty(global_mapping[index].index), value);
            break;
         default:
            throw interpreter::Error::notAssignable();
        }
    }
}

afl::data::Value*
game::interface::GlobalContext::get(PropertyIndex_t index)
{
    // ex IntGlobalContext::get
    if (index >= NUM_GLOBAL_PROPERTIES) {
        return afl::data::Value::cloneOf(m_session.world().globalValues()[index - NUM_GLOBAL_PROPERTIES]);
    } else {
        // Check global properties
        switch (GlobalDomain(global_mapping[index].domain)) {
         case GlobalPropertyDomain:
            return getGlobalProperty(GlobalProperty(global_mapping[index].index), m_session);
         case MyPlayerPropertyDomain: {
            Game* game = m_session.getGame().get();
            Root* root = m_session.getRoot().get();
            if (game != 0 && root != 0) {
                return getPlayerProperty(game->getViewpointPlayer(),
                                         PlayerProperty(global_mapping[index].index),
                                         root->playerList(),
                                         *game,
                                         root->hostConfiguration());
            } else {
                return 0;
            }
         }
                                         
         case UIPropertyDomain:
            return m_session.uiPropertyStack().get(UserInterfaceProperty(global_mapping[index].index));
        }
        return 0;
    }
}

game::interface::GlobalContext*
game::interface::GlobalContext::clone() const
{
    return new GlobalContext(m_session);
}

game::map::Object*
game::interface::GlobalContext::getObject()
{
    // ex IntGlobalContext::getObject
    return 0;
}

void
game::interface::GlobalContext::enumProperties(interpreter::PropertyAcceptor& acceptor)
{
    // ex IntGlobalContext::enumProperties
    acceptor.enumNames(m_session.world().globalPropertyNames());
    acceptor.enumTable(global_mapping);
}


// BaseValue:
String_t
game::interface::GlobalContext::toString(bool /*readable*/) const
{
    // ex IntGlobalContext::toString
    return "#<global>";
}

void
game::interface::GlobalContext::store(interpreter::TagNode& out, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, interpreter::SaveContext& /*ctx*/) const
{
    // ex IntGlobalContext::store
    out.tag = out.Tag_Global;
    out.value = 0;
}
