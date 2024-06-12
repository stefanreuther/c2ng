/**
  *  \file test/game/v3/resultloadertest.cpp
  *  \brief Test for game::v3::ResultLoader
  */

#include "game/v3/resultloader.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/root.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/specificationloader.hpp"
#include "game/test/stringverifier.hpp"
#include "game/turn.hpp"

namespace {
    struct TestHarness {
        // Environment [ResultLoader]
        afl::base::Ref<afl::io::InternalDirectory> dir;
        afl::string::NullTranslator tx;
        afl::sys::Log log;
        afl::io::NullFileSystem fs;
        game::v3::DirectoryScanner scanner;
        afl::sys::InternalEnvironment env;
        util::ProfileDirectory profile;

        // Testee [ResultLoader]
        game::v3::ResultLoader testee;

        // Environment [loadTurnfile]
        game::Root root;
        game::Turn turn;

        TestHarness()
            : dir(afl::io::InternalDirectory::create("spec")),
              tx(), log(), fs(),
              scanner(*dir, tx, log),
              env(),
              profile(env, fs),
              testee(dir, dir, std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()), scanner, fs, &profile, 0),
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

    /* The timestamp used above. Yeah I know it's wrong... */
    const uint8_t MOCK_TIMESTAMP[] = {
        0x32, 0x32, 0x2d, 0x33, 0x33, 0x2d, 0x34, 0x34, 0x34, 0x34, 0x3a, 0x35, 0x35, 0x3a, 0x36, 0x36, 0x3a, 0x37
    };
}


/** Test loadTurnfile(), success case.
    Prepare a universe with three objects.
    Load a turn file refering to the three objects.
    Load must succeed and update the objects. */
AFL_TEST("game.v3.ResultLoader:loadTurnfile", a)
{
    // (essentially, the same as "game.v3.Loader:loadTurnfile:success"; this one was earlier.)
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
    a.checkEqual("01. getFriendlyCode", h.turn.universe().ships().get(9)->getFriendlyCode().orElse(""), "xyz");
    a.checkEqual("02. getColonistTax",  h.turn.universe().planets().get(270)->getColonistTax().orElse(0), 12);
    a.checkEqual("03. getBaseMission",  h.turn.universe().planets().get(400)->getBaseMission().orElse(0), 1);
    h.turn.setTimestamp(MOCK_TIMESTAMP);

    // File to test
    afl::io::ConstMemoryStream file(THREE_COMMAND_TURN);

    // Test it
    h.testee.loadTurnfile(h.turn, h.root, file, PLAYER, h.log, h.tx);

    // Verify result
    a.checkEqual("11. getFriendlyCode", h.turn.universe().ships().get(9)->getFriendlyCode().orElse(""), "abc");
    a.checkEqual("12. getColonistTax",  h.turn.universe().planets().get(270)->getColonistTax().orElse(0), 5);
    a.checkEqual("13. getBaseMission",  h.turn.universe().planets().get(400)->getBaseMission().orElse(0), 3);
}

/** Test loadTurnfile(), failure case: invalid file.
    Loading an invalid turn file (not a turn file) must fail. */
AFL_TEST("game.v3.ResultLoader:error:invalid-file", a)
{
    // (essentially, the same as TestGameV3Loader::testInvalidFile; this one was earlier.)
    TestHarness h;
    afl::io::ConstMemoryStream file(afl::base::Nothing);
    AFL_CHECK_THROWS(a, h.testee.loadTurnfile(h.turn, h.root, file, PLAYER, h.log, h.tx), afl::except::FileFormatException);
}
