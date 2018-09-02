/**
  *  \file game/v3/trn/dumper.cpp
  *  \brief Class game::v3::trn::Dumper
  */

#include "game/v3/trn/dumper.hpp"
#include "afl/base/countof.hpp"
#include "afl/bits/int16le.hpp"
#include "afl/bits/int32le.hpp"
#include "afl/bits/uint16le.hpp"
#include "afl/checksums/bytesum.hpp"
#include "afl/string/format.hpp"
#include "game/v3/structures.hpp"
#include "game/v3/trn/filter.hpp"

namespace {
    /** Indentation level for equal sign in assignments. */
    const size_t AS_INDENT = 20;

    /** Indentation level for comments. */
    const size_t COM_INDENT = 50;
   
    /** Quote a string, C-like. */
    String_t quoteString(const String_t& value)
    {
        String_t result = "\"";
        for (String_t::size_type i = 0; i < value.size(); ++i) {
            switch (value[i]) {
             case '\n':
                result += "\\n";
                break;
             case '\r':
                result += "\\r";
                break;
             case '\\':
             case '\"':
                result += '\\';
                result += value[i];
                break;
             default:
                if (value[i] >= 0 && value[i] < 32) {
                    result += '\\';
                    result += char('0' + value[i] / 64);
                    result += char('0' + value[i] / 8 % 8);
                    result += char('0' + value[i] % 8);
                } else {
                    result += value[i];
                }
                break;
            }
        }
        result += "\"";
        return result;
    }

    String_t formatHex(uint32_t n)
    {
        return afl::string::Format("%08X", n);
    }

    /** Decode reg string, Winplan trailer. */
    String_t decodeStringPair(const game::v3::structures::String25_t (&pair)[2], afl::charset::Charset& cs)
    {
        // ex game/un-trn.cc:decode
        game::v3::structures::String25_t tmp;
        for (size_t i = 0; i < sizeof(tmp); ++i) {
            tmp.m_bytes[i] = pair[0].m_bytes[i] ^ pair[1].m_bytes[i];
        }
        return cs.decode(tmp);
    }

    /** Decode reg string, standard trailer. */
    String_t decodeEncryptedString(const game::v3::structures::UInt32_t t[], afl::charset::Charset& cs, bool& errorFlag)
    {
        // ex game/un-trn.cc:decode13
        uint8_t tmp[25];
        for (int i = 0; i < 25; ++i) {
            if (t[i] % (13+13*i)) {
                errorFlag = true;
                tmp[i] = '?';
            } else {
                tmp[i] = char(t[i] / (13+13*i));
            }
        }
        return cs.decode(afl::bits::unpackFixedString(tmp));
    }
}

/***************************** CommandReader *****************************/

class game::v3::trn::Dumper::CommandReader {
 public:
    CommandReader(afl::base::ConstBytes_t data)
        : m_data(data)
        { }

    int32_t getInt16();
    uint16_t getUInt16();
    int32_t getInt32();
    uint8_t getUInt8();
    afl::base::ConstBytes_t getBlob(size_t size);
    String_t getString(size_t size, afl::charset::Charset& cs);

 private:
    afl::base::ConstBytes_t m_data;
};


int32_t
game::v3::trn::Dumper::CommandReader::getInt16()
{
    // ex CommandReader::getInt16
    typedef afl::bits::Int16LE T;
    if (const T::Bytes_t* p = m_data.eatN<sizeof(T::Bytes_t)>()) {
        return T::unpack(*p);
    } else {
        m_data.reset();
        return 0;
    }
}

uint16_t
game::v3::trn::Dumper::CommandReader::getUInt16()
{
    typedef afl::bits::UInt16LE T;
    if (const T::Bytes_t* p = m_data.eatN<sizeof(T::Bytes_t)>()) {
        return T::unpack(*p);
    } else {
        m_data.reset();
        return 0;
    }
}

int32_t
game::v3::trn::Dumper::CommandReader::getInt32()
{
    // ex CommandReader::getInt32
    typedef afl::bits::Int32LE T;
    if (const T::Bytes_t* p = m_data.eatN<sizeof(T::Bytes_t)>()) {
        return T::unpack(*p);
    } else {
        m_data.reset();
        return 0;
    }
}

uint8_t
game::v3::trn::Dumper::CommandReader::getUInt8()
{
    if (const uint8_t* p = m_data.eat()) {
        return *p;
    } else {
        return 0;
    }
}

afl::base::ConstBytes_t
game::v3::trn::Dumper::CommandReader::getBlob(size_t size)
{
    return m_data.split(size);
}

String_t
game::v3::trn::Dumper::CommandReader::getString(size_t size, afl::charset::Charset& cs)
{
    // ex CommandReader::getString
    return cs.decode(afl::bits::unpackFixedString(getBlob(size)));
}


/********************************* Dumper ********************************/

// Constructor.
game::v3::trn::Dumper::Dumper(afl::io::TextWriter& out)
    : m_output(out),
      m_showComments(true),
      m_showHeader(true),
      m_showTrailer(true),
      m_verifyTrailerChecksum(true),
      m_filter(0)
{ }

// Set "show comments" option.
void
game::v3::trn::Dumper::setShowComments(bool flag)
{
    m_showComments = flag;
}

// Set "show header" option.
void
game::v3::trn::Dumper::setShowHeader(bool flag)
{
    m_showHeader = flag;
}

// Set "show trailer" option.
void
game::v3::trn::Dumper::setShowTrailer(bool flag)
{
    m_showTrailer = flag;
}

// Set "verify trailer checksum" option.
void
game::v3::trn::Dumper::setVerifyTrailerChecksum(bool flag)
{
    m_verifyTrailerChecksum = flag;
}

// Set command filter.
void
game::v3::trn::Dumper::setFilter(const Filter* f)
{
    m_filter = f;
}

// Main entry point: dump a turn file.
bool
game::v3::trn::Dumper::dump(const TurnFile& trn) const
{
    bool nl = false, match = false;
    if (m_showHeader) {
        showHeader(trn);
        nl = true;
    }

    /* Now process commands */
    for (size_t i = 0, n = trn.getNumCommands(); i < n; ++i) {
        if (m_filter == 0 || m_filter->accept(trn, i)) {
            if (nl) {
                m_output.writeLine();
            }
            nl = true;
            showCommand(trn, i);
            match = true;
        }
    }
    if (m_showTrailer == 1) {
        if (nl) {
            m_output.writeLine();
        }
        showTrailer(trn);
    }
    return match;
}

/** Show a line in output, most general version.
    \param name    left-hand side of "="
    \param value   right-hand side of "="
    \param comment comment */
void
game::v3::trn::Dumper::showLine(const String_t& name, const String_t& value, const String_t& comment) const
{
    // ex game/un-trn.cc:showLine
    String_t output = name;
    if (!value.empty()) {
        if (output.size() < AS_INDENT) {
            output.append(AS_INDENT - output.size(), ' ');
        }
        output += " = ";
        output += value;
    }
    if (!comment.empty() && m_showComments) {
        if (output.length() >= COM_INDENT - 2) {
            m_output.writeLine(output);
            output.clear();
        }
        output.append(COM_INDENT - output.size(), ' ');
        output += "; ";
        output += comment;
    }
    m_output.writeLine(output);
}

void
game::v3::trn::Dumper::showValue(const char* name, int32_t value) const
{
    showValue(String_t(name), value, String_t());
}

void
game::v3::trn::Dumper::showValue(const String_t& name, int32_t value) const
{
    // ex game/un-trn.cc:var
    showValue(name, value, String_t());
}

void
game::v3::trn::Dumper::showValue(const String_t& name, int32_t value, const String_t& comment) const
{
    // ex game/un-trn.cc:var
    showLine(name, afl::string::Format("%d", value), comment);
}

void
game::v3::trn::Dumper::showValue(const char* name, String_t value) const
{
    showValue(String_t(name), value, String_t());
}

void
game::v3::trn::Dumper::showValue(const String_t& name, String_t value) const
{
    // ex game/un-trn.cc:var
    showValue(name, value, String_t());
}

void
game::v3::trn::Dumper::showValue(const String_t& name, String_t value, const String_t& comment) const
{
    // ex game/un-trn.cc:var
    showLine(name, quoteString(value), comment);
}

void
game::v3::trn::Dumper::showValueArray(const String_t& name, CommandReader& rdr, size_t n) const
{
    // ex game/un-trn.cc:showWordIndexed
    for (size_t i = 0; i < n; ++i) {
        showValue(afl::string::Format("  %s%d", name, i+1), rdr.getInt16());
    }
}

void
game::v3::trn::Dumper::showMessage(const TurnFile& trn, CommandReader& rdr, size_t size) const
{
    String_t msg(afl::string::fromBytes(rdr.getBlob(size)));
    for (String_t::size_type i = 0; i < msg.size(); ++i) {
        msg[i] = char(msg[i] - 13);
    }
    msg = trn.charset().decode(afl::string::toBytes(msg));

    String_t::size_type pos = 0;
    String_t::size_type n;
    while ((n = msg.find('\r', pos)) != String_t::npos) {
        m_output.writeLine("    " + quoteString(String_t(msg, pos, n - pos)));
        pos = n+1;
    }
    if (pos < msg.size()) {
        m_output.writeLine("    " + quoteString(String_t(msg, pos)));
    }
}

inline void
game::v3::trn::Dumper::showUtilData(const TurnFile& trn, CommandReader& rdr, uint16_t type, uint16_t size) const
{
    // ex game/un-trn.cc:showUtilData
    if (type == 34) {
        showValue("    File Name", rdr.getString(12, trn.charset()));
        showValue("    File Size", size - 13);
        showValue("    Flags", rdr.getUInt8());
        showHex(rdr, size - 13);
    } else {
        showHex(rdr, size);
    }
}

void
game::v3::trn::Dumper::showHex(CommandReader& rdr, size_t size) const
{
    // ex game/un-trn.cc:showHex
    const size_t MAX_LINES = 16;
    const size_t BYTES_PER_LINE = 16;

    afl::base::ConstBytes_t bytes = rdr.getBlob(size);
    for (size_t lineNr = 0; lineNr < MAX_LINES && !bytes.empty(); ++lineNr) {
        // Build a line
        String_t hexText;
        String_t charText;

        afl::base::ConstBytes_t lineBytes = bytes.split(BYTES_PER_LINE);
        for (size_t i = 0; i < BYTES_PER_LINE; ++i) {
            if (const uint8_t* p = lineBytes.eat()) {
                hexText += afl::string::Format("%02X", *p);
                if (*p >= ' ' && *p < 127) {
                    charText += *p;
                } else {
                    charText += '.';
                }
            } else {
                hexText += "  ";
            }
            hexText += " ";
            if (i == 7) {
                hexText += " ";
            }
        }

        m_output.writeLine(afl::string::Format("  %08X: %s%s", lineNr*BYTES_PER_LINE, hexText, charText));
    }

    if (!bytes.empty()) {
        m_output.writeLine("; ...rest omitted");
    }
}

inline void
game::v3::trn::Dumper::showTaccom(const TurnFile& trn) const
{
    // ex game/un-trn.cc:showTaccom
    const game::v3::structures::TaccomTurnHeader& hdr = trn.getTaccomHeader();
    const size_t place = trn.getTaccomTurnPlace();
    bool shown = false;
    m_output.writeLine(";\n; Taccom-format Turn File Directory:");
    for (size_t i = 0; i < countof(hdr.attachments); ++i) {
        if (place == i) {
            m_output.writeLine(afl::string::Format(";   turn data, %d bytes, position %d", int32_t(hdr.turnSize), int32_t(hdr.turnAddress)));
            shown = true;
        }

        const game::v3::structures::TaccomTurnFile& f = hdr.attachments[i];
        String_t name = trn.charset().decode(f.name);
        if (!name.empty()) {
            m_output.writeLine(afl::string::Format(";   file \"%s\", %d bytes, position %d", name, int32_t(f.length), int32_t(f.address)));
        }
    }
    if (!shown) {
        m_output.writeLine(afl::string::Format(";   turn data, %d bytes, position %d", int32_t(hdr.turnSize), int32_t(hdr.turnAddress)));
    }
}

inline void
game::v3::trn::Dumper::showHeader(const TurnFile& trn) const
{
    // ex game/un-trn.cc:showHeader
    if (trn.getFeatures().contains(trn.TaccomFeature)) {
        m_output.writeLine("; Taccom file format");
    }
    if (trn.getFeatures().contains(trn.WinplanFeature)) {
        m_output.writeLine(afl::string::Format("; Winplan trailer present, sub-version %d", trn.getVersion()));
    }
    if (trn.getFeatures().contains(trn.TaccomFeature)) {
        showTaccom(trn);
    }
    m_output.writeLine();
    showValue("Player",    trn.getPlayer());
    showValue("Commands",  int32_t(trn.getNumCommands()));
    showValue("Timestamp", trn.getTimestamp().getTimestampAsString());

    uint32_t storedChecksum = trn.getTurnHeader().timeChecksum;
    uint32_t actualChecksum = afl::checksums::ByteSum().add(trn.getTurnHeader().timestamp, 0);

    String_t comment;
    if (storedChecksum == actualChecksum) {
        comment = "okay";
    } else {
        comment = afl::string::Format("WRONG, should be %d", actualChecksum);
    }
    showValue("Time checksum", storedChecksum, comment);
}

inline void
game::v3::trn::Dumper::showTrailer(const TurnFile& trn) const
{
    // ex game/un-trn.cc:showTrailer
    if (trn.getFeatures().contains(trn.WinplanFeature)) {
        /* Windows trailer */
        m_output.writeLine("; Version 3.5 file format (Winplan)");
        m_output.writeLine(afl::string::Format("; Sub-version %d", trn.getVersion()));

        const game::v3::structures::TurnWindowsTrailer& wt = trn.getWindowsTrailer();

        String_t turnComment;
        if (int turnNumber = trn.tryGetTurnNr()) {
            turnComment = afl::string::Format("   Turn = %d", turnNumber);
        } else {
            turnComment = "   Unknown turn?";
        }

        uint32_t vphA = wt.vphKey[0];
        uint32_t vphB = wt.vphKey[1];
        showLine("VPH A", formatHex(vphA), afl::string::Format("-> VPH = %08X", vphA ^ vphB));
        showLine("VPH B", formatHex(vphB), turnComment);
        showValue("RegStr1", decodeStringPair(wt.regstr1, trn.charset()));
        showValue("RegStr2", decodeStringPair(wt.regstr2, trn.charset()));
        showValue("RegStr3", trn.charset().decode(wt.regstr3), "Player Name");
        showValue("RegStr4", trn.charset().decode(wt.regstr4), "Player Address");
        m_output.writeLine();
        m_output.writeLine("; DOS Trailer follows:");
    } else {
        m_output.writeLine("; Version 3.0 file format (DOS)");
    }

    /* DOS Trailer */
    const game::v3::structures::TurnDosTrailer& dt = trn.getDosTrailer();
    uint32_t storedChecksum = dt.checksum;
    if (m_verifyTrailerChecksum) {
        uint32_t computedChecksum = trn.computeTurnChecksum();
        if (computedChecksum == storedChecksum) {
            showLine("Checksum", formatHex(storedChecksum), "okay");
        } else {
            showLine("Checksum", formatHex(storedChecksum), afl::string::Format("ERROR: should be %08X", computedChecksum));
        }
    } else {
        showLine("Checksum", formatHex(storedChecksum), String_t());
    }

    const uint32_t sig = dt.signature;
    const char* mtName =
        sig == 0x32434350 || sig == 0x49494343 ? "PCC2"
        : sig == 0x21434350 ? "PCC"
        : sig == 0x474E3243 ? "c2ng"
        : sig == 0x2E522E53 ? "Stefan's Portable Maketurn"
        : sig == 0x6F72656B || sig == 0x6F72654B ? "k-Maketurn"
        : sig == 0 ? "Tim's Maketurn or VPmaketurn"
        : "Tim's Maketurn";
    showLine("Unused", formatHex(sig), mtName);

    bool errorFlag = false;
    showValue("RegStr1", decodeEncryptedString(&dt.registrationKey[0], trn.charset(), errorFlag));
    showValue("RegStr2", decodeEncryptedString(&dt.registrationKey[25], trn.charset(), errorFlag));

    uint32_t regSum = 668;
    for (int i = 0; i < 50; ++i) {
        regSum += dt.registrationKey[i];
    }
    if (regSum == dt.registrationKey[50]) {
        showLine("RegSum", formatHex(dt.registrationKey[50]), "okay");
    } else {
        showLine("RegSum", formatHex(dt.registrationKey[50]), afl::string::Format("ERROR: should be %08X", regSum));
    }
    if (errorFlag) {
        m_output.writeLine("; WARNING: Encoding error (indicated with \"?\")");
    }

    m_output.writeLine();
    m_output.writeLine("PlayerLog");
    for (int i = 0; i < game::v3::structures::NUM_PLAYERS; ++i) {
        showLine(afl::string::Format("  Player%d", i+1), formatHex(dt.playerSecret.data[i]), String_t());
    }
}

void
game::v3::trn::Dumper::showCommand(const TurnFile& trn, size_t index) const
{
    // ex game/un-trn.cc:showCommand
    // Fetch metadata
    TurnFile::CommandCode_t code;
    TurnFile::CommandType type;
    int32_t pos;
    if (!trn.getCommandCode(index, code) || !trn.getCommandType(index, type) || !trn.getCommandPosition(index, pos)) {
        return;
    }

    // Fetch command name
    const char* name = trn.getCommandName(index);
    if (name == 0) {
        showValue("Command", code, afl::string::Format("index %d, position %08X", index+1, pos));
        m_output.writeLine("; Unknown command");
        return;
    }

    // It's a known command, show it
    showLine(name, String_t(), afl::string::Format("index %d, position %08X", index+1, pos));
    int id;
    switch(type) {
     case TurnFile::ShipCommand:
        if (trn.getCommandId(index, id)) {
            showValue("  Ship Id", id);
        }
        break;
     case TurnFile::PlanetCommand:
        if (trn.getCommandId(index, id)) {
            showValue("  Planet Id", id);
        }
        break;
     case TurnFile::BaseCommand:
        if (trn.getCommandId(index, id)) {
            showValue("  Base Id", id);
        }
        break;
     case TurnFile::OtherCommand:
     case TurnFile::UndefinedCommand:
        break;
    }

    CommandReader rdr(trn.getCommandData(index));
    switch (code) {
     case tcm_ShipChangeFc:
     case tcm_PlanetChangeFc:                     // pid, 3 bytes
        showValue("  FCode", rdr.getString(3, trn.charset()));
        break;
     case tcm_ShipChangeSpeed:                    // sid, 1 word
        showValue("  Speed", rdr.getInt16());
        break;
     case tcm_ShipChangeWaypoint:                 // sid, 2 words
        showValue("  WaypointDX", rdr.getInt16());
        showValue("  WaypointDY", rdr.getInt16());
        break;
     case tcm_ShipChangeMission:                  // sid, 1 word
        showValue("  Mission", rdr.getInt16());
        break;
     case tcm_ShipChangePrimaryEnemy:             // sid, 1 word
        showValue("  Player", rdr.getInt16());
        break;
     case tcm_ShipTowShip:                        // sid, 1 word
        showValue("  Towee Id", rdr.getInt16());
        break;
     case tcm_ShipChangeName:                     // sid, 20 bytes
        showValue("  Name", rdr.getString(20, trn.charset()));
        break;
     case tcm_ShipBeamDownCargo:                  // sid, 7 words NTDMCS+id
     case tcm_ShipTransferCargo:                  // sid, 7 words NTDMCS+id
        showValue("  Neutronium", rdr.getInt16());
        showValue("  Tritanium",  rdr.getInt16());
        showValue("  Duranium",   rdr.getInt16());
        showValue("  Molybdenum", rdr.getInt16());
        showValue("  Clans",      rdr.getInt16());
        showValue("  Supplies",   rdr.getInt16());
        showValue("  Target Id",  rdr.getInt16());
        break;
     case tcm_ShipIntercept:                      // sid, 1 word
        showValue("  Target Id", rdr.getInt16());
        break;
     case tcm_ShipChangeNeutronium:               // sid, 1 word
        showValue("  Neutronium", rdr.getInt16());
        break;
     case tcm_ShipChangeTritanium:                // sid, 1 word
        showValue("  Tritanium", rdr.getInt16());
        break;
     case tcm_ShipChangeDuranium:                 // sid, 1 word
        showValue("  Duranium", rdr.getInt16());
        break;
     case tcm_ShipChangeMolybdenum:               // sid, 1 word
        showValue("  Molybdenum", rdr.getInt16());
        break;
     case tcm_ShipChangeSupplies:                 // sid, 1 word
        showValue("  Supplies", rdr.getInt16());
        break;
     case tcm_ShipChangeColonists:                // sid, 1 word
        showValue("  Clans", rdr.getInt16());
        break;
     case tcm_ShipChangeTorpedoes:                // sid, 1 word
        showValue("  Ammo", rdr.getInt16());
        break;
     case tcm_ShipChangeMoney:                    // sid, 1 word
        showValue("  Money", rdr.getInt16());
        break;

        /* Planet commands */
     case tcm_PlanetChangeMines:                  // pid, 1 word <-
        showValue("  Mines", rdr.getInt16());
        break;
     case tcm_PlanetChangeFactories:              // pid, 1 word
        showValue("  Factories", rdr.getInt16());
        break;
     case tcm_PlanetChangeDefense:                // pid, 1 word
        showValue("  Defense", rdr.getInt16());
        break;
     case tcm_PlanetChangeNeutronium:             // pid, 1 dword
        showValue("  Neutronium", rdr.getInt32());
        break;
     case tcm_PlanetChangeTritanium:              // pid, 1 dword
        showValue("  Tritanium", rdr.getInt32());
        break;
     case tcm_PlanetChangeDuranium:               // pid, 1 dword
        showValue("  Duranium", rdr.getInt32());
        break;
     case tcm_PlanetChangeMolybdenum:             // pid, 1 dword
        showValue("  Molybdenum", rdr.getInt32());
        break;
     case tcm_PlanetChangeColonists:              // pid, 1 dword
        showValue("  Clans", rdr.getInt32());
        break;
     case tcm_PlanetChangeSupplies:               // pid, 1 dword
        showValue("  Supplies", rdr.getInt32());
        break;
     case tcm_PlanetChangeMoney:                  // pid, 1 dword
        showValue("  Money", rdr.getInt32());
        break;
     case tcm_PlanetColonistTax:                  // pid, 1 word
     case tcm_PlanetNativeTax:                    // pid, 1 word
        showValue("  Tax Rate", rdr.getInt16());
        break;
     case tcm_PlanetBuildBase:                    // pid, NO DATA
        m_output.writeLine("; no data for this command");
        break;

        /* Starbase commands */
     case tcm_BaseChangeDefense:                  // bid, 1 word
        showValue("  Defense", rdr.getInt16());
        break;
     case tcm_BaseUpgradeEngineTech:              // bid, 1 word
     case tcm_BaseUpgradeHullTech:                // bid, 1 word <-
     case tcm_BaseUpgradeWeaponTech:              // bid, 1 word <-
     case tcm_BaseUpgradeTorpTech:                // bid, 1 word
        showValue("  Tech", rdr.getInt16());
        break;
     case tcm_BaseBuildEngines:                   // bid, 9 words
        showValueArray("Engine", rdr, 9);
        break;
     case tcm_BaseBuildHulls:                     // bid, 20 words
        showValueArray("Hull", rdr, 20);
        break;
     case tcm_BaseBuildWeapons:                   // bid, 10 words
        showValueArray("Beam", rdr, 10);
        break;
     case tcm_BaseBuildLaunchers:                 // bid, 10 words
        showValueArray("Launcher", rdr, 10);
        break;
     case tcm_BaseBuildTorpedoes:                 // bid, 10 words
        showValueArray("Torp", rdr, 10);
        break;
     case tcm_BaseBuildFighters:                  // bid, 1 word
        showValue("  Fighters", rdr.getInt16());
        break;
     case tcm_BaseFixRecycleShipId:               // bid, 1 word <-
        showValue("  Ship Id", rdr.getInt16());
        break;
     case tcm_BaseFixRecycleShip:                 // bid, 1 word action
        switch (int32_t v = rdr.getInt16()) {
         case 0:
            showValue("  Action", 0, "none");
            break;
         case 1:
            showValue("  Action", 1, "Fix");
            break;
         case 2:
            showValue("  Action", 2, "Recycle");
            break;
         default:
            showValue("  Action", v, "INVALID");
        }
        break;
     case tcm_BaseChangeMission:                  // bid, 1 word
        showValue("  Mission", rdr.getInt16());
        break;
     case tcm_BaseBuildShip:                      // bid, 7 words
        showValue("  Hull Type",   rdr.getInt16());
        showValue("  Engine Type", rdr.getInt16());
        showValue("  Beam Type",   rdr.getInt16());
        showValue("  Beam Count",  rdr.getInt16());
        showValue("  Torp Type",   rdr.getInt16());
        showValue("  Torp Count",  rdr.getInt16());
        showValue("  Unused",      rdr.getInt16());
        break;

        /* Rest */
     case tcm_SendMessage:                        // len, from, to, text
        showValue("  From", rdr.getInt16());
        showValue("  To", rdr.getInt16());
        if (trn.getCommandId(index, id) && id > 0) {
            m_output.writeLine("  Text =");
            showMessage(trn, rdr, id);
        } else {
            showValue("  Text", "", "missing/empty");
        }
        break;
     case tcm_ChangePassword:                     // zero, 10 bytes
        m_output.writeLine("; Intentionally not decoded.");
        break;
     case tcm_SendBack:
        if (trn.getCommandId(index, id)) {
            uint16_t type = rdr.getUInt16();
            uint16_t size = rdr.getUInt16();
            showValue("  Receiver", id);
            showValue("  Type", type);
            showValue("  Size", size);
            showUtilData(trn, rdr, type, size);
        }
        break;
    }

    /* Diagnose positioning errors */
    int32_t nextPos;
    int thisLength;
    if (trn.getCommandPosition(index+1, nextPos) && trn.getCommandLength(index, thisLength)) {
        // thisLength does not include the type/id fields that are present for all commands
        thisLength += 4;
        int32_t endPos = pos + thisLength;
        if (endPos != nextPos) {
            m_output.writeLine(afl::string::Format("; WARNING: next command not at expected position %08X", endPos));
            if (endPos < nextPos) {
                m_output.writeLine(afl::string::Format("; there's a %d bytes gap", nextPos - endPos));
            } else {
                int32_t overlap = endPos - nextPos;
                if (overlap < thisLength) {
                    m_output.writeLine(afl::string::Format("; there's a %d bytes overlap between commands", overlap));
                } else {
                    /* next command is at/before the start of this one. How do we name that? */
                    m_output.writeLine("; this TRN is screwed.");
                }
            }
        }
    }
}
