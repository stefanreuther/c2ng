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
Dim Shared CC$LibraryVersion = '2.40.1'

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

  % FIXME: IFUIInput(game::Session& session, ScriptSide& si, RequestLink1 link, interpreter::Arguments& args);

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


%%% Internals

% c2ng internal: This function is used to execute commands bound to keys.
% @since PCC2 2.40
Sub C2$Eval(code, UI.Prefix)
  Eval AtomStr(code)
EndSub

% c2ng internal: Game setup
% @since PCC2 2.40.1
Sub C2$RunLoadHook
  RunHook Load
  If Turn.IsNew Then RunHook NewTurn
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
Bind Global          "quit"   := "CCUI.ExitClient"
Bind Global          "a-up"   := "CCUI.History.PreviousTurn"
Bind Global          "a-down" := "CCUI.History.NextTurn"

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
Bind ControlScreen    "c-f2"   := "CCUI.GotoPlanetHere"
Bind ControlScreen    "c-f3"   := "CCUI.GotoBaseHere"
Bind ControlScreen    "c-f8"   := "CCUI.GotoBaseHere"

Bind Ship             "e"      := "CCUI.Ship.SetEnemy"
Bind Ship             "g"      := "CCUI.Give"
Bind Ship             "m"      := "CCUI.Ship.SetMission"
Bind Ship             "n"      := "CCUI.Ship.Rename"
Bind Ship             "f9"     := "CCUI.Ship.SetComment"
Bind ShipScreen       "f8"     := "CCUI.GotoBaseHere"
Bind ShipTaskScreen   "f9"     := "CCUI.Ship.SetComment"
Bind ShipTaskScreen   "f8"     := "CCUI.GotoBaseHere"

Bind Planet           "g"      := "CCUI.Give"
Bind Planet           "f9"     := "CCUI.Planet.SetComment"
Bind PlanetTaskScreen "f8"     := "CCUI.GotoBaseHere"
Bind PlanetTaskScreen "f9"     := "CCUI.Planet.SetComment"

Bind Base             "m"      := "CCUI.Base.SetMission"
Bind Base             "f9"     := "CCUI.Planet.SetComment"
Bind BaseScreen       "f8"     := "CCUI.GotoPlanetHere"
Bind BaseTaskScreen   "f9"     := "CCUI.Planet.SetComment"
Bind BaseTaskScreen   "f8"     := "CCUI.GotoPlanetHere"

Bind FleetScreen      "g"      := "CCUI.Give"
Bind FleetScreen      "f9"     := "CCUI.Planet.SetComment"


% Starchart Bindings

% Race Screen Bindings
Bind RaceScreen       "esc"    := "CCUI.ExitRace"
Bind RaceScreen       "f1"     := "CCUI.GotoScreen 1"
Bind RaceScreen       "f2"     := "CCUI.GotoScreen 2"
Bind RaceScreen       "f3"     := "CCUI.GotoScreen 3"

% Selection Dialog Bindings
Bind SelectionDialog  "esc"    := "UI.EndDialog 0"
Bind SelectionDialog  "enter"  := "UI.EndDialog 1"
Bind SelectionDialog  "up"     := "CCUI.SelectPrevious",       "pgup"   := "CCUI.SelectPrevious",       "wheelup"   := "CCUI.SelectPrevious",       "-"   := "CCUI.SelectPrevious"
Bind SelectionDialog  "down"   := "CCUI.SelectNext",           "pgdn"   := "CCUI.SelectNext",           "wheeldn"   := "CCUI.SelectNext",           "+"   := "CCUI.SelectNext"
Bind SelectionDialog  "c-up"   := "CCUI.SelectPreviousMarked", "c-pgup" := "CCUI.SelectPreviousMarked", "c-wheelup" := "CCUI.SelectPreviousMarked", "c--" := "CCUI.SelectPreviousMarked"
Bind SelectionDialog  "c-down" := "CCUI.SelectNextMarked",     "c-pgdn" := "CCUI.SelectNextMarked",     "c-wheeldn" := "CCUI.SelectNextMarked",     "c-+" := "CCUI.SelectNextMarked"
Bind SelectionDialog  "."      := "CCUI.ToggleSelection"

On EnterDirectory Do TryLoad "autoexec.q"

If System.GUI Then Print "[", System.Program, " ", System.Version, ", core.q ", CC$LibraryVersion, "]"

%% Experimental
Function Compile(expr)
  Local CC$Result
  Eval "Option LocalSubs(1)", "Local Function CC$Fun", "Return " & expr, "EndFunction", "CC$Result = CC$Fun"
  Return CC$Result
EndFunction
