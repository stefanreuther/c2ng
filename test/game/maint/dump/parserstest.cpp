/**
  *  \file test/game/maint/dump/parserstest.cpp
  *  \brief Test for game::maint::dump::Parsers
  */

#include "game/maint/dump/parsers.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/textfile.hpp"
#include "afl/test/testrunner.hpp"
#include "game/maint/dump/input.hpp"
#include "game/maint/dump/textoutput.hpp"
#include "game/test/files.hpp"
#include "util/io.hpp"

namespace {
    struct Environment {
        Environment(afl::base::ConstBytes_t data)
            : charset(),
              input(data, charset),
              stream(),
              textFile(stream),
              output(textFile)
            { }

        // Input
        afl::charset::Utf8Charset charset;
        game::maint::dump::Input input;

        // Output
        afl::io::InternalStream stream;
        afl::io::TextFile textFile;
        game::maint::dump::TextOutput output;
    };

    String_t getResult(Environment& env)
    {
        env.textFile.flush();
        return util::normalizeLinefeeds(env.stream.getContent());
    }
}

/*
 *  Test cases mostly created using PCC2 c2dump
 */

AFL_TEST("game.maint.dump.Parsers:parseShipFile", a)
{
    static const uint8_t FILE[] = {
        0x01, 0x00, 0x0a, 0x00, 0x0a, 0x00, 0x6a, 0x6a, 0x6a, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf4,
        0x01, 0xf4, 0x01, 0x09, 0x00, 0x4e, 0x00, 0x0a, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x74,
        0x65, 0x73, 0x74, 0x65, 0x72, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x03, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x34, 0x31,
        0x34, 0x30, 0x28, 0x30, 0x50, 0x2b, 0x3a
    };

    Environment env(FILE);
    game::maint::dump::Parsers::parseShipFile(env.input, env.output);
    a.checkEqual("result", getResult(env),
                 "Header:\n"
                 "  Count                          = 1\n"
                 "\n"
                 "Ship(#1):\n"
                 "  Ship_Id                        = 10\n"
                 "  Owner                          = 10\n"
                 "  FCode                          = 'jjj'\n"
                 "  Speed                          = 9\n"
                 "  Waypoint_delta                 = (0,0)\n"
                 "  Location                       = (500,500)\n"
                 "  Engine_type                    = 9\n"
                 "  Hull_type                      = 78\n"
                 "  Beam_type                      = 10\n"
                 "  Beam_count                     = 4\n"
                 "  Bay_count                      = 0\n"
                 "  Tube_type                      = 0\n"
                 "  Ammo                           = 0\n"
                 "  Tube_count                     = 0\n"
                 "  Mission                        = 2\n"
                 "  Primary_enemy                  = 0\n"
                 "  Tow_Id                         = 0\n"
                 "  Damage                         = 0\n"
                 "  Crew                           = 10\n"
                 "  Cargo_Colonist_clans           = 0\n"
                 "  Name                           = 'tester'\n"
                 "  Cargo_Neutronium               = 3\n"
                 "  Cargo_Tritanium                = 14\n"
                 "  Cargo_Duranium                 = 0\n"
                 "  Cargo_Molybdenum               = 0\n"
                 "  Cargo_Supplies                 = 0\n"
                 "  Unload_Neutronium              = 0\n"
                 "  Unload_Tritanium               = 0\n"
                 "  Unload_Duranium                = 0\n"
                 "  Unload_Molybdenum              = 0\n"
                 "  Unload_Colonists               = 0\n"
                 "  Unload_Supplies                = 0\n"
                 "  Unload_Id                      = 0\n"
                 "  Transfer_Neutronium            = 0\n"
                 "  Transfer_Tritanium             = 0\n"
                 "  Transfer_Duranium              = 0\n"
                 "  Transfer_Molybdenum            = 0\n"
                 "  Transfer_Colonists             = 0\n"
                 "  Transfer_Supplies              = 0\n"
                 "  Transfer_Id                    = 0\n"
                 "  Intercept_Id                   = 0\n"
                 "  Cargo_Money                    = 0\n"
                 "\n"
                 "Trailer:\n"
                 "  Unparsed = 36 34 31 34 30 28 30 50 2B 3A\n"
                 "\n");
}

AFL_TEST("game.maint.dump.Parsers:parsePDataFile", a)
{
    static const uint8_t FILE[] = {
        0x01, 0x00, 0x08, 0x00, 0x39, 0x00, 0x37, 0x30, 0x36, 0x64, 0x00, 0x96, 0x00, 0x64, 0x00, 0xef,
        0x15, 0x00, 0x00, 0x9a, 0x0a, 0x00, 0x00, 0x7e, 0x0c, 0x00, 0x00, 0x84, 0x0b, 0x00, 0x00, 0xa0,
        0x86, 0x01, 0x00, 0xe4, 0x6a, 0x00, 0x00, 0xd1, 0xa4, 0x01, 0x00, 0x57, 0x03, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0xed, 0x04, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x14, 0x00, 0x14, 0x00, 0x0f,
        0x00, 0x5f, 0x00, 0x06, 0x00, 0x00, 0x00, 0x64, 0x00, 0x64, 0x00, 0x03, 0x00, 0x60, 0x61, 0x02,
        0x00, 0x08, 0x00, 0x32, 0x00, 0x00, 0x00, 0x46, 0x2c, 0x43, 0x33, 0x30, 0x4b, 0x40, 0x41, 0x3a,
        0x2e
    };
    Environment env(FILE);
    game::maint::dump::Parsers::parsePDataFile(env.input, env.output);
    a.checkEqual("result", getResult(env),
                 "Header:\n"
                 "  Count                          = 1\n"
                 "\n"
                 "Planet(#1):\n"
                 "  Owner                          = 8\n"
                 "  Planet_Id                      = 57\n"
                 "  FCode                          = '706'\n"
                 "  Mines                          = 100\n"
                 "  Factories                      = 150\n"
                 "  Defense                        = 100\n"
                 "  Mined_Neutronium               = 5615\n"
                 "  Mined_Tritanium                = 2714\n"
                 "  Mined_Duraniumm                = 3198\n"
                 "  Mined_Molybdenum               = 2948\n"
                 "  Colonist_clans                 = 100000\n"
                 "  Supplies                       = 27364\n"
                 "  Money                          = 107729\n"
                 "  Ground_Neutronium              = 855\n"
                 "  Ground_Tritanium               = 1\n"
                 "  Ground_Duraniumm               = 1261\n"
                 "  Ground_Molybdenum              = 5\n"
                 "  Density_Neutronium             = 20\n"
                 "  Density_Tritanium              = 20\n"
                 "  Density_Duranium               = 15\n"
                 "  Density_Molybdenum             = 95\n"
                 "  Colonist_tax                   = 6\n"
                 "  Native_tax                     = 0\n"
                 "  Colonist_happiness             = 100\n"
                 "  Native_happiness               = 100\n"
                 "  Native_government              = 3\n"
                 "  Native_clans                   = 156000\n"
                 "  Native_race                    = 8\n"
                 "  Temperature_code               = 50\n"
                 "  Build_base_flag                = 0\n"
                 "\n"
                 "Trailer:\n"
                 "  Unparsed = 46 2C 43 33 30 4B 40 41 3A 2E\n"
                 "\n");
}

AFL_TEST("game.maint.dump.Parsers:parseBDataFile", a)
{
    static const uint8_t FILE[] = {
        0x01, 0x00, 0x56, 0x00, 0x05, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x07, 0x00, 0x01, 0x00, 0x01, 0x00,
        0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0x4a,
        0x3e, 0x24, 0x3c, 0x2a, 0x2b, 0x48, 0x34, 0x2b
    };
    Environment env(FILE);
    game::maint::dump::Parsers::parseBDataFile(env.input, env.output);
    a.checkEqual("result", getResult(env),
                 "Header:\n"
                 "  Count                          = 1\n"
                 "\n"
                 "Starbase(#1):\n"
                 "  Planet_Id                      = 86\n"
                 "  Owner                          = 5\n"
                 "  Base_defense                   = 10\n"
                 "  Damage                         = 0\n"
                 "  Engine_tech                    = 7\n"
                 "  Hull_tech                      = 1\n"
                 "  Beam_tech                      = 1\n"
                 "  Torpedo_tech                   = 1\n"
                 "  Engine_storage(1)              = 4\n"
                 "  Engine_storage(2)              = 0\n"
                 "  Engine_storage(3)              = 2\n"
                 "  Engine_storage(4)              = 0\n"
                 "  Engine_storage(5)              = 0\n"
                 "  Engine_storage(6)              = 0\n"
                 "  Engine_storage(7)              = 0\n"
                 "  Engine_storage(8)              = 0\n"
                 "  Engine_storage(9)              = 0\n"
                 "  Hull_storage(1)                = 1\n"
                 "  Hull_storage(2)                = 1\n"
                 "  Hull_storage(3)                = 0\n"
                 "  Hull_storage(4)                = 0\n"
                 "  Hull_storage(5)                = 0\n"
                 "  Hull_storage(6)                = 0\n"
                 "  Hull_storage(7)                = 0\n"
                 "  Hull_storage(8)                = 0\n"
                 "  Hull_storage(9)                = 0\n"
                 "  Hull_storage(10)               = 0\n"
                 "  Hull_storage(11)               = 0\n"
                 "  Hull_storage(12)               = 0\n"
                 "  Hull_storage(13)               = 0\n"
                 "  Hull_storage(14)               = 0\n"
                 "  Hull_storage(15)               = 0\n"
                 "  Hull_storage(16)               = 0\n"
                 "  Hull_storage(17)               = 0\n"
                 "  Hull_storage(18)               = 0\n"
                 "  Hull_storage(19)               = 0\n"
                 "  Hull_storage(20)               = 0\n"
                 "  Beam_storage(1)                = 0\n"
                 "  Beam_storage(2)                = 8\n"
                 "  Beam_storage(3)                = 0\n"
                 "  Beam_storage(4)                = 0\n"
                 "  Beam_storage(5)                = 0\n"
                 "  Beam_storage(6)                = 0\n"
                 "  Beam_storage(7)                = 0\n"
                 "  Beam_storage(8)                = 0\n"
                 "  Beam_storage(9)                = 0\n"
                 "  Beam_storage(10)               = 0\n"
                 "  Tube_storage(1)                = 0\n"
                 "  Tube_storage(2)                = 3\n"
                 "  Tube_storage(3)                = 0\n"
                 "  Tube_storage(4)                = 0\n"
                 "  Tube_storage(5)                = 0\n"
                 "  Tube_storage(6)                = 0\n"
                 "  Tube_storage(7)                = 0\n"
                 "  Tube_storage(8)                = 0\n"
                 "  Tube_storage(9)                = 0\n"
                 "  Tube_storage(10)               = 0\n"
                 "  Torpedo_storage(1)             = 0\n"
                 "  Torpedo_storage(2)             = 20\n"
                 "  Torpedo_storage(3)             = 0\n"
                 "  Torpedo_storage(4)             = 0\n"
                 "  Torpedo_storage(5)             = 0\n"
                 "  Torpedo_storage(6)             = 0\n"
                 "  Torpedo_storage(7)             = 0\n"
                 "  Torpedo_storage(8)             = 0\n"
                 "  Torpedo_storage(9)             = 0\n"
                 "  Torpedo_storage(10)            = 0\n"
                 "  Fighters                       = 20\n"
                 "  Shipyard_Id                    = 0\n"
                 "  Shipyard_Order                 = 0\n"
                 "  Mission                        = 1\n"
                 "  Build_hull_slot                = 0\n"
                 "  Build_engine_type              = 0\n"
                 "  Build_beam_type                = 0\n"
                 "  Build_beam_count               = 0\n"
                 "  Build_tube_type                = 0\n"
                 "  Build_tube_count               = 0\n"
                 "  Unused                         = 0\n"
                 "\n"
                 "Trailer:\n"
                 "  Unparsed = 25 4A 3E 24 3C 2A 2B 48 34 2B\n"
                 "\n");
}

AFL_TEST("game.maint.dump.Parsers:parseGenFile", a)
{
    static const uint8_t FILE[] = {
        0x30, 0x35, 0x2d, 0x30, 0x39, 0x2d, 0x32, 0x30, 0x30, 0x34, 0x31, 0x37, 0x3a, 0x34, 0x38, 0x3a,
        0x34, 0x35, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0b, 0x00, 0xdf, 0x01,
        0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x6b, 0x70, 0x6f, 0x41,
        0x72, 0x5f, 0x62, 0x66, 0x67, 0x60, 0x3c, 0x35, 0x37, 0x2b, 0x2c, 0x3f, 0x20, 0x3f, 0x41, 0x3d,
        0x00, 0xa7, 0x14, 0x00, 0x00, 0x6d, 0x04, 0x00, 0x00, 0x6d, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb4, 0x00, 0x9f, 0x03
    };
    Environment env(FILE);
    game::maint::dump::Parsers::parseGenFile(env.input, env.output);
    a.checkEqual("result", getResult(env),
                 "Timestamp:\n"
                 "  Timestamp                      = '05-09-200417:48:45'\n"
                 "\n"
                 "Score(1):\n"
                 "  Num_planets                    = 0\n"
                 "  Num_capital_ships              = 1\n"
                 "  Num_freighters                 = 0\n"
                 "  Num_starbases                  = 0\n"
                 "\n"
                 "Score(2):\n"
                 "  Num_planets                    = 0\n"
                 "  Num_capital_ships              = 0\n"
                 "  Num_freighters                 = 0\n"
                 "  Num_starbases                  = 0\n"
                 "\n"
                 "Score(3):\n"
                 "  Num_planets                    = 0\n"
                 "  Num_capital_ships              = 0\n"
                 "  Num_freighters                 = 0\n"
                 "  Num_starbases                  = 0\n"
                 "\n"
                 "Score(4):\n"
                 "  Num_planets                    = 0\n"
                 "  Num_capital_ships              = 1\n"
                 "  Num_freighters                 = 0\n"
                 "  Num_starbases                  = 0\n"
                 "\n"
                 "Score(5):\n"
                 "  Num_planets                    = 2\n"
                 "  Num_capital_ships              = 1\n"
                 "  Num_freighters                 = 0\n"
                 "  Num_starbases                  = 1\n"
                 "\n"
                 "Score(6):\n"
                 "  Num_planets                    = 0\n"
                 "  Num_capital_ships              = 1\n"
                 "  Num_freighters                 = 0\n"
                 "  Num_starbases                  = 0\n"
                 "\n"
                 "Score(7):\n"
                 "  Num_planets                    = 0\n"
                 "  Num_capital_ships              = 1\n"
                 "  Num_freighters                 = 0\n"
                 "  Num_starbases                  = 0\n"
                 "\n"
                 "Score(8):\n"
                 "  Num_planets                    = 1\n"
                 "  Num_capital_ships              = 11\n"
                 "  Num_freighters                 = 479\n"
                 "  Num_starbases                  = 1\n"
                 "\n"
                 "Score(9):\n"
                 "  Num_planets                    = 0\n"
                 "  Num_capital_ships              = 1\n"
                 "  Num_freighters                 = 0\n"
                 "  Num_starbases                  = 0\n"
                 "\n"
                 "Score(10):\n"
                 "  Num_planets                    = 0\n"
                 "  Num_capital_ships              = 1\n"
                 "  Num_freighters                 = 0\n"
                 "  Num_starbases                  = 0\n"
                 "\n"
                 "Score(11):\n"
                 "  Num_planets                    = 0\n"
                 "  Num_capital_ships              = 0\n"
                 "  Num_freighters                 = 0\n"
                 "  Num_starbases                  = 0\n"
                 "\n"
                 "Identification:\n"
                 "  Owner                          = 1\n"
                 "  Password_1                     = 6B 70 6F 41 72 5F 62 66 67 60\n"
                 "  Password_2                     = 3C 35 37 2B 2C 3F 20 3F 41 3D\n"
                 "  Unused                         = 00\n"
                 "  Ship_checksum                  = 5287\n"
                 "  Planet_checksum                = 1133\n"
                 "  Starbase_checksum              = 1133\n"
                 "  New_password_flag              = 0\n"
                 "  New_password                   = 00 00 00 00 00 00 00 00 00 00\n"
                 "  Turn_number                    = 180\n"
                 "  Timestamp_checksum             = 927\n"
                 "\n");
}

AFL_TEST("game.maint.dump.Parsers:parseVcrFile", a)
{
    static const uint8_t FILE[] = {
        0x01, 0x00, 0x9f, 0x2f, 0x50, 0x8f, 0x00, 0x00, 0x00, 0x00, 0x90, 0x01, 0x5e, 0x01, 0x74, 0x65,
        0x73, 0x74, 0x65, 0x72, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x00, 0x00, 0x0a, 0x00, 0x01, 0x00, 0x01, 0x00, 0x75, 0x4e, 0x0a, 0x00, 0x00, 0x00,
        0x09, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x74, 0x65, 0x73, 0x74, 0x65, 0x72, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x0a, 0x00,
        0x0b, 0x00, 0x0b, 0x00, 0x75, 0x4e, 0x0a, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x64, 0x00, 0x64, 0x00, 0x2f, 0x36, 0x29, 0x37, 0x2f, 0x3f, 0x3d, 0x2e, 0x4f, 0x44
    };
    Environment env(FILE);
    game::maint::dump::Parsers::parseVcrFile(env.input, env.output);
    a.checkEqual("result", getResult(env),
                 "Header:\n"
                 "  Count                          = 1\n"
                 "\n"
                 "VCR(#1):\n"
                 "  Seed                           = 12191\n"
                 "  Signature                      = -28848\n"
                 "  Temp_or_Capabilities           = 0\n"
                 "  Type                           = 0\n"
                 "  Left_mass                      = 400\n"
                 "  Right_mass                     = 350\n"
                 "  Left_object:\n"
                 "    Name                           = 'tester'\n"
                 "    Damage                         = 0\n"
                 "    Crew                           = 10\n"
                 "    Id                             = 1\n"
                 "    Owner                          = 1\n"
                 "    Owner_race                     = 0\n"
                 "    Picture                        = 117\n"
                 "    Hull                           = 78\n"
                 "    Beam_type                      = 10\n"
                 "    Beam_count                     = 0\n"
                 "    Experience_level               = 0\n"
                 "    Bay_count                      = 9\n"
                 "    Tube_type                      = 0\n"
                 "    Ammo                           = 56\n"
                 "    Tube_count                     = 0\n"
                 "\n"
                 "  Right_object:\n"
                 "    Name                           = 'tester'\n"
                 "    Damage                         = 0\n"
                 "    Crew                           = 10\n"
                 "    Id                             = 11\n"
                 "    Owner                          = 11\n"
                 "    Owner_race                     = 0\n"
                 "    Picture                        = 117\n"
                 "    Hull                           = 78\n"
                 "    Beam_type                      = 10\n"
                 "    Beam_count                     = 4\n"
                 "    Experience_level               = 0\n"
                 "    Bay_count                      = 0\n"
                 "    Tube_type                      = 0\n"
                 "    Ammo                           = 0\n"
                 "    Tube_count                     = 0\n"
                 "\n"
                 "  Left_shield                    = 100\n"
                 "  Right_shield                   = 100\n"
                 "\n"
                 "Trailer:\n"
                 "  Unparsed = 2F 36 29 37 2F 3F 3D 2E 4F 44\n"
                 "\n");
}

AFL_TEST("game.maint.dump.Parsers:parseTargetFile", a)
{
    static const uint8_t FILE[] = {
        0x02, 0x00, 0x72, 0x01, 0x03, 0x00, 0x06, 0x00, 0x73, 0x06, 0x2e, 0x05, 0x1d, 0x00, 0x7d, 0x00,
        0x44, 0x41, 0x52, 0x4b, 0x20, 0x57, 0x49, 0x4e, 0x47, 0x20, 0x43, 0x4c, 0x41, 0x53, 0x53, 0x20,
        0x42, 0x41, 0x54, 0x54, 0xbf, 0x01, 0x06, 0x00, 0x04, 0x00, 0x2a, 0x08, 0xe8, 0x05, 0x36, 0x00,
        0x03, 0x01, 0x42, 0x34, 0x31, 0x20, 0x45, 0x58, 0x50, 0x4c, 0x4f, 0x52, 0x45, 0x52, 0x20, 0x34,
        0x34, 0x37, 0x20, 0x20, 0x20, 0x20, 0x49, 0x48, 0x3c, 0x33, 0x2f, 0x2c, 0x2d, 0x41, 0x3d, 0x4d
    };
    Environment env(FILE);
    game::maint::dump::Parsers::parseTargetFile(env.input, env.output);
    a.checkEqual("result", getResult(env),
                 "Header:\n"
                 "  Count                          = 2\n"
                 "\n"
                 "Target(#1):\n"
                 "  Ship_Id                        = 370\n"
                 "  Owner                          = 3\n"
                 "  Speed                          = 6\n"
                 "  Location                       = (1651,1326)\n"
                 "  Hull_type                      = 29\n"
                 "  Heading                        = 125\n"
                 "  Name                           = 'DARK WING CLASS BATT'\n"
                 "\n"
                 "Target(#2):\n"
                 "  Ship_Id                        = 447\n"
                 "  Owner                          = 6\n"
                 "  Speed                          = 4\n"
                 "  Location                       = (2090,1512)\n"
                 "  Hull_type                      = 54\n"
                 "  Heading                        = 259\n"
                 "  Name                           = 'B41 EXPLORER 447'\n"
                 "\n"
                 "Trailer:\n"
                 "  Unparsed = 49 48 3C 33 2F 2C 2D 41 3D 4D\n"
                 "\n");
}

AFL_TEST("game.maint.dump.Parsers:parseHullSpec", a)
{
    Environment env(game::test::getDefaultHulls().subrange(0, 105));
    game::maint::dump::Parsers::parseHullSpec(env.input, env.output);
    a.checkEqual("result", getResult(env),
                 "Hull(1):\n"
                 "  Name                           = 'OUTRIDER CLASS SCOUT'\n"
                 "  Picture                        = 9\n"
                 "  Unused                         = 1\n"
                 "  Tritanium                      = 40\n"
                 "  Duranium                       = 20\n"
                 "  Molybdenum                     = 5\n"
                 "  Fuel_capacity                  = 260\n"
                 "  Crew                           = 180\n"
                 "  Num_engines                    = 1\n"
                 "  Mass                           = 75\n"
                 "  Tech_level                     = 1\n"
                 "  Cargo_capacity                 = 40\n"
                 "  Num_bays                       = 0\n"
                 "  Max_tubes                      = 0\n"
                 "  Max_beams                      = 1\n"
                 "  Cost                           = 50\n"
                 "\n");
}

AFL_TEST("game.maint.dump.Parsers:parseTorpSpec", a)
{
    Environment env(game::test::getDefaultTorpedoes().subrange(0, 38));
    game::maint::dump::Parsers::parseTorpSpec(env.input, env.output);
    a.checkEqual("result", getResult(env),
                 "Tube(1):\n"
                 "  Name                           = 'Mark 1 Photon'\n"
                 "  Torp_cost                      = 1\n"
                 "  Cost                           = 1\n"
                 "  Tritanium                      = 1\n"
                 "  Duranium                       = 1\n"
                 "  Molybdenum                     = 0\n"
                 "  Mass                           = 2\n"
                 "  Tech_level                     = 1\n"
                 "  Kill_power                     = 4\n"
                 "  Damage_power                   = 5\n"
                 "\n");
}

AFL_TEST("game.maint.dump.Parsers:parseBeamSpec", a)
{
    Environment env(game::test::getDefaultBeams().subrange(0, 36));
    game::maint::dump::Parsers::parseBeamSpec(env.input, env.output);
    a.checkEqual("result", getResult(env),
                 "Beam(1):\n"
                 "  Name                           = 'Laser'\n"
                 "  Cost                           = 1\n"
                 "  Tritanium                      = 1\n"
                 "  Duranium                       = 0\n"
                 "  Molybdenum                     = 0\n"
                 "  Mass                           = 1\n"
                 "  Tech_level                     = 1\n"
                 "  Kill_power                     = 10\n"
                 "  Damage_power                   = 3\n"
                 "\n");
}

AFL_TEST("game.maint.dump.Parsers:parseEngSpec", a)
{
    Environment env(game::test::getDefaultEngines().subrange(0, 66));
    game::maint::dump::Parsers::parseEngSpec(env.input, env.output);
    a.checkEqual("result", getResult(env),
                 "Engine(1):\n"
                 "  Name                           = 'StarDrive 1'\n"
                 "  Cost                           = 1\n"
                 "  Tritanium                      = 5\n"
                 "  Duranium                       = 1\n"
                 "  Molybdenum                     = 0\n"
                 "  Tech                           = 1\n"
                 "  Fuel_factor(1)                 = 100\n"
                 "  Fuel_factor(2)                 = 800\n"
                 "  Fuel_factor(3)                 = 2700\n"
                 "  Fuel_factor(4)                 = 6400\n"
                 "  Fuel_factor(5)                 = 12500\n"
                 "  Fuel_factor(6)                 = 21600\n"
                 "  Fuel_factor(7)                 = 34300\n"
                 "  Fuel_factor(8)                 = 51200\n"
                 "  Fuel_factor(9)                 = 72900\n"
                 "\n");
}

AFL_TEST("game.maint.dump.Parsers:parseTrueHull", a)
{
    Environment env(game::test::getDefaultHullAssignments().subrange(0, 40));
    game::maint::dump::Parsers::parseTrueHull(env.input, env.output);
    a.checkEqual("result", getResult(env),
                 "Player(1):\n"
                 "  Slot(1)                        = 1\n"
                 "  Slot(2)                        = 2\n"
                 "  Slot(3)                        = 3\n"
                 "  Slot(4)                        = 16\n"
                 "  Slot(5)                        = 8\n"
                 "  Slot(6)                        = 4\n"
                 "  Slot(7)                        = 5\n"
                 "  Slot(8)                        = 6\n"
                 "  Slot(9)                        = 17\n"
                 "  Slot(10)                       = 9\n"
                 "  Slot(11)                       = 10\n"
                 "  Slot(12)                       = 13\n"
                 "  Slot(13)                       = 7\n"
                 "  Slot(14)                       = 11\n"
                 "  Slot(15)                       = 12\n"
                 "  Slot(16)                       = 19\n"
                 "  Slot(17)                       = 104\n"
                 "  Slot(18)                       = 18\n"
                 "  Slot(19)                       = 20\n"
                 "  Slot(20)                       = 105\n"
                 "\n");
}

AFL_TEST("game.maint.dump.Parsers:parseNameList", a)
{
    Environment env(game::test::getDefaultIonStormNames().subrange(0, 60));
    game::maint::dump::Parsers::parseNameList(env.input, env.output);
    a.checkEqual("result", getResult(env),
                 "Names:\n"
                 "  Name(1)                        = 'Hillery'\n"
                 "  Name(2)                        = 'Leah'\n"
                 "  Name(3)                        = 'Leonora'\n"
                 "\n");
}

AFL_TEST("game.maint.dump.Parsers:parseXYPlan", a)
{
    Environment env(game::test::getDefaultPlanetCoordinates().subrange(0, 24));
    game::maint::dump::Parsers::parseXYPlan(env.input, env.output);
    a.checkEqual("result", getResult(env),
                 "Planet(1):\n"
                 "  Location                       = (1337,2352)\n"
                 "  Owner                          = 0\n"
                 "\n"
                 "Planet(2):\n"
                 "  Location                       = (2117,1269)\n"
                 "  Owner                          = 0\n"
                 "\n"
                 "Planet(3):\n"
                 "  Location                       = (1847,1882)\n"
                 "  Owner                          = 0\n"
                 "\n"
                 "Planet(4):\n"
                 "  Location                       = (2842,1642)\n"
                 "  Owner                          = 0\n"
                 "\n");
}

AFL_TEST("game.maint.dump.Parsers:parseRaceNames", a)
{
    Environment env(game::test::getDefaultRaceNames());
    game::maint::dump::Parsers::parseRaceNames(env.input, env.output);
    a.checkEqual("result", getResult(env),
                 "Long_names:\n"
                 "  Player(1)                      = 'The Solar Federation'\n"
                 "  Player(2)                      = 'The Lizard Alliance'\n"
                 "  Player(3)                      = 'The Empire of the Birds'\n"
                 "  Player(4)                      = 'The Fascist Empire'\n"
                 "  Player(5)                      = 'The Privateer Bands'\n"
                 "  Player(6)                      = 'The Cyborg'\n"
                 "  Player(7)                      = 'The Crystal Confederation'\n"
                 "  Player(8)                      = 'The Evil Empire'\n"
                 "  Player(9)                      = 'The Robotic Imperium'\n"
                 "  Player(10)                     = 'The Rebel Confederation'\n"
                 "  Player(11)                     = 'The Missing Colonies of Man'\n"
                 "\n"
                 "Short_names:\n"
                 "  Player(1)                      = 'The Feds'\n"
                 "  Player(2)                      = 'The Lizards'\n"
                 "  Player(3)                      = 'The Bird Men'\n"
                 "  Player(4)                      = 'The Fascists'\n"
                 "  Player(5)                      = 'The Privateers'\n"
                 "  Player(6)                      = 'The Cyborg'\n"
                 "  Player(7)                      = 'The Crystal People'\n"
                 "  Player(8)                      = 'The Evil Empire'\n"
                 "  Player(9)                      = 'The Robots'\n"
                 "  Player(10)                     = 'The Rebels'\n"
                 "  Player(11)                     = 'The Colonies'\n"
                 "\n"
                 "Adjectives:\n"
                 "  Player(1)                      = 'Fed'\n"
                 "  Player(2)                      = 'Lizard'\n"
                 "  Player(3)                      = 'Bird Man'\n"
                 "  Player(4)                      = 'Fascist'\n"
                 "  Player(5)                      = 'Privateer'\n"
                 "  Player(6)                      = 'Cyborg'\n"
                 "  Player(7)                      = 'Crystalline'\n"
                 "  Player(8)                      = 'Empire'\n"
                 "  Player(9)                      = 'Robotic'\n"
                 "  Player(10)                     = 'Rebel'\n"
                 "  Player(11)                     = 'Colonial'\n"
                 "\n");
}

AFL_TEST("game.maint.dump.Parsers:parseTeams", a)
{
    static const uint8_t FILE[] = {
        0x43, 0x43, 0x74, 0x65, 0x61, 0x6d, 0x30, 0x1a, 0x03, 0x00, 0x01, 0x07, 0x01, 0x07, 0x0b, 0x01,
        0x07, 0x01, 0x01, 0x01, 0x0b, 0x0c, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x03, 0x04, 0x04, 0x04,
        0x04, 0x04, 0x0b, 0x6e, 0x6f, 0x6e, 0x2d, 0x70, 0x6c, 0x61, 0x79, 0x65, 0x72, 0x73, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x03, 0x77, 0x69, 0x72, 0x00, 0x00, 0x00, 0x03, 0x64, 0x69, 0x65, 0x00, 0x00,
        0x09, 0x00, 0x09, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x00,
        0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    Environment env(FILE);
    game::maint::dump::Parsers::parseTeams(env.input, env.output);
    a.checkEqual("result", getResult(env),
                 "PCC_Teams:\n"
                 "  Flags                          = 3\n"
                 "  Team(1)                        = 1\n"
                 "  Team(2)                        = 7\n"
                 "  Team(3)                        = 1\n"
                 "  Team(4)                        = 7\n"
                 "  Team(5)                        = 11\n"
                 "  Team(6)                        = 1\n"
                 "  Team(7)                        = 7\n"
                 "  Team(8)                        = 1\n"
                 "  Team(9)                        = 1\n"
                 "  Team(10)                       = 1\n"
                 "  Team(11)                       = 11\n"
                 "  Team(12)                       = 12\n"
                 "  Color(1)                       = 4\n"
                 "  Color(2)                       = 4\n"
                 "  Color(3)                       = 4\n"
                 "  Color(4)                       = 4\n"
                 "  Color(5)                       = 4\n"
                 "  Color(6)                       = 4\n"
                 "  Color(7)                       = 3\n"
                 "  Color(8)                       = 4\n"
                 "  Color(9)                       = 4\n"
                 "  Color(10)                      = 4\n"
                 "  Color(11)                      = 4\n"
                 "  Color(12)                      = 4\n"
                 "  Name(1)                        = 'non-players'\n"
                 "  Name(2)                        = ''\n"
                 "  Name(3)                        = ''\n"
                 "  Name(4)                        = ''\n"
                 "  Name(5)                        = ''\n"
                 "  Name(6)                        = ''\n"
                 "  Name(7)                        = 'wir'\n"
                 "  Name(8)                        = ''\n"
                 "  Name(9)                        = ''\n"
                 "  Name(10)                       = ''\n"
                 "  Name(11)                       = 'die'\n"
                 "  Name(12)                       = ''\n"
                 "  Send_config(1)                 = 0\n"
                 "  Send_config(2)                 = 9\n"
                 "  Send_config(3)                 = 0\n"
                 "  Send_config(4)                 = 9\n"
                 "  Send_config(5)                 = 0\n"
                 "  Send_config(6)                 = 0\n"
                 "  Send_config(7)                 = 9\n"
                 "  Send_config(8)                 = 0\n"
                 "  Send_config(9)                 = 0\n"
                 "  Send_config(10)                = 0\n"
                 "  Send_config(11)                = 0\n"
                 "  Receive_config(1)              = 0\n"
                 "  Receive_config(2)              = 15\n"
                 "  Receive_config(3)              = 0\n"
                 "  Receive_config(4)              = 15\n"
                 "  Receive_config(5)              = 0\n"
                 "  Receive_config(6)              = 0\n"
                 "  Receive_config(7)              = 15\n"
                 "  Receive_config(8)              = 0\n"
                 "  Receive_config(9)              = 0\n"
                 "  Receive_config(10)             = 0\n"
                 "  Receive_config(11)             = 0\n"
                 "  Passcode                       = 0\n"
                 "\n");
}

