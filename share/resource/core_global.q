%
%  Global Actions
%

% Warn if there is an error logged.
% For many actions, errors are normal (e.g. SetFCode skipping over unplayed planets).
% For others where we have more precise conditions, this provides a safeguard in case something has been forgotten.
Sub CCGA$WarnOnError(globalState)
  Local UI.Result
  If globalState->NumErrors Then
    UI.Message Format(Translate("Warning: executing this action produced %d error%!1{s%}, last was \"%s\". This should not have happened and indicates a bug in PCC2."), globalState->NumErrors, globalState->Error), Translate("Global Actions")
  EndIf
EndSub


%%% Friendly Codes

% State is array (counter) or (counter, fcode)

Function CCGA.FCode.InitRandom()
  % ex WGlobalRandomFCodeAction::prepare()
  Local UI.Result
  UI.Message Translate("Randomize friendly codes?"), Translate("Global Actions"), Translate("Yes No")
  If UI.Result=1 Then Return Array(0)
EndFunction

Sub CCGA.FCode.ExecuteRandom(obj, state)
  % ex WGlobalRandomFCodeAction::Executer::execute, globact.pas:RandomizeFCodes
  Call obj->SetFCode RandomFCode()
  state(0) := state(0) + 1
EndSub

Function CCGA.FCode.InitSet()
  % ex WGlobalSetFCodeAction::prepare, globact.pas:SetFCodeInit
  Local UI.Result
  UI.InputFCode "spbac"
  If Not IsEmpty(UI.Result) Then Return Array(0, UI.Result)
EndFunction

Sub CCGA.FCode.ExecuteSet(obj, state)
  % ex WGlobalSetFCodeAction::Executer::execute, globact.pas:SetFCodeAction
  Call obj->SetFCode state(1)
  state(0) := state(0) + 1
EndSub

Sub CCGA.FCode.Finish(state, globalState)
  % ex WGlobalRandomFCodeAction::Executer::finish, WGlobalSetFCodeAction::Executer::finish, globact.pas:ChangeFCodeDone
  % Anticipated errors: object not played
  UI.Message Format(Translate("%d friendly code%!1{s%} changed."), state(0)), Translate("Global Actions")
EndSub

AddGlobalAction Translate("Friendly Codes | Randomize"), CCGA.FCode.InitRandom, CCGA.FCode.ExecuteRandom, CCGA.FCode.Finish
AddGlobalAction Translate("Friendly Codes | Set to..."), CCGA.FCode.InitSet,    CCGA.FCode.ExecuteSet,    CCGA.FCode.Finish


%%% Remote Control actions

% State: (verb, responseFormat, count)

Function CCGA.Remote.InitCommon(q, resp, verb)
  % ex WGlobalRemoteControlAction::prepare
  Local UI.Result
  If Not Cfg("CPEnableRemote") Then
    UI.Message Format(Translate("Remote control is not active in this game."), Translate("Global Actions"))
  Else
    UI.Message q, Translate("Global Actions"), Translate("Yes No")
    If UI.Result=1 Then
      Return Array(verb, resp, 0)
    EndIf
  EndIf
EndFunction

Function CCGA.Remote.InitAllow()
  Return CCGA.Remote.InitCommon(Translate("Allow remote control for all affected ships?"), Translate("Remote control has been allowed for %d ship%!1{s%}."), "allow")
EndFunction

Function CCGA.Remote.InitForbid()
  Return CCGA.Remote.InitCommon(Translate("Forbid remote control for all affected ships?"), Translate("Remote control has been forbidden for %d ship%!1{s%}."), "forbid")
EndFunction

Function CCGA.Remote.InitDrop()
  Return CCGA.Remote.InitCommon(Translate("Drop remote control for all affected ships?"), Translate("Remote control has been dropped for %d ship%!1{s%}."), "drop")
EndFunction

Function CCGA.Remote.InitControl()
  Return CCGA.Remote.InitCommon(Translate("Request remote control for all affected ships?"), Translate("Remote control has been requested for %d ship%!1{s%}."), "control")
EndFunction

Sub CCGA.Remote.Execute(obj, state)
  % ex WGlobalRemoteControlAction::Executer::execute, globact.pas:SetRCAction
  If obj->Ref->Kind='ship' Then
    CC$RemoteSet obj->Id, state(0)
    state(2) := state(2) + 1
  EndIf
EndSub

Sub CCGA.Remote.Finish(state, globalState)
  % Anticipated errors: object not played or in wrong state
  Local UI.Result
  UI.Message Format(state(1), state(2)), Translate("Global Actions")
EndSub

AddGlobalAction Translate("Remote Control | Allow"),   CCGA.Remote.InitAllow,   CCGA.Remote.Execute, CCGA.Remote.Finish
AddGlobalAction Translate("Remote Control | Forbid"),  CCGA.Remote.InitForbid,  CCGA.Remote.Execute, CCGA.Remote.Finish
AddGlobalAction Translate("Remote Control | Drop"),    CCGA.Remote.InitDrop,    CCGA.Remote.Execute, CCGA.Remote.Finish
AddGlobalAction Translate("Remote Control | Request"), CCGA.Remote.InitControl, CCGA.Remote.Execute, CCGA.Remote.Finish


%%% Mission actions

% State is array of (count, mission[, args])

Function CCGA.SetBaseMission.Init()
  % ex WGlobalBaseMissionAction::prepare, globact.pas:InitBaseMission
  Local UI.Result
  CCUI.Base.ChooseMission 0, "pcc2:globact"
  If Not IsEmpty(UI.Result) Then Return Array(0, UI.Result)
EndFunction

Sub CCGA.SetBaseMission.Execute(obj, state)
  % ex WGlobalBaseMissionAction::Executer::execute, globact.pas:SetBaseMission
  If obj->Ref->Kind='planet' And obj->Played And obj->Base.YesNo Then
    If obj->Mission$<>state(1) Then
      obj->Mission$ := state(1)
      state(0) := state(0) + 1
    EndIf
  EndIf
EndSub

Sub CCGA.SetBaseMission.Finish(state, globalState)
  % ex WGlobalBaseMissionAction::Executer::finish, globact.pas:DoneMission
  CCGA$WarnOnError globalState
  UI.Message Format(Translate("Mission changed on %d starbase%!1{s%}."), state(0)), Translate("Global Actions")
EndSub


Function CCGA.SetShipMission.Init()
  % ex WGlobalShipMissionAction::prepare, globact.pas:InitShipMission
  % For simplicity, provide fake current-mission data, so called functions use correct defaults
  Local Mission$=0, Mission.Intercept=0, Mission.Tow=0, Owner.Real=My.Race$

  % Build listbox
  Local UI.Result, a, msn
  a := Listbox(Translate("Ship Mission"), 0, 340, 12, "pcc2:globact")
  ForEach Global.Mission As msn Do
    If CCVP.MissionWorksGlobally(msn) Then
      Call a->AddItem msn->Number, Format("%s - %s", msn->Key, msn->Name)
    EndIf
  Next
  Call a->AddItem, -1, Translate("# - Extended Mission")
  Call a->Run

  % Process result
  Local r := CCUI$Ship.CompleteMissionSelection(UI.Result, 0)
  If Not IsEmpty(r) Then Return Array(0, r(0), r(1), r(2), r(3))
EndFunction

Sub CCGA.SetShipMission.Execute(obj, state)
  % ex WGlobalShipMissionAction::Executer::execute, globact.pas:SetShipMission
  Option LocalSubs(1)
  Local Function Describe(obj)
    Return Format("%d,%d,%d", obj->Mission$, obj->Mission.Intercept, obj->Mission.Tow)
  EndFunction
  If obj->Ref->Kind='ship' And obj->Played Then
    % Check whether ship is on intercept course, and if so, refuse if locked
    If InStr(Global.Mission(obj->Mission$, obj->Owner.Real).Flags, "i") Then
      If GetLockInfo(Format("S%d.WAYPOINT", obj->Id)) Then Return
    EndIf

    % Change the mission. SetMission will fail for fleet members.
    Local t = Describe(obj)
    Call obj->SetMission state(1), state(2), state(3)
    If t<>Describe(obj) Then state(0) := state(0) + 1
  EndIf
EndSub

Sub CCGA.SetShipMission.Finish(state, globalState)
  % WGlobalShipMissionAction::Executer::finish
  % Anticipated errors: fleet
  Local UI.Result
  UI.Message Format(Translate("Mission changed on %d ship%!1{s%}."), state(0)), Translate("Global Actions")
EndSub


Function CCGA.SetEnemy.Init()
  % ex WGlobalShipEnemyAction::prepare, globact.pas:InitShipPE
  Local UI.Result
  With Listbox(Translate("Change Primary Enemy"), 0, 0, 0, "pcc2:globact") Do
    AddItem 0, Translate("0 - none")
    ForEach Player Do AddItem Race$, Format("%X - %s", Race$, Race.Short)
    Call Run
  EndWith
  If Not IsEmpty(UI.Result) Then Return Array(0, UI.Result)
EndFunction

Sub CCGA.SetEnemy.Execute(obj, state)
  % ex WGlobalShipEnemyAction::Executer::execute, globact.pas:SetShipPE
  If obj->Ref->Kind='ship' And obj->Played Then
    If obj->Enemy$<>state(1) Then
      obj->Enemy$ := state(1)
      state(0) := state(0) + 1
    EndIf
  EndIf
EndSub

Sub CCGA.SetEnemy.Finish(state, globalState)
  % ex WGlobalShipEnemyAction::Executer::finish, globact.pas:DoneShipPE
  Local UI.Result
  CCGA$WarnOnError globalState
  UI.Message Format(Translate("Primary Enemy changed on %d ship%!1{s%}."), state(0)), Translate("Global Actions")
EndSub


AddGlobalAction Translate("Missions | Set starbase mission..."),   CCGA.SetBaseMission.Init, CCGA.SetBaseMission.Execute, CCGA.SetBaseMission.Finish
AddGlobalAction Translate("Missions | Set ship mission..."),       CCGA.SetShipMission.Init, CCGA.SetShipMission.Execute, CCGA.SetShipMission.Finish
AddGlobalAction Translate("Missions | Set ship primary enemy..."), CCGA.SetEnemy.Init,       CCGA.SetEnemy.Execute,       CCGA.SetEnemy.Finish


%%% Build actions

% SetGoals state: array of (build-settings, count)
% Autobuild state: array of (count)

Function CCGA.SetGoals.Init()
  % ex WGlobalSetBuildGoalsAction::prepare, globact.pas:SetGoalsInit
  % @change PCC1 will cancel the action if the user does not specify any change
  Local UI.Result
  CC$EditAutobuildSettings
  If Not IsEmpty(UI.Result) Then Return Array(UI.Result, 0)
EndFunction

Sub CCGA.SetGoals.Execute(obj, state)
  % ex WGlobalSetBuildGoalsAction::Executer::execute, globact.pas:SetGoals
  Option LocalSubs(1)
  Local Function Describe(obj)
    Return Format("%d,%d,%d,%d,%d,%d,%d,%d", obj->Defense.Want, obj->Defense.Speed, obj->Defense.Base.Want, obj->Defense.Base.Speed, obj->Factories.Want, obj->Factories.Speed, obj->Mines.Want, obj->Mines.Speed)
  EndFunction
  If obj->Ref->Kind='planet' Then
    Local t = Describe(obj)
    Call obj->CC$ApplyBuildGoals state(0)
    If t<>Describe(obj) Then state(1) := state(1) + 1
  EndIf
EndSub

Sub CCGA.SetGoals.Finish(state, globalState)
  % ex WGlobalSetBuildGoalsAction::Executer::finish, globact.pas:SetGoalsDone
  CCGA$WarnOnError globalState
  UI.Message Format(Translate("Auto-build goals have been changed on %d planet%!1{s%}."), state(1)), Translate("Global Actions")
EndSub

Function CCGA.Autobuild.Init()
  % ex WGlobalAutobuildAction::prepare()
  Local UI.Result
  UI.Message Translate("Perform auto-build on all planets?"), Translate("Global Actions")
  If UI.Result=1 Then Return Array(0)
EndFunction

Sub CCGA.Autobuild.Execute(obj, state)
  % WGlobalAutobuildAction::Executer::execute, globact.pas:Autobuild
  Option LocalSubs(1)
  Local Function Describe(obj)
    Return Format("%d,%d,%d,%d", obj->Defense, If(obj->Defense.Base, obj->Defense.Base, "0"), obj->Factories, obj->Mines)
  EndFunction
  If obj->Ref->Kind='planet' And obj->Played Then
    With Lock(Format("P%d.BUILD", obj->Id)) Do
      Local t = Describe(obj)
      Call obj->Autobuild
      If t<>Describe(obj) Then state(0) := state(0) + 1
    EndWith
  EndIf
EndSub

Sub CCGA.Autobuild.Finish(state, globalState)
  % WGlobalAutobuildAction::Executer::finish, globact.pas:AutobuildDone
  % Anticipated errors: locks
  Local UI.Result
  UI.Message Format(Translate("Structures have been built on %d planet%!1{s%}."), state(0)), Translate("Global Actions")
EndSub

AddGlobalAction Translate("Building | Autobuild"),          CCGA.Autobuild.Init, CCGA.Autobuild.Execute, CCGA.Autobuild.Finish
AddGlobalAction Translate("Building | Set Build Goals..."), CCGA.SetGoals.Init,  CCGA.SetGoals.Execute,  CCGA.SetGoals.Finish


%%% Taxation actions

% State is array (counter, colonists, natives)

Function CCGA.AutoTax.Init()
  % ex WGlobalAutotaxAction::prepare, globact.pas:OptimizeInitCN
  Local UI.Result
  UI.Message Translate("Do you want to optimize all tax rates?"), Translate("Global Actions"), Translate("Yes No")
  If UI.Result=1 Then Return Array(0, True, True)
EndFunction

Function CCGA.AutoTax.InitColonists()
  % ex globact.pas:OptimizeInitC
  Local UI.Result
  UI.Message Translate("Do you want to optimize all colonist tax rates?"), Translate("Global Actions"), Translate("Yes No")
  If UI.Result=1 Then Return Array(0, True, False)
EndFunction

Function CCGA.AutoTax.InitNatives()
  % ex globact.pas:OptimizeInitN
  Local UI.Result
  UI.Message Translate("Do you want to optimize all native tax rates?"), Translate("Global Actions"), Translate("Yes No")
  If UI.Result=1 Then Return Array(0, False, True)
EndFunction

Sub CCGA.AutoTax.Execute(obj, state)
  % ex WGlobalAutotaxAction::Executer::execute, globact.pas:OptimizeTaxes
  Local did, old
  If obj->Played And obj->Ref->Kind='planet' And Not IsEmpty(obj->Colonists.Tax) Then
    With Lock(Format("P%d.TAX", obj->Id)) Do
      % Colonists
      If state(1) Then
        old := obj->Colonists.Tax
        Call obj->AutoTaxColonists
        If old<>obj->Colonists.Tax Then did:=True
      EndIf

      % Natives
      If state(2) And Not IsEmpty(obj->Natives.Tax)
        old := obj->Natives.Tax
        Call obj->AutoTaxNatives
        If old<>obj->Natives.Tax Then did:=True
      EndIf
    EndWith
  EndIf
  If did Then state(0) := state(0) + 1
EndSub

Sub CCGA.AutoTax.Finish(state, globalState)
  % WGlobalAutotaxAction::Executer::finish, globact.pas:OptimizeDone
  % Anticipated errors: locks
  UI.Message Format(Translate("Taxes have been changed on %d planet%!1{s%}."), state(0)), Translate("Global Actions")
EndSub

AddGlobalAction Translate("Taxes | Optimize"),                  CCGA.AutoTax.Init,          CCGA.AutoTax.Execute, CCGA.AutoTax.Finish
AddGlobalAction Translate("Taxes | Optimize (colonists only)"), CCGA.AutoTax.InitColonists, CCGA.AutoTax.Execute, CCGA.AutoTax.Finish
AddGlobalAction Translate("Taxes | Optimize (natives only)"),   CCGA.AutoTax.InitNatives,   CCGA.AutoTax.Execute, CCGA.AutoTax.Finish


%%% Export

% State is a ReferenceList() in which we collect objects

Function CCGA.Export.Init()
  % ex globact.pas:InitExport
  Return ReferenceList()
EndFunction

Sub CCGA.Export.Execute(obj, state)
  % ex WGlobalExportAction::Executer::execute, globact.pas:CountExport
  Call state->Add obj->Ref
EndSub

Sub CCGA.Export.Finish(state, globalState)
  % ex WGlobalExportAction::Executer::finish, globact.pas:DoneExport
  CCGA$WarnOnError globalState
  If Dim(state->Objects)=0 Then
    UI.Message Translate("No units selected for export."), Translate("Export")
  Else
    Try
      CC$Export state
      UI.Message Translate("Export succeeded."), Translate("Export")
    Else
      UI.Message Translate("Your selection includes ships and planets. PCC cannot export ships and planets at the same time. Please use the \"1\" and \"2\" buttons to select only one type."), Translate("Export")
    EndTry
  EndIf
EndSub

AddGlobalAction Translate("Export..."), CCGA.Export.Init, CCGA.Export.Execute, CCGA.Export.Finish


%%% Script

% State is the compiled subroutine

Function CCGA.ScriptCommand.Init
  % ex WGlobalScriptAction::prepare, globact.pas:InitCommand
  Local UI.Result, fcn, cmd
  Do
    % Ask for command
    UI.InputCommand Translate("Command:"), Translate("Global Actions"), "", cmd, "pcc2:globact"
    If IsEmpty(UI.Result) Then Break
    cmd := UI.Result

    % Try to compile
    Try
      Eval 'Option LocalSubs(1)', 'Local Sub CC$Sub', cmd, 'EndSub', 'fcn := CC$Sub'
      Return fcn
    Else
      UI.Message Format(Translate("Error compiling this statement: %s"), System.Err), Translate("Global Actions")
    EndTry
  Loop
EndFunction

Sub CCGA.ScriptCommand.Execute(obj, cc$state)
  % ex WGlobalScriptAction::Executer::execute, globact.pas:ExecCommand
  % @change We execute everything in one process; PCC2 spawns a new one for each unit
  With obj Do Call cc$state
EndSub

Sub CCGA.ScriptCommand.Finish(state, globalState)
  % ex WGlobalScriptAction::Executer::finish, globact.pas:DoneCommand
  If globalState->Error And Not globalState->NumSuccess Then
    UI.Message Format(Translate("Error executing this statement: %s"), globalState->Error), Translate("Global Actions")
  Else
    UI.Message Format(Translate("Command executed successfully on %d unit%!1{s%}."), globalState->NumSuccess), Translate("Global Actions")
  EndIf
EndSub

AddGlobalAction Translate("Script command..."), CCGA.ScriptCommand.Init, CCGA.ScriptCommand.Execute, CCGA.ScriptCommand.Finish
