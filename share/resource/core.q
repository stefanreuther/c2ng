%
%   PCC2 Standard Library
%
%   This file defines commands and keys used internally by PCC2ng,
%   and a number of user-callable routines described in the
%   manual. Do not modify unless you are absolutely 100% positively
%   sure what you are doing.
%
%   This file will be overwritten by PCC2 upgrades. It will most
%   likely only work with the PCC2 version it was intended for.
%
%   This version of core.q is for PCC2ng, and is incompatible with
%   PCC 1.x or PCC2, and vice versa.
%

%Option Optimize(2), Lexical

% @q CC$LibraryVersion:Str (Internal)
% Version of the standard library (<tt>core.q</tt>).
Dim Shared CC$LibraryVersion = '2.40.13'

%%% Console-Mode Replacements for GUI routines %%%%%%%%%%%%%%%%%%%%%%

If Not System.GUI Then
  % c2ng does not need replacements for rich-text operations here.

  %%% Utilities

  % Return the index of the first word in list which starts with the given word.
  Function CC$MatchWord(list, word)
    Local i:=1, e
    Do While list<>''
      e := First(' ', list)
      If Mid(e,1,Len(word)) = word Then Return i
      i := i + 1
      list := Trim(Rest(' ', list))
    Loop
  EndFunction

  % Count number of words in list
  Function CC$CountWords(list)
    Local i:=0
    Do While list<>''
      i:=i+1
      list:=Trim(Rest(' ', list))
    Loop
    Return i
  EndFunction

  %%% Replacements for client/si/commands.cc

  % LoadResource has no effect
  Sub LoadResource(name)
  EndSub

  % MessageBox generates a console message.
  Sub MessageBox(text, Optional heading)
    % FIXME: PCC 1.x replaces \r by \n in text.
    If IsEmpty(heading) Then heading := Translate("Message")
    Print heading, ": ", text
  EndSub

  %  void IFSystemExitClient(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);
  %  void IFSystemExitRace(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);

  Sub UI.ChooseObject(screen)
    Abort "Not in graphics mode"
  EndSub

  Sub UI.ChooseTurn(Optional delta)
    Abort "Not in graphics mode"
  EndSub

  Sub UI.EndDialog(Optional code)
    Abort "Not in graphics mode"
  EndSub

  Sub UI.GotoScreen (sid, Optional oid)
    Abort "Not in graphics mode"
  EndSub

  % UI.Input is implemented in game/interface/consolecommands.cpp.

  % UI.Message generates a console message and a prompt
  Sub UI.Message (text, Optional heading, buttons)
    MessageBox text, Heading
    If IsEmpty(buttons) Then buttons := Translate("OK")
    Do
      UI.Input buttons
      If IsEmpty(UI.Result) Then
        % This means the last choice
        UI.Result := Z(CC$CountWords(buttons))
        Break
      Else If UI.Result = "" Then
        % This means the first choice
        UI.Result := 1
        Break
      Else
        % Does this match any button?
        UI.Result := CC$MatchWord(buttons, UI.Result)
        If Not IsEmpty(UI.Result) Then Break
      EndIf
    Loop
  EndSub

  % No effect in console mode
  Sub UI.PopupConsole
  EndSub
EndIf

% Create an array on-the-fly
% @since PCC2 2.40.1
Function Array(a())
  Return a
EndFunction

% Utility: append value to an array.
% @since PCC2 2.40.1
% (also in namer.q since 1.99.22)
Sub Array.Push(a, val)
  ReDim a(Dim(a)+1)
  a(Dim(a)-1) := val
EndSub

% Utility: pop value from an array.
% @since PCC2 2.40.1
Function Array.Pop(a)
  Local result
  If Dim(a) > 0
    result := a(Dim(a)-1)
    ReDim a(Dim(a)-1)
  EndIf
  Return result
EndFunction


%%% User-callable Subroutines %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

Load "core_game.q"


%%% GUI Subroutines %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

If System.GUI Then
  %% When the GUI is not active, don't define all these to save memory
  Load "core_tiles.q"
  Load "core_ui.q"
EndIf % System.GUI


%%% Task Support %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% @q Notify s:Str, Optional t:Int (Global Command)
% Notify message.
% This command stops execution of an auto task, and informs the player of an event using the message text %s.
% These messages are displayed in the notification message window,
% from which users can resume execution of an auto task, or stop it.
% The text will be word-wrapped to fit in the window;
% it can also contain carriage returns to force line breaks.
%
% The message window will automatically include the name of the object the
% auto task belongs to, so the message need not repeat that. For example,
% <pre class="ccscript">
%   Notify "out of fuel"
% </pre>
% will generate the following message:
% <pre>
%   (-s0124)<<< Notify >>>
%   FROM: Auto Task Ship #124
%
%   out of fuel
% </pre>
%
% When the turn number %t is specified, the message will pop up in the specified turn
% (and execution of the calling script will suspend that long),
% otherwise it will be sent immediately.
%
% A message sent using %Notify will remain active until it is confirmed.
% If the player does not confirm a message during this PCC session, it will pop up again in the next session.
% @see AddNotify
% @since PCC2 1.99.16, PCC 1.0.18, PCC2 2.40.6
Sub Notify (s, Optional t)
  % This is implemented totally different from PCC 1.x.
  % This command initially appeared in 1.0.16, where it was morally equivalent to 'Print'.
  If Not IsEmpty(t) Then
    Do While Turn < t
      Stop
    Loop
  EndIf
  Do
    CC$Notify s, True
    Stop
  Loop While Not CC$NotifyConfirmed()
EndSub

% @q AddNotify s:Str (Global Command)
% Notify message.
% This command informs the player of an event using the message text %s.
% These messages are displayed in the notification message window.
% The text will be word-wrapped to fit in the window;
% it can also contain carriage returns to force line breaks.
%
% The message window will automatically include the name of the object the
% auto task belongs to, so the message need not repeat that. For example,
% <pre class="ccscript">
%   AddNotify "out of fuel"
% </pre>
% will generate the following message:
% <pre>
%   (-s0124)<<< Notify >>>
%   FROM: Auto Task Ship #124
%
%   out of fuel
% </pre>
%
% Messages sent using %AddNotify will only be visible for the PCC session in which the command was used.
% Unlike %Notify, execution of the script will automatically continue after the command,
% the message needs not be confirmed by the user.
% @see Notify
% @since PCC2 1.99.16, PCC 1.1.16, PCC2 2.40.6
Sub AddNotify (s)
  CC$Notify s, False
EndSub

% @q CC$AutoReCheck:void (Internal)
% Part of the implementation of {Restart}. A call of this routine is generated before the loop jump.
% This routine makes sure we do not enter infinite loops.
% This routine is used by PCC2/c2ng auto tasks and therefore needs to be present in both for interoperability.
Sub CC$AutoReCheck
  % Create process-local variable upon first entry into this routine
  Dim Static CC$AutoReState
  % Error when we're getting here again with a turn number we've already seen
  If CC$AutoReState = Turn
    Notify Translate("This Auto Task performed a \"Restart\" loop without intervening delays. You should probably use \"WaitOneTurn\".")
    CC$AutoReState := 0
  Else
    CC$AutoReState := Turn
  EndIf
EndSub

% @q CC$AutoExec cmd:Str (Internal)
% Execute auto task command. An auto task command <tt>foo</tt> is
% actually coded as <tt>CC$AutoExec "foo"</tt>. This implements error handling.
% This routine is used by PCC2/c2ng auto tasks and therefore needs to be present in both for interoperability.
Sub CC$AutoExec(cmd)
  Do
    Try
      Eval cmd
      Return
    Else
      Notify Translate("Error during execution of auto task: ") + System.Err
    EndTry
  Loop
EndSub


%%% Internals

% c2ng internal: This function is used to execute commands bound to keys.
% @since PCC2 2.40
Sub C2$Eval(code, UI.Prefix, UI.Key)
  Eval AtomStr(code)
EndSub

% c2ng internal: Game setup part 1
% Run as high-priority process during startup.
% @since PCC2 2.40.1
Sub C2$RunLoadHook
  % ex cc.pas:InitVM
  % Internal initialisation
  % FIXME: use actual Chart.Geo configuration
  Chart.X := 2000
  Chart.Y := 2000

  % Hooks
  RunHook Load
  If Turn.IsNew Then RunHook NewTurn
EndSub

% c2ng internal: Game setup
% Run as low-priority process during startup.
% @since PCC2 2.40.10
Sub C2$ShowInitialNotifications
  If CC$NumNotifications() <> 0 Then CC$ViewNotifications
EndSub

%%% Initialisation %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Properties.
CreateShipProperty   Comment
CreatePlanetProperty Comment

% Global Variables (System.Err is referenced by the interpreter core and therefore defined there).

% @q UI.Result:Str (Global Variable)
% Result of last user-interface operation.
% Operations like {UI.Message} store their result in a variable %UI.Result.
% If a local variable %UI.Result is visible, the result is stored in that instead of the global one.
% It is therefore a good idea to define a local variable to capture results without disturbing
% or being disturbed by other scripts.
% @assignable
Dim Shared UI.Result

% @q UI.Key:Str (Global Variable)
% Key that invoked the last user-interface operation.
% If an operation is invoked from a keymap, the invoking key is stored here.
% This variable is used by {UI.Menu} to anchor the menu.
% @assignable
% @since PCC2 2.40.8
Dim Shared UI.Key

% @q UI.Directory:Str (Global Variable)
% Current directory.
% This is the default directory for operations like {UI.FileWindow}.
% @assignable
Dim Shared UI.Directory

% @q UI.Prefix:Int (Global Variable)
% Current prefix argument.
% In keyboard actions (commands bound to a key using {Bind}),
% this variable contains the current <a href="prefixarg">prefix argument</a> (0 if none).
% @assignable
Dim Shared UI.Prefix

% @q Cargo.Remainder:Cargo (Global Variable)
% Remaining cargo after partial operation.
% If an operations like {CargoUnload} cannot finish completely, because of lacking cargo or cargo room,
% it can be told to do as much as possible instead of failing using the <tt>"n"</tt> flag.
% In this case, it stores the amount of unprocessed cargo in this variable.
%
% If a local variable %Cargo.Remainder is visible, the result is stored in that instead of the global one.
% It is therefore a good idea to define a local variable to capture results without disturbing
% or being disturbed by other scripts.
% @assignable
Dim Shared Cargo.Remainder

% @q Build.Remainder:Int (Global Variable)
% Remaining amount after partial build.
% If an operation like {BuildFactories} cannot finish completely, because of lacking resources or room,
% it can be told to build as much as possible instead of failing using the <tt>"n"</tt> flag.
% In this case, it stores the amount of structures not built in this variable.
%
% If a local variable %Build.Remainder is visible, the result is stored in that instead of the global one.
% It is therefore a good idea to define a local variable to capture results without disturbing
% or being disturbed by other scripts.
% @assignable
Dim Shared Build.Remainder

% Keymaps. Note that those differ from PCC 1.x because we support multiple inheritance.
CreateKeymap Global, Ship, Planet, Base, Fleet

CreateKeymap ControlScreen(Global)
CreateKeymap ShipScreen(Ship, ControlScreen)
CreateKeymap PlanetScreen(Planet, ControlScreen)
CreateKeymap BaseScreen(Base, ControlScreen)
CreateKeymap FleetScreen(Fleet, ControlScreen)
CreateKeymap HistoryScreen(Ship, ControlScreen)
CreateKeymap AutoTaskScreen(ControlScreen)
CreateKeymap ShipTaskScreen(AutoTaskScreen)
CreateKeymap PlanetTaskScreen(AutoTaskScreen)
CreateKeymap BaseTaskScreen(AutoTaskScreen)

CreateKeymap Starchart(Global)
CreateKeymap ShipLock(Ship, Starchart)
CreateKeymap PlanetLock(Planet, Starchart)
CreateKeymap UnknownPlanetLock(PlanetLock)
CreateKeymap BaseLock(Base, Starchart)

CreateKeymap RaceScreen(Global)
CreateKeymap ShipBuildScreen(Global)

CreateKeymap SelectionDialog(Global)
CreateKeymap ShipSelectionDialog(SelectionDialog)
CreateKeymap PlanetSelectionDialog(SelectionDialog)
CreateKeymap BaseSelectionDialog(SelectionDialog)
CreateKeymap FleetSelectionDialog(SelectionDialog)

% Global Bindings
Bind Global           "a-c"    := "UI.PopupConsole"
Bind Global           "a-k"    := "UI.KeymapInfo"
Bind Global           "quit"   := "CCUI.ExitClient"
Bind Global           "a-up"   := "CCUI.History.PreviousTurn"
Bind Global           "a-down" := "CCUI.History.NextTurn"
Bind Global           "bs"     := "CC$PopScreenHistory"
Bind Global           "a-bs"   := "CC$ListScreenHistory"
Bind Global           "a-."    := "UI.SelectionManager"
Bind Global           "a-left" := "CC$PreviousSelection"
Bind Global           "a-right" := "CC$NextSelection"

% Control Screen Bindings
Bind ControlScreen    "."      := "CCUI.ToggleSelection"
Bind ControlScreen    "esc"    := "UI.GotoScreen 0"
Bind ControlScreen    "pgup"   := "CCUI.SelectPrevious",       "wheelup"   := "CCUI.SelectPrevious",       "-"   := "CCUI.SelectPrevious"
Bind ControlScreen    "pgdn"   := "CCUI.SelectNext",           "wheeldn"   := "CCUI.SelectNext",           "+"   := "CCUI.SelectNext"
Bind ControlScreen    "c-pgup" := "CCUI.SelectPreviousMarked", "c-wheelup" := "CCUI.SelectPreviousMarked", "c--" := "CCUI.SelectPreviousMarked"
Bind ControlScreen    "c-pgdn" := "CCUI.SelectNextMarked",     "c-wheeldn" := "CCUI.SelectNextMarked",     "c-+" := "CCUI.SelectNextMarked"
Bind ControlScreen    "f1"     := "CCUI.GotoScreen 1"
Bind ControlScreen    "f2"     := "CCUI.GotoScreen 2"
Bind ControlScreen    "f3"     := "CCUI.GotoScreen 3"
Bind ControlScreen    "f4"     := "CC$GotoChart Chart.X, Chart.Y"
Bind ControlScreen    "f5"     := "CC$PlanetInfo Chart.X, Chart.Y"
Bind ControlScreen    "f6"     := "CCUI.GotoScreen 6"
Bind ControlScreen    "f7"     := "UI.Search"
Bind ControlScreen    "f10"    := "CCUI.GotoScreen 10"
Bind ControlScreen    "c-f1"   := "CCUI.ListShips UI.X, UI.Y, 'e'"
Bind ControlScreen    "c-f2"   := "CCUI.GotoPlanetThere"
Bind ControlScreen    "c-f3"   := "CCUI.GotoBaseThere"
Bind ControlScreen    "c-f4"   := "CC$GotoChart UI.X, UI.Y"
Bind ControlScreen    "c-f5"   := "CC$PlanetInfo UI.X, UI.Y"
Bind ControlScreen    "c-f8"   := "CCUI.GotoBaseThere"
Bind ControlScreen    "c-f9"   := "CCUI.Planet.SetCommentAt UI.X, UI.Y"
Bind ControlScreen    "c-h"    := "UI.Search Chart.X & ', ' & Chart.Y, 'spbuo4'"
Bind ControlScreen    "c-l"    := "CCUI.ListShips UI.X, UI.Y, 'a'"
Bind ControlScreen    "c-n"    := "CCUI.ListShipPrediction UI.X, UI.Y"
Bind ControlScreen    "s-f1"   := "CCUI.ListShips Chart.X, Chart.Y, 'e'"
Bind ControlScreen    "s-f2"   := "CCUI.GotoPlanetHere"
Bind ControlScreen    "s-f3"   := "CCUI.GotoBaseHere"
Bind ControlScreen    "s-f4"   := "CC$GotoChart Chart.X, Chart.Y"
Bind ControlScreen    "s-f5"   := "CC$PlanetInfo Chart.X, Chart.Y"
Bind ControlScreen    "s-f6"   := "CCUI.GotoScreen 6"
Bind ControlScreen    "s-f8"   := "CCUI.GotoBaseHere"
Bind ControlScreen    "s-f9"   := "CCUI.Planet.SetCommentAt Chart.X, Chart.Y"
Bind ControlScreen    "s-l"    := "CCUI.ListShips Chart.X, Chart.Y, 'a'"
Bind ControlScreen    "l"      := "CCUI.ListShips UI.X, UI.Y, 'a'"
Bind ControlScreen    "a-q"    := "CC$ProcessManager"
Bind ControlScreen    "a-r"    := "CC$Reset Loc.X, Loc.Y"
Bind ControlScreen    "y"      := "UI.ScanKeyboardMode"

Bind AutoTaskScreen   "%"      := "CCUI.Task.ToggleComment"
Bind AutoTaskScreen   "k"      := "CCUI.Task.ToggleComment"
Bind AutoTaskScreen   "m"      := "CCUI.Task.ConfirmMessage"
Bind AutoTaskScreen   "c-n"    := "CCUI.Task.SetCurrent"
Bind AutoTaskScreen   "c-r"    := "CCUI.Task.LoadFromFile"
Bind AutoTaskScreen   "c-s"    := "CCUI.Task.SaveToFile"
Bind AutoTaskScreen   "del"    := "CCUI.Task.DeleteCurrent"
Bind AutoTaskScreen   "c-del"  := "CCUI.Task.DeleteAll"
Bind AutoTaskScreen   "ins"    := "CCUI.Task.InsertCommand"
Bind AutoTaskScreen   "spc"    := "CCUI.Task.EditCommand"

Bind Ship             "c"      := "CC$ShipCargo"
Bind Ship             "e"      := "CCUI.Ship.SetEnemy"
Bind Ship             "f"      := "CC$ChangeFCode"
Bind Ship             "c-f"    := "Try SetFCode Planet(Orbit$).FCode"
Bind Ship             "g"      := "CCUI.Give"
Bind Ship             "m"      := "CCUI.Ship.SetMission"
Bind Ship             "n"      := "CCUI.Ship.Rename"
Bind Ship             "r"      := "CCUI.Ship.ToggleRemote"
Bind Ship             "s"      := "CC$ShipSpec"
Bind Ship             "u"      := "CC$ShipUnload"
Bind Ship             "c-i"    := "Try UI.GotoScreen 1, Mission.Intercept"
Bind Ship             "c-j"    := "Try CC$TransferShip 2, 0"
Bind Ship             "c-p"    := "CC$ReviewShipTransfer 2, Transfer.Unload.Id"
Bind Ship             "c-s"    := "CC$ReviewShipTransfer 1, Transfer.Ship.Id"
Bind Ship             "c-t"    := "Try UI.GotoScreen 1, Mission.Tow"
Bind Ship             "alt-i"  := "UI.Search 'Mission.Intercept=' & Id, 's2'"
Bind Ship             "alt-t"  := "UI.Search 'Mission.Tow=' & Id, 's2'"
Bind Ship             "f9"     := "CCUI.Ship.SetComment"
Bind Ship             "ins"    := "CC$AddToSim"
Bind ShipScreen       "a"      := "CC$WithShipWaypoint 'CC$ChangeWaypoint', False"
Bind ShipScreen       "b"      := "CCUI.ShipBaseMenu"
Bind ShipScreen       "d"      := "CC$TransferMulti"
Bind ShipScreen       "h"      := "UI.Help 'pcc2:shipscreen'"
Bind ShipScreen       "x"      := "CCUI.Ship.Exchange"
Bind Ship             "s-m"    := "CC$ViewMessages"
Bind ShipScreen       "w"      := "CC$WithShipWaypoint 'CC$ChangeSpeed', False"
Bind ShipScreen       "c-w"    := "CC$WithShipWaypoint 'SetWaypoint UI.X, UI.Y', True"
Bind Ship             "c-z"    := "CC$WithShipWaypoint 'SetWaypoint Loc.X, Loc.Y', True"
Bind ShipScreen       "dblclick" := "CC$WithShipWaypoint 'SetWaypoint UI.X, UI.Y', True"
Bind ShipScreen       "alt-h"  := "UI.Help 'pcc2:shipscreen'"
Bind ShipScreen       "alt-m"  := "CC$SearchMate"
Bind ShipScreen       "c-m"    := "CC$GotoMate"
Bind ShipScreen       "c-n"    := "CCUI.ListShipPrediction UI.X, UI.Y, Id"
Bind ShipScreen       "s-f1"   := "CCUI.ListShips Chart.X, Chart.Y, 'e' & Id"
Bind ShipScreen       "c-f1"   := "CCUI.ListShips UI.X, UI.Y, 'e' & Id"
Bind ShipScreen       "f6"     := "CCUI$GotoObject 6, Id"
Bind ShipScreen       "f8"     := "CCUI.GotoBaseHere"
Bind ShipScreen       "f10"    := "UI.Menu 'ShipFleetMenu'"
Bind Ship             "s-f10"  := "CCUI.Ship.GotoFleet"
Bind ShipScreen       "ret"    := "CCUI$GotoObject 11, Id"
Bind ShipScreen       "tab"    := "CC$SelectNextShip"
Bind ShipScreen       "s-tab"  := "CC$SelectNextShip 'b'"
Bind ShipScreen       "c-tab"  := "CC$SelectNextShip 'm'"
Bind ShipScreen       "s-c-tab" := "CC$SelectNextShip 'bm'"
Bind ShipTaskScreen   "a"      := "CCUI.Task.AddMoveTo"
Bind ShipTaskScreen   "c"      := "CC$ShipCargo"
Bind ShipTaskScreen   "d"      := "CCUI.Task.ToggleShowDistances"
Bind ShipTaskScreen   "e"      := "CCUI.Task.TogglePredictToEnd"
Bind ShipTaskScreen   "g"      := "CCUI.Task.GoToPredictedLocation"
Bind ShipTaskScreen   "o"      := "UI.Menu 'ShipTaskOptionsMenu'"
Bind ShipTaskScreen   "h"      := "UI.Help 'pcc2:shiptaskscreen'"
Bind ShipTaskScreen   "alt-h"  := "UI.Help 'pcc2:shiptaskscreen'"
Bind ShipTaskScreen   "u"      := "CC$ShipUnload"
Bind ShipTaskScreen   "w"      := "CC$WithShipWaypoint 'CC$ChangeSpeed', False, True"
Bind ShipTaskScreen   "c-w"    := "CCUI.Task.AddMoveToScanner"
Bind ShipTaskScreen   "1"      := "UI.Menu 'ShipTaskMovementMenu'"
Bind ShipTaskScreen   "2"      := "UI.Menu 'ShipTaskCargoMenu'"
Bind ShipTaskScreen   "3"      := "UI.Menu 'ShipTaskMissionMenu'"
Bind ShipTaskScreen   "4"      := "UI.Menu 'CommonTaskMenu'"
Bind ShipTaskScreen   "dblclick" := "CCUI.Task.AddMoveToScanner"
Bind ShipTaskScreen   "f6"     := "CCUI$GotoObject 6, Id"
Bind ShipTaskScreen   "f8"     := "CCUI.GotoBaseHere"
Bind ShipTaskScreen   "f9"     := "CCUI.Ship.SetComment"
Bind ShipTaskScreen   "ret"    := "CCUI$GotoObject 1, Id"

Bind HistoryScreen    "h"      := "UI.Help 'pcc2:historyscreen'"
Bind HistoryScreen    "alt-h"  := "UI.Help 'pcc2:historyscreen'"
Bind HistoryScreen    "x"      := "CCUI.Ship.Exchange"
Bind HistoryScreen    "f1"     := "CCUI$ShipScreenFromHistory"
Bind HistoryScreen    "f6"     := "CCUI.GotoScreen 6"

Bind FleetScreen      "a"      := "CC$WithFleetWaypoint 'CC$ChangeWaypoint', False"
Bind FleetScreen      "b"      := "CCUI.Fleet.ChangeLeader"
Bind FleetScreen      "c"      := "CC$ShipCargo"
Bind FleetScreen      "d"      := "CC$TransferMulti 1"
Bind FleetScreen      "f"      := "CC$ChangeFCode"
Bind FleetScreen      "g"      := "CCUI.Give"
Bind FleetScreen      "j"      := "CCUI.Fleet.Join"
Bind FleetScreen      "n"      := "CCUI.Fleet.Rename"
Bind FleetScreen      "h"      := "UI.Help 'pcc2:fleetscreen'"
Bind FleetScreen      "alt-h"  := "UI.Help 'pcc2:fleetscreen'"
Bind FleetScreen      "p"      := "CCUI.Fleet.Split"
Bind FleetScreen      "r"      := "CCUI.Ship.ToggleRemote"
Bind FleetScreen      "u"      := "CC$ShipUnload"
Bind FleetScreen      "s"      := "CC$ShipSpec"
Bind FleetScreen      "t"      := "CCUI.Fleet.TowMember"
Bind FleetScreen      "c-w"    := "CC$WithFleetWaypoint 'SetWaypoint UI.X, UI.Y', True"
Bind FleetScreen      "w"      := "CCUI.Fleet.SetSpeed"
Bind FleetScreen      "x"      := "CCUI.Ship.Exchange"
Bind FleetScreen      "c-z"    := "CC$WithFleetWaypoint 'SetWaypoint Loc.X, Loc.Y', True"
Bind FleetScreen      "dblclick" := "CC$WithFleetWaypoint 'SetWaypoint UI.X, UI.Y', True"
Bind FleetScreen      "ins"    := "CC$AddToSim"
Bind FleetScreen      "c-ins"  := "CCUI.Fleet.AddToSim"
Bind FleetScreen      "del"    := "CCUI.Fleet.Leave"
Bind FleetScreen      "c-del"  := "CCUI.Fleet.Dissolve"
Bind FleetScreen      "f1"     := "CCUI$GotoObject 1, Id"
Bind FleetScreen      "f6"     := "CCUI$GotoObject 6, Id"
Bind FleetScreen      "f9"     := "CCUI.Ship.SetComment"

Bind PlanetScreen     "b"      := "CC$BuildStructures 0"
Bind Planet           "c"      := "Try CC$TransferPlanet 0"
Bind PlanetScreen     "d"      := "CC$BuildStructures 2"
Bind Planet           "f"      := "CC$ChangeFCode"
Bind Planet           "g"      := "CCUI.Give"
Bind PlanetScreen     "h"      := "UI.Help 'pcc2:planetscreen'"
Bind PlanetScreen     "alt-h"  := "UI.Help 'pcc2:planetscreen'"
Bind PlanetScreen     "m"      := "CC$BuildStructures 1"
Bind Planet           "s-m"    := "CC$ViewMessages"
Bind Planet           "s"      := "CC$SellSupplies"
Bind Planet           "c-s"    := "CC$BuySupplies"
Bind Planet           "t"      := "CC$ChangeTaxes"
Bind Planet           "u"      := "Try CC$TransferPlanet 1"
Bind PlanetScreen     "x"      := "CCUI.Planet.Exchange"
Bind Planet           "f8"     := "CCUI.Planet.BuildOrGoToBase"
Bind Planet           "f9"     := "CCUI.Planet.SetComment"
Bind Planet           "ins"    := "CC$AddToSim"
Bind PlanetScreen     "ret"    := "CCUI$GotoObject 12, Id"
Bind PlanetScreen     "tab"    := "CC$BuildStructures 0"
Bind PlanetTaskScreen "1"      := "UI.Menu 'PlanetTaskOrdersMenu'"
Bind PlanetTaskScreen "2"      := "UI.Menu 'PlanetTaskCargoMenu'"
Bind PlanetTaskScreen "3"      := "UI.Menu 'CommonTaskMenu'"
Bind PlanetTaskScreen "h"      := "UI.Help 'pcc2:planettaskscreen'"
Bind PlanetTaskScreen "alt-h"  := "UI.Help 'pcc2:planettaskscreen'"
Bind PlanetTaskScreen "f8"     := "CCUI.GotoBaseHere"
Bind PlanetTaskScreen "f9"     := "CCUI.Planet.SetComment"
Bind PlanetTaskScreen "ret"    := "CCUI$GotoObject 2, Id"

Bind Base             "a"      := "CC$BuildAmmo"
Bind Base             "b"      := "CC$BuildShip"
Bind Base             "c"      := "CC$TransferPlanet 0"
Bind Base             "c-c"    := "CCUI$GotoObject 1, FindShipCloningAt(Id)"
Bind Base             "d"      := "CC$BuildStructures 2"
Bind Base             "f"      := "CC$ChangeFCode"
Bind Base             "m"      := "CCUI.Base.SetMission"
Bind Base             "s-m"    := "CC$ViewMessages"
Bind Base             "q"      := "CC$ManageBuildQueue"
Bind Base             "t"      := "CC$ChangeTech"
Bind Base             "u"      := "CC$TransferPlanet 1"
Bind Base             "f9"     := "CCUI.Planet.SetComment"
Bind Base             "ins"    := "CC$AddToSim"
Bind BaseScreen       "h"      := "UI.Help 'pcc2:basescreen'"
Bind BaseScreen       "r"      := "CCUI.BaseShipyardMenu"
Bind BaseScreen       "c-r"    := "CCUI$GotoObject 1, Shipyard.Id"
Bind BaseScreen       "s"      := "CC$ShipCostCalc"
Bind BaseScreen       "x"      := "CCUI.Base.Exchange"
Bind BaseScreen       "alt-h"  := "UI.Help 'pcc2:basescreen'"
Bind BaseScreen       "f8"     := "CCUI.GotoPlanetHere"
Bind BaseScreen       "tab"    := "CC$BuildShip"
Bind BaseScreen       "ret"    := "CCUI$GotoObject 13, Id"
Bind BaseTaskScreen   "1"      := "UI.Menu 'BaseTaskOrdersMenu'"
Bind BaseTaskScreen   "2"      := "UI.Menu 'PlanetTaskCargoMenu'"
Bind BaseTaskScreen   "3"      := "UI.Menu 'CommonTaskMenu'"
Bind BaseTaskScreen   "e"      := "CCUI.Task.EditBuildCommand"
Bind BaseTaskScreen   "h"      := "UI.Help 'pcc2:basetaskscreen'"
Bind BaseTaskScreen   "alt-h"  := "UI.Help 'pcc2:basetaskscreen'"
Bind BaseTaskScreen   "f8"     := "CCUI.GotoPlanetHere"
Bind BaseTaskScreen   "f9"     := "CCUI.Planet.SetComment"
Bind BaseTaskScreen   "ret"    := "CCUI$GotoObject 3, Id"

% Starchart Bindings
Bind Starchart        "esc"    := "UI.GotoScreen 0"
Bind Starchart        "f1"     := "CCUI.ListShips UI.X, UI.Y, 'e'"
Bind Starchart        "f2"     := "CCUI.GotoPlanetHere"
Bind Starchart        "f3"     := "CCUI.GotoBaseHere"
Bind Starchart        "f4"     := "UI.GotoScreen 0"
Bind Starchart        "f5"     := "CC$PlanetInfo UI.X, UI.Y"
Bind Starchart        "f6"     := "CCUI.GotoScreen 6"
Bind Starchart        "f7"     := "UI.Search"
Bind Starchart        "f8"     := "CCUI.GotoBaseHere"
Bind Starchart        "l"      := "CCUI.ListShips UI.X, UI.Y, 'a'"
Bind Starchart        "s-l"    := "CCUI.ListShips UI.X, UI.Y, 'a'"
Bind Starchart        "g"      := "CC$GotoCoordinates"
Bind Starchart        "h"      := "UI.Help 'pcc2:starchart'"
Bind Starchart        "i"      := "CCUI$Chart.IonStormInfo"
Bind Starchart        "m"      := "CCUI$Chart.MinefieldOrMarker"
Bind Starchart        "u"      := "CCUI$Chart.UfoInfo"
Bind Starchart        "s-f1"   := "CCUI.ListShips UI.X, UI.Y, 'e'"
Bind Starchart        "s-f2"   := "CCUI.GotoPlanetHere"
Bind Starchart        "s-f3"   := "CCUI.GotoBaseHere"
Bind Starchart        "s-f5"   := "CC$PlanetInfo UI.X, UI.Y"
Bind Starchart        "s-f6"   := "CCUI.GotoScreen 6"
Bind Starchart        "s-f8"   := "CCUI.GotoBaseHere"
Bind Starchart        "s-f9"   := "CCUI.Planet.SetCommentAt UI.X, UI.Y"
Bind Starchart        "alt-h"  := "UI.Help 'pcc2:starchart'"
Bind Starchart        "alt-l"  := "CC$EditLabelConfig"
Bind Starchart        "c-h"    := "UI.Search UI.X & ', ' & UI.Y, 'spbuo4'"
Bind Starchart        "c-m"    := "CCUI$Chart.NewCannedMarker"
Bind Starchart        "c-n"    := "CCUI.ListShipPrediction UI.X, UI.Y"
Bind Starchart        "c-f1"   := "CCUI.ListShips UI.X, UI.Y, 'e'"
Bind Starchart        "c-f2"   := "CCUI.GotoPlanetHere"
Bind Starchart        "c-f3"   := "CCUI.GotoBaseHere"
Bind Starchart        "c-f5"   := "CC$PlanetInfo UI.X, UI.Y"
Bind Starchart        "c-f6"   := "CCUI.GotoScreen 6"
Bind Starchart        "c-f8"   := "CCUI.GotoBaseHere"
Bind Starchart        "c-f9"   := "CCUI.Planet.SetCommentAt UI.X, UI.Y"

Bind ShipLock         "."      := "CCUI.ToggleSelection"
Bind ShipLock         "esc"    := "CC$HidePanel"
Bind ShipLock         "c-n"    := "CCUI.ListShipPrediction UI.X, UI.Y, Id"
Bind ShipLock         "f1"     := "CCUI$GotoObject 1, Id"
Bind ShipLock         "s-f1"   := "CCUI$GotoObject 1, Id"
Bind ShipLock         "c-f1"   := "CCUI$GotoObject 1, Id"
Bind ShipLock         "f6"     := "CCUI$GotoObject 6, Id"
Bind ShipLock         "s-f6"   := "CCUI$GotoObject 6, Id"
Bind ShipLock         "c-f6"   := "CCUI$GotoObject 6, Id"

Bind PlanetLock       "."      := "CCUI.ToggleSelection"
Bind PlanetLock       "a"      := "Try CC$BuildStructures 0"
Bind PlanetLock       "b"      := "CC$BaseLock"
Bind PlanetLock       "esc"    := "CC$HidePanel"

Bind BaseLock         "esc"    := "CC$HidePanel"
Bind BaseLock         "p"      := "CC$PlanetLock"

% Race Screen Bindings
Bind RaceScreen       "esc"    := "CCUI.ExitRace"
Bind RaceScreen       "f1"     := "CCUI.GotoScreen 1"
Bind RaceScreen       "f2"     := "CCUI.GotoScreen 2"
Bind RaceScreen       "f3"     := "CCUI.GotoScreen 3"
Bind RaceScreen       "f4"     := "UI.GotoScreen 4"
Bind RaceScreen       "f6"     := "CCUI.GotoScreen 6"
Bind RaceScreen       "f7"     := "UI.Search"
Bind RaceScreen       "f10"    := "CCUI.GotoScreen 10"
Bind RaceScreen       "a"      := "UI.EditAlliances"
Bind RaceScreen       "s-a"    := "CC$SpecBrowser"
Bind RaceScreen       "b"      := "UI.BattleSimulator"
Bind RaceScreen       "alt-e"  := "CC$EditCommands"
Bind RaceScreen       "h"      := "UI.Help 'pcc2:racescreen'"
Bind RaceScreen       "alt-h"  := "UI.Help 'pcc2:racescreen'"
Bind RaceScreen       "i"      := "CC$ImperialStats"
Bind RaceScreen       "m"      := "CC$ViewInbox"
Bind RaceScreen       "alt-n"  := "CC$ViewNotifications"
Bind RaceScreen       "alt-q"  := "CC$ProcessManager"
Bind RaceScreen       "s"      := "UI.ShowScores"
Bind RaceScreen       "c-s"    := "SaveGame"
Bind RaceScreen       "t"      := "UI.EditTeams"
Bind RaceScreen       "v"      := "CC$ViewCombat"
Bind RaceScreen       "w"      := "CC$SendMessage"

% Selection Dialog Bindings
% ex WObjectSelectionControl::handleEvent
Bind SelectionDialog  "esc"    := "UI.EndDialog 0"
Bind SelectionDialog  "enter"  := "UI.EndDialog 1"
Bind SelectionDialog  "up"     := "CCUI.SelectPrevious",       "pgup"   := "CCUI.SelectPrevious",       "wheelup"   := "CCUI.SelectPrevious",       "-"   := "CCUI.SelectPrevious"
Bind SelectionDialog  "down"   := "CCUI.SelectNext",           "pgdn"   := "CCUI.SelectNext",           "wheeldn"   := "CCUI.SelectNext",           "+"   := "CCUI.SelectNext"
Bind SelectionDialog  "c-up"   := "CCUI.SelectPreviousMarked", "c-pgup" := "CCUI.SelectPreviousMarked", "c-wheelup" := "CCUI.SelectPreviousMarked", "c--" := "CCUI.SelectPreviousMarked"
Bind SelectionDialog  "c-down" := "CCUI.SelectNextMarked",     "c-pgdn" := "CCUI.SelectNextMarked",     "c-wheeldn" := "CCUI.SelectNextMarked",     "c-+" := "CCUI.SelectNextMarked"
Bind SelectionDialog  "home"   := "CCUI.SelectFirst"
Bind SelectionDialog  "end"    := "CCUI.SelectLast"
Bind SelectionDialog  "c-home" := "CCUI.SelectFirstMarked"
Bind SelectionDialog  "c-end"  := "CCUI.SelectLastMarked"
Bind SelectionDialog  "tab"    := "CCUI.SelectNextHere"
Bind SelectionDialog  "s-tab"  := "CCUI.SelectPreviousHere"
Bind SelectionDialog  "c-tab"  := "CCUI.SelectNextMarkedHere"
Bind SelectionDialog  "c-s-tab" := "CCUI.SelectPreviousMarkedHere"
Bind SelectionDialog  "."      := "CCUI.ToggleSelection"

On EnterDirectory Do TryLoad "autoexec.q"

If System.GUI Then Print "[", System.Program, " ", System.Version, ", core.q ", CC$LibraryVersion, "]"

%% Experimental
Function Compile(expr)
  Local CC$Result
  Eval "Option LocalSubs(1)", "Local Function CC$Fun", "Return " & expr, "EndFunction", "CC$Result = CC$Fun"
  Return CC$Result
EndFunction
