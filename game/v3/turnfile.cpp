/**
  *  \file game/v3/turnfile.cpp
  *  \brief Class game::v3::TurnFile
  */

#include <cstring>
#include <algorithm>
#include "game/v3/turnfile.hpp"
#include "afl/base/countof.hpp"
#include "afl/bits/int16le.hpp"
#include "afl/bits/int32le.hpp"
#include "afl/bits/pack.hpp"
#include "afl/checksums/bytesum.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/except/filetooshortexception.hpp"
#include "game/v3/registrationkey.hpp"
#include "util/randomnumbergenerator.hpp"
#include "util/translation.hpp"
#include "afl/base/inlinememory.hpp"

namespace {
    const int CURRENT_VERSION = 1;

    /** Magic numbers. (indexed by turn number) */
    const uint32_t MAGIC_NUMBERS[] = {
        1585242373,
        458484639, 1702713875, 2131768570, 943874411, 1531045611,
        622829488, 660770929, 473301358, 1868910709, 439267666, 1259778247,
        187160419, 205520992, 1162432602, 2048525217, 663275107, 1945076761,
        1912495862, 372583676, 2110506768, 972564220, 1627953855, 1696231547,
        1825551059, 690525357, 1425805634, 1273009202, 1643106825, 1033503714,
        1773067018, 1444056607, 841306782, 1311137219, 472310692, 1658228604,
        214806212, 1638334074, 870981249, 1438230436, 1722981495, 383237037,
        1014208183, 1950729749, 1381216466, 1149684732, 1475271197, 990158844,
        659846975, 131158828, 1269952134, 1929873739, 149943298, 94038386,
        1639179540, 519578396, 649680371, 2139806121, 48126387, 1820750093,
        2002158429, 834011058, 127330762, 1341047341, 45011247, 1210785240,
        102394054, 1033444233, 1452787209, 1636216880, 2001004855, 196571844,
        768753436, 1715639759, 9036553, 550413001, 1195957868, 566073290,
        1386247611, 725117880, 637842515, 782679024, 614960412, 1259473924,
        710893647, 137748852, 808495109, 1174108532, 2141228605, 1298353301,
        1989952843, 607318838, 1868217839, 2046567417, 1297732528, 886928938,
        533473933, 667670866, 1241783877, 1634258231, 1529167548, 1048674755,
        108553737, 442206379, 1427828321, 178793040, 57025576, 1886069810,
        1452681265, 392872129, 1749094387, 1931946557, 610131601, 497923660,
        800378618, 833787008, 1047995126, 867114247, 108316439, 1889137816,
        1566927898, 1606954817, 2129997452, 176508207, 1504084876, 781656333,
        1575411145, 952282888, 1920012969, 725392878, 442033280, 2055008888,
        125996860, 648896510, 1271579722, 734745843, 457213090, 101154514,
        1253209494, 649313503, 665663012, 1284757233, 526008074, 1128559135,
        708376521, 1888247159, 637430572, 1297014774, 84473586, 1938406737,
        278055502, 2082329430, 784004382, 886858342, 487519681, 979889529,
        2118032563, 376523135, 2037399162, 494383465, 1744352698, 533745717,
        752066469, 1518627158, 347571084, 1270232880, 460005993, 1754379254,
        1431354806, 103810045, 676346171, 948969734, 1270441550, 562587328,
        305781542, 48494333, 263492952, 1020466270, 190108896, 1009887493,
        1263640424, 2136294797, 951195719, 1154885409, 533815976, 707619918,
        1293089160, 1565561820, 1424862457, 2024541688, 1849356050, 804648133,
        1041775421, 1752468846, 2051572786, 749910457, 1708669854, 1592915884,
        1123095599, 1460717743, 1948843781, 1082061162, 1152635918,
        1881839283, 760734026, 1910315568, 1258782923, 2051380841, 1725205147,
        585278536, 1106219491, 444629203, 1099824661, 734821072, 2025557656,
        657473172, 255537853, 291983710, 286553905, 42517818, 670349676,
        870581336, 1127381655, 1839475352, 632654867, 547547534, 1471914002,
        1512583684, 890892484, 1857789058, 1587065657, 709203658, 1447182906,
        950862839, 1854232374, 1589606089, 18301536, 700074959, 415606342,
        1405416566, 1289157530, 1227135268, 340764183, 419122630, 1884968096,
        326246210, 540566661, 853062096, 1975701318, 1492562570, 1963382636,
        1075710563, 758982437, 2060895641, 1152739182, 1371354866, 800770398,
        1598945131, 79563287, 694771023, 1704620086, 248109047, 95128540,
        1062172273, 810095152, 2013227291, 1998220334, 1498632230, 1836447618,
        217773428, 986641406, 603013591, 1230144401, 1075426659, 1746848829,
        817629711, 186988432, 1484074762, 843442591, 776096924, 1024866700,
        2027642148, 1049701698, 247896996, 387855251, 857506062, 165410039,
        1748384075, 1958279260, 1593211160, 1998805368, 1633675306,
        2048559498, 1569149953, 1404385053, 784606841, 1589733669, 373455454,
        909199500, 1312922206, 408034973, 997233876, 963117498, 742951874,
        10752697, 574771227, 794412355, 92609016, 392712605, 964282276,
        1732686549
    };

    const char TACCOM_MAGIC[] = "NCC1701AD9";
    const char V35_MAGIC[] = "VER3.5";

    /** Definition of a TRN command. */
    struct CommandDefinition {
        /** Type of command. */
        game::v3::TurnFile::CommandType type : 8;
        /** Size of associated data, if fixed. Does not include Id word.
            tcm_PlanetBuildBase and tcm_SendMessage are special. */
        uint8_t size;
        /** Associated position in DOS structure, if applicable. */
        uint8_t index;
        /** Name of command, null if undefined. */
        const char* name;
    };

    /** Definition of all TRN commands. */
    const CommandDefinition COMMAND_DEFINITIONS[] = {
        { game::v3::TurnFile::UndefinedCommand,  0,   0, 0 },                        // 00 -- undefined
        { game::v3::TurnFile::ShipCommand,       3,   4, "ShipChangeFc" },           // 01 -- FCode
        { game::v3::TurnFile::ShipCommand,       2,   7, "ShipChangeSpeed" },        // 02 -- speed
        { game::v3::TurnFile::ShipCommand,       4,   9, "ShipChangeWaypoint" },     // 03 -- waypoint
        { game::v3::TurnFile::ShipCommand,       2,  33, "ShipChangeMission" },      // 04 -- mission
        { game::v3::TurnFile::ShipCommand,       2,  35, "ShipChangePrimaryEnemy" }, // 05 -- PE
        { game::v3::TurnFile::ShipCommand,       2,  37, "ShipTowShip" },            // 06 -- Tow id
        { game::v3::TurnFile::ShipCommand,      20,  45, "ShipChangeName" },         // 07 -- Name
        { game::v3::TurnFile::ShipCommand,      14,  75, "ShipBeamDownCargo" },      // 08 -- unload
        { game::v3::TurnFile::ShipCommand,      14,  89, "ShipTransferCargo" },      // 09 -- transfer
        { game::v3::TurnFile::ShipCommand,       2, 103, "ShipIntercept" },          // 10 -- Intercept id
        { game::v3::TurnFile::ShipCommand,       2,  65, "ShipChangeNeutronium" },   // 11 -- Neutro
        { game::v3::TurnFile::ShipCommand,       2,  67, "ShipChangeTritanium" },    // 12 -- Trit
        { game::v3::TurnFile::ShipCommand,       2,  69, "ShipChangeDuranium" },     // 13 -- Dur
        { game::v3::TurnFile::ShipCommand,       2,  71, "ShipChangeMolybdenum" },   // 14 -- Moly
        { game::v3::TurnFile::ShipCommand,       2,  73, "ShipChangeSupplies" },     // 15 -- Sup
        { game::v3::TurnFile::ShipCommand,       2,  43, "ShipChangeColonists" },    // 16 -- Clans
        { game::v3::TurnFile::ShipCommand,       2,  29, "ShipChangeTorpedoes" },    // 17 -- T/F
        { game::v3::TurnFile::ShipCommand,       2, 105, "ShipChangeMoney" },        // 18 -- mc
        { game::v3::TurnFile::UndefinedCommand,  0,   0, 0 },                        // 19 -- undefined
        { game::v3::TurnFile::UndefinedCommand,  0,   0, 0 },                        // 20 -- undefined
        { game::v3::TurnFile::PlanetCommand,     3,   4, "PlanetChangeFc" },         // 21 -- FCode
        { game::v3::TurnFile::PlanetCommand,     2,   7, "PlanetChangeMines" },      // 22 -- Mines
        { game::v3::TurnFile::PlanetCommand,     2,   9, "PlanetChangeFactories" },  // 23 -- Factories
        { game::v3::TurnFile::PlanetCommand,     2,  11, "PlanetChangeDefense" },    // 24 -- Defense
        { game::v3::TurnFile::PlanetCommand,     4,  13, "PlanetChangeNeutronium" }, // 25 -- N
        { game::v3::TurnFile::PlanetCommand,     4,  17, "PlanetChangeTritanium" },  // 26 -- T
        { game::v3::TurnFile::PlanetCommand,     4,  21, "PlanetChangeDuranium" },   // 27 -- D
        { game::v3::TurnFile::PlanetCommand,     4,  25, "PlanetChangeMolybdenum" }, // 28 -- M
        { game::v3::TurnFile::PlanetCommand,     4,  29, "PlanetChangeColonists" },  // 29 -- Clans
        { game::v3::TurnFile::PlanetCommand,     4,  33, "PlanetChangeSupplies" },   // 30 -- Sup
        { game::v3::TurnFile::PlanetCommand,     4,  37, "PlanetChangeMoney" },      // 31 -- mc
        { game::v3::TurnFile::PlanetCommand,     2,  65, "PlanetColonistTax" },      // 32 -- ColTax
        { game::v3::TurnFile::PlanetCommand,     2,  67, "PlanetNativeTax" },        // 33 -- NatTax
        { game::v3::TurnFile::PlanetCommand,     0,  83, "PlanetBuildBase" },        // 34 -- build base
        { game::v3::TurnFile::UndefinedCommand,  0,   0, 0 },                        // 35 -- undefined
        { game::v3::TurnFile::UndefinedCommand,  0,   0, 0 },                        // 36 -- undefined
        { game::v3::TurnFile::UndefinedCommand,  0,   0, 0 },                        // 37 -- undefined
        { game::v3::TurnFile::UndefinedCommand,  0,   0, 0 },                        // 38 -- undefined
        { game::v3::TurnFile::UndefinedCommand,  0,   0, 0 },                        // 39 -- undefined
        { game::v3::TurnFile::BaseCommand,       2,   4, "BaseChangeDefense" },      // 40 -- Def
        { game::v3::TurnFile::BaseCommand,       2,   8, "BaseUpgradeEngineTech" },  // 41 -- Eng Tech
        { game::v3::TurnFile::BaseCommand,       2,  10, "BaseUpgradeHullTech" },    // 42 -- Hull Tech
        { game::v3::TurnFile::BaseCommand,       2,  12, "BaseUpgradeWeaponTech" },  // 43 -- Beam Tech
        { game::v3::TurnFile::BaseCommand,      18,  16, "BaseBuildEngines" },       // 44 -- Eng Storage
        { game::v3::TurnFile::BaseCommand,      40,  34, "BaseBuildHulls" },         // 45 -- Hull Storage
        { game::v3::TurnFile::BaseCommand,      20,  74, "BaseBuildWeapons" },       // 46 -- Beam Storage
        { game::v3::TurnFile::BaseCommand,      20,  94, "BaseBuildLaunchers" },     // 47 -- Launcher Storage
        { game::v3::TurnFile::BaseCommand,      20, 114, "BaseBuildTorpedoes" },     // 48 -- Torp Storage
        { game::v3::TurnFile::BaseCommand,       2, 134, "BaseBuildFighters" },      // 49 -- Ftr
        { game::v3::TurnFile::BaseCommand,       2, 136, "BaseFixRecycleShipId" },   // 50 -- Fix/Recycle Id
        { game::v3::TurnFile::BaseCommand,       2, 138, "BaseFixRecycleShip" },     // 51 -- Fix/Recycle
        { game::v3::TurnFile::BaseCommand,       2, 140, "BaseChangeMission" },      // 52 -- Mission
        { game::v3::TurnFile::BaseCommand,      14, 142, "BaseBuildShip" },          // 53 -- Build order
        { game::v3::TurnFile::BaseCommand,       2,  14, "BaseUpgradeTorpTech" },    // 54 -- Torp Tech
        { game::v3::TurnFile::UndefinedCommand,  0,   0, 0 },                        // 55 -- undefined
        { game::v3::TurnFile::UndefinedCommand,  0,   0, 0 },                        // 56 -- undefined
        { game::v3::TurnFile::UndefinedCommand,  0,   0, 0 },                        // 57 -- undefined
        { game::v3::TurnFile::UndefinedCommand,  0,   0, 0 },                        // 58 -- undefined
        { game::v3::TurnFile::UndefinedCommand,  0,   0, 0 },                        // 59 -- undefined
        { game::v3::TurnFile::OtherCommand,      0,   0, "SendMessage" },            // 60 -- Message
        { game::v3::TurnFile::OtherCommand,     10,   0, "ChangePassword" },         // 61 -- Password
        { game::v3::TurnFile::OtherCommand,      0,   0, "SendBack" },               // 62 -- SendBack
    };

    // FIXME: move to afl
    template<typename Desc>
    bool
    get(afl::base::ConstBytes_t data, size_t offset, typename Desc::Word_t& out)
    {
        data.split(offset);
        if (const typename Desc::Bytes_t* p = data.eatN<sizeof(typename Desc::Bytes_t)>()) {
            out = Desc::unpack(*p);
            return true;
        } else {
            return false;
        }
    }

    // FIXME: move to afl
    template<typename Desc>
    bool
    put(afl::base::Bytes_t data, size_t offset, typename Desc::Word_t in)
    {
        data.split(offset);
        if (typename Desc::Bytes_t* p = data.eatN<sizeof(typename Desc::Bytes_t)>()) {
            Desc::pack(*p, in);
            return true;
        } else {
            return false;
        }
    }

    // FIXME: move to afl
    void readVector(afl::io::Stream& in, afl::base::GrowableMemory<uint8_t>& bytes)
    {
        uint8_t page[4096];
        while (size_t n = in.read(page)) {
            bytes.append(afl::base::ConstBytes_t(page).trim(n));
        }
    }


// //! Turn Command Compare.
    class CommandComparator {
     public:
        CommandComparator(afl::base::ConstBytes_t turnData)
            : m_turnData(turnData)
            { }
        bool operator()(int32_t a, int32_t b);

     private:
        afl::base::ConstBytes_t m_turnData;
    };

// /** Returns true iff command at offset a precedes command at offset b.
//     Canonical command order is:
//     - for all ships, in sid order, ship commands in command code order;
//     - for all planets, in pid order, planet commands in command code order;
//     - for all bases, in bid order, base commands in command code order;
//     - messages (60);
//     - change password (61);
//     - sendback (62).

//     Note that this will sort undefined commands at the beginning. */
    bool CommandComparator::operator()(int32_t a, int32_t b)
    {
        using game::v3::TurnFile;
        using afl::bits::Int16LE;

        // Get command codes
        int16_t cca, ccb;
        if (!get<Int16LE>(m_turnData, a, cca) || !get<Int16LE>(m_turnData, b, ccb)) {
            return false;
        }

        // Get types
        TurnFile::CommandType typa = TurnFile::getCommandCodeType(cca);
        TurnFile::CommandType typb = TurnFile::getCommandCodeType(ccb);
        if (typa == typb) {
            // types are equal
            if (typa == TurnFile::OtherCommand) {
                return cca < ccb;
            } else {
                int16_t ida, idb;
                if (!get<Int16LE>(m_turnData, a+2, ida) || !get<Int16LE>(m_turnData, b+2, idb)) {
                    return false;
                } else if (ida == idb) {
                    return cca < ccb;
                } else {
                    return ida < idb;
                }
            }
        } else {
            return typa < typb;
        }
    }



}




// Create new turn file.
game::v3::TurnFile::TurnFile(afl::charset::Charset& charset, int player, Timestamp time)
    : m_charset(charset),
      m_turnHeader(),           // zero-initializes!
      m_taccomHeader(),
      m_dosTrailer(),
      m_windowsTrailer(),
      m_data(),
      m_offsets(),
      m_version(CURRENT_VERSION),
      m_features(WinplanFeature),
      m_turnPlacement(0),
      m_isDirty(false)
{
    // ex GTurnfile::GTurnfile
    m_turnHeader.playerId = int16_t(player);
    time.storeRawData(m_turnHeader.timestamp);
}

// Read turn file.
game::v3::TurnFile::TurnFile(afl::charset::Charset& charset, afl::io::Stream& str, bool fullParse)
    : m_charset(charset),
      m_turnHeader(),           // zero-initializes!
      m_taccomHeader(),
      m_dosTrailer(),
      m_windowsTrailer(),
      m_data(),
      m_offsets(),
      m_version(CURRENT_VERSION),
      m_features(),
      m_turnPlacement(0),
      m_isDirty(false)
{
    // ex GTurnfile::GTurnfile
    init(str, fullParse);
}

// Destructor.
game::v3::TurnFile::~TurnFile()
{ }

// Get player number.
int
game::v3::TurnFile::getPlayer() const
{
    return m_turnHeader.playerId;
}

// Get turn timestamp.
game::Timestamp
game::v3::TurnFile::getTimestamp() const
{
    return game::Timestamp(m_turnHeader.timestamp);
}

// Get number of commands stored in this turn.
size_t
game::v3::TurnFile::getNumCommands() const
{
    return m_offsets.size();
}

// Get feature flags.
game::v3::TurnFile::FeatureSet_t
game::v3::TurnFile::getFeatures() const
{
    return m_features;
}

// Get sub-version of turn file.
int
game::v3::TurnFile::getVersion() const
{
    return m_version;
}

// Set turn timestamp.
void
game::v3::TurnFile::setTimestamp(const Timestamp& time)
{
    time.storeRawData(m_turnHeader.timestamp);
    m_isDirty = true;
}

// Set sub-version of turn file.
void
game::v3::TurnFile::setVersion(int n)
{
    m_version = n;
    m_isDirty   = true;
}

// Set turn format.
void
game::v3::TurnFile::setFeatures(FeatureSet_t f)
{
    if (f != m_features) {
        m_isDirty = true;
        m_features = f;
        if (!m_features.contains(TaccomFeature)) {
            afl::base::fromObject(m_taccomHeader).fill(0);
        }
        if (!m_features.contains(WinplanFeature)) {
            afl::base::fromObject(m_windowsTrailer).fill(0);
        }
    }
}


/*
 *  Trailer access
 */

// Try to get the turn number used to generate this turn.
int
game::v3::TurnFile::tryGetTurnNr() const
{
    if (m_features.contains(WinplanFeature)) {
        const uint32_t checker = (m_windowsTrailer.vphKey[0] ^ m_windowsTrailer.vphKey[1]) & 0x7FFFFFFF;
        for (size_t i = 0; i < countof(MAGIC_NUMBERS); ++i) {
            if (MAGIC_NUMBERS[i] == checker) {
                if (i == 0) {
                    return int(countof(MAGIC_NUMBERS));
                } else {
                    return int(i);
                }
            }
        }
    }

    // when we're here, it's either Dosplan or an invalid number
    return 0;
}

// Set player secret (templock, playerlog).
void
game::v3::TurnFile::setPlayerSecret(const structures::TurnPlayerSecret& data)
{
    // ex GTurnFile::setTemplock
    m_dosTrailer.playerSecret = data;
}

// Set registration info.
void
game::v3::TurnFile::setRegistrationKey(const RegistrationKey& key, int turnNr)
{
    // The seed can be arbitrary, but should somehow depend on the TRN's content.
    // Let's use the same as in PCC 1.x (we don't produce identical turns, though).
    util::RandomNumberGenerator rng(m_turnHeader.playerId + (turnNr << 16));

    // Dosplan half (authoritative)
    afl::bits::packArray<afl::bits::UInt32LE>(afl::base::fromObject(m_dosTrailer.registrationKey), key.getKey());

    // Winplan half (mostly informative)
    if (m_features.contains(WinplanFeature)) {
        m_windowsTrailer.regstr3 = afl::string::toBytes(encodeString(key.getLine(RegistrationKey::Line3)));
        m_windowsTrailer.regstr4 = afl::string::toBytes(encodeString(key.getLine(RegistrationKey::Line4)));
        afl::base::Bytes_t(m_windowsTrailer.unused).fill(0);
        m_windowsTrailer.regstr1[0] = afl::string::toBytes(encodeString(key.getLine(RegistrationKey::Line1)));
        m_windowsTrailer.regstr2[0] = afl::string::toBytes(encodeString(key.getLine(RegistrationKey::Line2)));

        for (size_t i = 0; i < sizeof(m_windowsTrailer.regstr1[0]); ++i) {
            m_windowsTrailer.regstr1[0].m_bytes[i] ^= (m_windowsTrailer.regstr1[1].m_bytes[i] = uint8_t(rng(256)));
        }
        for (size_t i = 0; i < sizeof(m_windowsTrailer.regstr2[0]); ++i) {
            m_windowsTrailer.regstr2[0].m_bytes[i] ^= (m_windowsTrailer.regstr2[1].m_bytes[i] = uint8_t(rng(256)));
        }

        uint32_t randomNr = rng() << 16;
        randomNr |= rng();
        randomNr &= 0x7FFFFFFF;

        m_windowsTrailer.vphKey[0] = MAGIC_NUMBERS[turnNr % countof(MAGIC_NUMBERS)] ^ randomNr;
        m_windowsTrailer.vphKey[1] = randomNr;
    }
    m_isDirty = true;
}

// Get Windows (v3.5) trailer.
const game::v3::structures::TurnWindowsTrailer&
game::v3::TurnFile::getWindowsTrailer() const
{
    return m_windowsTrailer;
}

// Get DOS (v3.0) trailer.
const game::v3::structures::TurnDosTrailer&
game::v3::TurnFile::getDosTrailer() const
{
    return m_dosTrailer;
}

// Get turn header.
const game::v3::structures::TurnHeader&
game::v3::TurnFile::getTurnHeader() const
{
    return m_turnHeader;
}

// Get Taccom header.
const game::v3::structures::TaccomTurnHeader&
game::v3::TurnFile::getTaccomHeader() const
{
    return m_taccomHeader;
}



/*
 *  Command accessors
 */

// Get command code.
bool
game::v3::TurnFile::getCommandCode(size_t index, CommandCode_t& out) const
{
    if (uint32_t* p = m_offsets.at(index)) {
        int16_t result;
        if (get<afl::bits::Int16LE>(m_data, *p, result)) {
            out = CommandCode_t(result);
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

// Get length of command data field.
bool
game::v3::TurnFile::getCommandLength(size_t index, int& out) const
{
    CommandCode_t cmd = 0;
    if (!getCommandCode(index, cmd)) {
        // Command code not accessible --> length not known
        return false;
    } else if (cmd == tcm_SendMessage) {
        // Sender, Receiver, Text --> 4 bytes for sender/receiver, plus length (in Id slot)
        int id;
        if (getCommandId(index, id)) {
            out = id + 4;
            return true;
        } else {
            return false;
        }
    } else if (cmd == tcm_SendBack) {
        // Type, Size, Data --> 4 bytes, plus length
        int16_t size;
        const uint32_t* p = m_offsets.at(index);
        if (p == 0) {
            return false;
        } else if (get<afl::bits::Int16LE>(m_data, *p + 6, size)) {
            out = size + 4;
            return true;
        } else {
            return false;
        }
    } else if (cmd < countof(COMMAND_DEFINITIONS)) {
        // It's in our command definition list. We know its size if it's not UndefinedCommand.
        if (COMMAND_DEFINITIONS[cmd].type != UndefinedCommand) {
            out = COMMAND_DEFINITIONS[cmd].size;
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

// Get command Id field.
bool
game::v3::TurnFile::getCommandId(size_t index, int& out) const
{
    if (const uint32_t* p = m_offsets.at(index)) {
        int16_t result;
        if (!get<afl::bits::Int16LE>(m_data, *p + 2, result)) {
            return false;
        } else {
            out = result;
            return true;
        }
    } else {
        return false;
    }
}

// Get command type.
bool
game::v3::TurnFile::getCommandType(size_t index, CommandType& type) const
{
    CommandCode_t code;
    if (getCommandCode(index, code)) {
        type = getCommandCodeType(code);
        return true;
    } else {
        return false;
    }
}

// Get position of a command in the file.
bool
game::v3::TurnFile::getCommandPosition(size_t index, int32_t& out) const
{
    if (uint32_t* p = m_offsets.at(index)) {
        out = *p;
        return true;
    } else {
        return false;
    }
}

// Get name of a command.
const char*
game::v3::TurnFile::getCommandName(size_t index) const
{
    CommandCode_t code;
    if (!getCommandCode(index, code)) {
        return 0;
    } else {
        return getCommandCodeName(code);
    }
}

size_t
game::v3::TurnFile::findCommandRunLength(size_t index) const
{
    // ex game/load-trn.cc:findFirstCommandForNextUnit, sort-of
    size_t runLength = 0;
    CommandType startType;
    int startId;
    if (getCommandType(index, startType) && getCommandId(index, startId)) {
        CommandType nextType;
        int nextId;
        do {
            ++runLength;
        } while (getCommandType(index+runLength, nextType) && getCommandId(index+runLength, nextId) && nextType == startType && nextId == startId);
    }
    return runLength;
}


// Get command data.
afl::base::ConstBytes_t 
game::v3::TurnFile::getCommandData(size_t index) const
{
    if (uint32_t* p = m_offsets.at(index)) {
        return m_data.subrange(*p + 4);
    } else {
        return afl::base::ConstBytes_t();
    }
}

// Send message data (create tcm_SendMessage command).
void
game::v3::TurnFile::sendMessageData(int from, int to, afl::base::ConstBytes_t data)
{
    game::v3::structures::Int16_t header[2];
    header[0] = static_cast<int16_t>(from);
    header[1] = static_cast<int16_t>(to == 0 ? 12 : to);

    addCommand(tcm_SendMessage, static_cast<int16_t>(data.size()), afl::base::fromObject(header));
    addData(data);
}

// Send THost alliance commands.
void
game::v3::TurnFile::sendTHostAllies(const String_t& commandSequence, int shipId, const String_t& shipFC)
{
    afl::base::GrowableBytes_t encodedBuffer = m_charset.encode(afl::string::toMemory(commandSequence));
    afl::base::Bytes_t encoded = encodedBuffer;
    afl::base::InlineMemory<uint8_t,3> fcBuffer;
    while (encoded.size() > 0) {
        fcBuffer.fill(0);
        fcBuffer.copyFrom(encoded.split(3));
        addCommand(tcm_ShipChangeFc, shipId, fcBuffer);
    }

    fcBuffer.fill(0);
    fcBuffer.copyFrom(m_charset.encode(afl::string::toMemory(shipFC)));
    addCommand(tcm_ShipChangeFc, shipId, fcBuffer);
}



/*
 *  Command definition accessors
 */

// Get command type, given a command code.
game::v3::TurnFile::CommandType
game::v3::TurnFile::getCommandCodeType(CommandCode_t cmd)
{
    if (cmd < countof(COMMAND_DEFINITIONS)) {
        return COMMAND_DEFINITIONS[cmd].type;
    } else {
        return UndefinedCommand;
    }
}

// Get command name, given a command code.
const char*
game::v3::TurnFile::getCommandCodeName(CommandCode_t cmd)
{
    if (cmd < countof(COMMAND_DEFINITIONS)) {
        return COMMAND_DEFINITIONS[cmd].name;
    } else {
        return 0;
    }
}

// Get command record index, given a command code.
size_t
game::v3::TurnFile::getCommandCodeRecordIndex(CommandCode_t code)
{
    if (code < countof(COMMAND_DEFINITIONS)) {
        return COMMAND_DEFINITIONS[code].index;
    } else {
        return 0;
    }
}


/*
 *  Modificators
 */

// Add a command.
void
game::v3::TurnFile::addCommand(CommandCode_t cmd, int id)
{
    afl::bits::Value<afl::bits::Int16LE> buf[2];
    m_offsets.append(uint32_t(m_data.size()));
    buf[0] = int16_t(cmd);
    buf[1] = int16_t(id);
    addData(afl::base::fromObject(buf));         // marks turn dirty
}

// Add a command with data.
void
game::v3::TurnFile::addCommand(CommandCode_t cmd, int id, afl::base::ConstBytes_t data)
{
    addCommand(cmd, id);
    addData(data);       // marks turn dirty
}

// Add command data.
void
game::v3::TurnFile::addData(afl::base::ConstBytes_t data)
{
    m_isDirty = true;
    m_data.append(data);
}

// Delete command.
void
game::v3::TurnFile::deleteCommand(size_t index)
{
    // FIXME? this fails when commands are aliased.
    // We do not generate that, but others might, and we don't block it.
    // PHost wouldn't be able to read it, so it is probably not worth bothering with.
    // However, a more solid version just adds a null and points the command there.
    int32_t pos;
    if (getCommandPosition(index, pos)) {
        put<afl::bits::Int16LE>(m_data, pos, 0);
    }
    m_isDirty = true;
}

// /** Make commands for a ship.
//     \param id Ship Id
//     \param old Serialized old ship data (*.dis)
//     \param neu Serialized new ship data (*.dat) */
void
game::v3::TurnFile::makeShipCommands(int id, const structures::Ship& oldShip, const structures::Ship& newShip)
{
    // ex GTurnFile::makeShipCommands
    makeCommands(id, tcm_ShipFIRST, tcm_ShipLAST, afl::base::fromObject(oldShip), afl::base::fromObject(newShip));
}

// /** Make commands for a planet.
//     \param id Planet Id
//     \param old Serialized old planet data (*.dis)
//     \param neu Serialized new planet data (*.dat) */
void
game::v3::TurnFile::makePlanetCommands(int id, const structures::Planet& oldPlanet, const structures::Planet& newPlanet)
{
    // ex GTurnFile::makePlanetCommands
    makeCommands(id, tcm_PlanetFIRST, tcm_PlanetLAST, afl::base::fromObject(oldPlanet), afl::base::fromObject(newPlanet));
    if (int16_t(oldPlanet.buildBaseFlag) != int16_t(newPlanet.buildBaseFlag)) {
        addCommand(tcm_PlanetBuildBase, id);
    }
}

// /** Make commands for a starbase.
//     \param id Base Id
//     \param old Serialized old base data (*.dis)
//     \param neu Serialized new base data (*.dat) */
void
game::v3::TurnFile::makeBaseCommands(int id, const structures::Base& oldBase, const structures::Base& newBase)
{
    // ex GTurnFile::makeBaseCommands
    makeCommands(id, tcm_BaseFIRST, tcm_BaseLAST, afl::base::fromObject(oldBase), afl::base::fromObject(newBase));
}


/*
 *  Structure access
 */

// Sort commands.
void
game::v3::TurnFile::sortCommands()
{
    // We don't need to mark the turn dirty here, as swapping offsets does not change checksums.
    // Currently (20070910, 20130209, 20170114), un-trn relies on that.
    std::stable_sort(m_offsets.begin(), m_offsets.end(), CommandComparator(m_data));
}

// Update image.
void
game::v3::TurnFile::update()
{
    afl::base::GrowableMemory<uint8_t> newData;
    afl::base::GrowableMemory<uint32_t> newOffsets;

    // Remove disallowed TRN commands.
    // We don't know how to copy them.
    // Do that only when there actually are such commands.
    size_t fm, to = 0;
    for (fm = 0; fm < m_offsets.size(); ++fm) {
        CommandType type;
        if (getCommandType(fm, type) && type != UndefinedCommand) {
            *m_offsets.at(to++) = *m_offsets.at(fm);
        }
    }
    if (fm != to) {
        m_offsets.trim(to);
    }

    if (m_features.contains(TaccomFeature)) {
        // Write Taccom headers, dummy version */
        structures::TaccomTurnHeader newHeader = m_taccomHeader;
        newData.ensureSize(sizeof(newHeader));

        // Write Taccom files
        bool didTurn = false;
        for (size_t i = 0; i < structures::MAX_TRN_ATTACHMENTS; ++i) {
            if (m_turnPlacement == i) {
                newHeader.turnAddress = int32_t(newData.size() + 1);
                updateTurnFile(newData, newOffsets);
                newHeader.turnSize = int32_t(newData.size() + 1 - newHeader.turnAddress);
                didTurn = true;
            }

            if (!afl::base::ConstBytes_t(m_taccomHeader.attachments[i].name).empty()) {
                newHeader.attachments[i].address = int32_t(newData.size() + 1);
                newData.append(m_data.subrange(m_taccomHeader.attachments[i].address-1, m_taccomHeader.attachments[i].length));
            }
        }
        if (!didTurn) {
            newHeader.turnAddress = int32_t(newData.size() + 1);
            updateTurnFile(newData, newOffsets);
            newHeader.turnSize = int32_t(newData.size() + 1 - newHeader.turnAddress);
        }

        // Update header
        std::memcpy(newHeader.magic, TACCOM_MAGIC, 10);
        newData.copyFrom(afl::base::fromObject(newHeader));
        m_taccomHeader = newHeader;
    } else {
        updateTurnFile(newData, newOffsets);
    }

    newOffsets.swap(m_offsets);
    newData.swap(m_data);
    m_isDirty = false;
}

// Update trailer.
void
game::v3::TurnFile::updateTrailer()
{
//     ASSERT(!dirty);
    if (m_features.contains(TaccomFeature)) {
//         ASSERT(taccom_header.turn_size > dos_trailer.size);
        m_data.subrange(m_taccomHeader.turnAddress-1 + m_taccomHeader.turnSize - sizeof(m_dosTrailer)).copyFrom(afl::base::fromObject(m_dosTrailer));
    } else {
//         ASSERT(data.getSize() > dos_trailer.size);
        m_data.subrange(m_data.size() - sizeof(m_dosTrailer)).copyFrom(afl::base::fromObject(m_dosTrailer));
    }
}

// Compute turn checksum.
uint32_t
game::v3::TurnFile::computeTurnChecksum() const
{
//     /* This really ought to compute the checksum, not just return the
//        value from the header, to allow verification of turns. */
//     ASSERT(!dirty);

    afl::base::ConstBytes_t area;
    if (m_features.contains(TaccomFeature)) {
//         ASSERT(taccom_header.turn_size > dos_trailer.size);
        area = m_data.subrange(m_taccomHeader.turnAddress-1, m_taccomHeader.turnSize - sizeof(m_dosTrailer));
    } else {
//         ASSERT(data.getSize() > dos_trailer.size);
        area = m_data.subrange(0, m_data.size() - sizeof(m_dosTrailer));
    }

    uint32_t turnChecksum = afl::checksums::ByteSum().add(area, 0);
    return turnChecksum + 3*m_turnHeader.timeChecksum + 13;
}


/*
 *  Taccom access
 */

// Attach a file.
bool
game::v3::TurnFile::addFile(afl::base::ConstBytes_t fileData, const String_t& name, size_t& pos)
{
    for (size_t i = 0; i < structures::MAX_TRN_ATTACHMENTS; ++i) {
        if (afl::base::ConstBytes_t(m_taccomHeader.attachments[i].name).empty()) {
            // Found empty slot.
            m_taccomHeader.attachments[i].address = int32_t(m_data.size() + 1);
            m_taccomHeader.attachments[i].length = int32_t(fileData.size());
            m_taccomHeader.attachments[i].name = afl::string::toBytes(encodeString(name));
            m_data.append(fileData);
            m_features += TaccomFeature;
            m_isDirty = true;
            pos = i;
            return true;
        }
    }
    return false;
}

// Delete an attached file.
void
game::v3::TurnFile::deleteFile(const size_t index)
{
    if (index < structures::MAX_TRN_ATTACHMENTS) {
        m_isDirty = true;
        afl::base::fromObject(m_taccomHeader.attachments[index]).fill(0);
    }
}

// Get number of attachments.
size_t
game::v3::TurnFile::getNumFiles() const
{
    size_t n = 0;
    for (size_t i = 0; i < structures::MAX_TRN_ATTACHMENTS; ++i) {
        if (!afl::base::ConstBytes_t(m_taccomHeader.attachments[i].name).empty()) {
            ++n;
        }
    }
    return n;
}

// /** Get position of turn data in TacCom container. */
size_t
game::v3::TurnFile::getTaccomTurnPlace() const
{
    return m_turnPlacement;
}



/*
 *  Output
 */

// Write turn file.
void
game::v3::TurnFile::write(afl::io::Stream& stream) const
{
//     ASSERT(!dirty);
    stream.fullWrite(m_data);
}

// Get associated character set.
afl::charset::Charset&
game::v3::TurnFile::charset() const
{
    return m_charset;
}



/*
 *  Internal
 */

/** Initialize by loading a stream.
    The stream contains a full turn file (including possible Taccom data).
    \param str Stream to read
    \param fullParse true to read full turn, false to read only headers */
void
game::v3::TurnFile::init(afl::io::Stream& str, bool fullParse)
{
    // ex GTurnfile::init
    if (fullParse) {
        readVector(str, m_data);
        // FIXME: replace memcmp?
        if (m_data.size() > sizeof(m_taccomHeader) && std::memcmp(m_data.at(0), TACCOM_MAGIC, 10) == 0) {
            // Taccom-enhanced TRN
            afl::base::fromObject(m_taccomHeader).copyFrom(m_data);
            m_features += TaccomFeature;
            parseTurnFile(str, m_taccomHeader.turnAddress - 1, m_taccomHeader.turnSize);
            for (size_t i = 0; i < structures::MAX_TRN_ATTACHMENTS; ++i) {
                if (!afl::base::ConstBytes_t(m_taccomHeader.attachments[i].name).empty()) {
                    // Attachment present
                    if (m_taccomHeader.turnAddress > m_taccomHeader.attachments[i].address) {
                        m_turnPlacement = i+1;
                    }
                    checkRange(str, m_taccomHeader.attachments[i].address-1, m_taccomHeader.attachments[i].length);
                }
            }
        } else {
            // Normal TRN
            parseTurnFile(str, 0, m_data.size());
        }
    } else {
        // Probe for taccom header
        structures::TaccomTurnHeader tmp;
        str.fullRead(afl::base::fromObject(tmp));
        if (std::memcmp(tmp.magic, TACCOM_MAGIC, 10) == 0) {
            // it's a Taccom turn
            // FIXME: validate ranges of turnAddress/turnSize
            parseTurnFileHeader(str, tmp.turnAddress - 1, tmp.turnSize);
        } else {
            // it's a real turn
            parseTurnFileHeader(str, 0, str.getSize());
        }
        m_isDirty = true;
    }
}

/** Check a file position.
    Assumes file already loaded into m_data.
    \param stream stream we're processing (for generating error messages only)
    \param offset,length position/range to verify, zero-based
    \throw afl::except::FileFormatException on error. */
void
game::v3::TurnFile::checkRange(afl::io::Stream& stream, afl::io::Stream::FileSize_t offset, afl::io::Stream::FileSize_t length)
{
    // ex GTurnfile::checkRange
    if (offset > m_data.size() || length > m_data.size() - offset) {
        throw afl::except::FileFormatException(stream, _("Invalid file format (bad pointer)"));
    }
}

/** Parse Turn File.
    To be called from the constructor only (assumes most things zeroed).
    \param offset Starting position into file (nonzero for Taccom).
    \param length Length of turn data.
    \throw afl::except::FileFormatException on error. */
void
game::v3::TurnFile::parseTurnFile(afl::io::Stream& stream, afl::io::Stream::FileSize_t offset, afl::io::Stream::FileSize_t length)
{
    // Imports
    using afl::bits::Int32LE;

    // ex GTurnfile::parseTurnfile
    checkRange(stream, offset, length);

    // An estimate of the maximum valid command count.
    // Maximum object commands are 18*999 (ships) + 15*500 (planets) + 15*500 (bases) = 32982, plus messages, password, sendfile and alliances.
    // The main reason of this check is to avoid overflows in further checks, so we can probably safely assume no turn will contain more than a million commands.
    // THost rejects everything that has more than 5000.
    static const int32_t MAX_COMMANDS = 1000000;

    // get a handle to the file data
    afl::base::ConstBytes_t data(m_data);

    // read & validate trn_header
    if (length < sizeof(m_turnHeader) + sizeof(m_dosTrailer)) {
        throw afl::except::FileTooShortException(stream);
    }
    afl::base::fromObject(m_turnHeader).copyFrom(data.subrange(offset));
    if (m_turnHeader.numCommands < 0 || m_turnHeader.numCommands > MAX_COMMANDS) {
        throw afl::except::FileFormatException(stream, _("Invalid file format (invalid command count)"));
    }
    if (length < sizeof(m_turnHeader) + (m_turnHeader.numCommands != 0) + 4*m_turnHeader.numCommands + sizeof(m_dosTrailer)) {
        throw afl::except::FileTooShortException(stream);
    }

    // read & populate command array
    checkRange(stream, sizeof(m_turnHeader)+1, 4*m_turnHeader.numCommands);
    afl::base::ConstBytes_t offsetTable = data.subrange(offset + sizeof(m_turnHeader) + 1, 4*m_turnHeader.numCommands);
    while (const Int32LE::Bytes_t* p = offsetTable.eatN<4>()) {
        m_offsets.append(static_cast<uint32_t>(offset + Int32LE::unpack(*p) - 1));
    }
    for (size_t i = 0, n = m_offsets.size(); i < n; ++i) {
        checkRange(stream, *m_offsets.at(i), 4);      // each command is at least 4 bytes

        CommandCode_t cmd;
        if (getCommandCode(i, cmd) && cmd == tcm_SendBack) {
            checkRange(stream, *m_offsets.at(i), 8);  // getCommandLength() will refer to offset+6
        }

        int length;
        if (getCommandLength(i, length)) {
            checkRange(stream, *m_offsets.at(i), length + 4);
        }
    }

    // now read the trailers
    // FIXME? In case the actual turn data contains "VER3.5nn", this will mis-interpret the turn file in the same way as host does.
    if (length >= sizeof(m_dosTrailer) + sizeof(m_windowsTrailer) + sizeof(m_turnHeader)) {
        afl::base::fromObject(m_windowsTrailer).copyFrom(data.subrange(offset + length - sizeof(m_dosTrailer) - sizeof(m_windowsTrailer)));
        if (std::memcmp(m_windowsTrailer.magic, V35_MAGIC, 6) == 0) {
            m_features += WinplanFeature;
            if (m_windowsTrailer.magic[6] >= '0' && m_windowsTrailer.magic[6] <= '9' && m_windowsTrailer.magic[7] >= '0' && m_windowsTrailer.magic[7] <= '9') {
                m_version = 10*(m_windowsTrailer.magic[6] - '0') + (m_windowsTrailer.magic[7] - '0');
            }
        }
    }
    if (!m_features.contains(WinplanFeature)) {
        afl::base::fromObject(m_windowsTrailer).fill(0);
    }

    afl::base::fromObject(m_dosTrailer).copyFrom(data.subrange(offset + length - sizeof(m_dosTrailer)));
}

/** Parse headers and trailers directly from a stream.
    \param offset Starting position into file (nonzero for Taccom).
    \param length Length of turn data. */
void
game::v3::TurnFile::parseTurnFileHeader(afl::io::Stream& stream, afl::io::Stream::FileSize_t offset, afl::io::Stream::FileSize_t length)
{
    // ex GTurnfile::parseTurnfileHeader
    afl::io::Stream::FileSize_t streamSize = stream.getSize();
    if (offset >= streamSize || length > streamSize - offset || length < sizeof(m_turnHeader) + sizeof(m_dosTrailer)) {
        throw afl::except::FileTooShortException(stream);
    }

    // get the DOS header
    stream.setPos(offset);
    stream.fullRead(afl::base::fromObject(m_turnHeader));

    // get the DOS trailer
    stream.setPos(offset + length - sizeof(m_dosTrailer));
    stream.fullRead(afl::base::fromObject(m_dosTrailer));

    // get the windows trailer, if existing
    if (length > sizeof(m_turnHeader) + sizeof(m_dosTrailer) + sizeof(m_windowsTrailer)) {
        stream.setPos(offset + length - sizeof(m_dosTrailer) - sizeof(m_windowsTrailer));
        stream.fullRead(afl::base::fromObject(m_windowsTrailer));
        if (std::memcmp(m_windowsTrailer.magic, V35_MAGIC, 6) == 0) {
            m_features += WinplanFeature;
            if (m_windowsTrailer.magic[6] >= '0' && m_windowsTrailer.magic[6] <= '9' && m_windowsTrailer.magic[7] >= '0' && m_windowsTrailer.magic[7] <= '9') {
                m_version = 10*(m_windowsTrailer.magic[6] - '0') + (m_windowsTrailer.magic[7] - '0');
            }
        }
    }
    if (!m_features.contains(WinplanFeature)) {
        afl::base::fromObject(m_windowsTrailer).fill(0);
    }
}

/** Generate turn file structure. Called by update().
    The turn must not contain any invalid commands.
    \param data buffer that will receive the turn data (initially empty)
    \param offsets buffer that will receive the command offsets (initially empty) */
void
game::v3::TurnFile::updateTurnFile(afl::base::GrowableMemory<uint8_t>& data, afl::base::GrowableMemory<uint32_t>& offsets)
{
    // ex GTurnfile::updateTurnfile(Buffer<char>& buf, Buffer<int32_t>& offsets)

    // Update turn header
    // .player, .timestamp already set
    m_turnHeader.timeChecksum = int16_t(afl::checksums::ByteSum().add(m_turnHeader.timestamp, 0));
    m_turnHeader.numCommands = int32_t(m_offsets.size());
    m_turnHeader.unused = 0;  // why not?

    // now, add commands
    size_t newTurnStart = data.size();
    data.append(afl::base::fromObject(m_turnHeader));
    if (!m_offsets.empty()) {
        // Make room for command pointers, beginning with a null byte
        data.append(0);
        size_t turnDirOffset = data.size();
        data.ensureSize(turnDirOffset + 4*m_offsets.size());
        for (size_t i = 0, n = m_offsets.size(); i < n; ++i) {
            // Copy individual commands
            int length = 0;
            if (!getCommandLength(i, length)) {
                // Cannot happen
                length = 0;
            }

            size_t thisCommandOffset = data.size();
            offsets.append(static_cast<uint32_t>(thisCommandOffset));
            data.append(m_data.subrange(*m_offsets.at(i), length + 4));
            put<afl::bits::Int32LE>(data, turnDirOffset + 4*i, int32_t(thisCommandOffset - newTurnStart + 1));
        }
    }

    // Append trailers
    if (m_features.contains(WinplanFeature)) {
        std::memcpy(m_windowsTrailer.magic, V35_MAGIC, 6);
        m_windowsTrailer.magic[6] = char('0' + m_version/10);
        m_windowsTrailer.magic[7] = char('0' + m_version%10);
        // .vphKey, .regstr[1..4], unused already set
        data.append(afl::base::fromObject(m_windowsTrailer));
    }

    // reg, players already set
    m_dosTrailer.checksum = afl::checksums::ByteSum().add(afl::base::ConstBytes_t(data).subrange(newTurnStart), 0)
        + 3*m_turnHeader.timeChecksum
        + 13;
    m_dosTrailer.signature = 0x474E3243;   // magic.
    data.append(afl::base::fromObject(m_dosTrailer));
}

// /** Generate commands for an object. Adds the commands to this turn file.
//     \param id      Object Id
//     \param low,up  Command range (e.g. tcm_ShipFIRST,tcm_ShipLAST)
//     \param old,neu Old and new data */
void
game::v3::TurnFile::makeCommands(int id, int low, int up, afl::base::ConstBytes_t oldObject, afl::base::ConstBytes_t newObject)
{
    // ex GTurnFile::makeCommands
    static const uint8_t zero[2] = {0,0};
    for (int i = low; i <= up; ++i) {
        size_t index = COMMAND_DEFINITIONS[i].index;
        size_t size = COMMAND_DEFINITIONS[i].size;
        if (!oldObject.subrange(index, size).equalContent(newObject.subrange(index, size))
            || (i == tcm_BaseBuildShip && !newObject.subrange(index, 2).equalContent(zero)))
        {
            addCommand(i, id, newObject.subrange(index, size));
        }
    }
}

/** Encode a string according to our character set.
    \param in String */
String_t
game::v3::TurnFile::encodeString(const String_t& in) const
{
    return afl::string::fromBytes(m_charset.encode(afl::string::toMemory(in)));
}
