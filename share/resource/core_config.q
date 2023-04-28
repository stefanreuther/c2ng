%
%  Configuration
%
%  This module provides content for the Configuration Editor dialog.
%  The Configuration Editor dialog runs the GamePreferences and UserPreferences hooks.
%  We provide appropriate settings for both branches.
%
%  Subroutine names start with "CCfg".
%  Some edit subroutines are implemented natively under a temporary name outside this namespace.
%

%%%%%%%%%%%%%%%%%%%% Game Options %%%%%%%%%%%%%%%%%%%%

%
%  Password Change
%

Function CCfg.Password.Value
  Return If(System.HasPassword, Translate("(set)"), Translate("(not set)"))
EndFunction

On GamePreferences Do If Not Cfg("DisablePasswords") Then Add Translate("Set result file password..."), CC$ChangePassword, CCfg.Password.Value


%
%  Race Name Editor
%

Sub CCfg.RaceName.Edit
  % ex editRaceNames, phost.pas:RenameMyRace
  % Original state
  Option LocalSubs(1)
  Local Function Pick(a, b)
    Return If(IsEmpty(b), a, b)
  EndFunction

  Local Sub Submit(cmd, val)
    If IsEmpty(Z(val)) Then
      DeleteCommand cmd
    Else
      AddCommand cmd & ' ' & val
    EndIf
  EndSub

  Local Sub OnOK
    Submit "race long",  Trim(fi->Value)
    Submit "race short", Trim(si->Value)
    Submit "race adj",   Trim(ai->Value)
    UI.EndDialog 1
  EndSub

  Local Sub OnCancel
    UI.EndDialog 0
  EndSub

  Local fn := Pick(My.Race.Full, GetCommand("race long"))
  Local sn := Pick(My.Race,      GetCommand("race short"))
  Local an := Pick(My.Race.Adj,  GetCommand("race adj"))
  Local a := UI.Dialog(Translate("Change Race Name"))

  Local aa := a->NewGridBox(2)
  aa->NewLabel(Translate("Full name"))
  Local fi := aa->NewFrame("lowered", 1)->NewInput(30, "15mg", fn, "alt-f")
  aa->NewLabel(Translate("Short name"))
  Local si := aa->NewFrame("lowered", 1)->NewInput(20, "15mg", sn, "alt-s")
  aa->NewLabel(Translate("Adjective"))
  Local ai := aa->NewFrame("lowered", 1)->NewInput(12, "15mg", an, "alt-a")

  a->NewLabel(Translate("Changes will get effective next turn."), "-")

  Local ab := a->NewHBox()
  ab->NewButton(Translate("OK"),     "ret", "OnOK")
  ab->NewButton(Translate("Cancel"), "esc", "OnCancel")
  ab->NewSpacer()

  a->NewKeyboardFocus("vt", fi, si, ai)

  Call a->Run
EndSub

Function CCfg.RaceName.Value
  % ex WConfigGameEditor::toString (part)
  If GetCommand("race long") Or GetCommand("race short") Or GetCommand("race adj") Then
    Return Translate("(changed)")
  Else
    Return Translate("(not changed)")
  EndIf
EndFunction

On GamePreferences Do If Cfg("CPEnableRaceName") Then Add Translate("Change race name..."), CCfg.RaceName.Edit, CCfg.RaceName.Value


%
%  Remote Control
%

Sub CCfg.Remote.Edit
  % ex WConfigGameEditor::editRemote
  Local UI.Result
  UI.Message RXml(Translate("Do you want newly-built ships to be available for remote control by allies by default?<br/><small>You can give this permission for each ship individually using <kbd>R</kbd> on the ship screen. The recommended setting is <b>No</b>.</small>")), Translate("Set remote control defaults..."), Translate("Yes No Cancel")
  Select Case UI.Result
    Case 1
      AddCommand 'remote allow default'
    Case 2
      AddCommand 'remote forbid default'
  EndSelect
EndSub

Function CCfg.Remote.Value
  Local v := GetCommand("remote control default")
  If IsEmpty(v) Then
    Return Translate("(not changed)")
  Else If Left(v, 1) = 'f' Then
    Return Translate("(forbidden by default)")
  Else
    Return Translate("(allowed by default)")
  EndIf
EndFunction

On GamePreferences Do If Cfg("CPEnableRemote") Then Add Translate("Set remote control defaults..."), CCfg.Remote.Edit, CCfg.Remote.Value


%
%  Filter
%

Sub CCfg.Filter.Edit
  % ex WConfigGameEditor::editFilter
  Local UI.Result
  UI.Message RXml(Translate("Do you want the host to filter out most bulk messages?<br/><small>When the filter is enabled, messages like Sensor Sweep are removed from the result file, reducing its size. PCC2 can still display the information because it is also received in the <tt>util.dat</tt> file.</small><br/><small>The recommended setting is <b>No</b> to keep the messages. You can then use PCC2's built-in filter to determine with finer granularity what you want to see.</small>")), Translate("Set message detail..."), Translate("Yes No Cancel")
  Select Case UI.Result
    Case 1
      AddCommand 'filter yes'
    Case 2
      AddCommand 'filter no'
  EndSelect
EndSub

Function CCfg.Filter.Value
  Local v := GetCommand("filter")
  If IsEmpty(v) Then
    Return Translate("(not changed)")
  Else If Left(v, 1) = 'y' Then
    Return Translate("(filter enabled)")
  Else
    Return Translate("(filter disabled)")
  EndIf
EndFunction

On GamePreferences Do If System.Host='PHost' Then Add Translate("Set message detail..."), CCfg.Filter.Edit, CCfg.Filter.Value


%
%  Language Change
%

Sub CCfg.Language.Edit
  % ex WConfigGameEditor::editLanguage
  Local UI.Result, i
  Local languages = Array("English", "German", "French", "Spanish", "Italian", "Dutch", "Russian", "Estonian", "Polish", "NewEnglish")

  % Determine current
  Local v := GetCommand("language")
  Local current
  For i:=0 To Dim(languages)-1 Do If v=languages(i) Then current:=i

  % Do it
  With Listbox(Translate("Set message language..."), current, Z(0), Z(0), "pcc2:settings") Do
    For i:=0 To Dim(languages)-1 Do AddItem i, languages(i)
    AddItem -1, Translate("(no change)")
    Run
  EndWith

  % Process command
  If Not IsEmpty(UI.Result) Then
    If UI.Result<0 Then
      DeleteCommand "language"
    Else
      AddCommand "language " & languages(UI.Result)
    EndIf
  EndIf
EndSub

Function CCfg.Language.Value
  Local v := GetCommand("language")
  If IsEmpty(v) Then
    Return Translate("(not changed)")
  Else
    Return v
  EndIf
EndFunction

On GamePreferences Do If Cfg("CPEnableLanguage") Then Add Translate("Set message language..."), CCfg.Language.Edit, CCfg.Language.Value


%
%  Request files from host
%  - LinkExtra "command to toggle"
%

Sub CCfg.Request.Edit
  % ex WConfigGameEditor::toggleCommand
  If IsEmpty(GetCommand(Extra)) Then
    AddCommand Extra
  Else
    DeleteCommand Extra
  EndIf
EndSub

Function CCfg.Request.Value
  If IsEmpty(GetCommand(Extra)) Then
    Return Translate("(not requested)")
  Else
    Return Translate("(requested)")
  EndIf
EndFunction

On GamePreferences Do
  If Cfg("CPEnableSend") Then
    Add Translate("Request file from host | Configuration (pconfig.src)"), CCfg.Request.Edit, CCfg.Request.Value
    LinkExtra "send config"

    Add Translate("Request file from host | Friendly code list (xtrafcode.txt)"), CCfg.Request.Edit, CCfg.Request.Value
    LinkExtra "send fcodes"

    Add Translate("Request file from host | Race names (race.nm)"), CCfg.Request.Edit, CCfg.Request.Value
    LinkExtra "send racenames"
  EndIf
EndOn


%%%%%%%%%%%%%%%%%%%% Preferences %%%%%%%%%%%%%%%%%%%%%

%
%  Boolean option
%

Sub CCfg.Boolean.Edit
  % ex WConfigBooleanEditor::edit [FIXME: we have that native!]
  AddPref Option & " = " & +Not Pref(Option)
EndSub

Function CCfg.Boolean.Value
  % ex WConfigBooleanEditor::toString [FIXME: we have that native!]
  Return If(Pref(Option), Translate("Yes"), Translate("No"))
EndFunction

Function CCfg.UnpackFormat.Value
  Return If(Pref(Option), "Windows (3.5)", "DOS (3.0)")
EndFunction

%
%  String option
%

Sub CCfg.String.Edit
  Local UI.Result
  UI.Input Translate("New Value:"), Translate("Edit Option"), 10000, 20, Pref(Option)
  If Not IsEmpty(UI.Result) Then
    AddPref Option & " = " & UI.Result
  EndIf
EndSub

Function CCfg.String.Value
  Return Pref(Option)
EndFunction

%
%  Dialog (placeholder)
%

Function CCfg.Dialog.Value
  % ex WConfigStarchartEditor::toString, WConfigLabelEditor::toString
  Return Translate("(dialog)")
EndFunction

%
%  Backup
%  - LinkPref "option"
%  - LinkExtra "default template"
%

Sub CCfg.Backup.Edit
  % ex WConfigBackupEditor::edit
  Local UI.Result
  CC$EditBackup Pref(Option), Extra
  If Not IsEmpty(UI.Result) Then AddPref Option & " = " & UI.Result
EndSub

Function CCfg.Backup.Value
  % ex WConfigBackupEditor::toString
  Local v = Pref(Option)
  If IsEmpty(Z(v)) Then
    Return Translate("disabled (no backup)")
  Else If v = Extra Then
    Return Translate("enabled (standard file name)")
  Else
    Return Format(Translate("custom: %s"), v)
  EndIf
EndFunction

%
%  Define all the User Preferences
%

On UserPreferences Do
  Local i

  Add Translate("Maketurn | Backup..."), CCfg.Backup.Edit, CCfg.Backup.Value
  LinkPref "Backup.Turn"
  LinkExtra "%d/backups/turn%p.%t"

  Add Translate("Maketurn | Upload target..."), CCfg.String.Edit, CCfg.String.Value
  LinkPref "Maketurn.Target"

  % TODO: Missing PCC1 Option: Maketurn | TRN file format
  % TODO: Missing PCC1 Option: Maketurn | Use all message files
  % TODO: Missing PCC1 Option: Maketurn | Non-Player messages
  % TODO: Missing PCC1 Option: Maketurn | PHost identification
  % TODO: Missing PCC1 Option: Maketurn | Wait after each race

  Add Translate("Unpack | Data file format"), CCfg.Boolean.Edit, CCfg.UnpackFormat.Value
  LinkPref "Unpack.Format"

  % TODO: Missing PCC1 Option: Unpack | Race name changes
  % TODO: Missing PCC1 Option: Unpack | Convert text files in UTILx.DAT

  Add Translate("Unpack | Create TARGETx.EXT files"), CCfg.Boolean.Edit, CCfg.Boolean.Value
  LinkPref "Unpack.TargetExt"

  % TODO: Missing PCC1 Option: Unpack | Source directory...

  Add Translate("Unpack | Backup..."), CCfg.Backup.Edit, CCfg.Backup.Value
  LinkPref "Backup.Result"
  LinkExtra "%d/backups/result%p.%t"

  Add Translate("Unpack | Error correction"), CCfg.Boolean.Edit, CCfg.Backup.Value
  LinkPref "Unpack.FixErrors"

  % TODO: Missing PCC1 Option: Unpack | Wait after each race
  % TODO: Missing PCC1 Option: Program Interoperation | Winplan directory...
  % TODO: Missing PCC1 Option: Program Interoperation | Decompile PHost commands
  % TODO: Missing PCC1 Option: Program Interoperation | Save PHost commands in messages
  % TODO: Missing PCC1 Option: Program Interoperation | Update HCONFIG.HST

  % TODO: Missing PCC2 Option: Add Translate("Program Interoperation | Rewrap incoming messages"), CCfg.Boolean.Edit, CCfg.Boolean.Value
  % TODO: Missing PCC2 Option: LinkPref "Messages.RewrapInbox"

  % TODO: Missing PCC1 Option: Program Interoperation | Host 3.22.031 Torp Safety
  % TODO: Missing PCC1 Option: Preferences | User Interface | Confirm exit from race screen
  % TODO: Missing PCC1 Option: Preferences | User Interface | Expensive graphical effects
  % TODO: Missing PCC1 Option: Preferences | User Interface | Display options...
  % TODO: Missing PCC1 Option: Preferences | User Interface | Tip of the day

  Add Translate("Preferences | User Interface | Thousands-separator"), CCfg.Boolean.Edit, CCfg.Boolean.Value
  LinkPref "Display.ThousandsSep"

  Add Translate("Preferences | User Interface | Use clans everywhere"), CCfg.Boolean.Edit, CCfg.Boolean.Value
  LinkPref "Display.Clans"

  % TODO: Missing PCC1 Option: Preferences | User Interface | Cache ship lists
  % TODO: Missing PCC2 Option: Preferences | User Interface | Waypoint tied to scanner --> Scanner.TieWaypoint

  Add Translate("Preferences | User Interface | Ship ability icons"), CCfg.Boolean.Edit, CCfg.Boolean.Value
  LinkPref "Display.HullfuncImages"

  % TODO: Missing PCC1 Option: Preferences | User Interface | Scanner/Waypoint independant
  % TODO: Missing PCC1 Option: Preferences | User Interface | Display Auto Waypoints in map
  % TODO: Missing PCC1 Option: Preferences | VCR...

  Add Translate("Preferences | Starchart | General settings..."), CC$StarchartConfig, CCfg.Dialog.Value
  % Link all the user options but NOT the geometry options!
  % Geometry needs to be treated special because we don't want it to appear in the user file.
  LinkPref "Chart.Small.Show",   "Chart.Small.Fill"
  LinkPref "Chart.Normal.Show",  "Chart.Normal.Fill"
  LinkPref "Chart.Scanner.Show", "Chart.Scanner.Fill"
  For i:=0 To 9 Do LinkPref "Chart.Marker"&i
  LinkPref "Lock.Left", "Lock.Right"

  Add Translate("Preferences | Starchart | Object labels..."), CC$EditLabelConfig, CCfg.Dialog.Value
  LinkPref "Label.Ship", "Label.Planet"

  % TODO: Missing PCC2 Option: Preferences | Collapse old messages --> Messages.CollapseOld

  Add Translate("Preferences | Automatically lock at warp"), CCfg.Boolean.Edit, CCfg.Boolean.Value
  LinkPref "Chart.Scanner.WarpWells"

  Add Translate("Preferences | Taxation predicts relative growth"), CCfg.Boolean.Edit, CCfg.Boolean.Value
  LinkPref "Tax.PredictRelative"

  % TODO: Missing PCC2 Option: Preferences | Instant battle result --> VCR.InstantResult
  % TODO: Missing PCC1 Option: Preferences | Guess ship positions
  % TODO: Missing PCC1 Option: Preferences | Starcharts file (CHARTx.CC)
  % TODO: Missing PCC1 Option: Preferences | Auto load...
  % TODO: Missing PCC1 Option: Preferences | If UTILx.DAT available...
  % TODO: Missing PCC1 Option: Preferences | Clear console on exit
  % TODO: Missing PCC1 Option: Preferences | Reset info dialogs!
  % TODO: Missing PCC1 Option: Universe | Delete far planets
  % TODO: Missing PCC1 Option: Universe | Starcharts geometry...
  Add Translate("Backups | Result file backups..."), CCfg.Backup.Edit, CCfg.Backup.Value
  LinkPref "Backup.Result"
  LinkExtra "%d/backups/result%p.%t"

  Add Translate("Backups | UTIL.DAT file backups..."), CCfg.Backup.Edit, CCfg.Backup.Value
  LinkPref "Backup.Util"
  LinkExtra "%d/backups/util%p.%t"

  Add Translate("Backups | Turn file backups..."), CCfg.Backup.Edit, CCfg.Backup.Value
  LinkPref "Backup.Turn"
  LinkExtra "%d/backups/turn%p.%t"

  Add Translate("Backups | Starchart database backups..."), CCfg.Backup.Edit, CCfg.Backup.Value
  LinkPref "Backup.Chart"
  LinkExtra "%d/backups/chart%p.%t"

  Add Translate("Backups | Scripts/Auto Task backups..."), CCfg.Backup.Edit, CCfg.Backup.Value
  LinkPref "Backup.Script"
  LinkExtra "%d/backups/script%p.%t"
EndOn


%%%%%%%%%%%%%%%%%% Main Entry Point %%%%%%%%%%%%%%%%%%


% @q UI.Preferences (Global Command)
% Preference/Configuration Editor user interface.
% Shows the list of available preferences and lets the user review and change them.
% To do so, this command first runs the hooks:
% - {GamePreferences (Hook)|GamePreferences}, to obtain a list of game preferences (e.g. host language, race name) shown on the "Game Settings" page
% - {UserPreferences (Hook)|UserPreferences}, to obtain a list of user preferences (e.g. backup path names) shown on the "Preferences" page
% - {Preferences (Hook)|Preferences}, to obtain a list of additional preferences possibly shown on additional pages
%
% @since PCC2 2.41
Sub UI.Preferences
  Dim p As ConfigurationEditorContext
  With p->Subtree(Translate("Game Settings")) Do RunHook GamePreferences
  With p->Subtree(Translate("Options")) Do RunHook UserPreferences
  With p Do RunHook Preferences
  Call p->UpdateAll
  CC$Settings p
EndSub
