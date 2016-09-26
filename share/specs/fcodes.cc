;=============================================================================
; Friendly code list for PCC
; Each line: xxx,yy,zzzzz
; * xxx = FCode
; * yy = Flags
;   s - ship, p - planet, b - base
;   r - registered only
;   a - alchemy ship only, c - capital ship only
;   u - don't consider this a special friendly code
;   -a - all but race a (a=1..b)
;   +a - only race a
; * zzzz = description, can contain %x (short race name x), %-x
;   (adjective x), up to 70 chars when expanded
; The list should be sorted alphabetically. It appears in exactly this
; order in PCC's selection list. As you see, you may write comments by
; starting a line with a semicolon.
;
; When checking whether a friendly code is special, PCC compares FCodes
; to this file *case-insensitive*. THost matches some fcodes in a case-
; insensitive manner, others not, and there's no official statement
; about this. Better safe than sorry.
;
; PCC also considers all codes from XTRFCODE.TXT special, but doesn't
; show them in the list.
;=============================================================================

AAA,us+1,New %-1 Ship
ald,sra,Alchemy Duranium only
alm,sra,Alchemy Molybdenum only
alt,sra,Alchemy Tritanium only
anc,s+6,(VPHOST) Assimilate native clans on ship
ATT,p,Attack ships in orbit that have fuel
BBB,us+2,New %-2 Ship
bdm,s,Beam down money
btf,sc,Transfer fighters to ships at this location
btm,s,Transfer money to ships at this location
btt,sc,Transfer torpedoes to ships at this location
bum,p,Beam up money
CCC,us+3,New %-3 Ship
cln,sr-57,Clone ship when at starbase
con,p,Get the HConfig settings
crc,s+9,(VPHOST) Construct Robot clans on ship
DDD,us+4,New %-4 Ship
dmp,br,Recycle parts not used for ship building ("Dump")
ee1,s-1,Cancel alliance with %1
ee2,s-2,Cancel alliance with %2
ee3,s-3,Cancel alliance with %3
ee4,s-4,Cancel alliance with %4
ee5,s-5,Cancel alliance with %5
ee6,s-6,Cancel alliance with %6
ee7,s-7,Cancel alliance with %7
ee8,s-8,Cancel alliance with %8
ee9,s-9,Cancel alliance with %9
eea,s-a,Cancel alliance with %A
eeb,s-b,Cancel alliance with %B
EEE,us+5,New %-5 Ship
ff1,s-1,Offer/accept alliance with %1
ff2,s-2,Offer/accept alliance with %2
ff3,s-3,Offer/accept alliance with %3
ff4,s-4,Offer/accept alliance with %4
ff5,s-5,Offer/accept alliance with %5
ff6,s-6,Offer/accept alliance with %6
ff7,s-7,Offer/accept alliance with %7
ff8,s-8,Offer/accept alliance with %8
ff9,s-9,Offer/accept alliance with %9
ffa,s-a,Offer/accept alliance with %A
ffb,s-b,Offer/accept alliance with %B
FFF,us+6,New %-6 Ship
GGG,us+7,New %-7 Ship
gs1,s-1,Give ship to %1
gs2,s-2,Give ship to %2
gs3,s-3,Give ship to %3
gs4,s-4,Give ship to %4
gs5,s-5,Give ship to %5
gs6,s-6,Give ship to %6
gs7,s-7,Give ship to %7
gs8,s-8,Give ship to %8
gs9,s-9,Give ship to %9
gsa,s-a,Give ship to %A
gsb,s-b,Give ship to %B
HHH,us+8,New %-8 Ship
HYP,s,Activate Hyperdrive
III,us+9,New %-9 Ship
JJJ,us+a,New %-A Ship
KKK,us+b,New %-B Ship
lfm,sc+9ab,Load minerals for fighters and build them
LFM,sc+9ab,Build fighters from minerals in cargo space
md0,src,Lay 100 torpedoes as mines
md1,src,Lay 10 torpedoes as mines
md2,src,Lay 20 torpedoes as mines
md3,src,Lay 30 torpedoes as mines
md4,src,Lay 40 torpedoes as mines
md5,src,Lay 50 torpedoes as mines
md6,src,Lay 60 torpedoes as mines
md7,src,Lay 70 torpedoes as mines
md8,src,Lay 80 torpedoes as mines
md9,src,Lay 90 torpedoes as mines
mdh,src,Lay half of your torpedoes as mines
mdq,src,Lay a quarter of your torpedoes as mines
mi1,src,Lay a %-1 mine field
mi2,src,Lay a %-2 mine field
mi3,src,Lay a %-3 mine field
mi4,src,Lay a %-4 mine field
mi5,src,Lay a %-5 mine field
mi6,src,Lay a %-6 mine field
mi7,src,Lay a %-7 mine field
mi8,src,Lay a %-8 mine field
mi9,src,Lay a %-9 mine field
mia,src,Lay a %-A mine field
mib,src,Lay a %-B mine field
mkt,src,Build torpedoes from minerals in cargo
msc,src,Collect mines when sweeping, and build torps ("Mine Scoop")
NAL,sra,No alchemy, to transport supplies
nat,sra,PHost only: Alchemy Dur + Mol only
nad,sra,PHost only: Alchemy Tri + Mol only
nam,sra,PHost only: Alchemy Tri + Dur only
nbr,s+57,Don't use Tow Capture mission
noc,p,Do not send HConfig messages
NTP,sc,Use neither fighters nor torps in fight
NUK,p,Attack all ships in orbit, whether they have fuel or not ("Nuke")
PB1,b,Priority Build, highest priority
PB2,b,Priority Build, 2nd
PB3,b,Priority Build, 3rd
PB4,b,Priority Build, 4th
PB5,b,Priority Build, 5th
PB6,b,Priority Build, 6th
PB7,b,Priority Build, 7th
PB8,b,Priority Build, 8th
PB9,b,Priority Build, 9th
pop,s,Explode Glory Device after movement
trg,s,Explode Glory Device when detecting a cloaked enemy ("Trigger")
WRS,s,PHost only: Wormhole Scan
WRT,s,PHost only: Wormhole Travel
???,usp,*DO NOT USE* - PCC's fallback-friendly code
