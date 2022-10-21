/**
  *  \file client/si/commands.hpp
  *  \brief Script Commands
  */
#ifndef C2NG_CLIENT_SI_COMMANDS_HPP
#define C2NG_CLIENT_SI_COMMANDS_HPP

#include "game/session.hpp"
#include "interpreter/arguments.hpp"

namespace client { namespace si {

    class UserSide;
    class ScriptSide;
    class RequestLink1;

    /*
     *  Miscellaneous Commands
     *
     *  These commands are stable and documented.
     */

    void IFLoadResource(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFLoadHelpFile(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFMessageBox(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFSystemExitClient(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFSystemExitRace(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);

    /*
     *  Internal Commands (CC$Whatever)
     *
     *  Those commands are used internally.
     *  They usually have an ad-hoc interface and are subject to change.
     */

    void IFCCAddToSim(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCAddWaypoint(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCBuildAmmo(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCBuildBase(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCBuildShip(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCBuildStructures(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCBuySupplies(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCCargoHistory(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCCloneShip(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCChangePassword(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCChangeSpeed(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCChangeTaxes(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCChangeTech(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCChangeWaypoint(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCChooseInterceptTarget(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCEditAutobuildSettings(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCEditBackup(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCEditCommands(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCEditCurrentBuildOrder(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCEditLabelConfig(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCEditNewBuildOrder(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCEditShowCommand(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCExport(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCGlobalActions(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCGotoCoordinates(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCIonStormInfo(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCImperialStats(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCListScreenHistory(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCManageBuildQueue(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCMinefieldInfo(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCPopScreenHistory(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCProcessManager(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCReset(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCSellSupplies(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCSendMessage(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCSettings(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCShipCostCalc(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCShipSpec(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCSpecBrowser(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCStarchartConfig(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCTransferMulti(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCTransferPlanet(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCTransferShip(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCTransferUnload(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCUfoInfo(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCUseKeymap(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCViewCombat(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCViewInbox(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCViewMessages(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFCCViewNotifications(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);

    /*
     *  User-Interface Commands (Chart.Xxxx, UI.Xxxx)
     *
     *  These commands are stable and documented.
     *
     *  In particular, the UI.Xxxx commands have the convention of returning their result in UI.Result.
     */

    void IFChartSetView(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);

    void IFUIBattleSimulator(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFUIChooseObject(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFUIChooseTurn(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFUIEditAlliances(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFUIEditTeams(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFUIEndDialog(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFUIFileWindow(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFUIGotoChart(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFUIGotoScreen(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFUIHelp(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFUIInput(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFUIInputCommand(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFUIInputFCode(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFUIInputNumber(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFUIKeymapInfo(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFUIListFleets(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFUIListShipPrediction(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFUIListShips(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFUIMessage(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFUIOverlayMessage(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFUIPlanetInfo(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFUIPopupConsole(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFUIScanKeyboardMode(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFUISelectionManager(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFUISearch(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFUIShowScores(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
    void IFUIUpdate(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);

    /** Register script commands.
        Registers all client/user-interface based commands.
        @param ui UserSide instance */
    void registerCommands(UserSide& ui);

} }

#endif
