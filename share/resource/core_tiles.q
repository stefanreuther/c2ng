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

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Headers %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

Option LocalSubs(1)

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
    s := Format(Translate("%.2f ly"), Waypoint.Dist) & ", " & If(Speed$, Speed, Translate("not moving"))
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

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Ship Data %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Ship mission
% - called in ship context
% - call SetContent with a 30x3 rich-text string
% - call SetButton to configure buttons "b","e","m","f" if desired
Sub Tile.ShipMission
  % ex WShipMissionTile::drawData
  Local t, bc, System.Err

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
  Try
    % FIXME: handle cloning. PCC 1.x uses green if cloning, yellow if conflict (multiple cloners)
    If Planet(Orbit$).Shipyard.Id = Id Then
      Select Case Planet(Orbit$).Shipyard.Action
        Case "Fix"
          bc := "green"
        Case "Recycle"
          bc := "red"
      EndSelect
    EndIf
  EndTry
  SetButton "b", bc
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
  Option LocalSubs(1)
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
  Option LocalSubs(1)
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
  Option LocalSubs(1)
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


% Base orders
% - called in starbase context
% - call SetLeftText/SetRightText for buttons "b","r","m","a"
% - call SetButton for these buttons if desired
Sub Tile.BaseOrder
  % ex WBaseOrderTile::drawData
  Local c = Z(FindShipCloningAt(Id))

  % Ship building
  If Build.YesNo Then
    If IsEmpty(c) Then
      SetLeftText 'b', RStyle("yellow", Format(Translate("Building a %s"), Build))
    Else
      SetLeftText 'b', RStyle("red", Format(Translate("Cloning, and building a %s"), Build))
    EndIf
  Else
    If IsEmpty(c) Then
      SetLeftText 'b', Translate("Build a new ship...")
    Else
      SetLeftText 'b', RStyle("yellow", Format(Translate("Cloning %s"), ShipName(c)))
      
    EndIf
  EndIf
  If Not IsEmpty(Build.QPos)
    SetRightText 'b', RStyle("yellow", Format(" Q:%d", Build.QPos))
  Else
    SetRightText 'b', ''
  EndIf

  % Fix/Recycle
  If Shipyard.Action = 'Fix' Then
    SetLeftText 'r', RStyle("yellow", Format(Translate("Repairing %s"), ShipName(Shipyard.Id)))
  Else If Shipyard.Action = 'Recycle' Then
    SetLeftText 'r', RStyle("yellow", Format(Translate("Recycling %s"), ShipName(Shipyard.Id)))
  Else
    SetLeftText 'r', Translate("Repair/recycle a starship...")
  EndIf
  
  % Mission
  SetLeftText 'm', RAdd(Translate("Mission: "), RStyle("green", Mission))

  % Ammo
  SetLeftText 'a', Translate("Build Fighters/Torpedoes...")
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
