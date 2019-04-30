/**
  *  \file u/t_game_v3_resultloader.cpp
  *  \brief Test for game::v3::ResultLoader
  */

#include "game/v3/resultloader.hpp"

#include "t_game_v3.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/root.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/specificationloader.hpp"
#include "game/test/stringverifier.hpp"
#include "game/turn.hpp"
#include "game/v3/command.hpp"
#include "game/v3/commandcontainer.hpp"
#include "game/v3/commandextra.hpp"

namespace {
    struct TestHarness {
        // Environment [ResultLoader]
        afl::base::Ref<afl::io::InternalDirectory> dir;
        afl::string::NullTranslator tx;
        afl::sys::Log log;
        afl::io::NullFileSystem fs;
        game::v3::DirectoryScanner scanner;

        // Testee [ResultLoader]
        game::v3::ResultLoader testee;

        // Environment [loadTurnfile]
        game::Root root;
        game::Turn turn;

        TestHarness()
            : dir(afl::io::InternalDirectory::create("spec")),
              tx(), log(), fs(),
              scanner(*dir, tx, log),
              testee(dir, dir, std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()), tx, log, scanner, fs),
              root(dir, *new game::test::SpecificationLoader(),
                   game::HostVersion(),
                   std::auto_ptr<game::RegistrationKey>(new game::test::RegistrationKey(game::RegistrationKey::Unregistered, 5)),
                   std::auto_ptr<game::StringVerifier>(new game::test::StringVerifier()),
                   std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()),
                   game::Root::Actions_t()),
              turn()
            { }
    };

    /* Player number used for generating the following turn files */
    const int PLAYER = 7;

    /* perl -I../c2systest -Mc2service -e 'print c2service::vp_make_turn(7, "22-33-4444:55:66:77", pack("vvA3", 1, 9, "abc"))' | xxd -i
       Single ShipChangeFc(9) command */
    const uint8_t SHIP_TURN[] = {
        0x07, 0x00, 0x01, 0x00, 0x00, 0x00, 0x32, 0x32, 0x2d, 0x33, 0x33, 0x2d, 0x34, 0x34, 0x34, 0x34, 0x3a, 0x35, 0x35, 0x3a, 0x36, 0x36, 0x3a, 0x37,
        0x00, 0x00, 0xaf, 0x03, 0x78, 0x22, 0x00, 0x00, 0x00, 0x01, 0x00, 0x09, 0x00, 0x61, 0x62, 0x63, 0x4d, 0x11, 0x00, 0x00, 0x2a, 0x00, 0x00, 0x00,
        0xcb, 0x02, 0x00, 0x00, 0x40, 0x03, 0x00, 0x00, 0xe0, 0x04, 0x00, 0x00, 0x80, 0x06, 0x00, 0x00, 0x20, 0x08, 0x00, 0x00, 0xc0, 0x09, 0x00, 0x00,
        0x60, 0x0b, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0xa0, 0x0e, 0x00, 0x00, 0x40, 0x10, 0x00, 0x00, 0xe0, 0x11, 0x00, 0x00, 0x80, 0x13, 0x00, 0x00,
        0x20, 0x15, 0x00, 0x00, 0xc0, 0x16, 0x00, 0x00, 0x60, 0x18, 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00, 0xa0, 0x1b, 0x00, 0x00, 0x40, 0x1d, 0x00, 0x00,
        0xe0, 0x1e, 0x00, 0x00, 0x80, 0x20, 0x00, 0x00, 0x20, 0x22, 0x00, 0x00, 0xc0, 0x23, 0x00, 0x00, 0x60, 0x25, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00,
        0xa0, 0x28, 0x00, 0x00, 0xa0, 0x01, 0x00, 0x00, 0x40, 0x03, 0x00, 0x00, 0xe0, 0x04, 0x00, 0x00, 0x80, 0x06, 0x00, 0x00, 0x20, 0x08, 0x00, 0x00,
        0xc0, 0x09, 0x00, 0x00, 0x60, 0x0b, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0xa0, 0x0e, 0x00, 0x00, 0x40, 0x10, 0x00, 0x00, 0xe0, 0x11, 0x00, 0x00,
        0x80, 0x13, 0x00, 0x00, 0x20, 0x15, 0x00, 0x00, 0xc0, 0x16, 0x00, 0x00, 0x60, 0x18, 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00, 0xa0, 0x1b, 0x00, 0x00,
        0x40, 0x1d, 0x00, 0x00, 0xe0, 0x1e, 0x00, 0x00, 0x80, 0x20, 0x00, 0x00, 0x20, 0x22, 0x00, 0x00, 0xc0, 0x23, 0x00, 0x00, 0x60, 0x25, 0x00, 0x00,
        0x00, 0x27, 0x00, 0x00, 0xa0, 0x28, 0x00, 0x00, 0x07, 0x24, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x4d, 0x11, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
        0x0a, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00
    };

    /* perl -I../c2systest -Mc2service -e 'print c2service::vp_make_turn(7, "22-33-4444:55:66:77", pack("v3", 32, 270, 5))' | xxd -i
       Single PlanetColonistTax(270) command */
    const uint8_t PLANET_TURN[] = {
        0x07, 0x00, 0x01, 0x00, 0x00, 0x00, 0x32, 0x32, 0x2d, 0x33, 0x33, 0x2d, 0x34, 0x34, 0x34, 0x34, 0x3a, 0x35, 0x35, 0x3a, 0x36, 0x36, 0x3a, 0x37,
        0x00, 0x00, 0xaf, 0x03, 0x78, 0x22, 0x00, 0x00, 0x00, 0x20, 0x00, 0x0e, 0x01, 0x05, 0x00, 0x51, 0x10, 0x00, 0x00, 0x2a, 0x00, 0x00, 0x00, 0xcb,
        0x02, 0x00, 0x00, 0x40, 0x03, 0x00, 0x00, 0xe0, 0x04, 0x00, 0x00, 0x80, 0x06, 0x00, 0x00, 0x20, 0x08, 0x00, 0x00, 0xc0, 0x09, 0x00, 0x00, 0x60,
        0x0b, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0xa0, 0x0e, 0x00, 0x00, 0x40, 0x10, 0x00, 0x00, 0xe0, 0x11, 0x00, 0x00, 0x80, 0x13, 0x00, 0x00, 0x20,
        0x15, 0x00, 0x00, 0xc0, 0x16, 0x00, 0x00, 0x60, 0x18, 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00, 0xa0, 0x1b, 0x00, 0x00, 0x40, 0x1d, 0x00, 0x00, 0xe0,
        0x1e, 0x00, 0x00, 0x80, 0x20, 0x00, 0x00, 0x20, 0x22, 0x00, 0x00, 0xc0, 0x23, 0x00, 0x00, 0x60, 0x25, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00, 0xa0,
        0x28, 0x00, 0x00, 0xa0, 0x01, 0x00, 0x00, 0x40, 0x03, 0x00, 0x00, 0xe0, 0x04, 0x00, 0x00, 0x80, 0x06, 0x00, 0x00, 0x20, 0x08, 0x00, 0x00, 0xc0,
        0x09, 0x00, 0x00, 0x60, 0x0b, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0xa0, 0x0e, 0x00, 0x00, 0x40, 0x10, 0x00, 0x00, 0xe0, 0x11, 0x00, 0x00, 0x80,
        0x13, 0x00, 0x00, 0x20, 0x15, 0x00, 0x00, 0xc0, 0x16, 0x00, 0x00, 0x60, 0x18, 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00, 0xa0, 0x1b, 0x00, 0x00, 0x40,
        0x1d, 0x00, 0x00, 0xe0, 0x1e, 0x00, 0x00, 0x80, 0x20, 0x00, 0x00, 0x20, 0x22, 0x00, 0x00, 0xc0, 0x23, 0x00, 0x00, 0x60, 0x25, 0x00, 0x00, 0x00,
        0x27, 0x00, 0x00, 0xa0, 0x28, 0x00, 0x00, 0x07, 0x24, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04,
        0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x51, 0x10, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x0a,
        0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00
    };

    /* perl perl -I../c2systest -Mc2service -e 'print c2service::vp_make_turn(7, "22-33-4444:55:66:77", pack("v3", 52, 400, 3))' | xxd -i
       Single BaseChangeMission(400) command */
    const uint8_t BASE_TURN[] = {
        0x07, 0x00, 0x01, 0x00, 0x00, 0x00, 0x32, 0x32, 0x2d, 0x33, 0x33, 0x2d, 0x34, 0x34, 0x34, 0x34, 0x3a, 0x35, 0x35, 0x3a, 0x36, 0x36, 0x3a, 0x37,
        0x00, 0x00, 0xaf, 0x03, 0x78, 0x22, 0x00, 0x00, 0x00, 0x34, 0x00, 0x90, 0x01, 0x03, 0x00, 0xe5, 0x10, 0x00, 0x00, 0x2a, 0x00, 0x00, 0x00, 0xcb,
        0x02, 0x00, 0x00, 0x40, 0x03, 0x00, 0x00, 0xe0, 0x04, 0x00, 0x00, 0x80, 0x06, 0x00, 0x00, 0x20, 0x08, 0x00, 0x00, 0xc0, 0x09, 0x00, 0x00, 0x60,
        0x0b, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0xa0, 0x0e, 0x00, 0x00, 0x40, 0x10, 0x00, 0x00, 0xe0, 0x11, 0x00, 0x00, 0x80, 0x13, 0x00, 0x00, 0x20,
        0x15, 0x00, 0x00, 0xc0, 0x16, 0x00, 0x00, 0x60, 0x18, 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00, 0xa0, 0x1b, 0x00, 0x00, 0x40, 0x1d, 0x00, 0x00, 0xe0,
        0x1e, 0x00, 0x00, 0x80, 0x20, 0x00, 0x00, 0x20, 0x22, 0x00, 0x00, 0xc0, 0x23, 0x00, 0x00, 0x60, 0x25, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00, 0xa0,
        0x28, 0x00, 0x00, 0xa0, 0x01, 0x00, 0x00, 0x40, 0x03, 0x00, 0x00, 0xe0, 0x04, 0x00, 0x00, 0x80, 0x06, 0x00, 0x00, 0x20, 0x08, 0x00, 0x00, 0xc0,
        0x09, 0x00, 0x00, 0x60, 0x0b, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0xa0, 0x0e, 0x00, 0x00, 0x40, 0x10, 0x00, 0x00, 0xe0, 0x11, 0x00, 0x00, 0x80,
        0x13, 0x00, 0x00, 0x20, 0x15, 0x00, 0x00, 0xc0, 0x16, 0x00, 0x00, 0x60, 0x18, 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00, 0xa0, 0x1b, 0x00, 0x00, 0x40,
        0x1d, 0x00, 0x00, 0xe0, 0x1e, 0x00, 0x00, 0x80, 0x20, 0x00, 0x00, 0x20, 0x22, 0x00, 0x00, 0xc0, 0x23, 0x00, 0x00, 0x60, 0x25, 0x00, 0x00, 0x00,
        0x27, 0x00, 0x00, 0xa0, 0x28, 0x00, 0x00, 0x07, 0x24, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04,
        0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0xe5, 0x10, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x0a,
        0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00
    };

    /* perl -I../c2systest -Mc2service -e 'print c2service::vp_make_turn(7, "22-33-4444:55:66:77", pack("vvA3", 1, 9, "abc"), pack("v3", 32, 270, 5), pack("v3", 52, 400, 3))'  | xxd -i
       This is ShipChangeFc(9,"abc"), PlanetColonistTax(270,5), BaseChangeMission(400,3) */
    const uint8_t THREE_COMMAND_TURN[] = {
        0x07, 0x00, 0x03, 0x00, 0x00, 0x00, 0x32, 0x32, 0x2d, 0x33, 0x33, 0x2d, 0x34, 0x34, 0x34, 0x34, 0x3a, 0x35, 0x35, 0x3a, 0x36, 0x36, 0x3a, 0x37,
        0x00, 0x00, 0xaf, 0x03, 0x78, 0x2a, 0x00, 0x00, 0x00, 0x31, 0x00, 0x00, 0x00, 0x37, 0x00, 0x00, 0x00, 0x01, 0x00, 0x09, 0x00, 0x61, 0x62, 0x63,
        0x20, 0x00, 0x0e, 0x01, 0x05, 0x00, 0x34, 0x00, 0x90, 0x01, 0x03, 0x00, 0xbb, 0x12, 0x00, 0x00, 0x2a, 0x00, 0x00, 0x00, 0xcb, 0x02, 0x00, 0x00,
        0x40, 0x03, 0x00, 0x00, 0xe0, 0x04, 0x00, 0x00, 0x80, 0x06, 0x00, 0x00, 0x20, 0x08, 0x00, 0x00, 0xc0, 0x09, 0x00, 0x00, 0x60, 0x0b, 0x00, 0x00,
        0x00, 0x0d, 0x00, 0x00, 0xa0, 0x0e, 0x00, 0x00, 0x40, 0x10, 0x00, 0x00, 0xe0, 0x11, 0x00, 0x00, 0x80, 0x13, 0x00, 0x00, 0x20, 0x15, 0x00, 0x00,
        0xc0, 0x16, 0x00, 0x00, 0x60, 0x18, 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00, 0xa0, 0x1b, 0x00, 0x00, 0x40, 0x1d, 0x00, 0x00, 0xe0, 0x1e, 0x00, 0x00,
        0x80, 0x20, 0x00, 0x00, 0x20, 0x22, 0x00, 0x00, 0xc0, 0x23, 0x00, 0x00, 0x60, 0x25, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00, 0xa0, 0x28, 0x00, 0x00,
        0xa0, 0x01, 0x00, 0x00, 0x40, 0x03, 0x00, 0x00, 0xe0, 0x04, 0x00, 0x00, 0x80, 0x06, 0x00, 0x00, 0x20, 0x08, 0x00, 0x00, 0xc0, 0x09, 0x00, 0x00,
        0x60, 0x0b, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0xa0, 0x0e, 0x00, 0x00, 0x40, 0x10, 0x00, 0x00, 0xe0, 0x11, 0x00, 0x00, 0x80, 0x13, 0x00, 0x00,
        0x20, 0x15, 0x00, 0x00, 0xc0, 0x16, 0x00, 0x00, 0x60, 0x18, 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00, 0xa0, 0x1b, 0x00, 0x00, 0x40, 0x1d, 0x00, 0x00,
        0xe0, 0x1e, 0x00, 0x00, 0x80, 0x20, 0x00, 0x00, 0x20, 0x22, 0x00, 0x00, 0xc0, 0x23, 0x00, 0x00, 0x60, 0x25, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00,
        0xa0, 0x28, 0x00, 0x00, 0x07, 0x24, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
        0x05, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0xbb, 0x12, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00,
        0x0b, 0x00, 0x00, 0x00
    };

    /* perl -I../c2systest -Mc2service -e 'print c2service::vp_make_turn(7, "22-33-4444:55:66:77", pack("vvA3", 1, 9, "abc"), pack("vvA3", 1, 9, "ff3"), pack("vvA3", 1, 9, "ee4"), pack("vvA3", 1, 9, "ghi"))' | xxd -i
       This is ShipChangeFc(9, "abc"), ..."ff3", "ee4", "ghi" */
    const uint8_t ALLIES_COMMAND_TURN[] = {
        0x07, 0x00, 0x04, 0x00, 0x00, 0x00, 0x32, 0x32, 0x2d, 0x33, 0x33, 0x2d, 0x34, 0x34, 0x34, 0x34, 0x3a, 0x35, 0x35, 0x3a, 0x36, 0x36, 0x3a, 0x37,
        0x00, 0x00, 0xaf, 0x03, 0x78, 0x2e, 0x00, 0x00, 0x00, 0x35, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x43, 0x00, 0x00, 0x00, 0x01, 0x00, 0x09,
        0x00, 0x61, 0x62, 0x63, 0x01, 0x00, 0x09, 0x00, 0x66, 0x66, 0x33, 0x01, 0x00, 0x09, 0x00, 0x65, 0x65, 0x34, 0x01, 0x00, 0x09, 0x00, 0x67, 0x68,
        0x69, 0x63, 0x15, 0x00, 0x00, 0x2a, 0x00, 0x00, 0x00, 0xcb, 0x02, 0x00, 0x00, 0x40, 0x03, 0x00, 0x00, 0xe0, 0x04, 0x00, 0x00, 0x80, 0x06, 0x00,
        0x00, 0x20, 0x08, 0x00, 0x00, 0xc0, 0x09, 0x00, 0x00, 0x60, 0x0b, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0xa0, 0x0e, 0x00, 0x00, 0x40, 0x10, 0x00,
        0x00, 0xe0, 0x11, 0x00, 0x00, 0x80, 0x13, 0x00, 0x00, 0x20, 0x15, 0x00, 0x00, 0xc0, 0x16, 0x00, 0x00, 0x60, 0x18, 0x00, 0x00, 0x00, 0x1a, 0x00,
        0x00, 0xa0, 0x1b, 0x00, 0x00, 0x40, 0x1d, 0x00, 0x00, 0xe0, 0x1e, 0x00, 0x00, 0x80, 0x20, 0x00, 0x00, 0x20, 0x22, 0x00, 0x00, 0xc0, 0x23, 0x00,
        0x00, 0x60, 0x25, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00, 0xa0, 0x28, 0x00, 0x00, 0xa0, 0x01, 0x00, 0x00, 0x40, 0x03, 0x00, 0x00, 0xe0, 0x04, 0x00,
        0x00, 0x80, 0x06, 0x00, 0x00, 0x20, 0x08, 0x00, 0x00, 0xc0, 0x09, 0x00, 0x00, 0x60, 0x0b, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0xa0, 0x0e, 0x00,
        0x00, 0x40, 0x10, 0x00, 0x00, 0xe0, 0x11, 0x00, 0x00, 0x80, 0x13, 0x00, 0x00, 0x20, 0x15, 0x00, 0x00, 0xc0, 0x16, 0x00, 0x00, 0x60, 0x18, 0x00,
        0x00, 0x00, 0x1a, 0x00, 0x00, 0xa0, 0x1b, 0x00, 0x00, 0x40, 0x1d, 0x00, 0x00, 0xe0, 0x1e, 0x00, 0x00, 0x80, 0x20, 0x00, 0x00, 0x20, 0x22, 0x00,
        0x00, 0xc0, 0x23, 0x00, 0x00, 0x60, 0x25, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00, 0xa0, 0x28, 0x00, 0x00, 0x07, 0x24, 0x04, 0x00, 0x01, 0x00, 0x00,
        0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x63, 0x15, 0x00,
        0x00, 0x08, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00
    };

    /* perl -I../c2systest -Mc2service -e 'print c2service::vp_make_turn(7, "22-33-4444:55:66:77", pack("vvvA3", 60, 3, 7, 11, "nop"))' | xxd -i
       This is SendMessage(7,11,"abc") */
    const uint8_t MESSAGE_COMMAND_TURN[] = {
        0x07, 0x00, 0x01, 0x00, 0x00, 0x00, 0x32, 0x32, 0x2d, 0x33, 0x33, 0x2d, 0x34, 0x34, 0x34, 0x34, 0x3a, 0x35, 0x35, 0x3a, 0x36, 0x36, 0x3a, 0x37,
        0x00, 0x00, 0xaf, 0x03, 0x78, 0x22, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x03, 0x00, 0x07, 0x00, 0x0b, 0x00, 0x6e, 0x6f, 0x70, 0xbb, 0x11, 0x00, 0x00,
        0x2a, 0x00, 0x00, 0x00, 0xcb, 0x02, 0x00, 0x00, 0x40, 0x03, 0x00, 0x00, 0xe0, 0x04, 0x00, 0x00, 0x80, 0x06, 0x00, 0x00, 0x20, 0x08, 0x00, 0x00,
        0xc0, 0x09, 0x00, 0x00, 0x60, 0x0b, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0xa0, 0x0e, 0x00, 0x00, 0x40, 0x10, 0x00, 0x00, 0xe0, 0x11, 0x00, 0x00,
        0x80, 0x13, 0x00, 0x00, 0x20, 0x15, 0x00, 0x00, 0xc0, 0x16, 0x00, 0x00, 0x60, 0x18, 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00, 0xa0, 0x1b, 0x00, 0x00,
        0x40, 0x1d, 0x00, 0x00, 0xe0, 0x1e, 0x00, 0x00, 0x80, 0x20, 0x00, 0x00, 0x20, 0x22, 0x00, 0x00, 0xc0, 0x23, 0x00, 0x00, 0x60, 0x25, 0x00, 0x00,
        0x00, 0x27, 0x00, 0x00, 0xa0, 0x28, 0x00, 0x00, 0xa0, 0x01, 0x00, 0x00, 0x40, 0x03, 0x00, 0x00, 0xe0, 0x04, 0x00, 0x00, 0x80, 0x06, 0x00, 0x00,
        0x20, 0x08, 0x00, 0x00, 0xc0, 0x09, 0x00, 0x00, 0x60, 0x0b, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0xa0, 0x0e, 0x00, 0x00, 0x40, 0x10, 0x00, 0x00,
        0xe0, 0x11, 0x00, 0x00, 0x80, 0x13, 0x00, 0x00, 0x20, 0x15, 0x00, 0x00, 0xc0, 0x16, 0x00, 0x00, 0x60, 0x18, 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00,
        0xa0, 0x1b, 0x00, 0x00, 0x40, 0x1d, 0x00, 0x00, 0xe0, 0x1e, 0x00, 0x00, 0x80, 0x20, 0x00, 0x00, 0x20, 0x22, 0x00, 0x00, 0xc0, 0x23, 0x00, 0x00,
        0x60, 0x25, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00, 0xa0, 0x28, 0x00, 0x00, 0x07, 0x24, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0xbb, 0x11, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
        0x09, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00
    };

    /* perl -I../c2systest -Mc2service -e 'print c2service::vp_make_turn(7, "22-33-4444:55:66:77", pack("v*", 100, 1,2,3))' | xxd -i
       100 is an invalid command */
    const uint8_t INVALID_COMMAND_TURN[] = {
        0x07, 0x00, 0x01, 0x00, 0x00, 0x00, 0x32, 0x32, 0x2d, 0x33, 0x33, 0x2d, 0x34, 0x34, 0x34, 0x34, 0x3a, 0x35, 0x35, 0x3a, 0x36, 0x36, 0x3a, 0x37,
        0x00, 0x00, 0xaf, 0x03, 0x78, 0x22, 0x00, 0x00, 0x00, 0x64, 0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x87, 0x10, 0x00, 0x00, 0x2a, 0x00, 0x00,
        0x00, 0xcb, 0x02, 0x00, 0x00, 0x40, 0x03, 0x00, 0x00, 0xe0, 0x04, 0x00, 0x00, 0x80, 0x06, 0x00, 0x00, 0x20, 0x08, 0x00, 0x00, 0xc0, 0x09, 0x00,
        0x00, 0x60, 0x0b, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0xa0, 0x0e, 0x00, 0x00, 0x40, 0x10, 0x00, 0x00, 0xe0, 0x11, 0x00, 0x00, 0x80, 0x13, 0x00,
        0x00, 0x20, 0x15, 0x00, 0x00, 0xc0, 0x16, 0x00, 0x00, 0x60, 0x18, 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00, 0xa0, 0x1b, 0x00, 0x00, 0x40, 0x1d, 0x00,
        0x00, 0xe0, 0x1e, 0x00, 0x00, 0x80, 0x20, 0x00, 0x00, 0x20, 0x22, 0x00, 0x00, 0xc0, 0x23, 0x00, 0x00, 0x60, 0x25, 0x00, 0x00, 0x00, 0x27, 0x00,
        0x00, 0xa0, 0x28, 0x00, 0x00, 0xa0, 0x01, 0x00, 0x00, 0x40, 0x03, 0x00, 0x00, 0xe0, 0x04, 0x00, 0x00, 0x80, 0x06, 0x00, 0x00, 0x20, 0x08, 0x00,
        0x00, 0xc0, 0x09, 0x00, 0x00, 0x60, 0x0b, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0xa0, 0x0e, 0x00, 0x00, 0x40, 0x10, 0x00, 0x00, 0xe0, 0x11, 0x00,
        0x00, 0x80, 0x13, 0x00, 0x00, 0x20, 0x15, 0x00, 0x00, 0xc0, 0x16, 0x00, 0x00, 0x60, 0x18, 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00, 0xa0, 0x1b, 0x00,
        0x00, 0x40, 0x1d, 0x00, 0x00, 0xe0, 0x1e, 0x00, 0x00, 0x80, 0x20, 0x00, 0x00, 0x20, 0x22, 0x00, 0x00, 0xc0, 0x23, 0x00, 0x00, 0x60, 0x25, 0x00,
        0x00, 0x00, 0x27, 0x00, 0x00, 0xa0, 0x28, 0x00, 0x00, 0x07, 0x24, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
        0x00, 0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x87, 0x10, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00,
        0x00, 0x0a, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00
    };
}


/** Test loadTurnfile(), success case.
    Prepare a universe with three objects.
    Load a turn file refering to the three objects.
    Load must succeed and update the objects. */
void
TestGameV3ResultLoader::testLoadTurnFile()
{
    // Prepare
    TestHarness h;
    {
        game::map::Ship* p = h.turn.universe().ships().create(9);
        game::map::ShipData sd;
        sd.friendlyCode = "xyz";
        sd.owner = PLAYER;
        p->addCurrentShipData(sd, game::PlayerSet_t(PLAYER));
        p->setPlayability(game::map::Object::Playable);
    }
    {
        game::map::Planet* p = h.turn.universe().planets().create(270);
        game::map::PlanetData pd;
        pd.friendlyCode = "xyz";
        pd.owner = PLAYER;
        pd.colonistTax = 12;
        p->addCurrentPlanetData(pd, game::PlayerSet_t(PLAYER));
        p->setPlayability(game::map::Object::Playable);
    }
    {
        game::map::Planet* p = h.turn.universe().planets().create(400);
        game::map::PlanetData pd;
        pd.friendlyCode = "qqq";
        pd.owner = PLAYER;
        pd.colonistTax = 12;
        game::map::BaseData bd;
        bd.mission = 1;
        p->addCurrentPlanetData(pd, game::PlayerSet_t(PLAYER));
        p->addCurrentBaseData(bd, game::PlayerSet_t(PLAYER));
        p->setPlayability(game::map::Object::Playable);
    }
    TS_ASSERT_EQUALS(h.turn.universe().ships().get(9)->getFriendlyCode().orElse(""), "xyz");
    TS_ASSERT_EQUALS(h.turn.universe().planets().get(270)->getColonistTax().orElse(0), 12);
    TS_ASSERT_EQUALS(h.turn.universe().planets().get(400)->getBaseMission().orElse(0), 1);

    // File to test
    afl::io::ConstMemoryStream file(THREE_COMMAND_TURN);

    // Test it
    h.testee.loadTurnfile(h.turn, h.root, file, PLAYER);

    // Verify result
    TS_ASSERT_EQUALS(h.turn.universe().ships().get(9)->getFriendlyCode().orElse(""), "abc");
    TS_ASSERT_EQUALS(h.turn.universe().planets().get(270)->getColonistTax().orElse(0), 5);
    TS_ASSERT_EQUALS(h.turn.universe().planets().get(400)->getBaseMission().orElse(0), 3);
}

/** Test loadTurnfile(), failure case: missing ship.
    Prepare an empty universe.
    Loading a turn refering to a ship must fail. */
void
TestGameV3ResultLoader::testMissingShip()
{
    TestHarness h;
    afl::io::ConstMemoryStream file(SHIP_TURN);
    TS_ASSERT_THROWS(h.testee.loadTurnfile(h.turn, h.root, file, PLAYER), afl::except::FileFormatException);
}

/** Test loadTurnfile(), failure case: missing planet.
    Prepare an empty universe.
    Loading a turn refering to a planet must fail. */
void
TestGameV3ResultLoader::testMissingPlanet()
{
    TestHarness h;
    afl::io::ConstMemoryStream file(PLANET_TURN);
    TS_ASSERT_THROWS(h.testee.loadTurnfile(h.turn, h.root, file, PLAYER), afl::except::FileFormatException);
}

/** Test loadTurnfile(), failure case: missing base.
    Prepare an empty universe.
    Loading a turn refering to a base must fail. */
void
TestGameV3ResultLoader::testMissingBase()
{
    TestHarness h;
    afl::io::ConstMemoryStream file(BASE_TURN);
    TS_ASSERT_THROWS(h.testee.loadTurnfile(h.turn, h.root, file, PLAYER), afl::except::FileFormatException);
}

/** Test loadTurnfile(), failure case: ship present but not played.
    Prepare a universe containing an unplayed ship.
    Loading a turn refering to that ship must fail. */
void
TestGameV3ResultLoader::testUnplayedShip()
{
    TestHarness h;
    h.turn.universe().ships().create(9);
    afl::io::ConstMemoryStream file(SHIP_TURN);
    TS_ASSERT_THROWS(h.testee.loadTurnfile(h.turn, h.root, file, PLAYER), afl::except::FileFormatException);
}

/** Test loadTurnfile(), failure case: planet present but not played.
    Prepare a universe containing an unplayed planet.
    Loading a turn refering to that planet must fail. */
void
TestGameV3ResultLoader::testUnplayedPlanet()
{
    TestHarness h;
    h.turn.universe().planets().create(270);
    afl::io::ConstMemoryStream file(PLANET_TURN);
    TS_ASSERT_THROWS(h.testee.loadTurnfile(h.turn, h.root, file, PLAYER), afl::except::FileFormatException);
}

/** Test loadTurnfile(), failure case: base present but not played.
    Prepare a universe containing an unplayed planet.
    Loading a turn refering to that planet as a base must fail. */
void
TestGameV3ResultLoader::testUnplayedBase()
{
    TestHarness h;
    h.turn.universe().planets().create(400);
    afl::io::ConstMemoryStream file(BASE_TURN);
    TS_ASSERT_THROWS(h.testee.loadTurnfile(h.turn, h.root, file, PLAYER), afl::except::FileFormatException);
}

/** Test loadTurnfile(), failure case: planet played but has no base.
    Prepare a universe containing a played planet without base.
    Loading a turn refering to that planet as a base must fail. */
void
TestGameV3ResultLoader::testNoBase()
{
    TestHarness h;
    {
        game::map::Planet* p = h.turn.universe().planets().create(400);
        game::map::PlanetData pd;
        pd.friendlyCode = "qqq";
        pd.owner = PLAYER;
        pd.colonistTax = 12;
        p->addCurrentPlanetData(pd, game::PlayerSet_t(PLAYER));
        p->setPlayability(game::map::Object::Playable);
    }
    afl::io::ConstMemoryStream file(BASE_TURN);
    TS_ASSERT_THROWS(h.testee.loadTurnfile(h.turn, h.root, file, PLAYER), afl::except::FileFormatException);
}

/** Test loadTurnfile(), failure case: invalid command.
    Loading a turn containing an invalid command must fail. */
void
TestGameV3ResultLoader::testInvalidCommand()
{
    TestHarness h;
    afl::io::ConstMemoryStream file(INVALID_COMMAND_TURN);
    TS_ASSERT_THROWS(h.testee.loadTurnfile(h.turn, h.root, file, PLAYER), afl::except::FileFormatException);
}

/** Test loadTurnfile(), failure case: invalid file.
    Loading an invalid turn file (not a turn file) must fail. */
void
TestGameV3ResultLoader::testInvalidFile()
{
    TestHarness h;
    afl::io::ConstMemoryStream file(afl::base::Nothing);
    TS_ASSERT_THROWS(h.testee.loadTurnfile(h.turn, h.root, file, PLAYER), afl::except::FileFormatException);
}

/** Test loadTurnfile(), failure case: invalid player.
    Loading an turn file belonging to a different player must fail. */
void
TestGameV3ResultLoader::testInvalidPlayer()
{
    // Different player than turn image!
    const int PLAYER_HERE = PLAYER-1;

    // Prepare
    TestHarness h;
    {
        game::map::Ship* p = h.turn.universe().ships().create(9);
        game::map::ShipData sd;
        sd.friendlyCode = "xyz";
        sd.owner = PLAYER;
        p->addCurrentShipData(sd, game::PlayerSet_t(PLAYER_HERE));
        p->setPlayability(game::map::Object::Playable);
    }

    // File to test
    afl::io::ConstMemoryStream file(SHIP_TURN);

    // Test it
    TS_ASSERT_THROWS(h.testee.loadTurnfile(h.turn, h.root, file, PLAYER_HERE), afl::except::FileFormatException);
}

/** Test loadTurnfile(), success case, alliance command.
    Prepare universe with one ship.
    Loading a turn file containing multiple friendly code commands for that ship must produce a command. */
void
TestGameV3ResultLoader::testAllianceCommand()
{
    // Prepare
    TestHarness h;
    {
        game::map::Ship* p = h.turn.universe().ships().create(9);
        game::map::ShipData sd;
        sd.friendlyCode = "xyz";
        sd.owner = PLAYER;
        p->addCurrentShipData(sd, game::PlayerSet_t(PLAYER));
        p->setPlayability(game::map::Object::Playable);
    }
    TS_ASSERT_EQUALS(h.turn.universe().ships().get(9)->getFriendlyCode().orElse(""), "xyz");

    // File to test
    afl::io::ConstMemoryStream file(ALLIES_COMMAND_TURN);

    // Test it
    h.testee.loadTurnfile(h.turn, h.root, file, PLAYER);

    // Verify result:
    // - last command wins
    TS_ASSERT_EQUALS(h.turn.universe().ships().get(9)->getFriendlyCode().orElse(""), "ghi");
    // - command message
    const game::v3::Command* cmd = game::v3::CommandExtra::create(h.turn).create(PLAYER).getCommand(game::v3::Command::phc_TAlliance, 0);
    TS_ASSERT(cmd != 0);
    TS_ASSERT_EQUALS(cmd->getArg(), "ff3ee4");
}

/** Test loadTurnfile(), success case, message command.
    Loading a turn file containing a SendMessage command must produce an outbox message. */
void
TestGameV3ResultLoader::testMessageCommand()
{
    // Prepare
    TestHarness h;

    // File to test
    afl::io::ConstMemoryStream file(MESSAGE_COMMAND_TURN);

    // Test it
    h.testee.loadTurnfile(h.turn, h.root, file, PLAYER);

    // Verify result
    TS_ASSERT_EQUALS(h.turn.outbox().getNumMessages(), 1U);
    TS_ASSERT_EQUALS(h.turn.outbox().getMessageRawText(0), "abc");
}

