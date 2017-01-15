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

% Previous unit [PgUp etc.]
% @since PCC2 2.40.1
Sub CCUI.SelectPrevious
  Try With UI.Iterator Do CurrentIndex := PreviousIndex(CurrentIndex, "w")
EndSub

% Next unit [PgDn etc.]
% @since PCC2 2.40.1
Sub CCUI.SelectNext
  Try With UI.Iterator Do CurrentIndex := NextIndex(CurrentIndex, "w")
EndSub

% Previous marked unit [Ctrl+PgUp etc.]
% @since PCC2 2.40.1
Sub CCUI.SelectPreviousMarked
  Try With UI.Iterator Do CurrentIndex := PreviousIndex(CurrentIndex, "wm")
EndSub

% Next marked unit [Ctrl+PgDn etc.]
% @since PCC2 2.40.1
Sub CCUI.SelectNextMarked
  Try With UI.Iterator Do CurrentIndex := NextIndex(CurrentIndex, "wm")
EndSub

% Planet or starbase at X,Y
% @since PCC2 2.40.1
Sub CCUI$GotoPlanet (screen, x, y)
  % ex CC$PlanetAt
  Local pid := PlanetAt(X, Y, 1)
  If pid Then Try UI.GotoScreen screen, pid
EndSub

% Go to planet here [Shift-F2]
% @since PCC2 2.40.1
Sub CCUI.GotoPlanetHere
  CCUI$GotoPlanet 2, Loc.X, Loc.Y
EndSub

% Go to starbase here [Shift-F3]
% @since PCC2 2.40.1
Sub CCUI.GotoBaseHere
  CCUI$GotoPlanet 3, Loc.X, Loc.Y
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

% Ship mission [M]
% @since PCC2 2.40.1
Sub CCUI.Ship.SetMission
  % FIXME: totally incomplete!
  Local UI.Result
  If Played Then
    CCUI.Ship.ChooseExtendedMission Mission$, Mission.Intercept, Mission.Tow
    If Not IsEmpty(UI.Result) Then
      SetMission UI.Result(0), UI.Result(1), UI.Result(2)
    EndIf
  EndIf
EndSub

% Mission Selection for a starbase
% (This dialog is required for regular ships and for Global Actions / Ship Tasks)
% Returns UI.Result=mission or empty
% @since PCC2 2.40.1
Sub CCUI.Base.ChooseMission (m)
  % ex client/act-planet.cc:doBaseChangeMission (part)
  Local _ := Translate
  Local a := Listbox(_("Starbase Mission"), m, 300, 7, "pcc2:basescreen")

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
  % ex client/act-planet.cc:doBaseChangeMission (part)
  Local UI.Result
  If Played Then
    CCUI.Base.ChooseMission Mission$
    If Not IsEmpty(UI.Result) Then
      SetMission UI.Result
    EndIf
  EndIf
EndSub


Sub CCUI$Give(title, command, me)
  % ex client/act-ship.cc:doGive, doGiveShip, doGivePlanet
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