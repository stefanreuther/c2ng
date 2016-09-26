; Definition of the hull functions          -*- ccspec -*-
;
;  NUM,FLAGS,NAME
;    - NUM: numeric code
;    - FLAGS: ignored; not yet specified. Used here to specify the stereotype
;      of the function:
;      . a = alchemy
;      . c = cloak
;      . d = glory device
;      . g = gravitonic
;      . h = hyperjump
;      . i = imperial assault
;      . l = anti-cloak or related
;      . r = ramscoop
;      . s = scanner improvement
;      . t = terraform
;      . u = chunnel
;    - NAME: name of device, as in hullfunc.txt
;
;  Assignments:
;    d = TEXT          description. Should be self-contained, it's the only
;                      thing we show the user. Defaults to NAME.
;    e = TEXT          explanation. Can appear several times for multiline
;                      explanation.
;    s = A,B,C         hulls which have it by default
;    i = NUM           this device implies another one. That is, a ship having
;                      this device and the other one will effectively do just
;                      this one, and the other one can be hidden from display.
;
;  The descriptions and explanations correspond to PCC 1.1.15.2, the feature
;  set corresponds to PHost 4.1a.
;
;  TODO:
;    We need a way to express default assignments of former racial abilities,
;    which currently need to be hardcoded (PlanetImmunity, Boarding,
;    FullWeaponry: abilities that depend on a config setting; Tow: ability
;    that depends on config setting and engine count).

0,a,Alchemy
        d = Alchemy: 3 sup -> 1 T/D/M
        e = FCode "NAL": disable function
        e = FCode "alX": make only X
        e = FCode "naX" (PHost): do not make X
        d[de] = Alchemie: 3 Vor -> 1 T/D/M
        s = 105
1,a,Refinery
        d = Alchemy: 1 sup + 1 T/D/M -> 1 N
        e = FCode "NAL": disable function
        e = FCode "alX" (PHost 4.0k+): consume only X
        e = FCode "naX" (PHost 4.0k+): do not consume X
        d[de] = Alchemie: 3 Vor -> 1 T/D/M
        s = 104
2,a,AdvancedRefinery
        d = Alchemy: 1 T/D/M -> 1 N
        e = FCode "NAL" (PHost): disable function
        e = FCode "alX" (PHost 4.0k+): consume only X
        e = FCode "naX" (PHost 4.0k+): do not consume X
        d[de] = Alchemie: 1 T/D/M -> 1 N
        s = 97
        i = 1
3,t,HeatsTo50
        d = Terraforming: heats to 50F
        e = If planet is colder than 50°F (most races' optimum temperature), increases its temperature. Happens after movement.
        d[de] = Terraformen: heizen auf 50F
        s = 3
4,t,CoolsTo50
        d = Terraforming: cools to 50F
        e = If planet is warmer than 50°F (most races' optimum temperature), decreases its temperature. Happens after movement.
        d[de] = Terraformen: kühlen auf 50F
        s = 8
5,t,HeatsTo100
        d = Terraforming: heats to 100F
        e = If planet is colder than 100°F (Crystalline optimum temperature), increases its temperature. Happens after movement.
        d[de] = Terraformen: heizen auf 100F
        s = 64
6,h,Hyperdrive
        d = Hyperdrive
        e = Ship can jump 340 .. 360 ly in one turn (if waypoint not in that range, jumps 350 ly). One jump needs 50 kt fuel.
        e = FCode "HYP": initiate jump
        d[de] = Hyperantrieb
        s = 51,77,87
7,g,Gravitonic
        d = Gravitonic Accelerators
        e = Ship moves twice as far each turn (162 ly at warp 9).
        d[de] = Gravitonen-Beschleuniger
        s = 44,45,46
8,s,ScansAllWormholes
        d = Scans all wormholes in range
        e = Unlike most ships, this one can find multiple wormholes per turn.
        d[de] = Findet alle Wurmlöcher in Reichweite
        s = 3
9,,Gambling
        d = Gambling
        e = Each colonist clan aboard this ship generates 1 mc per turn.
        d[de] = Spielkasino
        s = 42
10,l,AntiCloak
        d = Anti-Cloak
        e = De-cloaks all ships within 10 ly. Excess damage prevents function.
        d[de] = Enttarnung
        s = 7
11,i,ImperialAssault
        d = Imperial Assault
        e = Ship can drop storm troops on a planet to capture it.
        e = Unload at least 10 clans to a planet to do Imperial Assault. Only works if ship is undamaged.
        s = 69
12,u,Chunneling
        d = Chunneling
        e = Can initiate a jump to another Chunnel ship by setting its FCode to the Id of the other ship. It will take other ships with it.
        s = 56
13,r,Ramscoop
        d = Ramscoop (gathers fuel)
        e = Makes fuel while moving. Amount of fuel produced is proportional to distance moved.
        d[de] = Ramscoop (Treibstoff generieren)
        s = 96
14,s,FullBioscan
        d = Advanced Bioscanner
        d[de] = Verbesserter Bioscanner
        e = Detects presence or absence of native life on planets with less than 20 defense posts.
        e = This bioscanner detects all planets in range.
        s = 84
        i = 17    ;Bioscan
15,c,AdvancedCloak
        d = Advanced Cloaking Device
        e = Ship can cloak and will not be seen by others. Advanced cloaking does not burn fuel.
        d[de] = Verbesserte Tarnvorrichtung
        s = 29,31
        i = 16
16,c,Cloak
        d = Cloaking Device
        e = Ship can cloak and will not be seen by others. Cloaking burns fuel each turn.
        d[de] = Tarnvorrichtung
        s = 21,22,25,26,27,28,32,33,36,38,43,44,45,46,47
17,s,Bioscan
        d = Bioscanner
        e = Detects presence or absence of native life on planets with less than 20 defense posts.
        e = This bioscanner can only find 20% of all planets in range.
        s = 9,96
18,d,GloryDeviceLowDamage
        d = Glory Device (low damage to own units)
        e = Ship can be told to self-destroy. This will do the equivalent of one minehit damage to other ships at the same position, 10% of that to your own ships.
        e = FCode "pop": blow up this turn after movement, "trg" if a cloaked ship matching your PE is detected
        d[de] = Glory Device (wenig Schaden an eigenen Einheiten)
        s = 41
        i = 19  ;GloryDeviceHighDamage
19,d,GloryDeviceHighDamage
        d = Glory Device (high damage to own units)
        e = Ship can be told to self-destroy. This will do the equivalent of one minehit damage to other ships at the same position, 20% of that to your own ships.
        e = FCode "pop": blow up this turn after movement, "trg" if a cloaked ship matching your PE is detected
        d[de] = Glory Device (viel Schaden an eigenen Einheiten)
        s = 39
20,,Unclonable
        d = Unclonable
        e = Ship cannot be cloned using "cln".
        d[de] = Nicht kopierbar
        i = 21  ;CloneOnce
21,,CloneOnce
        d = Clonable once
        e = Ship can be cloned once using "cln". After that, original and copy will be unclonable.
        d[de] = Einmal kopierbar
22,,Ungiveable
        d = Ungiveable
        e = Ship cannot be given away using "gsX", "give ship" or "Force Surrender".
        i = 23  ;GiveOnce
23,,GiveOnce
        d = Give-once
        e = Ship can be given away once using "gsX", "give ship" or "Force Surrender". Afterwards, it will be ungiveable.
24,,Level2Tow
        d = Level 2 Tractor Beam
        e = Ship has a stronger tractor beam than others.
        d[de] = Traktorstrahl Stufe 2
        i = 25  ;Tow
25,,Tow
        d = Tractor Beam
        e = Ship can tow other ships using the "Tow" mission.
        d[de] = Traktorstrahl
26,u,ChunnelSelf
        d = Chunnel itself
        e = Can initiate a jump to another Chunnel ship by setting its FCode to the Id of the other ship. It will jump through the chunnel alone.
        d[de] = Chunnel allein
27,u,ChunnelOthers
        d = Chunnel other ships
        e = Can initiate a jump to another Chunnel ship by setting its FCode to the Id of the other ship. It will move other ships through the chunnel, but stay here.
        d[de] = Chunnel andere Schiffe
28,u,ChunnelTarget
        d = Chunnel target
        e = Ship can act as a target to a chunnel initiator. It cannot initiate a chunnel (unless it has one of the other chunnel abilities).
        d[de] = Chunnel-Ziel
29,,PlanetImmunity
        d = Immune against planet attacks
        e = Ship will not be attacked by planets doing "ATT" or "NUK".
        d[de] = Immun gegen Angriffe von Planeten
        s = 69  ;SSD
30,t,OreCondenser
        d = Terraforming: ore condenser
        e = Ship will increase ore densities on planet up to 50%. Happens after movement.
        d[de] = Terraformen: Erze verdichten
31,,Boarding
        d = Boarding ships
        e = Ship can board other ships by locking a tow beam on them.
        d[de] = Schiffe entern
32,l,AntiCloakImmunity
        d = Anti-Cloak Immunity
        d[de] = Immun gegen Enttarnung
        e = Ship will not be de-cloaked by Tachyon fields.
33,,Academy
        d = Crew Academy
        d[de] = Crew-Akademie
        e = Experience training yields 4x normal gain. Ship can exchange crew with others using the "Exchange Crew" mission.
34,,Repairs
        d = Can repair other ships
        d[de] = Kann andere Schiffe reparieren
        e = Ship can repair complete damage of a target ship each turn using the "Repair Ship" mission.
35,,FullWeaponry
        d = Full weaponry even if damaged
        d[de] = Voll bewaffnet auch wenn beschädigt
        e = Ship can use all its weapons even if it's damaged.
36,,HardenedEngines
        d = Hardened engines
        d[de] = Gepanzerte Triebwerke
        e = Ship does not need to slow down when it gets damaged.
37,,Commander
        d = Commander
        e = Improves other ships' combat experience level by one.
38,,IonShield
        d = Ion shield
        d[de] = Ionen-Schild
        e = Protects ship from being damaged in an ion storm.
39,,HardenedCloak
        d = Hardened cloaking device
        d[de] = Gepanzerte Tarnvorrichtung
        e = Ship does not need to decloak when it gets damaged.
        i = 16   ;Cloak
40,l,AdvancedAntiCloak
        d = Advanced de-cloaking Tachyon field
        d[de] = Verbesserte Enttarnung
        e = De-cloaks all ships within 10 ly, including those that are immune against regular anti-cloak. Excess damage prevents function.
        i = 10   ;AntiCloak
