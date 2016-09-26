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
Dim Shared CC$LibraryVersion = '2.40'

%%% Console-Mode Replacements for GUI routines %%%%%%%%%%%%%%%%%%%%%%

If Not System.GUI Then
  % c2ng does not need replacements for rich-text operations here.
EndIf

%%% User-callable Subroutines %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% @q WaitOneTurn (Global Command)
% Suspend script for one turn.
% Execution proceeds next turn (or later, when a turn is missed).
% See {Stop} for details and restrictions on suspension.
% @see Stop
% @since PCC2 1.99.10, PCC 1.0.6
Sub WaitOneTurn ()
  Local t = Turn
  Do While t = Turn
    Stop
  Loop
EndSub

%%% GUI Subroutines %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

If System.GUI Then
%% When the GUI is not active, don't define all these to save memory

% '.' everywhere
Sub CC$ToggleSelection
  Mark Not Marked
EndSub

% F1/F2/F3
Sub CC$GotoScreen (Screen)
  Local UI.Result
  UI.ChooseObject Screen
  If Not IsEmpty(UI.Result) Then UI.GotoScreen Screen, UI.Result
EndSub

% ExitRace with confirmation
Sub CC$ExitRace
  Local UI.Result
  UI.Message Translate("Do you want to exit this game?"), Translate("Exit Game"), Translate("Yes No")
  If UI.Result=1 Then System.ExitRace
EndSub

% ExitClient with confirmation
Sub CC$ExitClient
  Local UI.Result
  UI.Message Translate("Do you want to exit PCC2?"), Translate("PCC2"), Translate("Yes No")
  If UI.Result=1 Then System.ExitClient
EndSub

Sub C2$SelectPrevious
  With UI.Iterator Do CurrentIndex := PreviousIndex(CurrentIndex, "w")
EndSub

Sub C2$SelectNext
  With UI.Iterator Do CurrentIndex := NextIndex(CurrentIndex, "w")
EndSub

Sub C2$SelectPreviousMarked
  With UI.Iterator Do CurrentIndex := PreviousIndex(CurrentIndex, "wm")
EndSub

Sub C2$SelectNextMarked
  With UI.Iterator Do CurrentIndex := NextIndex(CurrentIndex, "wm")
EndSub

Sub C2$History(delta)
  % Pick next turn
  Local UI.Result
  UI.ChooseTurn delta
  If UI.Result
    History.ShowTurn UI.Result
  EndIf
EndSub

Sub C2$HistoryPrevious
  C2$History -1
EndSub

Sub C2$HistoryNext
  C2$History 1
EndSub

% c2ng internal: This function is used to execute commands bound to keys.
Sub C2$Eval(code, UI.Prefix)
  Eval AtomStr(code)
EndSub

EndIf % System.GUI

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

% Global Bindings
Bind Global          "a-c"    := "UI.PopupConsole"
Bind Global          "quit"   := "CC$ExitClient"
Bind Global          "a-up"   := "C2$HistoryPrevious"
Bind Global          "a-down" := "C2$HistoryNext"

% Control Screen Bindings
Bind ControlScreen   "."      := "CC$ToggleSelection"
Bind ControlScreen   "esc"    := "UI.GotoScreen 0"
Bind ControlScreen   "pgup"   := "C2$SelectPrevious",       "wheelup"   := "C2$SelectPrevious",       "-"   := "C2$SelectPrevious"
Bind ControlScreen   "pgdn"   := "C2$SelectNext",           "wheeldn"   := "C2$SelectNext",           "+"   := "C2$SelectNext"
Bind ControlScreen   "c-pgup" := "C2$SelectPreviousMarked", "c-wheelup" := "C2$SelectPreviousMarked", "c--" := "C2$SelectPreviousMarked"
Bind ControlScreen   "c-pgdn" := "C2$SelectNextMarked",     "c-wheeldn" := "C2$SelectNextMarked",     "c-+" := "C2$SelectNextMarked"
Bind ControlScreen   "f1"     := "CC$GotoScreen 1"
Bind ControlScreen   "f2"     := "CC$GotoScreen 2"
Bind ControlScreen   "f3"     := "CC$GotoScreen 3"

% Starchart Bindings

% Race Screen Bindings
Bind RaceScreen      "esc"    := "CC$ExitRace"
Bind RaceScreen      "f1"     := "CC$GotoScreen 1"
Bind RaceScreen      "f2"     := "CC$GotoScreen 2"
Bind RaceScreen      "f3"     := "CC$GotoScreen 3"

% Selection Dialog Bindings
Bind SelectionDialog "esc"    := "UI.EndDialog 0"
Bind SelectionDialog "enter"  := "UI.EndDialog 1"
Bind SelectionDialog "up"     := "C2$SelectPrevious",       "pgup"   := "C2$SelectPrevious",       "wheelup"   := "C2$SelectPrevious",       "-"   := "C2$SelectPrevious"
Bind SelectionDialog "down"   := "C2$SelectNext",           "pgdn"   := "C2$SelectNext",           "wheeldn"   := "C2$SelectNext",           "+"   := "C2$SelectNext"
Bind SelectionDialog "c-up"   := "C2$SelectPreviousMarked", "c-pgup" := "C2$SelectPreviousMarked", "c-wheelup" := "C2$SelectPreviousMarked", "c--" := "C2$SelectPreviousMarked"
Bind SelectionDialog "c-down" := "C2$SelectNextMarked",     "c-pgdn" := "C2$SelectNextMarked",     "c-wheeldn" := "C2$SelectNextMarked",     "c-+" := "C2$SelectNextMarked"
Bind SelectionDialog "."      := "CC$ToggleSelection"

On EnterDirectory Do TryLoad "autoexec.q"

If System.GUI Then Print "[", System.Program, " ", System.Version, ", core.q ", CC$LibraryVersion, "]"
