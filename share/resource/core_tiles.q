%
%  PCC2 Tiles
%
%  For script-backed tiles, one function for each tile.
%  This function is called whenever the underlying data of the tile changes.
%  The function is called in a context that includes the current object and the tile widget.
%  It should call methods or assign properties on the tile widgets.
%
%  The connection between tile widgets and functions is done in C++ code and currently not configurable.
%

Option LocalSubs(1)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Utilities %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Format ship build order. Returns rich text. inact=text for inactive order
Function Tile$BaseBuildOrder(inact)
  Local c = Z(FindShipCloningAt(Id))
  If Build.YesNo Then
    If IsEmpty(c) Then
      Return RStyle("yellow", Format(Translate("Building a %s"), Build))
    Else
      Return RStyle("red", Format(Translate("Cloning, and building a %s"), Build))
    EndIf
  Else
    If IsEmpty(c) Then
      Return inact
    Else
      Return RStyle("yellow", Format(Translate("Cloning %s"), ShipName(c)))
    EndIf
  EndIf
EndFunction

% Format shipyard order. Returns rich text. inact=text for inactive order
Function Tile$BaseShipyardOrder(inact)
  If Shipyard.Action = 'Fix' Then
    Return RStyle("yellow", Format(Translate("Repairing %s"), ShipName(Shipyard.Id)))
  Else If Shipyard.Action = 'Recycle' Then
    Return RStyle("yellow", Format(Translate("Recycling %s"), ShipName(Shipyard.Id)))
  Else
    Return inact
  EndIf
EndFunction


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Headers %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Still unused
% @since PCC2 2.40.1
Sub Tile.PlanetHeader
  Local lv = Level
  SetHeading Name

  If Not IsEmpty(Temp)
    If Not IsEmpty(lv)
      SetSubtitle Format(Translate("(Id #%d, %s - %d" & Chr(176) & "F, %s)"), Id, Temp, Temp$, CCVP.GetExperienceLevelName(lv))
    Else
      SetSubtitle Format(Translate("(Id #%d, %s - %d" & Chr(176) & "F)"), Id, Temp, Temp$)
    EndIf
    SetImage Format("planet.%d.%d", Temp$, Id)
  Else
    SetSubtitle Format(Translate("(Id #%d)"), Id)
    SetImage "planet"
  EndIf
  SetButton "image", If (Marked, "yellow", "none")
EndSub

% Still unused
% @since PCC2 2.40.1
Sub Tile.BaseHeader
  Local lv = Level
  SetHeading Name

  If Not IsEmpty(lv)
    SetSubtitle Format(Translate("(Id #%d, %s)"), Id, CCVP.GetExperienceLevelName(lv))
  Else
    SetSubtitle Format(Translate("(Id #%d)"), Id)
  EndIf
  SetImage Format("base.%d", Max(Tech.Hull, Tech.Engine, Tech.Beam, Tech.Torpedo))
  SetButton "image", If (Marked, "yellow", "none")
EndSub


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Overview Tiles %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% These tiles are used for the "select an object" dialogs.

% Ship overview:
% - called in ship context
% - call SetContent with a 30x12 rich-text string
% @since PCC2 2.40.1
Sub Tile.ShipOverview
  % ex WShipOverviewTile::drawData, CStarshipWindow.DrawData
  Local s, t

  % Line 1: Hull
  Local ownerAdj = ''
  If Owner$ <> Owner.Real Then ownerAdj := Global.Player(Owner.Real).Race.Adj & ' '

  Local levelName = ''
  If Not IsEmpty(Level) Then levelName := CCVP.GetExperienceLevelName(Level) & ' '

  t := RAdd(ownerAdj, levelName, RStyle("bold", Hull), "\n")

  % Line 2: Location
  t := RAdd(t, Translate("Location: "), CCVP.GetLocationName(Loc.X, Loc.Y, "vwo"), "\n")

  % Line 3: Waypoint
  t := RAdd(t, Translate("Waypoint: "), CCVP.GetLocationName(Waypoint.X, Waypoint.Y, "wv"), "\n")

  % Line 4: Movement, speed
  If Waypoint.DX=0 And Waypoint.DY=0 Then
    s := Translate("at waypoint")
  Else
    s := Format(Translate("%.2f ly"), Waypoint.Dist) & ", " & If(Speed$, If(Speed="Hyperdrive", Format(Translate("Hyperdrive, warp %d"), Speed$), Speed), Translate("not moving"))
  EndIf
  t := RAdd(t, "  (", s, ")\n\n")

  % Line 5: Mission
  t := RAdd(t, Format(Translate("Mission: %s"), CCVP.ShipMissionLabel()), "\n")

  % Line 6: PE
  t := RAdd(t, Format(Translate("Primary Enemy: %s"), If(Enemy, Enemy, Translate("none"))), "\n\n")

  % Line 7: Cargo
  t := RAdd(t, RAlign(Translate("Cargo:"),  50), RAlign(Format("%dN", CCVP.NumberToString(Cargo.N)),  80, 2), RAlign(Format("%dT", CCVP.NumberToString(Cargo.T)),  80, 2), RAlign(Format("%dD", CCVP.NumberToString(Cargo.D)),  80, 2), RAlign(Format("%dM", CCVP.NumberToString(Cargo.M)),  80, 2), "\n")

  % Line 8: More cargo
  t := RAdd(t, Format(Translate("%d colonist%!1{s%}, %d mc, %d suppl%1{y%|ies%}"), CCVP.ClansToString(Cargo.Colonists), CCVP.NumberToString(Cargo.Money), CCVP.NumberToString(Cargo.Supplies)), "\n\n")

  % Line 9: FCode
  t := RAdd(t, Format(Translate("FCode: %s"), FCode), "\n")

  % Line 11: Misc
  Select Case Fleet.Status
    Case 'leader'
      t := RAdd(t, Format(Translate("Leader of fleet #%d"), Fleet$))
    Case 'member'
      t := RAdd(t, Format(Translate("Member of fleet #%d"), Fleet$))
  EndSelect
  SetContent t
EndSub


% Planet overview:
% - called in planet context
% - call SetContent with a 30x10 rich-text string
% @since PCC2 2.40.1
Sub Tile.PlanetOverview
  % ex WPlanetOverviewTile::drawData, CPlanetWindow.DrawData
  Local t

  % Line 1: Climage/Location
  t = RAlign(Format(Translate("Climate: %s"), Temp), 130)
  t = RAdd(t, Format(Translate("Location: (%d,%d)"), Loc.X, Loc.Y), "\n\n")
  % FIXME: sector number
  %     if (int sector = pp.getPos().getSectorNumber())
  %         outTextF(ctx, x+130, y, extent.w - 130,
  %                  format(_("Location: (%d,%d) - Sector %d"),
  %                         pp.getPos().x, pp.getPos().y, sector));
  %
  %     // FIXME: I don't really like this because it uses hard
  %     // measurements (the same as in PCC 1.x)
  %     /*  |-35-|---70---|5|---95---|30|----175--------|
  %         Col:  N,NNN,NNN Race      xx% -> happy (100)
  %         Nat:  N,NNN,NNN Race
  %         |0   |35      |105|110   |205|235  */

  % Line 2: Colonists
  t = RAdd(t, RAlign(Translate("Col:"), 35))
  If Owner$ Then
    t = RAdd(t, RAlign(CCVP.ClansToString(Colonists), 70, 2), " ", RAlign(Player(Owner$).Race.Adj, 95))
    t = RAdd(t, RAlign(Format("%d%%", Colonists.Tax), 30, 2), Format(" "&Chr(8594)&" %s (%d)", Colonists.Happy, Colonists.Happy$), "\n")
  Else
    t = RAdd(t, Translate("none"), "\n")
  EndIf

  % Line 3: Native
  t = RAdd(t, RAlign(Translate("Nat:"), 35))
  If Natives.Race$ Then
    % FIXME: clansToString
    t = RAdd(t, RAlign(CCVP.ClansToString(Natives), 70, 2), " ", RAlign(Natives.Race, 95))
    t = RAdd(t, RAlign(Format("%d%%", Natives.Tax), 30, 2), Format(" "&Chr(8594)&" %s (%d)", Natives.Happy, Natives.Happy$), "\n\n")
  Else
    t = RAdd(t, Translate("none"), "\n\n")
  EndIf

  % Line 4+5: Industry+Funds
  t = RAdd(t, Format(Translate("Mines: %d; Factories: %d; Defense Posts: %d"), Mines, Factories, Defense), "\n")
  t = RAdd(t, Format(Translate("%d mc; %d kt supplies"), CCVP.NumberToString(Money), CCVP.NumberToString(Supplies)), "\n")

  % Line 6+7: Mined, Ground
  t = RAdd(t, RAlign(Translate("Mined:"),  50), RAlign(Format("%dN", CCVP.NumberToString(Mined.N)),  80, 2), RAlign(Format("%dT", CCVP.NumberToString(Mined.T)),  80, 2), RAlign(Format("%dD", CCVP.NumberToString(Mined.D)),  80, 2), RAlign(Format("%dM", CCVP.NumberToString(Mined.M)),  80, 2), "\n")
  t = RAdd(t, RAlign(Translate("Ground:"), 50), RAlign(Format("%dN", CCVP.NumberToString(Ground.N)), 80, 2), RAlign(Format("%dT", CCVP.NumberToString(Ground.T)), 80, 2), RAlign(Format("%dD", CCVP.NumberToString(Ground.D)), 80, 2), RAlign(Format("%dM", CCVP.NumberToString(Ground.M)), 80, 2), "\n\n")

  % Line 8: FCode
  t = RAdd(t, Format(Translate("Friendly Code: %s"), FCode))
  SetContent t
EndSub

% Base overview:
% - called in planet context
% - call SetContent with a 30x10 rich-text string
% @since PCC2 2.40.1
Sub Tile.BaseOverview
  % ex WBaseOverviewTile::drawData, CStarbaseWindow.DrawData
  Local t

  % Line 1: Funds
  t = RAdd(Format(Translate("%d mc; %d kt supplies"), Money, Supplies), "\n")

  % Line 2: Minerals
  t = RAdd(t, RAlign(Translate("Minerals:"),  70), RAlign(Format("%dN", CCVP.NumberToString(Mined.N)),  60, 2), RAlign(Format("%dT", CCVP.NumberToString(Mined.T)),  80, 2), RAlign(Format("%dD", CCVP.NumberToString(Mined.D)),  80, 2), RAlign(Format("%dM", CCVP.NumberToString(Mined.M)),  80, 2), "\n")

  % Line 3: Tech
  t = RAdd(t, RAlign(Translate("Tech Levels:"), 100), RAlign(Format("%dH", Tech.Hull), 30, 2), RAlign(Format("%dE", Tech.Engine), 80, 2), RAlign(Format("%dB", Tech.Beam), 80, 2), RAlign(Format("%dT", Tech.Torpedo), 80, 2), "\n\n")

  % Line 4+5: Orders
  % FIXME:
  % if (int sid = pp.findShipCloningHere(*getCurrentUniverse())) {
  %      outTextF(ctx, x, y, extent.w, format(_("Cloning %s"),
  %                                           getCurrentUniverse()->getShip(sid).getName(GObject::PlainName)));
  If Build Then
    t = RAdd(t, Format(Translate("Building a %s"), Build), "\n")
  Else
    t = RAdd(t, Chr(160) & "\n")
  EndIf

  If Shipyard.Action = 'Fix' Then
    t = RAdd(t, Format(Translate("Repairing %s"), ShipName(Shipyard.Id)), "\n\n")
  Else If Shipyard.Action = 'Recycle' Then
    t = RAdd(t, Format(Translate("Recycling %s"), ShipName(Shipyard.Id)), "\n\n")
  Else
    t = RAdd(t, Chr(160) & "\n\n")
  EndIf

  % Line 6+7: Others
  t = RAdd(t, Format(Translate("Planetary FCode: %s"), FCode), "\n")
  t = RAdd(t, Format(Translate("SB Defense: %d; Fighters: %d; Damage: %d%%"), Defense.Base, Fighters, Damage))

  SetContent t
EndSub

% Fleet Overview
% - called in ship context on leader
% - call SetContent with a 30x10 rich-text string
% @since PCC2 2.40.13
Sub Tile.FleetOverview
  % ex WFleetOverviewTile::drawData, CFleetWindow.DrawData
  Local t
  Local fre = 0, cap = 0, tow = 0
  Local sh

  % Line 1: Name
  t = Format(Translate("Leader of fleet #%d"), Id)
  If Fleet.Name Then
    t = RAdd(t, ": ", Fleet.Name)
  EndIf
  t = RAdd(t, "\n")

  % Line 2: Location
  t = RAdd(t, Translate("Location: "), CCVP.GetLocationName(Loc.X, Loc.Y, "vwo"), "\n")

  % Line 3: Waypoint
  If InStr(Global.Mission(Mission$, Owner.Real).Flags, "i") Then
    t = RAdd(t, Translate("Mission: "), CCVP.ShipMissionLabel(), "\n")
  Else
    t = RAdd(t, Translate("Waypoint: "), CCVP.GetLocationName(Waypoint.X, Waypoint.Y, "vw"), "\n")
  EndIf

  % Line 4 (not in PCC2, but in PCC1)
  t = RAdd(t, Translate("Speed: "), If(Speed$, Speed, Translate("not moving")), "\n")
  t = RAdd(t, "\n")

  % Line 5ff: Ship counts
  ForEach Ship As sh Do
    If sh->Fleet$ = Id Then
      If sh->Type.Short <> 'f' Then
        cap = cap + 1
      Else
        fre = fre + 1
      EndIf
      If sh->Mission$=7 And Global.Ship(sh->Mission.Tow).Fleet$=Id Then
        tow = tow + 1
      EndIf
    EndIf
  Next
  If cap Then t = RAdd(t, Format(Translate("%d capital ship%!1{s%}"), cap), "\n")
  If fre Then t = RAdd(t, Format(Translate("%d freighter%!1{s%}"), fre), "\n")
  If tow Then t = RAdd(t, Format(Translate("%d fleet member%1{ is%|s are%} being towed."), tow), "\n")

  SetContent t
EndSub

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Ship Data %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Ship mission
% - called in ship context
% - call SetContent with a 30x3 rich-text string
% - call SetButton to configure buttons "b","e","m","f" if desired
Sub Tile.ShipMission
  % ex WShipMissionTile::drawData
  Local t, bc, System.Err, isCloning

  % Line 1: Mission
  t := RAdd(Translate("Mission:"), " ", RStyle(If(CCVP.ShipHasMissionWarning(), "yellow", "green"), CCVP.ShipMissionLabel()), "\n")

  % Line 2: PE
  t := RAdd(t, Translate("Primary Enemy:"), " ")
  If Not IsEmpty(Enemy$) Then
    If Enemy$ Then
      t := RAdd(t, RStyle(If(CCVP.ShipHasEnemyWarning(), "yellow", "green"), Enemy))
    Else
      t := RAdd(t, RStyle("green", Translate("none")))
    EndIf
  EndIf
  t := RAdd(t, "\n")

  % Line 3: FCode
  t := RAdd(t, Translate("Friendly Code:"), " ")
  If Not IsEmpty(FCode)
    t := RAdd(t, RStyle("green", FCode))
  EndIf

  SetContent t

  % Buttons
  bc := "none"
  isCloning := StrCase(FCode="cln")
  Try
    If Orbit$ And Planet(Orbit$).Base.YesNo Then
      If Planet(Orbit$).Shipyard.Id = Id Then
        Select Case Planet(Orbit$).Shipyard.Action
          Case "Fix"
            bc := "green"
          Case "Recycle"
            bc := "red"
        EndSelect
      EndIf
      If bc= "none" And isCloning Then
        bc := If(FindShipCloningAt(Orbit$)=Id, "green", "yellow")
      EndIf
    Else
      If isCloning Then bc := "yellow"
    EndIf
  EndTry
  SetButton "b", bc
EndSub

% Narrow ship mission
% - called in ship context
% - call SetContent with 25x6 rich-text string
Sub Tile.NarrowShipMission
  % ex WNarrowShipMissionTile::drawData
  Local Function Row(left, right)
    Return RAdd(RAlign(left, 80), right, "\n")
  EndFunction

  Local t, s, n, c, e, cloak_fuel, turn_fuel

  % PE
  t :=         Row(Translate("Enemy:"), If(Not IsEmpty(Enemy$), RStyle("green", If(Enemy, Enemy, Translate("none"))), ""))

  % FC
  t := RAdd(t, Row(Translate("FCode:"), If(Not IsEmpty(FCode), RStyle("green", FCode), "")))

  % Mission
  t := RAdd(t, Row(Translate("Mission:"), If(Not IsEmpty(Mission$), RStyle("green", CCVP.ShipMissionLabel()), "")))

  % Course
  s := ''
  If Not IsEmpty(Waypoint.X) And Not IsEmpty(Waypoint.Y) Then
    n := Waypoint.Dist
    If Speed<>"Hyperdrive" Then
      c := "green"
    Else If n<20 Then
      c := "red"
    Else
      % FIXME: : !host.isExactHyperjumpDistance2(int32(dist*dist+.1)) /* FIXME: this may lose precision, hence the +.1 */ ? UIColor::tc_Yellow
      c := "green"
    EndIf
    s := RStyle(c, Format(Translate("%.2f ly"), n))
  Else If Not IsEmpty(Heading$) Then
    s := RStyle("green", Format(Translate("%d"+Chr(176)+", %s"), Heading$, Heading))
  Else
    s := ''
  EndIf
  t := RAdd(t, Row(Translate("Course:"), s))

  % Warp
  If Not IsEmpty(Speed$) Then
    If Speed$=0 Then
      s := RStyle(If(Waypoint.Dist, "red", "green"), Translate("not moving"))
    Else If Speed='Hyperdrive' Then
      s := RStyle("green", Format(Translate("Hyperdrive (warp %d)"), Speed$))
    Else
      s := RStyle("green", Speed)
    EndIf
  Else
    s := ''
  EndIf
  t := RAdd(t, Row(Translate("Speed:"), s))

  % Fuel usage
  n := Move.Fuel
  e := Move.ETA
  If Not IsEmpty(n) Then
    If Waypoint.DX=0 And Waypoint.DY=0 Then
      s := RStyle("green", Translate("at waypoint"))
    Else If Speed='Hyperdrive' Then
      s := RStyle("green", Translate("Hyperdrive"))
    Else If Speed$=0 Then
      s := RStyle("red", Translate("not moving"))
    Else If e>=30 Then          % FIXME: 30 is replacement for 'if (crystal_ball.isAtTurnLimit())'
      s := RStyle("yellow", Translate("too long"))
    Else
      s := Format(Translate("%d turn%!1{s%}, %d kt fuel"), CCVP.NumberToString(e), CCVP.NumberToString(n))
      cloak_fuel := CCVP.ShipCloakFuel(e)
      turn_fuel := CCVP.ShipTurnFuel(e)
      If cloak_fuel Or turn_fuel Then
        If turn_fuel Then
          s := s + Format(Translate(" (+%d kt)"), CCVP.NumberToString(turn_fuel + cloak_fuel))
        Else
          s := s + Format(Translate(" (+%d kt cloak)"), CCVP.NumberToString(cloak_fuel))
        EndIf
      EndIf
      If n > Cargo.N Or (Cargo.N=0 And Not Cfg("AllowNoFuelMovement")) Then
        s := RStyle("red", s)
      Else If n + turn_fuel + cloak_fuel > Cargo.N Then
        s := RStyle("yellow", s)
      Else
        s := RStyle("green", s)
      EndIf
    EndIf
  Else
    s := ''
  EndIf
  t := RAdd(t, Row(Translate("Usage:"), s))

  SetContent t
EndSub

Sub Tile$ShipEquipment.Common(forHistory)
  % ex WShipEquipmentTile::drawShipEquipmentTile
  Local t, s

  % FIXME: UIColor::Color equipColor = (ship.getHistoryTimestamp(sts_ArmsDamage) == getCurrentTurn().getTurnNumber() ? UIColor::tc_Green : UIColor::tc_Yellow);
  Local equipColor = 'green'
  Local unknownLabel = If(forHistory, RStyle("yellow", Translate("unknown")), "")

  % Line 1: engines
  t := RAdd(RAlign(Translate("Engines:"), 80))
  If Not IsEmpty(Engine$) Then
    s := Engine
    If HasFunction("HardenedEngines") Then s := Format(Translate("Hard. %s"), s)
    If HasFunction("Gravitonic")      Then s := Format(Translate("Grav. %s"), s)
    If Engine.Count>1 Then s := Format(Translate("%d "&Chr(215)&" %s"), Engine.Count, s)
    t := RAdd(t, RStyle(equipColor, s))
  Else
    t := RAdd(t, unknownLabel)
  EndIf
  t := RAdd(t, "\n")

  % Line 2: Primary
  t := RAdd(t, RAlign(Translate("Primary:"), 80))
  If Not IsEmpty(Beam) And Not IsEmpty(Beam.Count) Then
    If Beam$ Then
      s := Beam
      If Beam.Count>1 Then s := Format(Translate("%d "&Chr(215)&" %s"), Beam.Count, Beam)
      t := RAdd(t, RStyle(equipColor, s))
    Else
      t := RAdd(t, RStyle(equipColor, Translate("none")))
    EndIf
  Else
    If IsEmpty(Beam.Max) Then
      t := RAdd(t, unknownLabel)
    Else If Beam.Max Then
      t := RAdd(t, RStyle("yellow", Format(Translate("up to %d beam%!1{s%}"), Beam.Max)))
    Else
      t := RAdd(t, RStyle("yellow", Translate("none")))
    EndIf
  EndIf
  t := RAdd(t, "\n")

  % Line 3+4: Secondary
  t := RAdd(t, RAlign(Translate("Secondary:"), 80))
  If IsEmpty(Fighter.Bays) Or IsEmpty(Torp$) Or IsEmpty(Torp.LCount) Then
    If Torp.LMax > 0 Then
      % Max launchers
      t := RAdd(t, RStyle("yellow", Format(Translate("up to %d torpedo launcher%!1{s%}"), Torp.LMax)), "\n")

      % Ammo
      If Not IsEmpty(Aux.Ammo) Then
        t := RAdd(t, RAlign(" ", 80))
        If Aux.Ammo Then
          t := RAdd(t, RStyle("yellow", Format(Translate("%d torpedo%!1{es%} aboard"), Aux.Ammo)))
        Else
          t := RAdd(t, RStyle("yellow", Translate("no torpedoes aboard")))
        EndIf
      Else
        t := RAdd(t, " ")
      EndIf
      t := RAdd(t, "\n")
    Else If Global.Hull(Hull$).Fighter.Bays > 0 Then
      % Max launchers
      t := RAdd(t, RStyle("yellow", Format(Translate("%d fighter bay%!1{s%}"), Global.Hull(Hull$).Fighter.Bays)), "\n")

      % Number of fighters
      If Not IsEmpty(Aux.Ammo) Then
        t := RAdd(t, RAlign(" ", 80))
        If Aux.Ammo Then
          t := RAdd(t, RStyle("yellow", Format(Translate("%d fighter%!1{s%} ready"), Aux.Ammo)))
        Else
          t := RAdd(t, RStyle("yellow", Translate("no fighters aboard")))
        EndIf
      Else
        t := RAdd(t, " ")
      EndIf
      t := RAdd(t, "\n")
    Else
      t := RAdd(t, RStyle("yellow", Translate("none")), "\n \n")
    EndIf
  Else If Torp$>0 And Torp.LCount>0 Then
    % Number of launchers
    t := RAdd(t, RStyle(equipColor, Format(Translate("%d "&Chr(215)& " %s launcher%!1{s%}"), Torp.LCount, Torp)), "\n")

    % Number of torpedoes
    If Not IsEmpty(Torp.Count) Then
      t := RAdd(t, RAlign(" ", 80))
      If Torp.Count Then
        t := RAdd(t, RStyle(equipColor, Format(Translate("%d torpedo%!1{es%} aboard"), Torp.Count)))
      Else
        t := RAdd(t, RStyle(equipColor, Translate("no torpedoes aboard")))
      EndIf
    Else
      t := RAdd(t, " ")
    EndIf
    t := RAdd(t, "\n")
  Else If Fighter.Bays>0 Then
    % Number of bays
    t := RAdd(t, RStyle(equipColor, Format(Translate("%d fighter bay%!1{s%}"), Fighter.Bays)), "\n")

    % Number of fighters
    If Not IsEmpty(Fighter.Count) Then
      t := RAdd(t, RAlign(" ", 80))
      If Fighter.Count Then
        t := RAdd(t, RStyle(equipColor, Format(Translate("%d fighter%!1{s%} ready"), Fighter.Count)))
      Else
        t := RAdd(t, RStyle(equipColor, Translate("no fighters aboard")))
      EndIf
    Else
      t := RAdd(t, " ")
    EndIf
    t := RAdd(t, "\n")
  Else
    t := RAdd(t, RStyle(equipColor, Translate("none")), "\n \n")
  EndIf

  % Line 5: Damage
  Local beingFixed = Planet(Orbit$).Shipyard.Id = Id And Planet(Orbit$).Shipyard.Action = "Fix"
  t := RAdd(t, RAlign(Translate("Damage:"), 80))
  If IsEmpty(Damage) Then
    t := RAdd(t, unknownLabel)
  Else If Not Damage Then
    t := RAdd(t, RStyle(equipColor, Translate("none")))
  Else
    If beingFixed Then
      t := RAdd(t, RStyle("yellow", Format(Translate("%d%% (fixed by base)"), Damage)))
    Else
      s := Cargo.Supplies \ 5
      If Cfg('PlayerSpecialMission', Owner.Real)=6 And (Mission$=9 Or Mission$=Cfg('ExtMissionsStartAt')+11) Then
        % Borg self repair
        s := s + 10
      EndIf
      If s Then
        t := RAdd(t, RStyle(If(s >= Damage, "yellow", "red"), Format("%d%% (-%d: %d%%)", Damage, s, Damage-s)))
      Else
        t := RAdd(t, RStyle("red", Format("%d%%", Damage)))
      EndIf
    EndIf
  EndIf
  t := RAdd(t, "\n")

  % Line 6: Crew
  t := RAdd(t, RAlign(Translate("Crew:"), 80))
  If IsEmpty(Crew) Then
    t := RAdd(t, RStyle("yellow", Format(Translate("up to %d"), Crew.Normal)))
  Else If Crew<>Crew.Normal Then
    t := RAdd(t, RStyle(If(beingFixed Or Crew>Crew.Normal, "yellow", "red"), Format(Translate("%d of %d"), Crew, Crew.Normal)))
  Else
    t := RAdd(t, RStyle(equipColor, CCVP.NumberToString(Crew)))
  EndIf

  % FIXME: exp!
  %    int16_t exp = ship.unit_scores.getScore(ship_score_definitions.lookupScore(ScoreId_ExpPoints));
  %    if (exp >= 0) {
  %        if (ship.getMission().isKnown() && ship.getMission() == config.ExtMissionsStartAt() + GMission::pmsn_Training) {
  %            // Ship is attempting to train
  %            int sup = ship.getInterceptId().isKnown() ? ship.getInterceptId() : 0;
  %            double points = sup < 25 ? sup : 25 + std::sqrt(8.0 * (sup - 25));
  %            int32_t acaBonus = ship.canDoSpecial(hf_Academy) ? config.EPAcademyScale(ship.getRealOwner()) : 100;
  %            int32_t rate = config.EPTrainingScale(ship.getRealOwner()) * acaBonus / 100;
  %            int32_t added = int32_t(rate * points / (std::sqrt(double(hull.getCrew())) + 1));
  %            line += format(_(" (%d EP, +%d)"), numToString(exp), numToString(added));
  %
  %            // Warning for mission failure if...
  %            // ...not at a planet
  %            // ...not enough supplies on played planet
  %            int pid = ship.getOrbitPlanetId();
  %            GUniverse* univ = getCurrentUniverse();
  %            if (univ == 0
  %                || !univ->isValidPlanetId(pid)
  %                || (univ->ty_played_planets.isValidIndex(pid)
  %                    && sup > univ->getPlanet(pid).getCargoRaw(el_Supplies)))
  %            {
  %                if (c == UIColor::tc_Green) {
  %                    c = UIColor::tc_Yellow;
  %                }
  %            }
  %        } else {
  %            // Normal ship
  %            line += format(_(" (%d EP)"), numToString(exp));
  %        }
  %    }
  t := RAdd(t, "\n")

  SetContent t

  % Buttons
  If forHistory Then
    SetButton "g", ""
    SetButton "r", ""
    SetButton "c", "none"
  Else
    % G button
    If CCVP.AllowGive() Then
      SetButton "g", If(GetCommand("give ship "&Id), "yellow", "none")
    Else
      SetButton "g", ""
    EndIf

    % R button
    If CCVP.AllowRemote() Then
      SetButton "r", CC$RemoteGetColor(Id)
    Else
      SetButton "r", ""
    EndIf

    % C button
    SetButton "c", ""
  EndIf
EndSub

% Ship Equipment
% - called in ship context
% - call SetContent with a 30x6 rich-text string
% - call SetButton to configure buttons "r","g","s","c" (r,g: only for ship screen; c: only history screen)
Sub Tile.ShipEquipment
  Tile$ShipEquipment.Common False
EndSub

% Ship Equipment (for history); otherwise same as Tile.ShipEquipment
Sub Tile.HistoryEquipment
  Tile$ShipEquipment.Common True
EndSub

% Narrow ship equipment
% - called in ship context
% - call SetContent with 25x5 rich-text string
Sub Tile.NarrowShipEquipment
  % ex WNarrowShipEquipmentTile::drawData
  Local t, s

  % Engines
  If Not IsEmpty(Engine$) Then
    s := Engine
    If HasFunction("HardenedEngines") Then s := Format(Translate("Hard. %s"), s)
    If HasFunction("Gravitonic") Then s := Format(Translate("Grav. %s"), s)
    If Engine.Count>1 Then s := Format(Translate("%d "&Chr(215)&" %s"), Engine.Count, s)
    t := RStyle('green', s)
  Else If Hull$ Then
    % FIXME: Hardened? (missing in PCC2 as well)
    If HasFunction("Gravitonic") Then
      t := Format(Translate("%d grav. engine%!1{s%}"), Engine.Count)
    Else
      t := Format(Translate("%d engine%!1{s%}"), Engine.Count)
    EndIf
  Else
    t := ''
  EndIf
  t := RAdd(t, "\n")

  % Beams
  If Not IsEmpty(Beam$) Then
    If Beam$=0 Or Beam.Count=0 Then
      t := RAdd(t, RStyle("green", Translate("no beam weapons")))
    Else If Not IsEmpty(Beam.Count) Then
      % type and count
      t := RAdd(t, RStyle("green", Format(Translate("%!d%!1{%0$d "+Chr(215)+" %}%1$s"), Beam.Count, Beam)))
    Else If Not IsEmpty(Beam.Max) Then
      % just type known
      t := RAdd(t, Format(Translate("up to %d %s"), Beam.Max, Beam))
    Else
      t := RAdd(t, Beam)
    EndIf
  Else If Not IsEmpty(Beam.Max) Then
    If Beam.Max Then
      t := RAdd(t, Format(Translate("up to %d beam%!1{s%}"), Beam.Max))
    Else
      t := RAdd(t, Translate("no beam weapons"))
    EndIf
  Else
    t := RAdd(t, " ")
  EndIf
  t := RAdd(t, "\n")

  % Torps/Fighters
  If Torp.LCount Then
    % Ship has torpedoes
    If Not IsEmpty(Torp$) Then
      If Not IsEmpty(Torp.Count) Then
        t := RAdd(t, RStyle("green", Format(Translate("%!d%!1{%0$d "+Chr(215)+" %}%1$s, %d torpedo%!1{es%}"), Torp.LCount, Torp, CCVP.NumberToString(Torp.Count))))
      Else
        t := RAdd(t, RStyle("green", Format(Translate("%!d%!1{%0$d "+Chr(215)+" %}%1$s"), Torp.LCount, Torp)))
      EndIf
    Else
      If Not IsEmpty(Torp.Count) Then
        t := RAdd(t, Format(Translate(Translate("%d launcher%!1{s%}, %d torpedo%!1{es%}"), Torp.LCount, CCVP.NumberToString(Torp.Count))))
      Else
        t := RAdd(t, Format(Translate(Translate("%d torpedo launcher%!1{s%}"), Torp.LCount)))
      EndIf
    EndIf
  Else If Fighter.Bays Then
    % Ship has fighters
    If Not IsEmpty(Fighter.Count) Then
      t := RAdd(t, RStyle("green", Format(Translate("%d bay%!1{s%}, %d fighter%!1{s%}"), Fighter.Bays, CCVP.NumberToString(Fighter.Count))))
    Else
      t := RAdd(t, RStyle("green", Format(Translate("%d fighter bay%!1{s%}"), Fighter.Bays)))
    EndIf
  Else If Torp.LCount=0 And Fighter.Bays=0 Then
    % No torpedo launchers or fighters
    t := RAdd(t, RStyle("green", Translate("no secondary weapons")))
  Else If Hull$ Then
    % Hull is known
    If Torp.LMax Then
      t := RAdd(t, Format(Translate("up to %d torpedo launcher%!1{s%}"), Torp.LMax))
    Else If Global.Hull(Hull$).Fighter.Bays Then
      t := RAdd(t, Format(Translate("%d fighter bay%!1{s%}"), Global.Hull(Hull$).Fighter.Bays))
    Else
      t := RAdd(t, Translate("no secondary weapons"))
    EndIf
  Else
    % Nothing known
    t := RAdd(t, " ")
  EndIf
  t := RAdd(t, "\n")

  % Crew [of Crew]
  If Not IsEmpty(Crew) Then
    If Not IsEmpty(Crew.Normal) And Crew<>Crew.Normal Then
      t := RAdd(t, RStyle("red", Format(Translate("%d crew (of %d)"), CCVP.NumberToString(Crew), CCVP.NumberToString(Crew.Normal))))
    Else
      t := RAdd(t, RStyle("green", Format(Translate("%d crew"), CCVP.NumberToString(Crew))))
    EndIf
  Else If Not IsEmpty(Crew.Normal) Then
    t := RAdd(t, Format(Translate("up to %d crew"), CCVP.NumberToString(Crew.Normal)))
  Else
    t := RAdd(t, " ")
  EndIf
  t := RAdd(t, "\n")

  % Damage
  If Not IsEmpty(Damage) Then
    t := RAdd(t, RStyle(If(Damage, "yellow", "green"), Format(Translate("%d%% damage"), Damage)))
  Else
    % nix
  EndIf

  SetContent t
EndSub

% Narrow ship cargo
% - called in ship context
% - call SetContent with 25x4 rich-text string
Sub Tile.NarrowShipCargo
  Local t, kt
  Local Function Column(n, v, u, w)
    Local color = If(w, "red", "green")
    If IsEmpty(v) Then
      Return RAdd(RAlign(n, 40), RAlign(" ",                                   50, 2), RAlign(" ", 30))
    Else
      Return RAdd(RAlign(n, 40), RAlign(RStyle(color, CCVP.NumberToString(v)), 50, 2), RAlign(RStyle(color, " " + u), 30))
    EndIf
  EndFunction
  Local Function Row(n1, v1, u1, w1, n2, v2, u2, w2)
    Return RAdd(Column(n1, v1, u1, w1), " ", Column(n2, v2, u2, w2), "\n")
  EndFunction

  kt := Translate("kt")
  t :=         Row(Translate("Neu:"), Cargo.N, kt, Cargo.N<=0, Translate("Sup:"),  Cargo.Supplies,  kt,               False)
  t := RAdd(t, Row(Translate("Tri:"), Cargo.T, kt, False,      Translate("Col:"),  Cargo.Colonists, Translate("cl."), False))
  t := RAdd(t, Row(Translate("Dur:"), Cargo.D, kt, False,      Translate("Cash:"), Cargo.Money,     Translate("mc"),  False))
  t := RAdd(t, Row(Translate("Mol:"), Cargo.M, kt, False,      "",                 Z(0),            "",               False))

  SetContent t
EndSub

% Fleet waypoint
% - called in ship context on fleet member
% - call SetContent with 30x7 rich-text string
Sub Tile.FleetWaypoint
  % ex WFleetWaypointTile::drawData(GfxCanvas& can)
  Local Function Row(left, right)
    Return RAdd(RAlign(left, 80), right, "\n")
  EndFunction

  Local Function WaypointLabel(leader)
    If leader Then
      With leader Do
        If InStr(Global.Mission(Mission$, Owner.Real).Flags, "i") Then
          Return CCVP.ShipMissionLabel()
        Else
          Return CCVP.GetLocationName(Waypoint.X, Waypoint.Y, "vw")
        EndIf
      EndWith
    Else
      Return ""
    EndIf
  EndFunction

  Local t, w, c
  Local leader = Global.Ship(Fleet$)

  % Line 1: waypoint
  t :=         Row(Translate("Waypoint:"),    RStyle("green", WaypointLabel(leader)))

  % Line 2: speed
  t := RAdd(t, Row(Translate("Warp Factor:"), RStyle("green", If(leader->Speed$, leader->Speed, Translate("not moving")))))

  % Line 3+4: title
  t := RAdd(t, "\n", RStyle("big,heading-color", Translate("This ship:")), "\n")

  % Line 5: distance
  t := RAdd(t, Row(Translate("Distance:"), RStyle("green", Format(Translate("%.2f ly"), Waypoint.Dist))))

  % Line 6,7: ETA and fuel usage
  If Not Waypoint.Dist Then
    w := RStyle("green", Translate("at waypoint"))
  Else If Speed$=0 Then
    w := RStyle("red", Translate("not moving"))
  Else If Move.ETA >= 30 Then  % FIXME: hard-coded
    w := RStyle("yellow", Translate("too long"))
  Else
    w := RStyle("green", Format(Translate("%d turn%!1{s%}"), Move.ETA))
  EndIf
  t := RAdd(t, Row(Translate("E.T.A.:"), w))

  If Move.ETA >= 30 Then
    c := 'yellow'
  Else If Move.Fuel > Cargo.N Then
    c := 'red'
  Else
    c := 'green'
  EndIf
  % FIXME: cloak fuel, turn fuel
  t := RAdd(t, Row(Translate("Fuel Usage:"), RStyle(c, Format(Translate("%d kt"), CCVP.NumberToString(Move.Fuel)))))
  SetContent t
EndSub

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Planet Data %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Planet natives
% - called in planet context
% - call SetContent with a 30x4 rich-text string
% @since PCC2 2.40.1
Sub Tile.PlanetNatives
  % ex WPlanetNativeTile::drawData, CPlanetaryNativesTile
  Local t
  If Not Natives Then
    SetContent Translate("There are no natives on this planet.")
  Else
    t := RAdd(   RAlign(Translate("Race:"),       80), RStyle("green", Natives.Race, ", ", Natives.Gov), "\n")
    t := RAdd(t, RAlign(Translate("Population:"), 80), RStyle("green", CCVP.ClansToString(Natives)), "\n")
    t := RAdd(t, RAlign(Translate("Tax Rate:"),   80), RStyle("green", Format("%d%%", Natives.Tax)), "\n")
    t := RAdd(t, Translate("Natives are"), " ", RStyle(CCVP.HappyColor(Natives.Happy$), Format("%s (%d)", Natives.Happy, Natives.Happy$)), "\n")
    SetContent t
  EndIf
EndSub

% Planet colonists
% - called in planet context
% - call SetContent with a 30x3 rich-text string
% - call SetButton to configure button "t" if desired
% @since PCC2 2.40.1
Sub Tile.PlanetColonists
  % ex WPlanetColonistTile::drawPlanetColonistTile, CPlanetaryColonistsTile
  Local t
  t := RAdd(   RAlign(Translate("Population:"), 80), RStyle("green", CCVP.ClansToString(Colonists)), "\n")
  t := RAdd(t, RAlign(Translate("Tax Rate:"),   80), RStyle("green", Format("%d%%", Colonists.Tax)), "\n")
  t := RAdd(t, Translate("Colonists are"), " ", RStyle(CCVP.HappyColor(Colonists.Happy$), Format("%s (%d)", Colonists.Happy, Colonists.Happy$)), "\n")
  SetContent t
EndSub

% Planet economy
% - called in planet context
% - call SetContent with a 30x8 rich-text string
% - call SetButton to configure buttons "g","b","m","d","s","c" if desired
% @since PCC2 2.40.1
Sub Tile.PlanetEconomy
  % ex WPlanetEconomyTile::drawPlanetEconomyTile, CPlanetaryEconomyTile
  Local Function Row(key, value, unit)
    Return RAdd(RAlign(key, 120), RAlign(RStyle("green", CCVP.NumberToString(value)), 50, 2), RStyle("green", " " & unit), "\n")
  EndFunction

  Local t
  t := RAdd(   Row(Translate("Mineral Mines:"), Mines, ""))
  t := RAdd(t, Row(Translate("Factories:"),     Factories, ""))
  t := RAdd(t, Row(Translate("Defense Posts:"), Defense, ""))
  t := RAdd(t, Row(Translate("Neutronium:"),    Mined.N, Translate("kt")))
  t := RAdd(t, Row(Translate("Tritanium:"),     Mined.T, Translate("kt")))
  t := RAdd(t, Row(Translate("Duranium:"),      Mined.D, Translate("kt")))
  t := RAdd(t, Row(Translate("Molybdenum:"),    Mined.M, Translate("kt")))
  t := RAdd(t, Row(Translate("Supplies:"),      Supplies, Translate("kt")))
  SetContent t

  % Buttons
  If CCVP.AllowGive() Then
    If GetCommand("give planet " & Id) Then
      SetButton "g", "yellow"
    Else
      SetButton "g", "none"
    EndIf
  Else
    SetButton "g", ""
  EndIf
EndSub

% Planet fcode
% - called in planet context
% - call SetContent with a 30x3 rich-text string
% - call SetButton to configure button "f" if desired
% @since PCC2 2.40.1
Sub Tile.PlanetFCode
  % ex WPlanetFCodeTile::drawData, cscreen.pas:CPlanetaryFCodeTile
  Local t
  t := RAdd(   Translate("Funds:"), " ", RStyle("green", CCVP.NumberToString(Money), " ", Translate("mc")), "\n")
  t := RAdd(t, Translate("Friendly Code:"), " ", RStyle("green", FCode), "\n")
  SetContent t
EndSub

% Planet link
% - called in starbase context
% - call SetLeftText/SetRightText for buttons "f8","f5"
% - call SetButton for these buttons if desired
Sub Tile.PlanetLink
  % ex CPlanetarySBTile
  SetLeftText 'f5', RStyle("white,big", Translate("Planet overview"))
  If Base="present" Then
    SetLeftText 'f8', RStyle("white,big", Translate("Go to starbase"))
  Else If Base="being built" Then
    SetLeftText 'f8', RStyle("green,big", Translate("Starbase being built..."))
  Else
    SetLeftText 'f8', RStyle("white,big", Translate("Build starbase"))
  EndIf
EndSub

% Planet Minerals (Starchart)
% - called in planet context
% - call SetContent with 25x5 rich text
% @since PCC2 2.40.6
Sub Tile.NarrowPlanetMinerals
  % ex WNarrowPlanetMineralTile::drawData
  Local Function Green(t)
    Return RStyle("green", t)
  EndFunction
  Local Function Row(name, mined, ground, density)
    Local r = RAlign(name, 30)
    If IsEmpty(mined)
      r := RAdd(r, RAlign(' ', 100))
    Else
      r := RAdd(r, RAlign(Green(CCVP.NumberToString(mined)), 70, 2), RAlign(Green(' ' & Translate("kt")), 30))
    EndIf
    If IsEmpty(ground)
      r := RAdd(r, RAlign(' ', 100))
    Else
      r := RAdd(r, RAlign(Green(CCVP.NumberToString(ground)), 70, 2), RAlign(Green(' ' & Translate("kt")), 30))
    EndIf
    If Not IsEmpty(density)
      r := RAdd(r, RAlign(RStyle("green", density & "%"), 30, 2))
    EndIf
    Return RAdd(r, "\n")
  EndFunction


  Local _ = Translate
  Local t = RAdd(RAlign(_("Mined"), 100, 2), RAlign(_("Ground"), 100, 2), "\n")
  t := RAdd(t, Row(_("Neu:"), Mined.N, Ground.N, Density.N))
  t := RAdd(t, Row(_("Tri:"), Mined.T, Ground.T, Density.T))
  t := RAdd(t, Row(_("Dur:"), Mined.D, Ground.D, Density.D))
  t := RAdd(t, Row(_("Mol:"), Mined.M, Ground.M, Density.M))
  SetContent t
EndSub

% Planet Economy (Starchart)
% - called in planet context
% - call SetContent with 25x3 rich text
% @since PCC2 2.40.6
Sub Tile.NarrowPlanetEconomy
  % ex WNarrowPlanetEconomyTile::drawData
  Local Function Green(t)
    Return RStyle("green", t)
  EndFunction
  Local Function Line(e1, n1, e2, n2, u2)
    Local r := RAlign(e1 & ':', 60)
    r := RAdd(r, RAlign(If(IsEmpty(n1), ' ', Green(n1)), 40, 2), '  ')
    r := RAdd(r, RAlign(If(IsEmpty(e2), ' ', e2 & ':'),  60))
    r := RAdd(r, RAlign(If(IsEmpty(n2), ' ', Green(n2)), 60, 2), ' ')
    If Not IsEmpty(n2)
      r := RAdd(r, Green(u2))
    EndIf
    Return RAdd(r, "\n")
  EndFunction

  Local _ = Translate
  Local t =    Line(_("Mines"),     Mines,     _("Money"),    Money,    _("mc"))
  t := RAdd(t, Line(_("Factories"), Factories, _("Supplies"), Supplies, _("kt")))
  t := RAdd(t, Line(_("Defense"),   Defense,   Z(0),          Z(0),     ""))
  SetContent t
EndSub

% Planet Colonists (Starchart)
% - called in planet context
% - call SetContent with 25x3 rich text
% @since PCC2 2.40.6
Sub Tile.NarrowPlanetColonists
  % ex WNarrowPlanetColonistTile::drawData
  Local t, c
  If Not IsEmpty(Owner$) Then
    If Owner$=0 Then
      t := Translate("No colonists.")
    Else
      % Line 1: Owner/clans
      c := CCVP.PlayerColor(Owner$)
      If IsEmpty(Colonists) Then
        t := RStyle(c, Owner)
      Else
        t := RStyle(c, Format(Translate("%s, %d clan%!1{s%}"), Owner, CCVP.NumberToString(Colonists)))
      EndIf
      t := RAdd(t, "\n")

      % Line 2: Happiness (if known)
      If Not IsEmpty(Colonists.Happy$) Then
        t := RAdd(t, RStyle(CCVP.HappyColor(Colonists.Happy$), Format("%s (%d)", Colonists.Happy, Colonists.Happy$)))
      EndIf
      t := RAdd(t, "\n")

      % Line 3: Tax Rate
      If Not IsEmpty(Colonists.Tax) Then
        t := RAdd(t, Translate("Tax Rate:"), " ", RStyle("green", Format("%d%%", Colonists.Tax)))
      EndIf
    EndIf
  Else
    t := ''
  EndIf
  SetContent t
EndSub

% Planet Natives (Starchart)
% - called in planet context
% - call SetContent with 25x4 rich text
% @since PCC2 2.40.6
Sub Tile.NarrowPlanetNatives
  % ex WNarrowPlanetNativeTile::drawData
  % FIXME: this does not yet handle the "known to have natives, but not known which ones" case
  Local t
  If Not IsEmpty(Natives.Race$) Then
    % FIXME -> if (!pl->getNativeRace().isKnown()) {
    %     /* This means this planet is known to have natives, but we don't know which ones. */
    %     ctx.setColor(UIColor::tc_Static);
    %     outTextF(ctx, x, y, w, _("Planet has natives."));
    %     return;
    % }
    If Natives.Race$=0 Then
      t := Translate("No natives.")
    Else
      % Line 1: Race/Population
      If IsEmpty(Natives) Then
        t := RStyle("green", Natives.Race)
      Else
        t := RStyle("green", Format(Translate("%s natives, %d clan%!1{s%}"), Natives.Race, CCVP.NumberToString(Natives)))
      EndIf
      t := RAdd(t, "\n")

      % Line 2: Government
      If Not IsEmpty(Natives.Gov) Then
        t := RAdd(t, RStyle("green", Natives.Gov))
      Else
        t := RAdd(t, Translate("Unknown government."))
      EndIf
      t := RAdd(t, "\n")

      % Line 3: Happiness
      If Not IsEmpty(Natives.Happy) Then
        t := RAdd(t, RStyle(CCVP.HappyColor(Natives.Happy$), Format("%s (%d)", Natives.Happy, Natives.Happy$)))
      EndIf
      t := RAdd(t, "\n")

      % Line 4: Tax rate
      If Not IsEmpty(Natives.Tax) Then
        t := RAdd(t, Translate("Tax Rate:"), " ", RStyle("green", Format("%d%%", Natives.Tax)))
      EndIf
    EndIf
  Else
    t := ''
  EndIf
  SetContent t
EndSub

% Planet FCode (Starchart)
% - called in planet context
% - call SetContent with 25x2 rich text
% @since PCC2 2.40.6
Sub Tile.NarrowPlanetFCode
  % ex WNarrowPlanetFCodeTile::drawData
  % Line 1
  Local t = ''
  If Not IsEmpty(FCode) Then
    t := RAdd(Translate("Friendly Code:"), " ", RStyle("green", FCode))
  EndIf
  t := RAdd(t, "\n")

  % Line 2
  Select Case Base
    Case 'present'
      t := RAdd(t,                 Translate("Starbase present"))
    Case 'being built'
      t := RAdd(t, RStyle("green", Translate("Starbase being built")))
    Case Else
      t := RAdd(t,                 Translate("No starbase"))
  EndSelect
  SetContent t
EndSub

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Base Data %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Starbase minerals
Sub Tile.BaseMineral
  Local t, kts, mcs, _ = Translate
  Local Function MakeLine(aLabel, aValue, Optional bLabel, bValue)
    Local r
    r := RAdd(RAlign(aLabel, 30), RAlign(RStyle("green", aValue), 80, 2))
    If bLabel Then
      r := RAdd(r, RAlign(" ", 10), RAlign(bLabel, 80), RAlign(RStyle("green", bValue), 80, 2))
    EndIf
    Return RAdd(r, "\n")
  EndFunction

  kts := " " & _("kt")
  mcs := " " & _("mc")
  t :=         MakeLine(_("Neu:"), CCVP.NumberToString(Mined.N) & kts, _("Money:"), CCVP.NumberToString(Money) & mcs)
  t := RAdd(t, MakeLine(_("Tri:"), CCVP.NumberToString(Mined.T) & kts, _("Supplies:"), CCVP.NumberToString(Supplies) & kts))
  t := RAdd(t, MakeLine(_("Dur:"), CCVP.NumberToString(Mined.D) & kts))
  t := RAdd(t, MakeLine(_("Mol:"), CCVP.NumberToString(Mined.M) & kts))

  SetContent t
EndSub

% Starbase tech
% - called in starbase context
% - call SetContent with a 30x4 rich-text string
% - call SetButton to configure buttons "t","d","s" if desired
% @since PCC2 2.40.1
Sub Tile.BaseTech
  % ex drawBaseTechTile (sort-of)
  Local t, _ = Translate
  Local Function MakeLine(techLabel, techValue, otherLabel, otherValue)
    Return RAdd(RAlign(techLabel, 120), RAlign(RStyle("green", techValue), 20, 2), RAlign(" ", 10), RAlign(otherLabel, 80), RAlign(RStyle("green", otherValue), 40, 2), "\n")
  EndFunction

  t :=         MakeLine(_("Engines:"),      Tech.Engine, _("Defense:"),  CCVP.NumberToString(Defense.Base))
  t := RAdd(t, MakeLine(_("Hulls:"),        Tech.Hull,   _("Fighters:"), CCVP.NumberToString(Fighters)))
  t := RAdd(t, MakeLine(_("Beam Weapons:"), Tech.Beam,   _("Damage:"),   If(Damage, RStyle("red", Damage&"%"), _("none"))))
  t := RAdd(t, MakeLine(_("Torpedoes:"),    Tech.Torpedo, "", ""))

  SetContent t
EndSub

% Narrow base tech tile
% For now, the same as Tile.BaseTech; it has no buttons and in PCC2 slightly different text
Sub Tile.NarrowBaseTech
  Tile.BaseTech
EndSub

% Base orders
% - called in starbase context
% - call SetLeftText/SetRightText for buttons "b","r","m","a"
% - call SetButton for these buttons if desired
Sub Tile.BaseOrder
  % ex WBaseOrderTile::drawData
  % Ship building
  SetLeftText 'b', Tile$BaseBuildOrder(Translate("Build a new ship..."))
  If Not IsEmpty(Build.QPos)
    SetRightText 'b', RStyle("yellow", Format(" Q:%d", Build.QPos))
  Else
    SetRightText 'b', ''
  EndIf

  % Fix/Recycle
  SetLeftText 'r', Tile$BaseShipyardOrder(Translate("Repair/recycle a starship..."))

  % Mission
  SetLeftText 'm', RAdd(Translate("Mission: "), RStyle("green", Mission))

  % Ammo
  SetLeftText 'a', Translate("Build Fighters/Torpedoes...")
EndSub

% Base orders
% - called in starbase context
% - 4 lines of text
% @since PCC2 2.40.12
Sub Tile.NarrowBaseOrder
  % ex WNarrowBaseOrderTile::drawData
  Local t, _ = Translate
  Local c = Z(FindShipCloningAt(Id))

  t := RAdd(Tile$BaseBuildOrder(Translate('Not building a ship')), "\n")

  % Shipyard
  t := RAdd(t, Tile$BaseShipyardOrder(Translate("Not working on a starship")), "\n")

  % Mission/FCode
  t := RAdd(t, Translate("Mission: "), RStyle("green", Mission), "\n")
  t := RAdd(t, Translate("Friendly Code: "), RStyle("green", FCode), "\n")

  SetContent t
EndSub

% Base link
% - called in starbase context
% - call SetLeftText/SetRightText for buttons "f8","f5"
% - call SetButton for these buttons if desired
Sub Tile.BaseLink
  SetLeftText 'f5', RStyle("white,big", Translate("Planet overview"))
  SetLeftText 'f8', RStyle("white,big", Translate("Go to planet"))
EndSub


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Others %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Comment
% - called in ship or planet context
% - call SetLeftText/SetRightText for button "f9"
% - call SetButton for these buttons if desired
% @since PCC2 2.40.1
Sub Tile.Comment
  If IsEmpty(Z(Comment)) Then
    SetLeftText 'f9', Translate("(press [F9] to add a note)")
  Else
    SetLeftText 'f9', RStyle("yellow", Comment)
  EndIf
EndSub
