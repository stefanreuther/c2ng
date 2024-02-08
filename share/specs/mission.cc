;=============================================================================
; Starship Mission List for PCC2 NG
;
; Note: this file has been modified for PCC2 NG and is not intended
; for the other PCC/PCC2 versions.
;
; Format for each mission:    code,flags,name
;                               key=value
; * code = Mission Code
; * flags = set of the following:
;   +0-9ab      = only given races
;   -0-9ab      = except for given races
;   r           = registered only
;   *           = requires intercept argument (integer)
;   #           = requires tow argument (integer)
;   i           = "intercept" type mission (wrt. fleets etc)
;   p*, p#      = intercept/tow argument (planet id)
;   s*, s#      = intercept/tow argument (ship id)
;   h*, h#      = intercept/tow argument (ship id, ship must be here)
;   b*, b#      = intercept/tow argument (base id)
;   o[psh][*#]  = own ship/planet/here
;   ![sh][*#]   = don't allow this ship as argument
;   y*, y#      = intercept/tow argument (player #)
; * name = name of mission in [M] list. A letter prefixed with ~ is
;   the shortcut key. Default shortcut is mission number (if <=9) or a
;   still unused letter.
; * key=value pairs (indented in next lines). Only first character is
;   significant, but if you spell it out, use the word shown in [brackets].
;   s=name      [short] short mission name (<=7 chars), default=first 7
;               chars of long name
;   c=expr      [condition] condition saying when this mission is available
;   t=expr      [text] expression giving text to display if mission is set,
;               default=long name. If it returns EMPTY, the default is used.
;   i=name      [i] name of "I" parameter (<=7 chars)
;   j=name      [j] name of "T" parameter (<=7 chars)
;   w=expr      [works] condition saying whether this mission will work.
;               Defaults to same expression as "c=" if given
;   o=command   [onset] command to be run when the user sets this mission
;               using the mission setting dialog
;
; The difference between "c=" and "w=" is that "c" will determine whether
; the mission shall be offered on the mission selector while "w" will
; affect whether the mission name will be displayed in yellow (doesn't
; work) or green (will work). For example, we don't want users to set
; mission 0, so c=False, but we don't want to scare them if host sets that
; mission, so w=True. Likewise, we want the user to always be able to set
; the Cloak mission on capable ships, but we want to give visual feedback
; when cloaking is expected to fail due to excess damage.
;
; The "o" assignment allows you to do some additional things after a
; mission was set. For example, to clear a towee's warp when you set a
; Tow mission, you could do
;     7,!h#,Tow a ship
;        o = Try With Ships(Mission.Tow) Do SetSpeed 0
; The command is run after the mission parameters have been queried and
; the mission was set. To make extra "pseudo" missions which do funny things
; you could use pseudo mission numbers, like
;     777,!h#,Tow and reset warp factor
;        o = Try Eval 'With Ships(Mission.Tow) Do SetSpeed 0', 'SetMission 7,0,Mission.Tow'
; (good style would be using a subroutine instead of 'Eval').
;
; PCC also supports MISSION.INI, the extended mission file from WinPlan.
; That one is not as configurable as MISSION.CC. If a mission definition
; is found in both, MISSION.CC takes precedence.
;
; Missions appear in the same order as here in PCC.
;
; Note that some of the standard missions have hard-wired additional
; actions: mine laying => shows size of new field, tow => computes
; additional engine load and displays tower's waypoint in towee's control
; screen, cloak => fuel usage. Likewise, PHost's extended mine-laying
; missions have hardwired interpretation.
;
; SRace is re-interpreting mission numbers such that mission 1 is actually
; mission 9. This remapping is handled transparently by PCC, although this
; file still needs to have the right condition.
;=============================================================================

; If you want PCC to be able to name a mission, but not set it, use
; `c=False'. `none' is set by Host (and sometimes PCC) if it doesn't
; know better. You can't usually set this mission manually. Use
; `Explore' instead.
0,,none
                c = False
                w = True

;==== Normal Missions ====
1,,Explore
                c = System.Host$<>1
2,,Mine Sweep
                s = M-Sweep
3,,Lay Mines
                c = Torp
                w = Torp.Count
4,,Kill
5,,Sensor Sweep
                s = Sensor
6,,Land & Disassemble
                s = L&D
7,!h#,Tow a ship
                t = "Towing " # ShipNameAndId(Mission.Tow)
                s = Tow
                c = Engine.Count>1 Or Cfg("AllowOneEngineTowing")
8,!is*,Intercept a ship
                t = "Intercepting " # ShipNameAndId(Mission.Intercept)
                s = Interc.
                w = Speed$>0

;==== Race Specific Missions ====
9,+1,Super Refit
                s = S-Refit
9,+2,Hisssssssss
                c = Beam
9,+3,Super Spy
                s = S-Spy
9,+4,Pillage Planet
9,+5,Rob ship
                c = Beam
                s = RobShip
9,+6,Self Repair
                s = Repair
9,+7,Lay Web Mines
                c = Torp
                w = Torp.Count
                s = LayWeb
9,+8,Dark Sense
                s = DkSense
9,+9b,Build Fighters
                c = Fighter.Bays
                s = B-Ftr
9,+a,Rebel Ground Attack
                s = RGA

;==== More Missions ====
10,,~Cloak
                c = InStr(Hull.Special, "C")
                w = Damage < Cfg("DamageLevelForCloakFail")
11,,Beam Up ~Fuel
                s = BU Fuel
                w = Orbit$
12,,Beam Up ~Duranium
                s = BU Dur
                w = Orbit$
13,,Beam Up ~Tritanium
                s = BU Tri
                w = Orbit$
14,,Beam Up ~Molybdenum
                s = BU Mol
                w = Orbit$
15,,Beam Up ~Supplies
                s = BU Supp
                w = Orbit$

;=== PHost Extended Missions ===
; comment these out to save memory
; `Cloak', `Special' and `Beam up Multiple' have `c=False'. This
; prevents you from setting those missions manually; you shouldn't need
; to do it. `Cloak' and `Special' can be set the normal way, `Beam
; up Multiple' can be set using the normal cargo transfer screen.
; To set them anyway, use the MIT interface and enter the mission
; number by hand. And don't forget to tell me why you needed to do this
; so I can include that rule in PCC.
20,,Build Torpedoes from Cargo
                s = B-Torps
                c = Torp AND Cfg("AllowExtendedMissions")
21,*y#,Lay Minefield
                t = "Lay " # Z(Mission.Intercept) # " mines"
                c = Torp AND Cfg("AllowExtendedMissions")
                i = Torps
                j = Player
                w = Torp.Count
22,*y#+7,Lay Web Mines
                t = "Lay " # Z(Mission.Intercept) # " web mines"
                c = Torp AND Cfg("AllowExtendedMissions")
                i = Torps
                j = Player
                w = Torp.Count
23,*#,Scoop Torpedoes
                t = "Scoop " & Mission.Intercept & " torps" & (" from field #" # Z(Mission.Tow))
                s = Scoop
                c = Torp AND Beam AND Cfg("AllowExtendedMissions")
                i = Torps
                j = Mine Id
24,*,Gather-Build Torpedoes
                t = "Gather-Build " # Z(Mission.Intercept) # " torps"
                s = GB-Torp
                c = Torp AND Cfg("AllowExtendedMissions")
25,*,Beam Down Credits
                t = "Beam down " # Z(Mission.Intercept) # " mc"
                s = BD mc
                c = Cfg("AllowExtendedMissions")
26,#!h*,Transfer Torpedoes
                t = "Transfer " # Mission.Tow # " torps to " # ShipNameAndId(Mission.Intercept)
                s = XferTor
                c = Torp AND Cfg("AllowExtendedMissions")
                i = Target
                j = Amount
27,#!h*,Transfer Fighters
                t = "Transfer " # Mission.Tow # " fighters to " # ShipNameAndId(Mission.Intercept)
                s = XferFtr
                c = Fighter.Bays AND Cfg("AllowExtendedMissions")
                i = Target
                j = Amount
28,#!h*,Transfer Money
                t = "Transfer " # Mission.Tow # " mc to " # ShipNameAndId(Mission.Intercept)
                s = Xfer mc
                c = Cfg("AllowExtendedMissions")
                i = Target
                j = Amount
29,+3,Standard Super Spy
                s = Std Spy
                c = Cfg("AllowExtendedMissions")
30,,Cloak
                c = False
                w = True
31,,Special
                c = False
                w = True
32,*+9ab,Gather-Build Fighters
                t = "Gather-Build " # Z(Mission.Intercept) # " fighters"
                s = GB-Ftrs
                c = Fighter.Bays AND Cfg("AllowExtendedMissions")
33,*,Beam Up Credits
                t = "Beam up " # Z(Mission.Intercept) # " mc"
                s = BU mc
                c = Cfg("AllowExtendedMissions")
34,*,Beam Up Clans
                t = "Beam up " # Z(Mission.Intercept) # " clans"
                s = BU Clan
                c = Cfg("AllowBeamUpClans") AND Cfg("AllowExtendedMissions")
35,,Beam Up Multiple
                s = BU Many
                c = False
                w = True
36,*#,Add Mines to Field
                t = "Add " & If(Mission.Intercept, Mission.Intercept, "all") & " torps" & (" to minefield #" # Mission.Tow)
                s = Add Min
                c = Torp AND Cfg("AllowExtendedMissions")
                i = Amount
                j = Mine Id
37,*#+7,Add Web Mines to Field
                t = "Add " & If(Mission.Intercept, Mission.Intercept, "all") & " torps to web #" # Mission.Tow
                s = Add Web
                c = Torp AND Cfg("AllowExtendedMissions")
                i = Amount
                j = Mine Id
38,*,Training
                t = "Training for " # Z(Mission.Intercept) # " supplies"
                i = Supplies
                c = System.Host$=2 AND Orbit AND System.HostVersion>=400000 AND Cfg("NumExperienceLevels") AND Cfg("AllowExtendedMissions")
                w = Speed$=0
                s = Train
39,#!h*,Exchange Crew
                t = "Exchange " # If(Mission.Tow, Mission.Tow, "all") # " crew with " # ShipNameAndId(Mission.Intercept)
                i = Target
                j = Crew
                c = System.Host$=2 AND System.HostVersion>=400009 AND Cfg("AllowExtendedMissions") AND HasFunction("Academy")
                s = ExcCrew
40,!h*,Repair Ship
                t = "Repair " # ShipNameAndId(Mission.Intercept)
                i = Ship
                c = System.Host$=2 AND System.HostVersion>=400009 AND Cfg("AllowExtendedMissions") AND HasFunction("Repairs")
                s = RepShip
