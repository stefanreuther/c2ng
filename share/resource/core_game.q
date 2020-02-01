%
%  Game functions
%
%  This module contains documented official functions, and some still undocumented functions.
%  Undocumented functions have a "CCVP." prefix.
%  As usual, the "ex file.cc:function" comment means this function was implemented in C++ in PCC2.
%

% @q WaitOneTurn (Global Command)
% Suspend script for one turn.
% Execution proceeds next turn (or later, when a turn is missed).
% See {Stop} for details and restrictions on suspension.
% @see Stop
% @since PCC2 1.99.10, PCC 1.0.6, PCC2 2.40
Sub WaitOneTurn ()
  Local t = Turn
  Do While t = Turn
    Stop
  Loop
EndSub

% @q MoveTo x:Int, y:Int (Ship Command)
% Move ship to X,Y.
% This sets a waypoint, and suspends the script until the waypoint has been reached.
% As a special case, if the waypoint is in a warp well, the script continues when
% the planet has been reached.
%
% Example:
% <pre class="ccscript">
%   MoveTo 1240, 1500
%   MoveTo 1400, 1520
% </pre>
% This will move the ship to (1240,1500) and then to (1400,1520).
%
% This command is intended to be used in auto tasks.
% @see Stop
% @since PCC2 1.99.10, PCC 1.0.6, PCC2 2.40.1
Sub MoveTo (x, y)
  % We terminate when the waypoint is zero: when moving across a map border, we never
  % reach the specified x,y.
  Local pid = PlanetAt(x, y, 1)
  With Lock("s" & Id & ".waypoint")
    SetWaypoint x, y
    Do Until (Waypoint.DX=0 And Waypoint.DY=0) Or ((pid<>0) And (Orbit$=pid))
      Stop
    Loop
  EndWith
EndSub

% @q MoveTowards x:Int, y:Int (Ship Command)
% Move ship towards X,Y.
% This command sets the waypoint and waits one turn.
% This causes the ship to move towards the specified coordinate, but not necessarily reach it.
%
% This command is intended to be used in auto tasks.
% @see Stop
% @since PCC2 1.99.10, PCC 1.1.3, PCC2 2.40.1
Sub MoveTowards (x, y)
  With Lock("s" & Id & ".waypoint")
    SetWaypoint x, y
    WaitOneTurn
  EndWith
EndSub

% @q ShipName(sid:Int):Str (Function)
% Get name of a ship.
% Similar to <tt>Ship(sid).Name</tt>.
%
% @diff In PCC 1.x, this function returns a decorated name such as "USS Lincoln (#123)".
% In PCC2 since 1.99.20, this function returns just the name.
% Use {ShipNameAndId} for the old behaviour.
% @see ShipNameAndId, PlanetName
% @since PCC 0.99.7, PCC2 1.99.8, PCC2 2.40.1
Function ShipName (sid)
  % ex int/if/globalif.h:IFShipNameGet
  % ex ccexpr.pas:op_SHIPNAME_func
  Return Global.Ship(sid).Name
EndFunction

% @q PlanetName(pid:Int):Str (Function)
% Get name of a planet.
% @see ShipName
% @since PCC 0.99.7, PCC2 1.99.8, PCC2 2.40.1
Function PlanetName (pid)
  % ex int/if/globalif.h:IFPlanetNameGet
  % ex ccexpr.pas:op_PLANETNAME_func
  Return Global.Planet(pid).Name
EndFunction

% @q ShipNameAndId(sid:Int):Str (Function)
% Get name of a ship, together with its Id.
%
% Note that this function does what %ShipName does in PCC 1.x.
% In PCC2, %ShipName returns just the plain name which I believe is more useful
% and consistent with %PlanetName.
% Code that used %ShipName can now use %ShipNameAndId.
% @since PCC2 1.99.20, PCC2 2.40.1
% @see ShipName
Function ShipNameAndId (sid)
  Local N = ShipName(sid)
  If N
    Return N & " (#" & sid & ")"
  Else
    Return Format(Translate("Ship %d"), sid)
  EndIf
EndFunction

% @q FindShipCloningAt(pid:Int):Int (Function)
% Find ship cloning at a planet.
% Returns the Id of the ship that is cloning at planet %pid.
% If no ship is trying to clone, or cloning is forbidden, returns EMPTY.
% @since PCC2 1.99.10, PCC2 2.40.1
Function FindShipCloningAt (pid)
  If Cfg("AllowShipCloning")
    % FindShip returns the first ship, which is just what we need
    Return FindShip(Orbit$=pid And StrCase(FCode="cln"))
  Else
    Return Z(0)
  EndIf
EndFunction


% @q SelectionLoad file:File, Optional flags:Str (Global Command)
% Load selection from file.
%
% The %flags argument is a combination of the following options:
% - %t ("timeless") to ignore the timestamp of the selection file and load the file even if it doesn't match
% - %a ("all") to accept loading files that contain all selection layers (refused by default)
% - %m ("merge") to merge the selection instead of replacing it
% - %u ("user interaction") to ask the user for confirmation before loading the file
% - a selection layer number to load the file into that layer
% @see SelectionSave, Selection.Layer
% @since PCC 1.1.3, PCC2 1.99.13, PCC2 2.40.6
Sub SelectionLoad (file, Optional flags)
  % ex SelectionLoadAskUI::ask
  Local state, q, UI.Result, ok, oldPos
  oldPos := FPos(file)
  Try
    state := CC$SelReadHeader(file, flags)
    q := CC$SelGetQuestion(state)
    If q Then
      UI.Message Format(Translate("%s. Do you want to load this file?"), q), Translate("Load Selection"), Translate("Yes No")
      ok := (UI.Result = 1)
    Else
      ok := True
    EndIf
    If ok Then CC$SelReadContent(state)
  Else
    Seek file, oldPos
    Abort System.Err
  EndTry
EndSub

% @q EnqueueShip h:Int, e:Int, Optional bt:Int, bc:Int, tt:Int, tc:Int (Planet Command)
% Build a ship. Parameters are the same as for %BuildShip.
% Whereas %BuildShip immediately replaces the build order,
% this command waits until the starbase is done building its current order.
% This command also waits until sufficient resources are available to place the order.
%
% This command is intended to be used in auto tasks.
% @see BuildShip, Stop
% @since PCC2 1.99.10, PCC 1.0.6, PCC2 2.40.7
Sub EnqueueShip (h, e, Optional bt, bc, tt, tc)
  % wait until pending build order performed
  Do While Build.YesNo
    Stop
  Loop
  % wait until build order successfully delivered
  Do
    Try
      BuildShip h, e, bt, bc, tt, tc
      Break
    EndTry
    Stop
  Loop
EndSub

% @q CargoUploadWait amount:Cargo, Optional flags:Str (Ship Command)
% Load cargo from planet, wait until amount loaded.
% This command is similar to {CargoUpload}, see there for details about its parameters.
% It will try to load cargo from the planet this ship is orbiting.
% It will continue execution when enough cargo has been loaded.
%
% For example,
% | CargoUpload "200n"
% will load 200 kt Neutronium on this ship.
% If the planet has more than 200 kt Neutronium, they will be loaded immediately;
% if the planet has less, the ship will wait for newly-mined Neutronium, or Neutronium unloaded by ships,
% until 200 kt have been loaded.
% If this ship has not enough free cargo room, it will also wait until some becomes available.
%
% Note that when you unload cargo from this ship while %CargoUploadWait is active,
% that cargo will be uploaded again and count towards the limit.
% If the planet has 100 kt fuel while you try to load 200, everything will be uploaded.
% If you now unload these 100 kt again, %CargoUploadWait will load them up again and finish successfully,
% as it now has loaded 200 kt total.
% @see CargoUpload, CargoTransferWait
% @since PCC 1.0.11, PCC2 1.99.21, PCC2 2.40.7
Sub CargoUploadWait (cargo, Optional flags)
  Local Cargo.Remainder = cargo
  Do
    CargoUpload Cargo.Remainder, flags & "n"
    If Not Cargo.Remainder Then Return
    Stop
  Loop
EndSub

% @q CargoTransferWait amount:Cargo, target:Int, Optional flags:Any (Ship Command, Planet Command)
% Transfers cargo to a ship, wait until amount transferred.
% This command is similar to {CargoTransfer}, see there for details about its parameters.
% It will try to transfer cargo from this unit to ship %sid.
% It will continue execution when all cargo has been transferred.
%
% For example,
% | CargoTransfer "200n", 42
% will load 200 kt Neutronium onto ship 42.
% If the unit this command is invoked from has enough cargo, and the receiving ship has enough free room,
% the command will succeed immediately.
% Otherwise, the script will wait until cargo or free room becomes available,
% and continue until all cargo has been transferred.
% @see CargoUpload, CargoUploadWait
% @since PCC 1.0.11, PCC2 1.99.21, PCC2 2.40.7
Sub CargoTransferWait (sid, cargo, Optional flags)
  Local Cargo.Remainder = cargo
  Do
    CargoTransfer sid, Cargo.Remainder, flags & "n"
    If Not Cargo.Remainder Then Return
    Stop
  Loop
EndSub


% @q CargoUnloadAllShips (Ship Command, Planet Command)
% Unload all ships at this location. Can be called from ship or planet.
% @since PCC2 1.99.16, PCC2 2.40.7
Sub CargoUnloadAllShips
  Local thisX = Loc.X, thisY = Loc.Y
  Local Cargo.Remainder
  If Not PlanetAt(thisX, thisY) Then
    Abort "No planet at this location"
  Else
    ForEach Ship Do
      If Loc.X=thisX And Loc.Y=thisY And Played Then
        CargoUnload "10000tdmcs$", "n"
      EndIf
    Next
  EndIf
EndSub


% @q BuildMinesWait amount:Int (Planet Command)
% Build mines, wait as necessary.
% If %amount mines cannot be built immediately due to lacking resources or colonists,
% this command waits until they have been built.
% @see BuildMines, Stop
% @since PCC2 1.99.10, PCC 1.0.17, PCC2 2.40.7
Sub BuildMinesWait (amount)
  Local Build.Remainder = amount
  Do
    BuildMines Build.Remainder, "n"
    If Not Build.Remainder Then Return
    Stop
  Loop
EndSub

% @q BuildFactoriesWait amount:Int (Planet Command)
% Build factories, wait as necessary.
% If %amount factories cannot be built immediately due to lacking resources or colonists,
% this command waits until they have been built.
% @see BuildFactories, Stop
% @since PCC2 1.99.10, PCC 1.0.17, PCC2 2.40.7
Sub BuildFactoriesWait (amount)
  Local Build.Remainder = amount
  Do
    BuildFactories Build.Remainder, "n"
    If Not Build.Remainder Then Return
    Stop
  Loop
EndSub

% @q BuildDefenseWait amount:Int (Planet Command)
% Build defense posts, wait as necessary.
% If %amount defense posts cannot be built immediately due to lacking resources or colonists,
% this command waits until they have been built.
% @see BuildDefense, Stop
% @since PCC2 1.99.10, PCC 1.0.17, PCC2 2.40.7
Sub BuildDefenseWait (amount)
  Local Build.Remainder = amount
  Do
    BuildDefense Build.Remainder, "n"
    If Not Build.Remainder Then Return
    Stop
  Loop
EndSub

% @q BuildBaseDefenseWait amount:Int (Planet Command)
% Build starbase defense, wait as necessary.
% If %amount defense posts cannot be built immediately due to lacking resources or colonists,
% this command waits until they have been built.
% @see BuildBaseDefense, Stop
% @since PCC2 1.99.10, PCC 1.0.17, PCC2 2.40.7
Sub BuildBaseDefenseWait (amount)
  Local Build.Remainder = amount
  Do
    BuildBaseDefense Build.Remainder, "n"
    If Not Build.Remainder Then Return
    Stop
  Loop
EndSub


%%% More Game Functions

% Get name of experience level
% @since PCC2 2.40.1
Function CCVP.GetExperienceLevelName (n)
  If Not IsEmpty(n)
    Local s = Cfg("ExperienceLevelNames")
    Local i
    For i:=1 To n Do s := Rest(',', s)
    s := Trim(First(',', s))
    If Not s Then s := Format(Translate("Level %d"), n)
    Return s
  EndIf
EndFunction


% Get name of location.
% Equivalent to game::map::Universe::getLocationName:
% flags = s (ships), w (warp wells), v (verbose), o (orbit), e (empty if deep space)
% @since PCC2 2.40.1
Function CCVP.GetLocationName(findX, findY, flags)
  Local pid, sid, fmt

  pid = PlanetAt(findX, findY)
  If pid Then
    If InStr(flags, "o") Then
      fmt := If(InStr(flags, "v"), Translate("Orbit of %s (Planet #%d)"), Translate("Orbit of %s (#%d)"))
    Else
      fmt := If(InStr(flags, "v"), Translate("%s (Planet #%d)"), Translate("%s (#%d)"))
    EndIf
    Return Format(fmt, Global.Planet(pid).Name, pid)
  EndIf

  If InStr(flags, "s") Then
    sid := FindShip(Loc.X=findX And Loc.Y=findY)
    If sid Then
      If Not Name Then
        Return Format(Translate("Ship #%d"), sid)
      Else
        Return Format(Translate("Ship #%d: %s"), sid, Global.Ship(sid).Name)
      EndIf
    EndIf
  EndIf

  If InStr(flags, "w") Then
    pid := PlanetAt(findX, findY, 1)
    If pid Then
      Return Format(If(InStr(flags, "v"), Translate("near %s (Planet #%d)"), Translate("near %s (#%d)")), Global.Planet(pid).Name, pid)
    EndIf
  EndIf

  If InStr(flags, "e") Then
    Return ""
  Else
    Return Format(If(InStr(flags, "v"), Translate("Deep Space (%d,%d)"), "(%d,%d)"), findX, findY)
  EndIf
EndFunction


% Call from ship context: get label of current ship
% @since PCC2 2.40.1
Function CCVP.ShipMissionLabel
  % ex GMission::getLabel
  Local tmp, System.Err

  % Try the label expression
  tmp := Global.Mission(Mission$, Owner.Real).Label
  If tmp Then
    Try
      tmp := Eval(tmp)
      If tmp Then Return tmp
    EndTry
  EndIf

  % If that fails, try the name
  tmp := Global.Mission(Mission$, Owner.Real).Name
  If tmp Then Return tmp

  % If that still fails, report scary values
  If Mission.Intercept Or Mission.Tow Then
    Return Format("M%d I%d T%d", Mission$, Mission.Intercept, Mission.Tow)
  Else
    Return Format("M%d", Mission$)
  EndIf
EndFunction

% Call from ship context: check for possible mission warning
Function CCVP.ShipHasMissionWarning
  % ex GMission::isWarning
  Local tmp, System.Err

  % Try to find an expression
  tmp := Global.Mission(Mission$, Owner.Real).Warning
  If Not tmp Then tmp := Global.Mission(Mission$, Owner.Real).Condition

  % If we have an expression, return its value. Error means there is a warning.
  If tmp Then
    Try
      tmp := Eval(tmp)
      Return IsEmpty(tmp) Or Not tmp
    EndTry
    Return True
  Else
    Return False
  EndIf
EndFunction

% @since PCC2 2.40.1
Function CCVP.HappyColor(happy)
  If happy <= 40 Then
    Return "red"
  Else If happy < 70 Then
    Return "yellow"
  Else
    Return "green"
  EndIf
EndFunction

% @since PCC2 2.40.6
Function CCVP.PlayerColor(p)
  If p=My.Race$ Then
    Return "green"
  Else If Global.Player(p).Team = My.Team Then
    Return "yellow"
  Else
    Return "red"
  EndIf
EndFunction

% Render number
% @since PCC2 2.40.1
Function CCVP.NumberToString(n)
  % ex util/misc.h:numToString
  Local s = Str(Abs(n)), i
  If Pref("Display.ThousandsSep") Then
    i := Len(s)
    Do While i > 3
      i := i - 3
      s := Mid(s,1,i) & "," & Mid(s,i+1)
    Loop
  EndIf
  If n < 0 Then s:='-' & s
  Return s
EndFunction


% Render clans
% @since PCC2 2.40.1
Function CCVP.ClansToString(n)
  % ex util/misc.h:clansToString
  If Pref("Display.Clans") Then
    Return CCVP.NumberToString(n) & "c"
  Else
    Return CCVP.NumberToString(n*100)
  EndIf
EndFunction

% Check for "Give"
Function CCVP.AllowGive
  Return Cfg("CPEnableGive") And System.Host="PHost"
EndFunction

Function CCVP.AllowRemote
  Return Cfg("CPEnableRemote") And System.Host="PHost"
EndFunction

% Option declarations
On Load Do CreatePrefOption "Display.ThousandsSep", "bool"
On Load Do CreatePrefOption "Display.Clans", "bool"
