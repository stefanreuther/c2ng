/**
  *  \file game/maint/dump/parsers.cpp
  *  \brief Class game::maint::dump::Parsers
  */

#include "game/maint/dump/parsers.hpp"

#include <cstring>
#include "afl/string/format.hpp"
#include "game/maint/dump/input.hpp"
#include "game/maint/dump/output.hpp"

using afl::string::Format;

namespace {
    void parseVcrObject(game::maint::dump::Input& in, game::maint::dump::Output& out)
    {
        out.addField("Name",              in.readString(20));
        out.addField("Damage",            in.readWord());
        out.addField("Crew",              in.readWord());
        out.addField("Id",                in.readWord());
        out.addField("Owner",             in.readByte());
        out.addField("Owner_race",        in.readByte());
        out.addField("Picture",           in.readByte());
        out.addField("Hull",              in.readByte());
        out.addField("Beam_type",         in.readWord());
        out.addField("Beam_count",        in.readByte());
        out.addField("Experience_level",  in.readByte());
        out.addField("Bay_count",         in.readWord());
        out.addField("Tube_type",         in.readWord());
        out.addField("Ammo",              in.readWord());
        out.addField("Tube_count",        in.readWord());
    }
}

void
game::maint::dump::Parsers::parseShipFile(Input& in, Output& out)
{
    // Parse it
    if (in.getRemainingSize() >= 2) {
        out.startRecord("Header");
        out.addField("Count", in.readWord());
        out.endRecord();
    }

    int n = 0;
    while (in.getRemainingSize() >= 107) {
        ++n;
        out.startRecord(Format("Ship(#%d)", n));
        out.addField("Ship_Id",              in.readWord());
        out.addField("Owner",                in.readWord());
        out.addField("FCode",                in.readString(3));
        out.addField("Speed",                in.readWord());
        out.addField("Waypoint_delta",       in.readCoordinate());
        out.addField("Location",             in.readCoordinate());
        out.addField("Engine_type",          in.readWord());
        out.addField("Hull_type",            in.readWord());
        out.addField("Beam_type",            in.readWord());
        out.addField("Beam_count",           in.readWord());
        out.addField("Bay_count",            in.readWord());
        out.addField("Tube_type",            in.readWord());
        out.addField("Ammo",                 in.readWord());
        out.addField("Tube_count",           in.readWord());
        out.addField("Mission",              in.readWord());
        out.addField("Primary_enemy",        in.readWord());
        out.addField("Tow_Id",               in.readWord());
        out.addField("Damage",               in.readWord());
        out.addField("Crew",                 in.readWord());
        out.addField("Cargo_Colonist_clans", in.readWord());
        out.addField("Name",                 in.readString(20));
        out.addField("Cargo_Neutronium",     in.readWord());
        out.addField("Cargo_Tritanium",      in.readWord());
        out.addField("Cargo_Duranium",       in.readWord());
        out.addField("Cargo_Molybdenum",     in.readWord());
        out.addField("Cargo_Supplies",       in.readWord());
        out.addField("Unload_Neutronium",    in.readWord());
        out.addField("Unload_Tritanium",     in.readWord());
        out.addField("Unload_Duranium",      in.readWord());
        out.addField("Unload_Molybdenum",    in.readWord());
        out.addField("Unload_Colonists",     in.readWord());
        out.addField("Unload_Supplies",      in.readWord());
        out.addField("Unload_Id",            in.readWord());
        out.addField("Transfer_Neutronium",  in.readWord());
        out.addField("Transfer_Tritanium",   in.readWord());
        out.addField("Transfer_Duranium",    in.readWord());
        out.addField("Transfer_Molybdenum",  in.readWord());
        out.addField("Transfer_Colonists",   in.readWord());
        out.addField("Transfer_Supplies",    in.readWord());
        out.addField("Transfer_Id",          in.readWord());
        out.addField("Intercept_Id",         in.readWord());
        out.addField("Cargo_Money",          in.readWord());
        out.endRecord();
    }

    if (in.getRemainingSize() >= 10) {
        out.startRecord("Trailer");
        out.addUnparsedData(in.readUnparsed(10));
        out.endRecord();
    }
}

void
game::maint::dump::Parsers::parsePDataFile(Input& in, Output& out)
{
    // Parse it
    if (in.getRemainingSize() >= 2) {
        out.startRecord("Header");
        out.addField("Count", in.readWord());
        out.endRecord();
    }

    int n = 0;
    while (in.getRemainingSize() >= 85) {
        ++n;
        out.startRecord(Format("Planet(#%d)", n));
        out.addField("Owner",              in.readWord());
        out.addField("Planet_Id",          in.readWord());
        out.addField("FCode",              in.readString(3));
        out.addField("Mines",              in.readWord());
        out.addField("Factories",          in.readWord());
        out.addField("Defense",            in.readWord());
        out.addField("Mined_Neutronium",   in.readLong());
        out.addField("Mined_Tritanium",    in.readLong());
        out.addField("Mined_Duraniumm",    in.readLong());
        out.addField("Mined_Molybdenum",   in.readLong());
        out.addField("Colonist_clans",     in.readLong());
        out.addField("Supplies",           in.readLong());
        out.addField("Money",              in.readLong());
        out.addField("Ground_Neutronium",  in.readLong());
        out.addField("Ground_Tritanium",   in.readLong());
        out.addField("Ground_Duraniumm",   in.readLong());
        out.addField("Ground_Molybdenum",  in.readLong());
        out.addField("Density_Neutronium", in.readWord());
        out.addField("Density_Tritanium",  in.readWord());
        out.addField("Density_Duranium",   in.readWord());
        out.addField("Density_Molybdenum", in.readWord());
        out.addField("Colonist_tax",       in.readWord());
        out.addField("Native_tax",         in.readWord());
        out.addField("Colonist_happiness", in.readWord());
        out.addField("Native_happiness",   in.readWord());
        out.addField("Native_government",  in.readWord());
        out.addField("Native_clans",       in.readLong());
        out.addField("Native_race",        in.readWord());
        out.addField("Temperature_code",   in.readWord());
        out.addField("Build_base_flag",    in.readWord());
        out.endRecord();
    }

    if (in.getRemainingSize() >= 10) {
        out.startRecord("Trailer");
        out.addUnparsedData(in.readUnparsed(10));
        out.endRecord();
    }
}

void
game::maint::dump::Parsers::parseBDataFile(Input& in, Output& out)
{
    // Parse it
    if (in.getRemainingSize() >= 2) {
        out.startRecord("Header");
        out.addField("Count", in.readWord());
        out.endRecord();
    }

    int n = 0;
    while (in.getRemainingSize() >= 85) {
        ++n;
        out.startRecord(Format("Starbase(#%d)", n));
        out.addField("Planet_Id",         in.readWord());
        out.addField("Owner",             in.readWord());
        out.addField("Base_defense",      in.readWord());
        out.addField("Damage",            in.readWord());
        out.addField("Engine_tech",       in.readWord());
        out.addField("Hull_tech",         in.readWord());
        out.addField("Beam_tech",         in.readWord());
        out.addField("Torpedo_tech",      in.readWord());
        for (int i = 1; i <= 9; ++i) {
            out.addField(Format("Engine_storage(%d)", i), in.readWord());
        }
        for (int i = 1; i <= 20; ++i) {
            out.addField(Format("Hull_storage(%d)", i), in.readWord());
        }
        for (int i = 1; i <= 10; ++i) {
            out.addField(Format("Beam_storage(%d)", i), in.readWord());
        }
        for (int i = 1; i <= 10; ++i) {
            out.addField(Format("Tube_storage(%d)", i), in.readWord());
        }
        for (int i = 1; i <= 10; ++i) {
            out.addField(Format("Torpedo_storage(%d)", i), in.readWord());
        }

        out.addField("Fighters",          in.readWord());
        out.addField("Shipyard_Id",       in.readWord());
        out.addField("Shipyard_Order",    in.readWord());
        out.addField("Mission",           in.readWord());
        out.addField("Build_hull_slot",   in.readWord());
        out.addField("Build_engine_type", in.readWord());
        out.addField("Build_beam_type",   in.readWord());
        out.addField("Build_beam_count",  in.readWord());
        out.addField("Build_tube_type",   in.readWord());
        out.addField("Build_tube_count",  in.readWord());
        out.addField("Unused",            in.readWord());
        out.endRecord();
    }

    if (in.getRemainingSize() >= 10) {
        out.startRecord("Trailer");
        out.addUnparsedData(in.readUnparsed(10));
        out.endRecord();
    } else if (in.getRemainingSize() >= 2) {
        out.startRecord("Trailer");
        out.addField("Turn_number", in.readWord());
        out.endRecord();
    }
}

void
game::maint::dump::Parsers::parseGenFile(Input& in, Output& out)
{
    out.startRecord("Timestamp");
    out.addField("Timestamp", in.readString(18));
    out.endRecord();

    for (int i = 1; i <= 11; ++i) {
        out.startRecord(Format("Score(%d)", i));
        out.addField("Num_planets",       in.readWord());
        out.addField("Num_capital_ships", in.readWord());
        out.addField("Num_freighters",    in.readWord());
        out.addField("Num_starbases",     in.readWord());
        out.endRecord();
    }

    out.startRecord("Identification");
    out.addField("Owner",              in.readWord());
    out.addField("Password_1",         in.readUnparsed(10));
    out.addField("Password_2",         in.readUnparsed(10));
    out.addField("Unused",             in.readUnparsed(1));
    out.addField("Ship_checksum",      in.readLong());
    out.addField("Planet_checksum",    in.readLong());
    out.addField("Starbase_checksum",  in.readLong());
    out.addField("New_password_flag",  in.readWord());
    out.addField("New_password",       in.readUnparsed(10));
    out.addField("Turn_number",        in.readWord());
    out.addField("Timestamp_checksum", in.readWord());
    out.endRecord();
}

void
game::maint::dump::Parsers::parseVcrFile(Input& in, Output& out)
{
    // Parse it
    if (in.getRemainingSize() >= 2) {
        out.startRecord("Header");
        out.addField("Count", in.readWord());
        out.endRecord();
    }

    int n = 0;
    while (in.getRemainingSize() >= 100) {
        ++n;
        out.startRecord(Format("VCR(#%d)", n));
        out.addField("Seed",                 in.readWord());
        out.addField("Signature",            in.readWord());
        out.addField("Temp_or_Capabilities", in.readWord());
        out.addField("Type",                 in.readWord());
        out.addField("Left_mass",            in.readWord());
        out.addField("Right_mass",           in.readWord());
        out.startRecord("Left_object");
        parseVcrObject(in, out);
        out.endRecord();
        out.startRecord("Right_object");
        parseVcrObject(in, out);
        out.endRecord();
        out.addField("Left_shield",          in.readWord());
        out.addField("Right_shield",         in.readWord());
        out.endRecord();
    }

    if (in.getRemainingSize() >= 10) {
        out.startRecord("Trailer");
        out.addUnparsedData(in.readUnparsed(10));
        out.endRecord();
    }
}

void
game::maint::dump::Parsers::parseTargetFile(Input& in, Output& out)
{
    // Parse it
    if (in.getRemainingSize() >= 2) {
        out.startRecord("Header");
        out.addField("Count", in.readWord());
        out.endRecord();
    }

    int n = 0;
    while (in.getRemainingSize() >= 34) {
        ++n;
        out.startRecord(Format("Target(#%d)", n));
        out.addField("Ship_Id",              in.readWord());
        out.addField("Owner",                in.readWord());
        out.addField("Speed",                in.readWord());
        out.addField("Location",             in.readCoordinate());
        out.addField("Hull_type",            in.readWord());
        out.addField("Heading",              in.readWord());
        out.addField("Name",                 in.readString(20));
        out.endRecord();
    }

    if (in.getRemainingSize() >= 10) {
        out.startRecord("Trailer");
        out.addUnparsedData(in.readUnparsed(10));
        out.endRecord();
    }
}

void
game::maint::dump::Parsers::parseHullSpec(Input& in, Output& out)
{
    // All bytes used, so parse all hulls we find
    int n = 0;
    while (in.getRemainingSize() >= 60) {
        ++n;
        out.startRecord(Format("Hull(%d)", n));
        out.addField("Name", in.readString(30));
        out.addField("Picture", in.readWord());
        out.addField("Unused", in.readWord());
        out.addField("Tritanium", in.readWord());
        out.addField("Duranium", in.readWord());
        out.addField("Molybdenum", in.readWord());
        out.addField("Fuel_capacity", in.readWord());
        out.addField("Crew", in.readWord());
        out.addField("Num_engines", in.readWord());
        out.addField("Mass", in.readWord());
        out.addField("Tech_level", in.readWord());
        out.addField("Cargo_capacity", in.readWord());
        out.addField("Num_bays", in.readWord());
        out.addField("Max_tubes", in.readWord());
        out.addField("Max_beams", in.readWord());
        out.addField("Cost", in.readWord());
        out.endRecord();
    }
}

void
game::maint::dump::Parsers::parseTorpSpec(Input& in, Output& out)
{
    // torpspec contains 10x38 bytes, but is usually larger
    for (int slot = 1; slot <= 10 && in.getRemainingSize() > 0; ++slot) {
        out.startRecord(Format("Tube(%d)", slot));
        out.addField("Name", in.readString(20));
        out.addField("Torp_cost", in.readWord());
        out.addField("Cost", in.readWord());
        out.addField("Tritanium", in.readWord());
        out.addField("Duranium", in.readWord());
        out.addField("Molybdenum", in.readWord());
        out.addField("Mass", in.readWord());
        out.addField("Tech_level", in.readWord());
        out.addField("Kill_power", in.readWord());
        out.addField("Damage_power", in.readWord());
        out.endRecord();
    }
}

void
game::maint::dump::Parsers::parseBeamSpec(Input& in, Output& out)
{
    // beamspec contains 10x36 bytes
    for (int slot = 1; slot <= 10 && in.getRemainingSize() > 0; ++slot) {
        out.startRecord(Format("Beam(%d)", slot));
        out.addField("Name", in.readString(20));
        out.addField("Cost", in.readWord());
        out.addField("Tritanium", in.readWord());
        out.addField("Duranium", in.readWord());
        out.addField("Molybdenum", in.readWord());
        out.addField("Mass", in.readWord());
        out.addField("Tech_level", in.readWord());
        out.addField("Kill_power", in.readWord());
        out.addField("Damage_power", in.readWord());
        out.endRecord();
    }
}

void
game::maint::dump::Parsers::parseEngSpec(Input& in, Output& out)
{
    // engspec contains 9x66 bytes, but is usually larger
    for (int slot = 1; slot <= 10 && in.getRemainingSize() > 0; ++slot) {
        out.startRecord(Format("Engine(%d)", slot));
        out.addField("Name", in.readString(20));
        out.addField("Cost", in.readWord());
        out.addField("Tritanium", in.readWord());
        out.addField("Duranium", in.readWord());
        out.addField("Molybdenum", in.readWord());
        out.addField("Tech", in.readWord());
        for (int i = 1; i <= 9; ++i) {
            out.addField(Format("Fuel_factor(%d)", i), in.readLong());
        }
        out.endRecord();
    }
}

void
game::maint::dump::Parsers::parseTrueHull(Input& in, Output& out)
{
    // truehull contains 11x20 words, but is sometimes much larger
    for (int pl = 1; pl <= 11 && in.getRemainingSize() > 0; ++pl) {
        out.startRecord(Format("Player(%d)", pl));
        for (int slot = 1; slot <= 20; ++slot) {
            out.addField(Format("Slot(%d)", slot), in.readWord());
        }
        out.endRecord();
    }
}

void
game::maint::dump::Parsers::parseNameList(Input& in, Output& out)
{
    out.startRecord("Names");
    int n = 0;
    while (in.getRemainingSize() >= 20) {
        ++n;
        out.addField(Format("Name(%d)", n), in.readString(20));
    }
    out.endRecord();
}

void
game::maint::dump::Parsers::parseXYPlan(Input& in, Output& out)
{
    int n = 0;
    while (in.getRemainingSize() >= 6) {
        ++n;
        out.startRecord(Format("Planet(%d)", n));
        out.addField("Location", in.readCoordinate());
        out.addField("Owner", in.readWord());
        out.endRecord();
    }
}

void
game::maint::dump::Parsers::parseRaceNames(Input& in, Output& out)
{
    out.startRecord("Long_names");
    for (int i = 1; i <= 11; ++i) {
        out.addField(Format("Player(%d)", i), in.readString(30));
    }
    out.endRecord();

    out.startRecord("Short_names");
    for (int i = 1; i <= 11; ++i) {
        out.addField(Format("Player(%d)", i), in.readString(20));
    }
    out.endRecord();

    out.startRecord("Adjectives");
    for (int i = 1; i <= 11; ++i) {
        out.addField(Format("Player(%d)", i), in.readString(12));
    }
    out.endRecord();
}

void
game::maint::dump::Parsers::parseTeams(Input& in, Output& out)
{
    // Magic
    uint8_t magic[8];
    if (in.peek(magic) != 8 || std::memcmp(magic, "CCteam0\032", 8) != 0) {
        // ignore
    } else {
        in.skip(8);
        out.startRecord("PCC_Teams");
        out.addField("Flags", in.readWord());
        for (int i = 1; i <= 12; ++i) {
            out.addField(Format("Team(%d)", i), in.readByte());
        }
        for (int i = 1; i <= 12; ++i) {
            out.addField(Format("Color(%d)", i), in.readByte());
        }
        for (int i = 1; i <= 12; ++i) {
            out.addField(Format("Name(%d)", i), in.readPascalString());
        }

        // Optional part
        if (in.getRemainingSize() > 0) {
            for (int i = 1; i <= 11; ++i) {
                out.addField(Format("Send_config(%d)", i), in.readByte());
            }
            for (int i = 1; i <= 11; ++i) {
                out.addField(Format("Receive_config(%d)", i), in.readByte());
            }
            out.addField("Passcode", in.readWord());
        }
        out.endRecord();
    }
}
