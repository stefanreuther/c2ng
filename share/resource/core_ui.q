%
%  User-Interface Commands
%
%  Naming convention:
%     CCUI.xxx - public functions (which should probably documented)
%     CCUI$xxx - internal functions (not documented, subject to change)
%

%%% Globals

% ExitRace with confirmation [ESC on race screen]
Sub CCUI.ExitRace
  Local UI.Result
  UI.Message Translate("Do you want to exit this game?"), Translate("Exit Game"), Translate("Yes No")
  If UI.Result=1 Then System.ExitRace
EndSub

% ExitClient with confirmation [Key_Quit]
Sub CCUI.ExitClient
  Local UI.Result
  UI.Message Translate("Do you want to exit PCC2?"), Translate("PCC2"), Translate("Yes No")
  If UI.Result=1 Then System.ExitClient
EndSub



%%% Contexts

Sub CCUI$History(delta)
  Local UI.Result
  UI.ChooseTurn delta
  If UI.Result
    History.ShowTurn UI.Result
  EndIf
EndSub

% @since PCC2 2.40.1
Sub CCUI.History.PreviousTurn
  CCUI$History -1
EndSub

% @since PCC2 2.40.1
Sub CCUI.History.NextTurn
  CCUI$History 1
EndSub

% Open control screen. With selection.
% @since PCC2 2.40.1
Sub CCUI.GotoScreen (Screen)
  Local UI.Result
  UI.ChooseObject Screen
  If Not IsEmpty(UI.Result) Then UI.GotoScreen Screen, UI.Result
EndSub

% F1 on history screen
Sub CCUI$ShipScreenFromHistory
  % CC$ShipScreenFromHistory
  If Played Then
    UI.GotoScreen 1, Id
  Else
    CCUI.GotoScreen 1
  EndIf
EndSub

% Previous unit [PgUp etc.]
% @since PCC2 2.40.1
Sub CCUI.SelectPrevious
  Local System.Err
  Try With UI.Iterator Do CurrentIndex := PreviousIndex(CurrentIndex, "w")
EndSub

% Next unit [PgDn etc.]
% @since PCC2 2.40.1
Sub CCUI.SelectNext
  Local System.Err
  Try With UI.Iterator Do CurrentIndex := NextIndex(CurrentIndex, "w")
EndSub

% Previous marked unit [Ctrl+PgUp etc.]
% @since PCC2 2.40.1
Sub CCUI.SelectPreviousMarked
  Local System.Err
  Try With UI.Iterator Do CurrentIndex := PreviousIndex(CurrentIndex, "wm")
EndSub

% Next marked unit [Ctrl+PgDn etc.]
% @since PCC2 2.40.1
Sub CCUI.SelectNextMarked
  Local System.Err
  Try With UI.Iterator Do CurrentIndex := NextIndex(CurrentIndex, "wm")
EndSub

% First unit [Home]
% @since PCC2 2.40.9
Sub CCUI.SelectFirst
  Local System.Err
  Try With UI.Iterator Do CurrentIndex := NextIndex(0)
EndSub

% Last unit [Home]
% @since PCC2 2.40.9
Sub CCUI.SelectLast
  Local System.Err
  Try With UI.Iterator Do CurrentIndex := PreviousIndex(0)
EndSub

% First marked unit [Home]
% @since PCC2 2.40.9
Sub CCUI.SelectFirstMarked
  Local System.Err
  Try With UI.Iterator Do CurrentIndex := NextIndex(0, "m")
EndSub

% Last marked unit [Home]
% @since PCC2 2.40.9
Sub CCUI.SelectLastMarked
  Local System.Err
  Try With UI.Iterator Do CurrentIndex := PreviousIndex(0, "m")
EndSub

% Previous unit here [Shift-Tab]
% @since PCC2 2.40.9
Sub CCUI.SelectPreviousHere
  Local System.Err
  Try With UI.Iterator Do CurrentIndex := PreviousIndexAt(CurrentIndex, Loc.X, Loc.Y, "w")
EndSub

% Next unit here [Tab]
% @since PCC2 2.40.9
Sub CCUI.SelectNextHere
  Local System.Err
  Try With UI.Iterator Do CurrentIndex := NextIndexAt(CurrentIndex, Loc.X, Loc.Y, "w")
EndSub

% Previous marked unit here [Ctrl-Shift-Tab]
% @since PCC2 2.40.9
Sub CCUI.SelectPreviousMarkedHere
  Local System.Err
  Try With UI.Iterator Do CurrentIndex := PreviousIndexAt(CurrentIndex, Loc.X, Loc.Y, "wm")
EndSub

% Next unit here [Ctrl-Tab]
% @since PCC2 2.40.9
Sub CCUI.SelectNextMarkedHere
  Local System.Err
  Try With UI.Iterator Do CurrentIndex := NextIndexAt(CurrentIndex, Loc.X, Loc.Y, "wm")
EndSub

% Previous unit here, sorted [Shift-Tab]
% @since PCC2 2.40.13
Sub CCUI.SelectPreviousHereSorted
  Local System.Err
  Try With UI.Iterator Do CurrentIndex := PreviousIndexAt(CurrentIndex, Loc.X, Loc.Y, "ws")
EndSub

% Next unit here, sorted [Tab]
% @since PCC2 2.40.13
Sub CCUI.SelectNextHereSorted
  Local System.Err
  Try With UI.Iterator Do CurrentIndex := NextIndexAt(CurrentIndex, Loc.X, Loc.Y, "ws")
EndSub

% Previous marked unit here, sorted [Ctrl-Shift-Tab]
% @since PCC2 2.40.13
Sub CCUI.SelectPreviousMarkedHereSorted
  Local System.Err
  Try With UI.Iterator Do CurrentIndex := PreviousIndexAt(CurrentIndex, Loc.X, Loc.Y, "wms")
EndSub

% Next unit here, sorted [Ctrl-Tab]
% @since PCC2 2.40.13
Sub CCUI.SelectNextMarkedHereSorted
  Local System.Err
  Try With UI.Iterator Do CurrentIndex := NextIndexAt(CurrentIndex, Loc.X, Loc.Y, "wms")
EndSub

% X on ship screen
% @since PCC2 2.40.13
Sub CCUI.Ship.Exchange
  % ex CC$ShipXchg
  Local myX = UI.X, myY = UI.Y, myId = Id
  If FindShip(Loc.X=myX And Loc.Y=myY And Id<>myId And Played)
    % There is one of our ships other than this one at this location
    CCUI.ListShips myX, myY, 'e' & myId
  Else
    % No ship, so check for a planet
    Local pid = PlanetAt(myX, myY, 1)
    If pid And Planet(pid).Played Then UI.GotoScreen 2, pid
  EndIf
EndSub

% X on planet screen
% @since PCC2 2.40.13
Sub CCUI.Planet.Exchange
  % ex CC$PlanetXchg
  Local anyShip = FindShip(Loc.X=UI.X And Loc.Y=UI.Y And Played)
  Local pid = PlanetAt(UI.X, UI.Y, IsEmpty(Z(anyShip)))
  If pid And pid<>Id And Planet(pid).Played Then
    UI.GotoScreen 2, pid
  Else
    If anyShip Then CCUI.ListShips UI.X, UI.Y, 'e'
  EndIf
EndSub

% X on base screen
% @since PCC2 2.40.13
Sub CCUI.Base.Exchange
  % ex CC$BaseXchg, bdata.pas:BaseXchg
  Local anyShip = FindShip(Loc.X=UI.X And Loc.Y=UI.Y And Played)
  Local pid = PlanetAt(UI.X, UI.Y, IsEmpty(Z(anyShip)))
  If pid And pid<>Id And Planet(pid).Played Then
    If Planet(pid).Base.YesNo Then
      UI.GotoScreen 3, pid
    Else
      UI.GotoScreen 2, pid
    EndIf
  Else
    If anyShip Then CCUI.ListShips UI.X, UI.Y, 'e'
  EndIf
EndSub




% Go-to-object with "error handling"
% @since PCC2 2.40.7
Sub CCUI$GotoObject (Screen, Id)
  % ex CC$GotoObject
  If Id Then Try UI.GotoScreen Screen, Id
EndSub

% Planet or starbase at X,Y
% @since PCC2 2.40.1
Sub CCUI$GotoPlanet (screen, x, y)
  % ex CC$PlanetAt
  Local System.Err
  Local pid := PlanetAt(X, Y, 1)
  If pid Then Try UI.GotoScreen screen, pid
EndSub

% Go to planet here [Shift-F2]
% @since PCC2 2.40.1
Sub CCUI.GotoPlanetHere
  % CC$PlanetAt (sort-of)
  CCUI$GotoPlanet 2, Loc.X, Loc.Y
EndSub

% Go to starbase here [Shift-F3]
% @since PCC2 2.40.1
Sub CCUI.GotoBaseHere
  % CC$PlanetAt (sort-of)
  CCUI$GotoPlanet 3, Loc.X, Loc.Y
EndSub

% Go to planet there [Ctrl-F2]
% @since PCC2 2.40.13
Sub CCUI.GotoPlanetThere
  % CC$PlanetAt (sort-of)
  CCUI$GotoPlanet 2, UI.X, UI.Y
EndSub

% Go to starbase here [Ctrl-F3]
% @since PCC2 2.40.13
Sub CCUI.GotoBaseThere
  % CC$PlanetAt (sort-of)
  CCUI$GotoPlanet 3, UI.X, UI.Y
EndSub

% L, Shift-L, Shift-F1, etc.
% @since PCC2 2.40.5
Sub CCUI.ListShips (X, Y, flags)
  % CC$ListShips
  Local UI.Result
  UI.ListShips X, Y, flags
  If UI.Result
    UI.GotoScreen 1, UI.Result
  EndIf
EndSub

% Ctrl-N
% @since PCC2 2.40.9
Sub CCUI.ListShipPrediction (X, Y, Optional Id)
  % CC$ListShipPrediction in PCC2
  Local UI.Result
  UI.ListShipPrediction X, Y, Id
  If UI.Result
    UI.GotoScreen 1, UI.Result
  EndIf
EndSub

% F on ship/planet/base screen
% @since PCC2 2.40.6
Sub CC$ChangeFCode
  Local UI.Result
  UI.InputFCode "d", FCode
  SetFCode UI.Result
EndSub

% C-p/C-s on ship screen. Only review the transfer if it is present.
Sub CC$ReviewShipTransfer (kind, Id)
  If Id Then CC$TransferShip kind, Id
EndSub

% c on ship/history screen
Sub CC$ShipCargo
  If Played Then
    CC$TransferShip 0,0
  Else
    CC$CargoHistory
  EndIf
EndSub

% u on ship screen
Sub CC$ShipUnload
  If Played Then
    If Orbit$ Then
      CC$TransferUnload
    Else
      MessageBox Translate("We are not orbiting a planet, sir. If you want to jettison cargo, use the 'c' command instead."), Translate("Cargo Transfer")
    EndIf
  EndIf
EndSub

% Shift/Ctrl-F4
Sub CC$GotoChart (X, Y)
  If IsEmpty(X) Or IsEmpty(Y) Then
    UI.GotoScreen 4
  Else
    UI.GotoChart X,Y
  EndIf
EndSub

% C-m no ship screen: go to chunnel mate
Sub CC$GotoMate
  Local mate
  If HasFunction('Chunneling') Or HasFunction('ChunnelSelf') Or HasFunction('ChunnelOthers') Then
    mate := Val(FCode)
    If mate And Ship(mate).Played And (Ship(mate).HasFunction('Chunneling') Or Ship(mate).HasFunction('ChunnelTarget')) Then
      UI.GotoScreen 1, mate
    EndIf
  EndIf
EndSub

% Alt-m on ship screen: search ships that have us as chunnel mate
Sub CC$SearchMate
  If HasFunction("Chunneling") Or HasFunction("ChunnelTarget") Then
    UI.Search "(HasFunction('Chunneling') Or HasFunction('ChunnelSelf') Or HasFunction('ChunnelOthers')) And Val(FCode)=" & Id, "s2"
  EndIf
EndSub

% B on starchart in planet view
Sub CC$BaseLock
  Local System.Err
  If Base.YesNo Then
    Chart.SetView 'BaseLock'
  Else
    Try CC$BuildBase
  EndIf
EndSub

% P on starchart in starbase view
Sub CC$PlanetLock
  Chart.SetView 'PlanetLock'
EndSub

% ESC on starchart. Since this hides the view, it will also turn off the keymap,
% so the next ESC will exit the map.
Sub CC$HidePanel
  Chart.SetView ''
EndSub

% F5
Sub CC$PlanetInfo (X, Y)
  Local pid = PlanetAt(X,Y,True)
  If pid Then UI.PlanetInfo pid
EndSub


%%% Unit Manipulation

% Toggle selection ['.' everywhere]
% @since PCC2 2.40.1
Sub CCUI.ToggleSelection
  Mark Not Marked
EndSub

% Rename ship [N]
% @since PCC2 2.40.1
Sub CCUI.Ship.Rename
  % ex core.q:CC$ShipName
  If Played Then
    UI.Input Translate("Enter new name:"), Format(Translate("Rename Starship #%d"), Id), 20, "gm20", Name
    SetName UI.Result
  EndIf
EndSub

% Primary enemy [E]
% @since PCC2 2.40.1
Sub CCUI.Ship.SetEnemy
  % ex int/if/guiif.h:IFCCChangePE
  Local UI.Result
  If Played Then
    With Listbox(Translate("Change Primary Enemy"), Enemy$, 0, 0, "pcc2:shipscreen") Do
      AddItem 0, Translate("0 - none")
      ForEach Player Do AddItem Race$, Format("%X - %s", Race$, Race.Short)
      Call Run
    EndWith
    SetEnemy UI.Result
  EndIf
EndSub

% Ship comment [F9]
% @since PCC2 2.40.1
Sub CCUI.Ship.SetComment
  % ex core.q:CC$SetShipComment
  UI.Input Translate("Enter new comment for this ship:"), Translate("Edit Comment"), 255, "gm30", Comment
  SetComment UI.Result
EndSub

% Planet comment [F9]
% @since PCC2 2.40.1
Sub CCUI.Planet.SetComment
  % ex core.q:CC$SetPlanetComment
  UI.Input Translate("Enter new comment for this planet:"), Translate("Edit Comment"), 255, "gm30", Comment
  SetComment UI.Result
EndSub

% Planet comment here [Shift-F9]
% @since PCC2 2.40.13
Sub CCUI.Planet.SetCommentAt (x, y)
  % ex core.q:CC$SetPlanetCommentAt
  Local UI.Result
  Local pid = PlanetAt(x, y, True)
  If pid Then
    UI.Input Format(Translate("Enter new comment for %s:"), Planet(pid).Name), Translate("Edit Comment"), 255, "gm30", Planet(pid).Comment
    Call Planet(pid).SetComment UI.Result
  EndIf
EndSub


% Build or go to base [F8]
% @since PCC2 2.40.8
Sub CCUI.Planet.BuildOrGoToBase
  % ex core.q:CC$BuildOrGotoBase
  If Base.YesNo
    Try UI.GotoScreen 3, Id
  Else
    Try CC$BuildBase
  EndIf
EndSub



% Extended Mission Selection for a ship
% (This dialog is required for regular ships and for Global Actions / Ship Tasks)
% Returns UI.Result = Array(m,i,t) or empty
% @since PCC2 2.40.1
Sub CCUI.Ship.ChooseExtendedMission (m, i, t)
  % ex client/dlg-mission.cc:WExtendedMission
  Option LocalSubs(1)
  Local mr, ir, tr

  Local Sub OnOK
    % ex WExtendedMission::onOk
    mr = Val(mi->Value)
    If IsEmpty(mr) Or mr<0 Or mr>10000 Then
      Call mi->Focus
      Return
    EndIf
    ir = Val(ii->Value)
    If IsEmpty(ir) Or ir<0 Or ir>10000 Then
      Call ii->Focus
      Return
    EndIf
    tr = Val(ti->Value)
    If IsEmpty(tr) Or tr<0 Or tr>10000 Then
      Call ti->Focus
      Return
    EndIf
    UI.EndDialog 1
  EndSub

  Local Sub OnCancel
    % ex WExtendedMission::onCancel
    UI.EndDialog 0
  EndSub


  Local a := UI.Dialog(Translate("Extended Mission"))

  Local aa := a->NewGridBox(2)
  aa->NewLabel(Translate("Mission number:"))
  Local mi := aa->NewFrame("lowered", 1)->NewInput(5, "5nm", m, "alt-m")
  aa->NewLabel(Translate("Intercept number:"))
  Local ii := aa->NewFrame("lowered", 1)->NewInput(5, "5nm", i, "alt-i")
  aa->NewLabel(Translate("Tow number:"))
  Local ti := aa->NewFrame("lowered", 1)->NewInput(5, "5nm", t, "alt-t")

  Local ab := a->NewHBox()
  ab->NewButton(Translate("OK"),     "ret", "OnOK")
  ab->NewButton(Translate("Cancel"), "esc", "OnCancel")
  ab->NewSpacer()

  a->NewKeyboardFocus("vt", mi, ii, ti)

  Call a->Run

  If UI.Result Then
    UI.Result := Array(mr, ir, tr)
  Else
    UI.Result := Z(0)
  EndIf
EndSub

% Choose single mission parameter.
% - title: window title (mission name)
% - label: label/prompt for input (parameter name)
% - type: type of parameter (Tow.Type, Intercept.Type)
% - flags: parameter flags
% - value: initial value
% - sid: invoking ship
% Returns entered parameter, EMPTY if cancelled
% @since PCC2 2.40.8
Function CCUI$Ship.ChooseOneMissionParameter (title, label, type, flags, value, sid)
  % ex client/dlg-mission.cc:getMissionArg
  Local UI.Result
  Local HELP = 'pcc2:shipscreen'
  Local notThis = InStr(flags, '!')
  Local ownOnly = InStr(flags, 'o')
  Local p
  Select Case type
    Case 'p'
      % PlanetParameter
      If ownOnly Then
        UI.ChooseObject 2
        Return UI.Result
      Else
        % FIXME: this is slow because it performs 500 UI/Game transitions. Fortunately it is not used.
        % FIXME: this looks plain. Fortunately it is not used.
        Local list = Listbox(title, value)
        ForEach Global.Planet As p Do Call list->AddItem p->Id, p->Name
        Call list->Run
        Return UI.Result
      EndIf

    Case 's'
      % ShipParameter
      Local dialogFlags = ''
      If Not ownOnly Then dialogFlags := dialogFlags & 'f'
      If notThis And sid Then dialogFlags := dialogFlags & sid
      CC$ChooseInterceptTarget title, dialogFlags
      Return UI.Result

    Case 'h'
      % HereParameter
      Local dialogFlags = 'e'
      If Not ownOnly Then dialogFlags := dialogFlags & 'f'
      If notThis And sid Then dialogFlags := dialogFlags & sid
      UI.ListShips Global.Ship(sid).Loc.X, Global.Ship(sid).Loc.Y, dialogFlags, Translate("OK"), title
      Return UI.Result

    Case 'b'
      % BaseParameter
      UI.ChooseObject 3
      Return UI.Result

    Case 'y'
      % PlayerParameter
      Local list = Listbox(title, value)
      ForEach Global.Player As p Do
        If Not notThis Or Not sid Or Global.Ship(sid).Owner$<>p->Race$ Then
          Call list->AddItem p->Race$, Format("%X - %s", p->Race$, p->Race.Short)
        EndIf
      Next
      Call list->Run
      Return UI.Result

    Case Else
      % also 'n': IntegerParameter
      UI.InputNumber title, 0, 10000, value, HELP, If(IsEmpty(label), title, label)
      Return UI.Result
  EndSelect
EndFunction

Function CCUI$Ship.ChooseTwoMissionParameters (newM, args, sid)
  % ex getTwoMissionArgs
  % Event Handlers
  Option LocalSubs(1)
  Local Sub OnOK
    Local i, v
    For i:=0 To 1 Do
      If types(i) = "n" Then
        v := Val(widgets(i).Value)
        If IsEmpty(v) Or v<0 Or v>10000 Then
          Call widgets(i).Focus
          Return
        EndIf
        args(i) := v
      EndIf
    Next
    UI.EndDialog 1
  EndSub
  Local Sub OnCancel
    UI.EndDialog 0
  EndSub
  Local Sub OnEdit (i)
    Local v = CCUI$Ship.ChooseOneMissionParameter (newM->Name, names(i), types(i), flags(i), args(i), sid)
    If Not IsEmpty(v) Then
      args(i) := v
      widgets(i).Value := FormatParam(i)
    EndIf
  EndSub

  % Local state
  Dim widgets(2)
  Local types = Array(newM->Intercept.Type, newM->Tow.Type)
  Local names = Array(newM->Intercept.Name, newM->Tow.Name)
  Local flags = Array(newM->Intercept.Flags, newM->Tow.Flags)
  Local keys = Array('alt-i', 'alt-t')
  Local UI.Result

  Local Function FormatParam(i)
    % ex WShipArgWidget::drawContent
    Local s
    Local v = args(i)
    Select Case types(i)
      Case 'p', 'b'
        s := If(v, Global.Planet(v).Name, Translate("<not set>"))
      Case 's', 'h'
        s := If(v, Global.Ship(v).Name, Translate("<not set>"))
      Case 'y'
        s := If(v, Global.Player(v).Race.Short, Translate("<not set>"))
    EndSelect
    If IsEmpty(s) Or Not s Then s := v
    Return s
  EndFunction

  % Dialog
  Local a := UI.Dialog(newM->Name)
  Local aa := a->NewGridBox(2)
  Local i, wantAdvice
  For i:=0 To 1 Do
    % ex client/dlg-mission.cc:createWidget
    aa->NewLabel(names(i))
    If types(i) = "n" Then
      widgets(i) := aa->NewFrame("lowered", 1)->NewInput(5, "5nm", args(i), keys(i))
    Else
      widgets(i) := aa->NewFrame("lowered", 1)->NewPseudoInput(FormatParam(i), keys(i), "OnEdit " & i, "5nm")
      wantAdvice := True
    EndIf
  Next

  If wantAdvice Then
    a->NewLabel(Translate("Press space on a field to change it."), '-')
  EndIf

  Local ab := a->NewHBox()
  ab->NewButton(Translate("OK"),     "ret", "OnOK")
  ab->NewButton(Translate("Cancel"), "esc", "OnCancel")
  ab->NewSpacer()
  a->NewKeyboardFocus("vt", widgets(0), widgets(1))
  Call a->Run
  Return UI.Result
EndFunction

% Edit mission parameters
% - newM: mission (Mission() result)
% - args: two-element array of parameters, pre-initialized with current values
% - sid: invoking ship
% Returns nonzero on success, args updated in-place.
% @since PCC2 2.40.8
Function CCUI$Ship.ChooseMissionParameters (newM, args, sid)
  Local n
  If newM->Intercept.Type
    If newM->Tow.Type
      Return CCUI$Ship.ChooseTwoMissionParameters (newM, args, sid)
    Else
      % One parameter (intercept)
      n := CCUI$Ship.ChooseOneMissionParameter(newM->Name, newM->Intercept.Name, newM->Intercept.Type, newM->Intercept.Flags, args(0), sid)
      If Not IsEmpty(n)
        args(0) := n
        Return 1
      Else
        Return 0
      EndIf
    EndIf
  Else
    If newM->Tow.Type
      % One parameter (tow)
      n := CCUI$Ship.ChooseOneMissionParameter(newM->Name, newM->Tow.Name, newM->Tow.Type, newM->Tow.Flags, args(1), sid)
      If Not IsEmpty(n)
        args(1) := n
        Return 1
      Else
        Return 0
      EndIf
    Else
      % No parameters
      Return 1
    EndIf
  EndIf
EndFunction

% Prepare mission list for a ship
% - a: Listbox
% - i: ship Id
Sub CCUI$Ship.PrepareMissionList (a, i)
  Local msn
  ForEach Global.Mission As msn Do
    If CCVP.MissionWorksOnShip(msn, Ship(i)) Then
      Call a->AddItem msn->Number, Format("%s - %s", msn->Key, msn->Name)
    EndIf
  Next
  Call a->AddItem, -1, Translate("# - Extended Mission")
EndSub

% Complete mission selection by asking for parameters
% Returns Array(m, i, t, onSetCommand)
Function CCUI$Ship.CompleteMissionSelection (newNr, shipId)
  Local res
  If Not IsEmpty(newNr) Then
    % Tow chain warning
    If shipId And newNr=7 And FindShip(Mission$=7 And Mission.Tow=shipId) Then
      UI.Message Translate("This ship is already being towed. Tow chains will not work.\nContinue anyway?"), Translate("Ship Mission"), Translate("Yes No")
      If UI.Result<>1 Then Return res
    EndIf

    If newNr=-1 Then
      % Extended Mission
      CCUI.Ship.ChooseExtendedMission Mission$, Mission.Intercept, Mission.Tow
      If Not IsEmpty(UI.Result) Then
        res := Array(UI.Result(0), UI.Result(1), UI.Result(2), Z(0))
      EndIf
    Else
      % Normal mission
      Local args(2)
      Local newM = Global.Mission(newNr,    Owner.Real)
      Local oldM = Global.Mission(Mission$, Owner.Real)
      args(0) := If((newNr = Mission$) Or (newM->Intercept.Name = oldM->Intercept.Name), Mission.Intercept, 0)
      args(1) := If((newNr = Mission$) Or (newM->Tow.Name       = oldM->Tow.Name),       Mission.Tow,       0)
      If CCUI$Ship.ChooseMissionParameters(newM, args, shipId)
        res := Array(newNr, args(0), args(1), newM->Command)
      EndIf
    EndIf
  EndIf
  Return res
EndFunction

% Check presence of a lock and ask whether to override it.
% Returns true to proceed.
Function CC$QueryLock(lockName, title)
  % ex cscreen.pas:CsQueryLock
  Local UI.Result
  If GetLockInfo(lockName, 0) Then
    If GetLockInfo(lockName, 2) Then
      UI.Message Format(Translate("%s (%s)\nProceed anyway?"), GetLockInfo(lockName, 2), GetLockInfo(lockName, 1)), title, Translate("Yes No")
    Else
      UI.Message Format(Translate("This function is under control of \"%s\".\nProceed anyway?"), GetLockInfo(lockName, 1)), title, Translate("Yes No")
    EndIf
    If UI.Result<>1 Then Return False
  EndIf
  Return True
EndFunction

% Ship operation
% FIXME: when the waypoint is tied to the scanner, and keyboard move is enabled,
% this triggers on every movement.
Sub CC$WithShipWaypoint (cmd, opt, Optional overrideAT)
  % ex fleet.pas:NFleetWarning (sort-of)
  Local UI.Result
  Local sid = Id
  Local s, T
  If IsEmpty(overrideAT) Then overrideAT:=False

  % Check waypoint lock
  s := "s" & sid & ".waypoint"
  If Not overrideAT And Not CC$QueryLock("s" & sid & ".waypoint", Translate("Waypoint")) Then Return

  % Check fleet
  If Fleet$<>0 And Fleet$<>sid Then
    UI.Message Translate("This ship is member of a fleet. Do you want to set the waypoint of the whole fleet, or do you want to leave the fleet?"), Translate("Waypoint"), Translate("Set Leave Cancel")
    If UI.Result=3 Then Return
    If UI.Result=1 Then
      % Set
      sid:=Fleet$
    Else
      % Leave fleet
      Fleet$:=0
    EndIf
  EndIf

  % All tests passed
  With Ship(sid) Do Eval cmd

  % Set optimum speed
  If opt Then
    If Ship(sid).Speed<>"Hyperdrive" And Ship(sid).Waypoint.Dist>0 Then
      % Find out how long it takes to get there at this speed
      s := Global.Engine(Ship(sid).Engine$).Speed$
      Ship(sid).Speed$ := s
      T := Ship(sid).Move.Eta
      If T < 30 Then % FIXME: hardcoded value
        % Find whether we can reach the target with a slower speed
        Do While s > 1
          s := s-1
          Ship(sid).Speed$ := s
          If Ship(sid).Move.Eta > T Then
            % It's slower, so undo and stop
            Ship(sid).Speed$ := s+1
            Break
          EndIf
        Loop
      EndIf
    EndIf
  EndIf
EndSub

Sub CC$WithFleetWaypoint (cmd, opt)
  With Ship(Fleet$) Do CC$WithShipWaypoint cmd, opt
EndSub


% Ship mission [M]
% @since PCC2 2.40.1
Sub CCUI.Ship.SetMission
  % ex mission.pas:NSetMission (sort-of)
  Local _ := Translate
  Local UI.Result, System.Err
  Local i := Id
  If Played Then
    % Warning
    If GetCommand('beamup ' & i) Then
      UI.Message _("This ship has an active \"Beam Up Multiple\" order. Its mission will be overridden by PHost.\nContinue anyway?"), _("Ship Mission"), _("Yes No")
      If UI.Result<>1 Then Return
    EndIf
    If Fleet.Status='member' And CCVP.MissionLocksWaypoint(Global.Mission(Mission$, Owner.Real)) Then
      UI.Message _("This ship is member of a fleet which is on a intercept course. Changing the mission will lose the intercept lock.\nLeave the fleet and continue?"), _("Ship Mission"), _("Yes No")
      If UI.Result<>1 Then Return
      SetFleet 0
    EndIf

    % Build listbox
    Local a := Listbox(_("Ship Mission"), Mission$, 340, 12, "pcc2:shipscreen")
    CCUI$Ship.PrepareMissionList a, i
    Call a->Run

    % Process result
    Local r := CCUI$Ship.CompleteMissionSelection(UI.Result, i)
    If Not IsEmpty(r) Then
      If CCVP.MissionLocksWaypoint(Global.Mission(r(0), Owner.Real)) And Not CC$QueryLock('s' & Id & '.waypoint', _("Ship Mission")) Then Return
      SetMission r(0), r(1), r(2)

      % Execute 'OnSet=' command
      i := r(3)
      If i Then Eval i
    EndIf
  EndIf
EndSub

% Remote control (r)
% @since PCC2 2.40.9
Sub CCUI.Ship.ToggleRemote
  % ex IFCCRemoteControl (sort-of)
  % Also see VisualScanDialog::Window::toggleRemoteControl
  Local UI.Result
  Local q := CC$RemoteGetQuestion(Id)
  If q Then
    UI.Message q, Translate("Remote Control"), Translate("Yes No")
    If UI.Result=1 Then CC$RemoteToggle Id
  EndIf
EndSub


% @since PCC2 2.40.10
Function CCUI$Ship.ValidateShipyard()
  Local UI.Result
  Local p = Planet(Orbit$)
  If p->Shipyard.Action='Fix' Then
    UI.Message Format(Translate("This starbase is already repairing %s. If you continue, this order will be replaced. Continue?"), p->Shipyard.Name), Translate("Yes No")
    Return UI.Result=1
  Else If p->Shipyard.Action='Recycle' Then
    UI.Message Format(Translate("This starbase is already recycling %s. If you continue, this order will be replaced. Continue?"), p->Shipyard.Name), Translate("Yes No")
    Return UI.Result=1
  Else
    Return True
  EndIf
EndFunction

% Fix this ship
% @since PCC2 2.40.10
Sub CCUI.Ship.Fix
  If CCUI$Ship.ValidateShipyard() Then FixShip
EndSub

% Recycle this ship
% @since PCC2 2.40.10
Sub CCUI.Ship.Recycle
  If CCUI$Ship.ValidateShipyard() Then RecycleShip
EndSub

% Cancel Fix/Recycle order
% @since PCC2 2.40.10
Sub CCUI.Ship.CancelShipyard
  With Planet(Orbit$) Do FixShip 0
EndSub

% Go to or join a fleet
% S-F10 on ship screen
Sub CCUI.Ship.GotoFleet
  % ex CC$ShipGoToFleet
  Local UI.Result
  If Fleet$ Then
    UI.GotoScreen 10, Fleet$
  Else
    If Not FindShip(Fleet$) Then
      UI.Message RXml(Translate("You have not yet set up a fleet to join. Use the \"Start New Fleet\" command on the <kbd>F10</kbd> menu to start one.")), Translate("Join Fleet"), Translate("OK")
    Else
      UI.ListFleets Loc.X, Loc.Y, "a", Translate("OK"), Translate("Join Fleet")
      If UI.Result Then Fleet$ := UI.Result
    EndIf
  EndIf
EndSub

% Go to fleet screen
% @since PCC2 2.40.13
Sub CCUI.Ship.GotoFleetScreen
  Local UI.Result = Fleet$
  If Not UI.Result Then UI.ChooseObject 10
  If UI.Result Then UI.GotoScreen 10, UI.Result
EndSub

% Go to fleet leader
% @since PCC2 2.40.13
Sub CCUI.Ship.GotoFleetLeader
  If Fleet$ Then UI.GotoScreen 1, Fleet$
EndSub

% Leave fleet
% @since PCC2 2.40.13
Sub CCUI.Ship.LeaveFleet
  Fleet$:=0
EndSub

% Start new fleet
% @since PCC2 2.40.13
Sub CCUI.Ship.StartNewFleet
  Fleet$:=Id
  Iterator(10).CurrentIndex:=Id
EndSub

% Join a fleet
% @since PCC2 2.40.13
Sub CCUI.Ship.JoinFleet
  % ex ship.pas:JoinAFleet
  Local UI.Result
  UI.ListFleets Loc.X, Loc.Y, "a", Translate("OK"), Translate("Join Fleet")
  If UI.Result Then Fleet$:=UI.Result
EndSub

Function CCUI$Ship.IsValidFleetMember(sh)
  % ex isPossibleFleetMember(const GShip& caller, const GShip& other)
  % call in context of ship 'other', with sh being 'caller'
  Return Id<>sh->Id And Played And Owner$=sh->Owner$ And Fleet$=0 And Loc.X=sh->Loc.X And Loc.Y=sh->Loc.Y
EndFunction

% Add all lone ships at this place to a fleet
% @since PCC2 2.40.13
Sub CCUI.Ship.AddAllToFleet
  % Start fleet if necessary
  Local cc$fid = Fleet$
  If Not cc$fid Then Fleet$ := cc$fid := Id

  % Create fleet
  Local cc$me = Global.Ship(Id)
  ForEach Ship Do If CCUI$Ship.IsValidFleetMember(cc$me) Then Fleet$ := cc$fid
  Iterator(10).CurrentIndex:=cc$fid
EndSub

%%% Fleet

% Add all fleet members to simulation
% C-ins on fleet screen
% @since PCC2 2.40.13
Sub CCUI.Fleet.AddToSim
  Local fid = Fleet$
  ForEach Ship Do
    If Fleet$=fid Then CC$AddToSim False
  Next
EndSub

% Make a ship leader of its fleet
% 'b' in fleet screen
% @since PCC2 2.40.13
Sub CCUI.Fleet.ChangeLeader
  % ex CC$ChangeLeader
  Local UI.Result
  If Fleet$ = Id Then Return      % ship is already the leader
  UI.Message Translate("Make this ship the leader of the fleet?"), Translate("Fleet"), Translate("Yes No")
  If UI.Result=1 Then ChangeFleetLeader Fleet$, Id
EndSub

% Dissolve a fleet
% C-DEL on fleet screen
% @since PCC2 2.40.13
Sub CCUI.Fleet.Dissolve
  % ex CC$FleetDeleteFleet
  Local UI.Result
  Local fid = Fleet$
  UI.Message Translate("Dissolve this fleet?"), Translate("Fleet"), Translate("Yes No")
  If UI.Result=1 Then
    ForEach Ship Do If Id<>fid And Fleet$=fid Then Fleet$:=0
    Ship(fid).Fleet$:=0
  EndIf
EndSub

% Join a fleet into another
% 'j' on fleet screen
% @since PCC2 2.40.13
Sub CCUI.Fleet.Join
  % ex CC$FleetJoin, fleet.pas:JoinFleetWith
  Local UI.Result
  Local fid = Fleet$
  If Iterator(10).NextIndex(fid, "w")=fid Then
    MessageBox Translate("There is no other fleet we could join."), Translate("Join Fleet")
  Else
    UI.ListFleets Ship(fid).Loc.X, Ship(fid).Loc.Y, "a" & fid, Translate("OK"), Translate("Join Fleet")
    If Not IsEmpty(UI.Result) Then
      % Move members, then leader
      ForEach Ship Do If Id<>fid And Fleet$=fid Then Fleet$:=UI.Result
      Ship(fid).Fleet$:=UI.Result
    EndIf
  EndIf
EndSub

% Leave fleet
% DEL on fleet screen
% @since PCC2 2.40.13
Sub CCUI.Fleet.Leave
  % ex CC$FleetDeleteMember
  Local UI.Result
  If Fleet$ = Id Then
    % This is the fleet leader, so we'd better ask
    UI.Message Translate("Remove leader from fleet?"), Translate("Fleet"), Translate("Yes No")
    If UI.Result<>1 Then Return
  EndIf
  Fleet$ := 0
EndSub

% Rename a fleet
% 'n' on fleet screen
% @since PCC2 2.40.13
Sub CCUI.Fleet.Rename
  % ex CC$FleetName
  Local UI.Result
  Local fid = Fleet$
  UI.Input Translate("Enter new name:"), Format(Translate("Rename Fleet #%d"), fid), 255, "gm20", Ship(fid).Fleet.Name
  SetFleetName UI.Result
EndSub

% Set fleet speed
% @since PCC2 2.40.13
Sub CCUI.Fleet.SetSpeed
  CC$WithFleetWaypoint 'CC$ChangeSpeed', False
EndSub

% Split a fleet
% 'p' on fleet screen
% @since PCC2 2.40.13
Sub CCUI.Fleet.Split
  % ex CC$FleetSplit, fleet.pas:SplitFleet
  Local UI.Result
  Local oldfid = Fleet$                       % old fleet Id
  Local newmark = Not Ship(oldfid).Marked     % selection status for new fleet
  Local newcount = 0                          % number of ships in new fleet
  Local newfid = If (Marked=newmark, Id, 0)   % new leader is this ship if it has the right status

  % Count ships
  ForEach Ship Do
    If Fleet$=oldfid And Marked=newmark Then
      newcount:=newcount+1
      If newfid=0 Then newfid:=Id
    EndIf
  Next
  If newcount=0 Then
    UI.Message RXml(Translate("To split a fleet, please select the ships which should go into one fleet using <kbd>.</kbd>, and unselect those which should go into the other.")), Translate("Split Fleet")
    Return
  EndIf

  % Confirm
  UI.Message Format(Translate("Form a new fleet with %d ship%!1{s%}, leader is %s?"), newcount, Ship(newfid).Name), Translate("Split Fleet"), Translate("Yes No")
  If UI.Result<>1 Then Return

  % Do it
  Ship(newfid).Fleet$ := newfid
  ForEach Ship Do If Fleet$=oldfid And Marked=newmark Then Fleet$:=newfid

  % Bring user there
  If Fleet$=newfid Then Iterator(10).CurrentIndex:=newfid
EndSub

% Tow a fleet member
Sub CCUI.Fleet.TowMember
  % ex IFCCTowFleetMember, fleet.pas:TowMember
  % Must be part of a fleet
  Local TOW_MISSION = 7
  Local fid = Fleet$, sid = Id
  Local sh, list, UI.Result, ok, chosen
  If Not Fleet$ Then Abort "Ship is not member of a fleet."

  % Check whether this ship can tow
  Local msn = Global.Mission(7, Owner.Real)
  If Not IsEmpty(msn) And Not CCVP.MissionWorksOnShip(msn, Ship(Id)) Then
    UI.Message Translate("This ship cannot tow."), Translate("Tow")
    Return
  EndIf

  % Check intercept
  msn := Global.Mission(Ship(fid).Mission$, Ship(fid).Owner.Real)
  If InStr(msn->Flags, "i") Then
    UI.Message Translate("This ship is member of a fleet which is on a intercept course. You cannot tow and intercept at the same time."), Translate("Tow")
    Return
  EndIf

  % Build list of possible targets
  list := Listbox(Translate("Tow"), Mission.Tow, 0, -1, "pcc2:fleetscreen")
  ok := False
  ForEach Ship As sh Do
    If sh->Id<>sid And sh->Fleet$=fid And sh->Loc.X=Loc.X And sh->Loc.Y=Loc.Y Then
      Call list->AddItem sh->Id, Reference("s", sh->Id).Name.Full
      ok := True
    EndIf
  Next
  If Mission$=7 Then
    Call list->AddItem 0, Translate("Cancel tow")
    ok := True
  EndIf

  % Anything?
  If Not ok Then
    UI.Message Translate("There is no other fleet member at this position."), Translate("Tow")
    Return
  EndIf

  % Ask user
  Call list->Run
  If Not IsEmpty(UI.Result) Then
    If UI.Result Then
      chosen := UI.Result
      If FindShip(Mission$=7 And Mission.Tow=sid) Then
        UI.Message Translate("This ship is already being towed. Tow chains will not work.\nContinue anyway?"), Translate("Ship Mission"), Translate("Yes No")
        If UI.Result<>1 Then Return
      EndIf
      SetMission 7, 0, chosen
    Else
      SetMission 0
    EndIf
  EndIf
EndSub


% Mission Selection for a starbase
% (This dialog is required for regular ships and for Global Actions / Ship Tasks)
% Returns UI.Result=mission or empty
% @since PCC2 2.40.1
Sub CCUI.Base.ChooseMission (m, Optional h)
  % ex client/act-planet.cc:doBaseChangeMission (part), pdata.pas:GetStarbaseMission
  Local _ := Translate
  Local a := Listbox(_("Starbase Mission"), m, 300, 7, If(h, h, "pcc2:basescreen"))

  % FIXME: can we access these strings without duplicating them from compiled code?
  Call a->AddItem 0, Format("%X - %s", 0, _("none"))
  Call a->AddItem 1, Format("%X - %s", 1, _("Refuel ships"))
  Call a->AddItem 2, Format("%X - %s", 2, _("Maximize defense"))
  Call a->AddItem 3, Format("%X - %s", 3, _("Load torpedoes onto ships"))
  Call a->AddItem 4, Format("%X - %s", 4, _("Unload incoming ships"))
  Call a->AddItem 5, Format("%X - %s", 5, _("Repair base"))
  Call a->AddItem 6, Format("%X - %s", 6, _("Force surrender"))

  Call a->Run
EndSub

% Starbase mission [M]
% @since PCC 2.40.1
Sub CCUI.Base.SetMission
  % ex client/act-planet.cc:doBaseChangeMission (part), bdata.pas:ChangeStarbaseMission
  Local UI.Result
  If Played Then
    CCUI.Base.ChooseMission Mission$
    If Not IsEmpty(UI.Result) Then
      SetMission UI.Result
    EndIf
  EndIf
EndSub

% @since PCC 2.40.9
Sub CCUI.Base.FixShip
  Local UI.Result
  UI.ListShips Loc.X, Loc.Y, 'f', Translate("OK"), Translate("Fix Ship")
  FixShip UI.Result
EndSub

% @since PCC 2.40.9
Sub CCUI.Base.RecycleShip
  Local UI.Result
  UI.ListShips Loc.X, Loc.Y, '', Translate("OK"), Translate("Recycle Ship")
  RecycleShip UI.Result
EndSub


Sub CCUI$Give(title, command, me)
  % ex client/act-ship.cc:doGive, doGiveShip, doGivePlanet, phost.pas:NGive
  Local UI.Result, ll, cur

  % Figure out current
  cur := Val(GetCommand(command))
  If IsEmpty(cur) Then cur:=me

  % Build list box
  ll := Listbox(title, cur)
  ForEach Player Do
    If Race$=me Then
      Call ll->AddItem Race$, Format(Translate("%X - (don't give it away)"), Race$)
    Else
      Call ll->AddItem Race$, Format("%X - %s", Race$, Race.Short)
    EndIf
  Next

  % Run list box
  Call ll->Run

  % Produce command
  If Not IsEmpty(UI.Result) Then
    If UI.Result=me Then
      DeleteCommand command
    Else
      AddCommand command & " " & UI.Result
    EndIf
  EndIf
EndSub

% Give a unit away
Sub CCUI.Give
  % ex int/if/guiif.cc:IFCCGive
  If Played And CCVP.AllowGive() Then
    If Type.Short='p' Then
      CCUI$Give Translate("Give Planet To..."), "give planet " & Id, Owner$
    Else
      CCUI$Give Translate("Give Ship To..."), "give ship " & Id, Owner.Real
    EndIf
  EndIf
EndSub

% Search backend
%  flags - as for UI.Search
%  match - a function taking an object, returning match (must not throw)
% Returns
%  a ReferenceList() - success
%  a string - "regular" failure
% This function must not throw; throw means irregular failure with scary error message
% @since PCC2 2.40.7
Function CCUI$Search(flags, match)
  Dim obj, own, result, hasObjects=0
  result := ReferenceList()
  own := (InStr(flags, 'm'))

  % Planets/Bases
  If InStr(flags, 'p') Then
    ForEach Planet As obj Do
      If (Not own Or obj->Played) Then
        hasObjects := 1
        If Match(obj) Then Call result->AddObjects 'p', obj->Id
      EndIf
    Next
  Else
    If InStr(flags, 'b') Then
      ForEach Planet As obj Do
        If (Not own Or obj->Played) And Obj->Base.YesNo Then
          hasObjects := 1
          If Match(obj) Then Call result->AddObjects 'b', obj->Id
        EndIf
      Next
    EndIf
  EndIf

  % Ships
  If InStr(flags, 's') Then
    ForEach Ship As obj Do
      If (Not own Or obj->Played) Then
        hasObjects := 1
        If Match(obj) Then Call result->AddObjects 's', obj->Id
      EndIf
    Next
  EndIf

  % Ufos
  If InStr(flags, 'u') And Not own Then
    ForEach Ufo As obj Do
      hasObjects := 1
      If Match(obj) Then Call result->AddObjects 'u', obj->Id
    Next
  EndIf

  % Minefields and Ion Storms
  If InStr(flags, 'o') And Not own Then
    ForEach Minefield As Obj Do
      hasObjects := 1
      If Match(obj) Then Call result->AddObjects 'm', obj->Id
    Next
    ForEach Storm As Obj Do
      hasObjects := 1
      If Match(obj) Then Call result->AddObjects 'i', obj->Id
    Next
  EndIf

  % If we have no results, check why
  If Dim(result->Objects)=0 Then
    If Not hasObjects Then Return Translate("There are no objects of the requested kind.")
    % FIXME: deal with runtime errors from e.g. misspelled property names, and report those to north side
  EndIf

  Return result
EndFunction

% @since PCC2 2.40.7
Function CCUI$TryGotoScreen(screen, id)
  % ex postGoToScreen (sort-of)
  Local System.Err
  If IsEmpty(screen) Or IsEmpty(id) Then
    Return False
  Else
    Try
      UI.GotoScreen screen, id
      Return True
    Else
      Return False
    EndTry
  EndIf
EndFunction

% @since PCC2 2.40.7
Function CCUI$TryGotoChart(x, y)
  % ex postGoToChart (sort-of)
  Local System.Err
  If IsEmpty(x) Or IsEmpty(y) Then
    Return False
  Else
    UI.GotoChart x, y
    Return True
  EndIf
EndFunction

% @since PCC2 2.40.10
Function CCUI$TryGotoMinefield(id)
  If Minefield(id) Then
    Iterator(32).CurrentIndex := id
    CC$MinefieldInfo
    Return True
  Else
    Return False
  EndIf
EndFunction

% @since PCC2 2.40.10
Function CCUI$TryGotoIonStorm(id)
  If Storm(id) Then
    Iterator(31).CurrentIndex := id
    CC$IonStormInfo
    Return True
  Else
    Return False
  EndIf
EndFunction

% @since PCC2 2.40.10
Function CCUI$TryGotoUfo(id)
  Local x
  If Ufo(id) Then
    % For Ufos, Id and Index are not the same
    x := Iterator(30).Index(Id)
    If x Then
      Iterator(30).CurrentIndex := x
      CC$UfoInfo
      Return True
    EndIf
  EndIf
EndFunction


% @q UI.GotoReference ref:Reference (Global Command)
% Go to referenced object.
% If that object has a control screen or information window, opens that.
% Otherwise, shows it on the starchart.
% @since PCC2 2.40.7
% @see UI.GotoScreen
Sub UI.GotoReference(ref)
  % ex postGoToObject
  Select Case ref->Kind
    Case 'ship'
      CCUI$TryGotoScreen(1, ref->Id) Or CCUI$TryGotoChart(ref->Object->Loc.X, ref->Object->Loc.Y)
    Case 'planet'
      CCUI$TryGotoScreen(2, ref->Id) Or CCUI$TryGotoChart(ref->Object->Loc.X, ref->Object->Loc.Y)
    Case 'base'
      CCUI$TryGotoScreen(3, ref->Id) Or CCUI$TryGotoScreen(2, ref->Id) Or CCUI$TryGotoChart(ref->Object->Loc.X, ref->Object->Loc.Y)
    Case 'location'
      CCUI$TryGotoChart(ref->Loc.X, ref->Loc.Y)
    Case 'minefield'
      CCUI$TryGotoMinefield(ref->Id) Or CCUI$TryGotoChart(ref->Object->Loc.X, ref->Object->Loc.Y)
    Case 'storm'
      CCUI$TryGotoIonStorm(ref->Id) Or CCUI$TryGotoChart(ref->Object->Loc.X, ref->Object->Loc.Y)
    Case 'ufo'
      CCUI$TryGotoUfo(ref->Id) Or CCUI$TryGotoChart(ref->Object->Loc.X, ref->Object->Loc.Y)
  EndSelect
EndSub


%
%  Sharing Objects
%

Sub CC$Share
  % ex team.pas:MailCommand
  % As of 20221022, VPA data transmissions (Message.Encoded) are supported for planets, minefields and drawings,
  % whereas 'show' is supported for planets, minefields and ships.
  % For now, the UI can only share planets and minefields which are supported by both.
  Local mode, UI.Result
  If Cfg("CPEnableShow") Then
    With Listbox("") Do
      AddItem 1, Translate("Current information (message)")
      AddItem 2, Translate("Next-turn information (\"show\")")
      RunMenu UI.Key
    EndWith
    mode := UI.Result
  Else
    mode := 1
  EndIf

  Select Case mode
    Case 1
      CC$SendMessage Message.Encoded
    Case 2
      CC$EditShowCommand
  EndSelect
EndSub

Sub CCUI.Planet.Share
  CC$Share
EndSub

Sub CCUI.Minefield.Share
  CC$Share
EndSub


%
%  Selection Manager
%

Sub CCUI$SaveSelection(title, flags)
  % ex WSelectionManager::doSave
  Local UI.Result, fname, fd
  UI.FileWindow title, "*.sel", "pcc2:selectionmgr"
  If UI.Result
    fname := AppendFileNameExtension(UI.Result, "sel")
    Try
      fd := FreeFile()
      Open fname For Output As #fd
      SelectionSave #fd, flags
      Close #fd
    Else
      MessageBox Format(Translate("Unable to save %s: %s"), fname, System.Err)
    EndTry
  EndIf
EndSub

Sub CCUI$LoadSelection(title, flags)
  % ex WSelectionManager::doLoad
  Local UI.Result, fname, fd
  UI.FileWindow title, "*.sel", "pcc2:selectionmgr"
  If UI.Result
    fname := AppendFileNameExtension(UI.Result, "sel")
    Try
      fd := FreeFile()
      Open fname For Input As #fd
      SelectionLoad #fd, flags
      Close #fd
    Else
      MessageBox Format(Translate("Unable to load %s: %s"), fname, System.Err)
    EndTry
  EndIf
EndSub

% Alt-Left/Alt-Right
Sub CC$SwitchSelection (delta)
  Selection.Layer := (Selection.Layer + 8 + delta) Mod 8
  UI.OverlayMessage Format(Translate("Selection %c"), Chr(Selection.Layer + 65))
EndSub

Sub CC$NextSelection
  CC$SwitchSelection 1
EndSub

Sub CC$PreviousSelection
  Call CC$SwitchSelection, -1
EndSub



%
%  Starchart
%


% @since PCC2 2.40.10
Sub CCUI$Chart.NewCannedMarker
  Local slot = If(UI.Prefix, UI.Prefix Mod 10, 0)
  NewCannedMarker UI.X, UI.Y, slot
EndSub

% @since PCC2 2.40.10
Sub CCUI$Chart.MinefieldOrMarker
  If UI.Prefix Then
    CCUI$Chart.NewCannedMarker
  Else
    Local id := Iterator(32).NearestIndex(UI.X, UI.Y)
    If id Then
      Iterator(32).CurrentIndex := id
      CC$MinefieldInfo
    Else
      UI.OverlayMessage Translate("No Minefields")
    EndIf
  EndIf
EndSub

% @since PCC2 2.40.10
Sub CCUI$Chart.IonStormInfo
  Local id := Iterator(31).NearestIndex(UI.X, UI.Y)
  If id Then
    Iterator(31).CurrentIndex := id
    CC$IonStormInfo
  Else
    UI.OverlayMessage Translate("No Ion Storms")
  EndIf
EndSub

% @since PCC2 2.40.10
Sub CCUI$Chart.UfoInfo
  Local id := Iterator(30).NearestIndex(UI.X, UI.Y)
  If id Then
    Iterator(30).CurrentIndex := id
    CC$UfoInfo
  Else
    UI.OverlayMessage Translate("No Ufos")
  EndIf
EndSub




%
%  Auto Tasks
%
%  (ex WAutoTaskScreen::handleEvent)
%

% @since PCC2 2.40.12
Sub CCUI.Task.AddCommand (cmd)
  % ex WAutoTaskObjectSelection::insert
  If Not IsEmpty(cmd) Then
    Call UI.AutoTask->Add cmd
  EndIf
EndSub

% @since PCC2 2.40.7
Sub CCUI.Task.ToggleComment
  Local cmd = UI.AutoTask->Lines(UI.AutoTask->Cursor)
  If Not IsEmpty(Z(cmd))
    If Left(cmd, 1) = '%' Then
      cmd := Trim(Mid(cmd, 2))
    Else
      cmd := '% ' & cmd
    EndIf

    % This may fail if the new command is not valid, e.g. when trying to un-comment '% If 1'
    Try UI.AutoTask->Lines(UI.AutoTask->Cursor) := cmd
  EndIf
EndSub

% @since PCC2 2.40.7
Sub CCUI.Task.SetCurrent
  % ex WAutoTaskObjectSelection::setPC
  Try
    With UI.AutoTask Do Current := Cursor
  EndTry
EndSub

% @since PCC2 2.40.7
Sub CCUI.Task.DeleteCurrent
  % ex WAutoTaskScreen::handleEvent / case SDLK_DELETE
  Try
    With UI.AutoTask Do Delete Cursor, 1
  EndTry
EndSub

% @since PCC2 2.40.12
Sub CCUI.Task.InsertCommand
  % ex editCommand (part)
  Local UI.Result
  UI.InputCommand Translate("Command:"), Translate("Insert Command"), "ct", "", "pcc2:taskscreen"
  If Not IsEmpty(UI.Result) Then CCUI.Task.AddCommand UI.Result
EndSub

% @since PCC2 2.40.12
Sub CCUI.Task.EditCommand
  % ex editCommand (part)
  Local UI.Result
  If UI.AutoTask->Cursor >= Dim(UI.AutoTask->Lines) Then
    % at end
    CCUI.Task.InsertCommand
  Else
    % on existing command
    Local value = UI.AutoTask->Lines(UI.AutoTask->Cursor)
    UI.InputCommand Translate("Command:"), Translate("Edit Command"), "ct", value, "pcc2:taskscreen"
    If Not IsEmpty(UI.Result) Then UI.AutoTask->Lines(UI.AutoTask->Cursor) := UI.Result
  EndIf
EndSub

% @since PCC2 2.40.7
Sub CCUI.Task.DeleteAll
  % ex WAutoTaskScreen::handleEvent / case ss_Ctrl + SDLK_DELETE
  Local UI.Result
  UI.Message Translate("Delete this auto task?"), Translate("Auto Task"), Translate("Yes No")
  If UI.Result = 1 Then
    Try
      With UI.AutoTask Do Delete 0, Dim(Lines)
    EndTry
  EndIf
EndSub

% @since PCC2 2.40.7
Sub CCUI.Task.SaveToFile
  % ex WAutoTaskScreen::handleSave, autotask.pas:SaveAutoTask
  Local UI.Result, System.Err, name, fd, i, task
  UI.FileWindow Translate("Save Task"), "*.cct"
  If Not IsEmpty(UI.Result)
    name := UI.Result
    Try
      fd := FreeFile()
      Open name For Output As #fd
      task := UI.AutoTask->Lines
      For i:=0 To Dim(task)-1 Do Print #fd, task(i)
      Close #fd
    Else
      MessageBox Format(Translate("Unable to create file %s: %s"), name, System.Err), Translate("Save Task")
    EndTry
  EndIf
EndSub

% @since PCC2 2.40.7
Sub CCUI.Task.LoadFromFile
  % ex WAutoTaskScreen::handleLoad, autotask.pas:ReadAutoTask
  Local UI.Result, System.Err, name, fd, numBadLines, cmd
  UI.FileWindow Translate("Load Task"), "*.cct"
  If Not IsEmpty(UI.Result)
    name := UI.Result
    Try
      fd := FreeFile()
      Open name For Input As #fd

      % Load file line-by-line to catch errors
      numBadLines := 0
      Do
        Input #fd, cmd, 1
        If IsEmpty(cmd) Then Break
        Try
          Call UI.AutoTask->Add cmd
        Else
          numBadLines := numBadLines + 1
        EndTry
      Loop
      Close #fd

      If numBadLines Then
        MessageBox Format(Translate("This file contained %d line%1{ which is%|s which are%} not permitted in Auto Tasks. %1{It has%|They have%} been skipped."), numBadLines), Translate("Load Task")
      EndIf
    Else
      MessageBox Format(Translate("Unable to open file %s: %s"), name, System.Err), Translate("Load Task")
    EndTry
  EndIf
EndSub

% @since PCC2 2.40.12
Sub CCUI.Task.GoToPredictedLocation
  % ex WShipAutoTaskCommandTile::handleEvent (part)
  UI.GotoChart UI.AutoTask->Predicted.Loc.X, UI.AutoTask->Predicted.Loc.Y
EndSub

% @since PCC2 2.40.12
Sub CCUI.Task.EditBuildCommand
  % ex WBaseAutoTaskCommandTile::editCurrentCommand, CBaseTaskScreen.EditEnqueueShip
  Local UI.Result
  CC$EditCurrentBuildOrder
  If Not IsEmpty(UI.Result) Then Try UI.AutoTask->Lines(UI.AutoTask->Cursor) := UI.Result
EndSub

% @since PCC2 2.40.12
Sub CCUI.Task.ConfirmMessage
  Call UI.AutoTask->ConfirmMessage
EndSub

% @since PCC2 2.40.12
Sub CCUI.Task.TogglePredictToEnd
  AddPref "Task.PredictToEnd = " & Not Pref("Task.PredictToEnd")
EndSub

% @since PCC2 2.40.12
Sub CCUI.Task.ToggleShowDistances
  AddPref "Task.ShowDistances = " & Not Pref("Task.ShowDistances")
EndSub



% @since PCC2 2.40.12
Sub CCUI.Task.AddCargoCommand (title, verb, all)
  % ex WShipAutoTaskCommandTile::createCargoTransferCommand
  Local _ = Translate
  Local i, UI.Result
  Local types = Array(_("Neutronium"), _("Tritanium"), _("Duranium"), _("Molybdenum"), _("Colonists"), _("Supplies"), _("Money"), _("All cargo"))
  Local letters = Array('N',             'T',            'D',           'M',             'C',            'S',           '$',        all)

  With Listbox(title, 0, 0, 0, "pcc2:shiptaskscreen") Do
    For i:=0 To Dim(types)-1 Do AddItem i, types(i)
    Call Run
  EndWith
  If Not IsEmpty(UI.Result) Then CCUI.Task.AddCommand Format("%s \"10000%s\", \"n\"", verb, letters(UI.Result))
EndSub

% @since PCC2 2.40.12
Sub CCUI.Task.AddCargoUnload
  CCUI.Task.AddCargoCommand Translate("Cargo Unload"), "CargoUnload", "TDMCS$"
EndSub

% @since PCC2 2.40.12
Sub CCUI.Task.AddCargoUnloadAllShips
  CCUI.Task.AddCommand "CargoUnloadAllShips"
EndSub

% @since PCC2 2.40.12
Sub CCUI.Task.AddCargoUpload
  CCUI.Task.AddCargoCommand Translate("Cargo Upload"), "CargoUpload", "NTDMS$"
EndSub

% @since PCC2 2.40.12
Sub CCUI.Task.AddEnqueueShip
  % ex WBaseAutoTaskCommandTile::createEnqueueShipCommand, CBaseTaskScreen.GenerateEnqueueShip
  Local UI.Result
  CC$EditNewBuildOrder "EnqueueShip"
  CCUI.Task.AddCommand UI.Result
EndSub

% @since PCC2 2.40.12
Sub CCUI.Task.AddMoveTo
  CC$AddWaypoint Translate("Move To"), "MoveTo", "s"
EndSub

% @since PCC2 2.40.12
Sub CCUI.Task.AddMoveToScanner
  % ex WShipTaskScannerChartWidget::onDblClick
  Call UI.AutoTask->AddMovement "MoveTo", UI.X, UI.Y, "s"
EndSub

% @since PCC2 2.40.12
Sub CCUI.Task.AddMoveTowards
  CC$AddWaypoint Translate("Move Towards"), "MoveTowards"
EndSub

% @since PCC2 2.40.12
Sub CCUI.Task.AddNotify
  Local UI.Result
  UI.Input Translate("Enter message:"), Translate("Notify"), 255, "30m"
  If Not IsEmpty(UI.Result) Then CCUI.Task.AddCommand "Notify " + Quote(UI.Result)
EndSub

% @since PCC2 2.40.12
Sub CCUI.Task.AddRestart
  CCUI.Task.AddCommand "Restart"
EndSub

% @since PCC2 2.40.12
Sub CCUI.Task.AddSetBaseMission
  % ex WBaseAutoTaskCommandTile::createSetMissionCommand
  Local UI.Result
  % FIXME predicted base mission!
  CCUI.Base.ChooseMission 0, "pcc2:basetaskscreen"
  If Not IsEmpty(UI.Result) Then CCUI.Task.AddCommand "SetMission " & UI.Result
EndSub

% @since PCC2 2.40.12
Sub CCUI.Task.AddSetComment
  Local UI.Result
  UI.Input Translate("Enter new comment:"), Translate("Set Comment"), 255, "30m"
  If Not IsEmpty(UI.Result) Then CCUI.Task.AddCommand "SetComment " + Quote(UI.Result)
EndSub

% @since PCC2 2.40.12
Sub CCUI.Task.AddSetEnemy
  % ex WShipAutoTaskCommandTile::createSetEnemyCommand
  Local UI.Result
  With Listbox(Translate("Change Primary Enemy"), 0, 0, 0, "pcc2:shiptaskscreen") Do
    AddItem 0, Translate("0 - none")
    ForEach Player Do AddItem Race$, Format("%X - %s", Race$, Race.Short)
    Call Run
  EndWith
  If Not IsEmpty(UI.Result) Then CCUI.Task.AddCommand "SetEnemy " & UI.Result
EndSub

% @since PCC2 2.40.12
Sub CCUI.Task.AddSetFCode
  % ex WShipAutoTaskCommandTile::createSetFCodeCommand, WPlanetAutoTaskSelection::createSetFCodeCommand
  Local UI.Result
  UI.InputFCode "d", UI.AutoTask->Predicted.FCode
  If Not IsEmpty(UI.Result) Then CCUI.Task.AddCommand "SetFCode " & Quote(UI.Result)
EndSub

% @since PCC2 2.40.12
Sub CCUI.Task.AddSetName
  Local UI.Result
  UI.Input Translate("Enter new name:"), Translate("Set Name"), 20, "20mg"
  If Not IsEmpty(UI.Result) Then CCUI.Task.AddCommand "SetName " + Quote(UI.Result)
EndSub

% @since PCC2 2.40.12
Sub CCUI.Task.AddSetShipMission
  % ex WShipAutoTaskCommandTile::createSetMissionCommand
  Local UI.Result
  Local a := Listbox(Translate("Ship Mission"), Mission$, 340, 12, "pcc2:shiptaskscreen")
  CCUI$Ship.PrepareMissionList a, Id
  Call a->Run

  % Process result
  Local r := CCUI$Ship.CompleteMissionSelection(UI.Result, 0)
  If Not IsEmpty(r) Then
    CCUI.Task.AddCommand Format(If(r(2), "SetMission %d, %d, %d", If(r(1), "SetMission %d, %d", "SetMission %d")), r(0), r(1), r(2))
  EndIf
EndSub

% @since PCC2 2.40.12
Sub CCUI.Task.AddSetSpeed
  % ex WShipAutoTaskCommandTile::createSetSpeedCommand
  Local UI.Result
  UI.InputNumber Translate("Set Speed"), 0, 9, UI.AutoTask->Predicted.Speed$, "pcc2:shiptaskscreen", Translate("Warp Factor:")
  If Not IsEmpty(UI.Result) Then CCUI.Task.AddCommand "SetSpeed " & UI.Result
EndSub

% @since PCC2 2.40.12
Sub CCUI.Task.AddSetWaypoint
  CC$AddWaypoint Translate("Set Waypoint"), "SetWaypoint"
EndSub

% @since PCC2 2.40.12
Sub CCUI.Task.AddWaitOneTurn
  CCUI.Task.AddCommand "WaitOneTurn"
EndSub


%
%  Messages
%

% @since PCC2 2.41
Sub CCUI.Messages.ViewUtilData
  Local System.Err
  Dim m As Mailbox
  Try
    Call m->LoadUtilData
    CC$ViewMailbox m
  Else
    UI.Message Format("%s: %s", Translate("Unable to view utility data file"), System.Err)
  EndTry
EndSub

% @since PCC2 2.41
Sub CCUI.Messages.ViewFile
  % ex cc.pas:ReadFileMbox
  Local System.Err, fd, fname, UI.Result
  Dim m As Mailbox

  UI.FileWindow Translate("View Mailbox"), "*.*"
  If IsEmpty(UI.Result) Then Return
  fname := UI.Result

  fd := FreeFile()
  Try
    Open fname For Input As #fd
  Else
    UI.Message Format("%s: %s", fname, System.Err), Translate("View Mailbox")
    Return
  EndTry

  Call m->LoadFile, #fd
  Close #fd

  CC$ViewMailbox m
EndSub


%
%  Menus
%

% @q UI.Menu name:Str (Global Command)
% Show named menu and execute a command from it.
% This command creates a listbox and runs the hook given by %name to populate it.
% The hook will call {AddItem} to add commands to it; the %id in each call will be an atom.
% If the user chooses a menu item, the command given by the {Atom()|atom} will be run.
% @see Listbox(), UI.Key
% @since PCC2 2.40.8
Sub UI.Menu(name)
  Local UI.Result, m
  m := Listbox(name)
  With m Do RunHook ByName(name)
  Call m->RunMenu UI.Key
  If Not IsEmpty(UI.Result) Then
    Local UI.Key, UI.Prefix           % neutralize environment
    Eval AtomStr(UI.Result)
  EndIf
EndSub

% Starship 'b' menu
On ShipBaseMenu Do
  Local currentAction
  Local p = Planet(Orbit$)
  % FIXME: If Fighter.Bays Then AddItem 0, Translate("Build fighters")
  % FIXME: If Torp.LCount  Then AddItem 0, Translate("Build torpedoes")
  AddItem Atom("UI.GotoScreen 3, Orbit$"), Translate("Starbase screen")

  currentAction := If(p->Shipyard.Id = Id, p->Shipyard.Action, '')
  If currentAction = 'Fix' Then
    AddItem Atom("CCUI.Ship.CancelShipyard"), Translate("Cancel \"repair\" order")
  Else
    AddItem Atom("CCUI.Ship.Fix"), Translate("Fix (repair) this ship")
  EndIf
  If currentAction = 'Recycle' Then
    AddItem Atom("CCUI.Ship.CancelShipyard"), Translate("Cancel \"recycle\" order")
  Else
    AddItem Atom("CCUI.Ship.Recycle"), Translate("Recycle this ship")
  EndIf
  AddItem Atom("CC$CloneShip"), Translate("Clone this ship")
EndOn

Sub CCUI.ShipBaseMenu
  % ex WShipScreen::doShipBaseMenu
  If Planet(Orbit$).Played And Planet(Orbit$).Base.YesNo Then
    UI.Menu "ShipBaseMenu"
  Else
    MessageBox Translate("We are not at one of our starbases."), Translate("Starbase Commands")
  EndIf
EndSub


On BaseShipyardMenu Do
  Local s, canFix, canRecycle
  % FIXME: we also have this logic in C++
  ForEach Ship As s Do
    If s->Orbit$=Id Then
      canFix:=1
      If s->Owner$=Owner$ Then
        canRecycle:=1
        Break
      EndIf
    EndIf
  Next
  If canFix Then
    If canFix                    Then AddItem Atom("CCUI.Base.FixShip"),     Translate("Fix (repair) a ship")
    If canRecycle                Then AddItem Atom("CCUI.Base.RecycleShip"), Translate("Recycle a ship")
    If Shipyard.Action='Fix'     Then AddItem Atom("FixShip 0"),             Translate("Cancel \"fix\" order")
    If Shipyard.Action='Recycle' Then AddItem Atom("FixShip 0"),             Translate("Cancel \"recycle\" order")
  EndIf
EndOn


% Starbase 'r' menu
Sub CCUI.BaseShipyardMenu
  % ex client/act-planet.cc:doBaseFixRecycle
  If Orbit Then
    UI.Menu "BaseShipyardMenu"
  Else
    MessageBox Translate("There is no ship at this starbase which could be fixed or recycled."), Translate("Shipyard")
  EndIf
EndSub


On CommonTaskMenu Do
  % ex WAutoTaskObjectSelection::createCommonCommand, CProcessEditor.CommonMenu
  AddItem Atom("CCUI.Task.AddNotify"),      Translate("Notify message")
  AddItem Atom("CCUI.Task.AddSetComment"),  Translate("Set Comment")
  If UI.AutoTask->Type = 'ship' Then
    AddItem Atom("CCUI.Task.AddSetName"),   Translate("Set Name")
  EndIf
  AddItem Atom("CCUI.Task.AddWaitOneTurn"), Translate("Wait One Turn")
  AddItem Atom("CCUI.Task.AddRestart"),     Translate("Restart task from beginning")
EndOn

On ShipTaskMovementMenu Do
  % ex WShipAutoTaskCommandTile::handleCommand (part)
  AddItem Atom("CCUI.Task.AddSetSpeed"),    Translate("Set Speed")
  AddItem Atom("CCUI.Task.AddMoveTo"),      Translate("Move To")
  AddItem Atom("CCUI.Task.AddMoveTowards"), Translate("Move Towards")
  AddItem Atom("CCUI.Task.AddSetWaypoint"), Translate("Set Waypoint")
EndOn

On ShipTaskCargoMenu Do
  % ex WShipAutoTaskCommandTile::handleCommand (part)
  AddItem Atom("CCUI.Task.AddCargoUnload"), Translate("Cargo Unload")
  AddItem Atom("CCUI.Task.AddCargoUpload"), Translate("Cargo Upload")
EndOn

On ShipTaskMissionMenu Do
  % ex WShipAutoTaskCommandTile::handleCommand (part)
  AddItem Atom("CCUI.Task.AddSetEnemy"), Translate("Set Enemy")
  AddItem Atom("CCUI.Task.AddSetFCode"), Translate("Set FCode")
  AddItem Atom("CCUI.Task.AddSetShipMission"), Translate("Set Mission")
EndOn

On ShipTaskOptionsMenu Do
  % ex WShipAutoTaskCommandTile::handleCommand (part)
  AddItem Atom("CCUI.Task.TogglePredictToEnd"),  If(Pref("Task.PredictToEnd"),  Translate("Predict to cursor"),         Translate("Predict to end"))
  AddItem Atom("CCUI.Task.ToggleShowDistances"), If(Pref("Task.ShowDistances"), Translate("Don't show turn distances"), Translate("Show turn distances"))
EndOn

On PlanetTaskOrdersMenu Do
  AddItem Atom("CCUI.Task.AddSetFCode"), Translate("Set FCode")
EndOn

On PlanetTaskCargoMenu Do
  % ex WPlanetAutoTaskSelection::createCargoCommand
  AddItem Atom("CCUI.Task.AddCargoUnloadAllShips"), Translate("Unload All Ships")
EndOn

On BaseTaskOrdersMenu Do
  AddItem Atom("CCUI.Task.AddEnqueueShip"),    Translate("Enqueue Ship Build Order")
  AddItem Atom("CCUI.Task.AddSetBaseMission"), Translate("Set Mission")
  AddItem Atom("CCUI.Task.AddSetFCode"),       Translate("Set FCode")
EndOn

On ShipFleetMenu Do
  % ex WShipScreen::doShipFleetMenu, ship.pas:FleetCommands
  AddItem Atom("CCUI.Ship.GotoFleetScreen"),   Translate("Fleet screen")
  If Fleet$ Then
    AddItem Atom("CCUI.Ship.LeaveFleet"),      Translate("Leave fleet")
    If Fleet$<>Id Then
      AddItem Atom("CCUI.Ship.GotoFleetLeader"), Translate("Go to fleet leader")
    EndIf
  Else
    AddItem Atom("CCUI.Ship.StartNewFleet"),   Translate("Start new fleet")
    If Iterator(10).NextIndex(0) Then
      AddItem Atom("CCUI.Ship.JoinFleet"),     Translate("Join a fleet")
    EndIf
  EndIf

  % Only when there are matching ships
  Local cc$me = Global.Ship(Id)
  If FindShip(CCUI$Ship.IsValidFleetMember(cc$me)) Then
    AddItem Atom("CCUI.Ship.AddAllToFleet"),   Translate("Add all to fleet")
  EndIf
EndOn
