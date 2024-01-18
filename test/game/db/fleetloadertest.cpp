/**
  *  \file test/game/db/fleetloadertest.cpp
  *  \brief Test for game::db::FleetLoader
  */

#include "game/db/fleetloader.hpp"

#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/universe.hpp"
#include <stdexcept>

using afl::base::Ref;
using afl::io::Directory;
using afl::io::DirectoryEntry;

namespace {
    void loadFile(game::map::Universe& univ, int playerNr, afl::base::ConstBytes_t data)
    {
        afl::string::NullTranslator tx;
        afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
        Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("");
        dir->addStream(afl::string::Format("fleet%d.cc", playerNr), *new afl::io::ConstMemoryStream(data));

        game::db::FleetLoader(cs, tx).load(*dir, univ, playerNr);
    }

    void createShip(game::map::Universe& univ, int id, int owner)
    {
        game::map::ShipData sd;
        sd.x = 1001;
        sd.y = 2002;
        sd.owner = owner;
        univ.ships().create(id)->addCurrentShipData(sd, game::PlayerSet_t(owner));
    }
}

/** Test loading from empty directory. */
AFL_TEST("game.db.FleetLoader:empty", a)
{
    game::map::Universe univ;
    afl::charset::Utf8Charset cs;
    afl::string::NullTranslator tx;
    Ref<Directory> dir = afl::io::InternalDirectory::create("");

    AFL_CHECK_SUCCEEDS(a, game::db::FleetLoader(cs, tx).load(*dir, univ, 1));
}

/** Test loading a broken file: zero-length. */
AFL_TEST("game.db.FleetLoader:error:zero-length", a)
{
    game::map::Universe univ;
    AFL_CHECK_THROWS(a, loadFile(univ, 1, afl::base::Nothing), afl::except::FileFormatException);
}

/** Test loading a broken file: bad signature. */
AFL_TEST("game.db.FleetLoader:error:bad-signature", a)
{
    game::map::Universe univ;
    static const uint8_t DATA[] = {'x','x','x','x','x','x','x','x','x','x','x','x','x','x'};
    AFL_CHECK_THROWS(a, loadFile(univ, 1, DATA), afl::except::FileFormatException);
}

/** Test loading a broken file: bad version. */
AFL_TEST("game.db.FleetLoader:error:bad-version", a)
{
    game::map::Universe univ;
    static const uint8_t DATA[] = {'C','C','f','l','e','e','t',26,7,'x','x','x','x','x'};
    AFL_CHECK_THROWS(a, loadFile(univ, 1, DATA), afl::except::FileFormatException);
}

/** Test loading a broken file: truncated file. */
AFL_TEST("game.db.FleetLoader:error:truncated", a)
{
    game::map::Universe univ;
    static const uint8_t DATA[] = {'C','C','f','l','e','e','t',26,0,0,0,0,0,0,0,0,0,0,0};
    AFL_CHECK_THROWS(a, loadFile(univ, 1, DATA), afl::except::FileFormatException);
}

/** Test loading a correct file: simple case. */
AFL_TEST("game.db.FleetLoader:success:simple", a)
{
    game::map::Universe univ;
    static const uint8_t DATA[] = {
        'C','C','f','l','e','e','t',26,1,
        10,0,          // number of ships
        3,0, 3,0, 3,0,
        5,0, 5,0, 5,0,
        9,0, 9,0, 9,0,
        0,0,
    };
    for (int i = 1; i <= 10; ++i) {
        createShip(univ, i, 1);
    }
    AFL_CHECK_SUCCEEDS(a("01. loadFile"), loadFile(univ, 1, DATA));

    a.checkEqual("11", univ.ships().get(1)->getFleetNumber(), 3);
    a.checkEqual("12", univ.ships().get(2)->getFleetNumber(), 3);
    a.checkEqual("13", univ.ships().get(3)->getFleetNumber(), 3);
    a.checkEqual("14", univ.ships().get(4)->getFleetNumber(), 5);
    a.checkEqual("15", univ.ships().get(5)->getFleetNumber(), 5);
    a.checkEqual("16", univ.ships().get(6)->getFleetNumber(), 5);
    a.checkEqual("17", univ.ships().get(7)->getFleetNumber(), 9);
    a.checkEqual("18", univ.ships().get(8)->getFleetNumber(), 9);
    a.checkEqual("19", univ.ships().get(9)->getFleetNumber(), 9);
    a.checkEqual("20", univ.ships().get(10)->getFleetNumber(), 0);
}

/** Test moved fleet.
    If the file contains a fleet whose leader no longer exists, this must be fixed. */
AFL_TEST("game.db.FleetLoader:success:moved", a)
{
    game::map::Universe univ;
    static const uint8_t DATA[] = {
        'C','C','f','l','e','e','t',26,1,
        3,0,              // number of ships
        3,0, 3,0, 3,0,
    };
    createShip(univ, 1, 1);
    createShip(univ, 2, 1);
    createShip(univ, 3, 9);    // note different owner
    AFL_CHECK_SUCCEEDS(a("01. loadFile"), loadFile(univ, 1, DATA));

    a.checkEqual("11", univ.ships().get(1)->getFleetNumber(), 1);
    a.checkEqual("12", univ.ships().get(2)->getFleetNumber(), 1);
    a.checkEqual("13", univ.ships().get(3)->getFleetNumber(), 0);
}

/** Test moved fleet, out-of-range case. */
AFL_TEST("game.db.FleetLoader:success:out-of-range", a)
{
    game::map::Universe univ;
    static const uint8_t DATA[] = {
        'C','C','f','l','e','e','t',26,1,
        3,0,              // number of ships
        4,4, 4,4, 4,4,
    };
    createShip(univ, 1, 1);
    createShip(univ, 2, 1);
    createShip(univ, 3, 9);    // note different owner
    AFL_CHECK_SUCCEEDS(a("01. loadFile"), loadFile(univ, 1, DATA));

    a.checkEqual("11", univ.ships().get(1)->getFleetNumber(), 1);
    a.checkEqual("12", univ.ships().get(2)->getFleetNumber(), 1);
    a.checkEqual("13", univ.ships().get(3)->getFleetNumber(), 0);
}

/** Test moved fleet, moving the name. */
AFL_TEST("game.db.FleetLoader:success:moved-name", a)
{
    game::map::Universe univ;
    static const uint8_t DATA[] = {
        'C','C','f','l','e','e','t',26,1,
        3,0,              // number of ships
        3,0, 3,0, 3,128,
        2,'h','i'
    };
    createShip(univ, 1, 1);
    createShip(univ, 2, 1);
    createShip(univ, 3, 9);    // note different owner
    AFL_CHECK_SUCCEEDS(a("01. loadFile"), loadFile(univ, 1, DATA));

    a.checkEqual("11. getFleetNumber", univ.ships().get(1)->getFleetNumber(), 1);
    a.checkEqual("12. getFleetNumber", univ.ships().get(2)->getFleetNumber(), 1);
    a.checkEqual("13. getFleetNumber", univ.ships().get(3)->getFleetNumber(), 0);

    a.checkEqual("21. getFleetName", univ.ships().get(1)->getFleetName(), "hi");
    a.checkEqual("22. getFleetName", univ.ships().get(2)->getFleetName(), "");
    a.checkEqual("23. getFleetName", univ.ships().get(3)->getFleetName(), "");
}

/** Test deleted fleet. */
AFL_TEST("game.db.FleetLoader:success:deleted", a)
{
    game::map::Universe univ;
    static const uint8_t DATA[] = {
        'C','C','f','l','e','e','t',26,1,
        3,0,              // number of ships
        1,128, 3,0, 3,128,
        2,'h','i',
        2,'h','o',
    };
    createShip(univ, 2, 1);
    createShip(univ, 3, 1);
    AFL_CHECK_SUCCEEDS(a("01. loadFile"), loadFile(univ, 1, DATA));

    a.checkEqual("11. getFleetNumber", univ.ships().get(2)->getFleetNumber(), 3);
    a.checkEqual("12. getFleetNumber", univ.ships().get(3)->getFleetNumber(), 3);

    a.checkEqual("21. getFleetName", univ.ships().get(2)->getFleetName(), "");
    a.checkEqual("22. getFleetName", univ.ships().get(3)->getFleetName(), "ho");
}

/** Test comment handling.
    A comment attached to a non-leader must be ignored.
    Comments must be charset-translated. */
AFL_TEST("game.db.FleetLoader:comments", a)
{
    game::map::Universe univ;
    static const uint8_t DATA[] = {
        'C','C','f','l','e','e','t',26,1,
        4,0,              // number of ships
        2,128, 2,0, 3,128, 3,0,
        2,'h','i',
        2,'h',0x94,
    };
    for (int i = 1; i <= 4; ++i) {
        createShip(univ, i, 1);
    }
    AFL_CHECK_SUCCEEDS(a("01. loadFile"), loadFile(univ, 1, DATA));

    a.checkEqual("11. getFleetNumber", univ.ships().get(1)->getFleetNumber(), 2);
    a.checkEqual("12. getFleetNumber", univ.ships().get(2)->getFleetNumber(), 2);
    a.checkEqual("13. getFleetNumber", univ.ships().get(3)->getFleetNumber(), 3);
    a.checkEqual("14. getFleetNumber", univ.ships().get(4)->getFleetNumber(), 3);

    a.checkEqual("21. getFleetName", univ.ships().get(1)->getFleetName(), "");
    a.checkEqual("22. getFleetName", univ.ships().get(2)->getFleetName(), "");
    a.checkEqual("23. getFleetName", univ.ships().get(3)->getFleetName(), "h\xC3\xB6");
    a.checkEqual("24. getFleetName", univ.ships().get(4)->getFleetName(), "");
}

/** Test loading conflicting fleets.
    If a ship has changed owners and is member of a new fleet, loading must not overwrite this. */
AFL_TEST("game.db.FleetLoader:success:conflict", a)
{
    game::map::Universe univ;

    static const uint8_t DATA[] = {
        'C','C','f','l','e','e','t',26,1,
        3,0,              // number of ships
        0,0, 0,0, 2,0,
    };
    createShip(univ, 1, 1);
    createShip(univ, 2, 2);  // new owner, new ship Id
    createShip(univ, 3, 2);
    univ.ships().get(2)->setFleetNumber(3);
    univ.ships().get(3)->setFleetNumber(3);

    AFL_CHECK_SUCCEEDS(a("01. loadFile"), loadFile(univ, 1, DATA));

    a.checkEqual("11", univ.ships().get(1)->getFleetNumber(), 0);
    a.checkEqual("12", univ.ships().get(2)->getFleetNumber(), 3);
    a.checkEqual("13", univ.ships().get(3)->getFleetNumber(), 3);
}

/** Test saving. */
AFL_TEST("game.db.FleetLoader:save", a)
{
    Ref<Directory> dir = afl::io::InternalDirectory::create("");
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    afl::string::NullTranslator tx;

    // Create a universe and save it
    {
        game::map::Universe univ;
        createShip(univ, 1, 7);
        createShip(univ, 2, 7);
        createShip(univ, 3, 7);
        createShip(univ, 4, 7);
        univ.ships().get(1)->setFleetNumber(3);
        univ.ships().get(3)->setFleetNumber(3);
        univ.ships().get(4)->setFleetNumber(3);
        univ.ships().get(3)->setFleetName("three");
        game::db::FleetLoader(cs, tx).save(*dir, univ, 7);
    }

    // Verify that file was created and has appropriate size
    Ref<DirectoryEntry> entry = dir->getDirectoryEntryByName("fleet7.cc");
    a.checkEqual("01. getFileType", entry->getFileType(), DirectoryEntry::tFile);
    a.check("02. file size", entry->getFileSize() >= 1000);

    // Load into another universe
    {
        game::map::Universe univ;
        createShip(univ, 1, 7);
        createShip(univ, 2, 7);
        createShip(univ, 3, 7);
        createShip(univ, 4, 7);
        game::db::FleetLoader(cs, tx).load(*dir, univ, 7);

        a.checkEqual("11. getFleetNumber", univ.ships().get(1)->getFleetNumber(), 3);
        a.checkEqual("12. getFleetNumber", univ.ships().get(2)->getFleetNumber(), 0);
        a.checkEqual("13. getFleetNumber", univ.ships().get(3)->getFleetNumber(), 3);
        a.checkEqual("14. getFleetNumber", univ.ships().get(4)->getFleetNumber(), 3);
        a.checkEqual("15. getFleetName", univ.ships().get(3)->getFleetName(), "three");
    }
}

/** Test saving when there's nothing to do. */
AFL_TEST("game.db.FleetLoader:save:empty", a)
{
    // Create a directory with a file in it
    Ref<Directory> dir = afl::io::InternalDirectory::create("");
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    afl::string::NullTranslator tx;
    dir->openFile("fleet7.cc", afl::io::FileSystem::Create);

    // Create a universe and save it
    game::map::Universe univ;
    createShip(univ, 1, 7);
    createShip(univ, 2, 7);
    createShip(univ, 3, 7);
    createShip(univ, 4, 7);
    game::db::FleetLoader(cs, tx).save(*dir, univ, 7);

    // File is gone
    AFL_CHECK_THROWS(a, dir->openFile("fleet7.cc", afl::io::FileSystem::OpenRead), std::exception);
}

/** Test saving with big Ids.
    Same as testSave, but with Ids exceeding 500 to exercise the extended file format. */
AFL_TEST("game.db.FleetLoader:save:big-id", a)
{
    Ref<Directory> dir = afl::io::InternalDirectory::create("");
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    afl::string::NullTranslator tx;

    // Create a universe and save it
    {
        game::map::Universe univ;
        createShip(univ, 801, 7);
        createShip(univ, 802, 7);
        createShip(univ, 803, 7);
        createShip(univ, 804, 7);
        univ.ships().get(801)->setFleetNumber(803);
        univ.ships().get(803)->setFleetNumber(803);
        univ.ships().get(804)->setFleetNumber(803);
        univ.ships().get(803)->setFleetName("three");
        game::db::FleetLoader(cs, tx).save(*dir, univ, 7);
    }

    // Verify that file was created and has appropriate size
    Ref<DirectoryEntry> entry = dir->getDirectoryEntryByName("fleet7.cc");
    a.checkEqual("01. getFileType", entry->getFileType(), DirectoryEntry::tFile);
    a.check("02. file size", entry->getFileSize() >= 1000);

    // Load into another universe
    {
        game::map::Universe univ;
        createShip(univ, 801, 7);
        createShip(univ, 802, 7);
        createShip(univ, 803, 7);
        createShip(univ, 804, 7);
        game::db::FleetLoader(cs, tx).load(*dir, univ, 7);

        a.checkEqual("11. getFleetNumber", univ.ships().get(801)->getFleetNumber(), 803);
        a.checkEqual("12. getFleetNumber", univ.ships().get(802)->getFleetNumber(), 0);
        a.checkEqual("13. getFleetNumber", univ.ships().get(803)->getFleetNumber(), 803);
        a.checkEqual("14. getFleetNumber", univ.ships().get(804)->getFleetNumber(), 803);
        a.checkEqual("15. getFleetName", univ.ships().get(803)->getFleetName(), "three");
    }
}
