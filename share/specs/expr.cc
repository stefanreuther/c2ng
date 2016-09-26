;=============================================================================
; This file contains "ready-made" expressions and other drop-down list
; items for PCC. The format of each line is:
;    text   [options] expression
; * at least 2 spaces (no tabs!) between text and rest
; * options in brackets, only used for search:
;    [s] ships
;    [p] planets
;    [b] bases
;    [1-3] severity of error: 1=inconvenience, 2=omission, 3=error
;   Combinations (e.g, [sp]) are allowed
; * New sections start with [Section], only those sections defined here
;   exist to time.
; The text may not be longer than 30 characters
; No line may exceed 255 characters
;=============================================================================

[ShipLabels]
Name                    Name
Id Number               Id
Friendly Code           FCode
Cargo                   (Z(Cargo.N) # "N ") & (Z(Cargo.T) # "T ") & (Z(Cargo.D) # "D ") & (Z(Cargo.M) # "M ") & (Z(Cargo.Supplies) # "Sp ") & (Z(Cargo.Colonists) # "Col ") & (Z(Cargo.Money) # "mc")
Cargo: Fuel             Cargo.N
Cargo: Tritanium        Cargo.T
Cargo: Duranium         Cargo.D
Cargo: Molybdenum       Cargo.M
Cargo: Supplies         Cargo.Supplies
Cargo: Colonists        Cargo.Colonists
Cargo: Money            Cargo.Money
Waypoint                Waypoint # "(" # Z(Waypoint.Dist) # " ly)"
Hull Type               Hull.Short
Owner                   If(My.Race$=Owner$,"",Owner.Adj)
Weaponry                (Beam.Count # "x" # Beam.Short) & " " & (Aux.Count # "x" # Aux.Short)
Need be repaired        If(Damage=0 AND Crew=Crew.Normal, "", "YES")
Your Comment            Left("|", Comment)
Editor's Choice         If(Owner$=My.Race$, If(Comment,"*",""), Type.Short & Owner$)

[PlanetLabels]
Name                    Name
Name (own planets)      If(Owner$=My.Race$, Name, "")
Id Number               Id
Id Number (own planets)  If(Owner$=My.Race$, Id, "")
Friendly Code           FCode
Minerals: mined         (Z(Mined.N) # "N ") & (Z(Mined.T) # "T ") &(Z(Mined.D) # "D ") &(Z(Mined.M) # "M ")
Minerals: ground        (Z(Ground.N) # "N ") & (Z(Ground.T) # "T ") &(Z(Ground.D) # "D ") &(Z(Ground.M) # "M ")
Minerals: density       (Z(Density.N) # "N ") & (Z(Density.T) # "T ") &(Z(Density.D) # "D ") &(Z(Density.M) # "M ")
Minerals: Neutronium    Mined.N # " (" # Ground.N # "x" # Density.N # "%)"
Minerals: Tritanium     Mined.T # " (" # Ground.T # "x" # Density.T # "%)"
Minerals: Duranium      Mined.D # " (" # Ground.D # "x" # Density.D # "%)"
Minerals: Molybdenum    Mined.M # " (" # Ground.M # "x" # Density.M # "%)"
Money                   Money
Supplies                Supplies
Buildings               (Z(Mines) # "M ") & (Z(Factories) # "F ") & (Z(Defense) # "D ") & (Z(Defense.Base) # "SBD")
Colonists               Z(Colonists) # " " # Owner.Adj
Colonists: Happiness    Colonists.Happy$
Colonists: Tax          Colonists.Tax
Natives                 Z(Natives) # " " # Natives.Race
Natives: Happiness      If(Natives,Natives.Happy$,"")
Natives: Government     Natives.Gov
Natives: Tax            Natives.Tax
Starbase: Tech Levels   If(Base.YesNo,Tech.Hull & "H " & Tech.Engine & "E " & Tech.Beam & "B " & Tech.Torpedo & "T","")
Temperature             Temp$
Ships in orbit          ("own:" # Z(Orbit.Own)) & (" enemy:" # Z(Orbit.Enemy))
Your Comment            Left("|", Comment)
Editor's Choice         First("|", If(Comment, Comment, If(Natives, "*", "")))

[Find]
Ships: Need be repaired        [s] Damage<>0 OR Crew<>Crew.Normal
Ships: Out of fuel             [s3] Cargo.N=0
Ships: No waypoint set         [s1] Waypoint.Dist=0
Ships: No mission set          [s1] Mission$=0
Ships: Waypoint but no speed   [s3] Waypoint.Dist<>0 AND Speed$=0
Ships: Won't reach waypoint    [s3] Move.Fuel>Cargo.N
Ships: Waypoint in deep space  [s] Not PlanetAt(Waypoint.X, Waypoint.Y, 1)
Ships: Multi-turn Movement     [s] Move.Eta > 1
Ships: Overdriven              [s2] Speed$>Engine$
Ships: Capital Ships           [s] Type.Short<>"F"
Ships: Freighters              [s] Type.Short="F"
Ships: New Ships               [s1] Name = RTrim(Left(Hull & " " & Id, 20)) And Owner$=My.Race$
Ships: Affected by Warp Wells  [s] PlanetAt(Waypoint.X,Waypoint.Y) <> PlanetAt(Waypoint.X,Waypoint.Y,1)
Ships: Unloading to planet     [s] Transfer.Unload AND Transfer.Unload.ID<>0
Ships: Transfer to enemy ship  [s] Transfer.Ship
Ships: Tow/Intercept complete  [s2] (Waypoint.Dist=0) AND ((Mission$=7) OR (Mission$=8))
Ships: Unneeded Minelaying FC  [s2] ((Mission$<>3) AND (Left(FCode,2)='md' OR Left(FCode,2)='mi'))
Planets: Unhappy               [p3] Owner$=My.Race$ AND (Colonists.Happy$<70 OR If(Natives,Natives.Happy$<70,FALSE))
Planets: Not 100% happy        [p] Owner$=My.Race$ AND (Colonists.Happy$<100 OR If(Natives,Natives.Happy$<100,FALSE))
Planets: Colonists overtaxed   [p] Owner$=My.Race$ AND (Colonists.Happy$ + Colonists.Change$ <= 40) AND (Colonists.Tax > 0)
Planets: Natives overtaxed     [p] Owner$=My.Race$ AND (Natives.Happy$ + Natives.Change$ <= 40) AND (Natives.Tax > 0)
Planets: Can still build       [p] M:=Money+Supplies; Supplies>0 AND (Defense<Defense.Max AND M>=11 OR Factories<Factories.Max AND M>=4 OR Mines<Mines.Max AND M>=5)
Planets: Can build a starbase  [p1] (Owner$=My.Race$) AND (NOT Base.YesNo) AND (Money+Supplies>=900) AND (Mined.T>=402) AND (Mined.D>=120) AND (Mined.M>=340)
Planets: Auto-Build wanted     [p] If(Defense.Want<1000, Defense<Defense.Want, Defense<Defense.Max) OR If(Mines.Want<1000, Mines<Mines.Want, Mines<Mines.Max) OR If(Factories.Want<1000, Factories<Factories.Want, Factories<Factories.Max)
Planets: Enemy ships in orbit  [p2] Orbit.Enemy>0
Planets: Tech 10 natives       [p] Natives.Race$=1 OR Natives.Race$>=7
Planets: Orphaned              [p3] Owner$=My.Race$ AND Colonists=0
Bases: Not building a ship     [b1] IsEmpty(Build) And IsEmpty(FindShip(Orbit$=Planet.Id And FCode="cln"))
Bases: Need be repaired        [b3] Damage<>0
Bases: Fix/Recycle             [b] Shipyard
Bases: Unneeded priority FC    [b2] IsEmpty(Build) AND FCode>="PB0" AND FCode<="PB9"
Mines: Very old information    LastScan < Turn-10
Silly Friendly Code            [1] F:=FCode; F=String(3,Left(F,1)) OR (F="HYP" AND Waypoint.Dist<21)
Enemy objects                  Owner$<>My.Race$ AND Owner$>0
Has Comment                    Comment
Has Auto Task                  Task Or Task.Base

[Devices]
Standard output file           REPORT.TXT
1st parallel printer           LPT1
2nd parallel printer           LPT2
1st serial printer             COM1
2nd serial printer             COM2

; If these names don't contain pathnames, they are searched in
; the game directory first, then the PCC main directory.
[Logfiles]
Host Log                       HOST.LOG
Host Score Log                 SCORE.LOG
Tons Log                       TONS.LOG
Registration Log               REG.LOG
Host Configuration File        CONFIG.TXT
PHost Configuration File       PCONFIG.SRC
PHost Turn Statistics          TURNSTAT.LOG
VGAScore Log                   VGASCORE.BLT
PTScore Log                    PTSCORE.LOG
PCC: Standard Report File      REPORT.TXT
PCC: Main Documentation        CC.DOC
PCC: Using DPMI                DPMI.DOC
PCC: Scripting Manual          CCSCRIPT.DOC
PCC: Command line utilities    CCUTILS.DOC
PCC: Printer Definition Doc    PRINTER.DOC
PCC: Report Editor Doc         REPEDIT.DOC
