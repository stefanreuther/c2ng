/**
  *  \file u/t_game_db_fleetloader.cpp
  *  \brief Test for game::db::FleetLoader
  */

#include "game/db/fleetloader.hpp"

#include "t_game_db.hpp"
#include "afl/io/internaldirectory.hpp"
#include "game/map/universe.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/string/format.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/except/fileformatexception.hpp"

namespace {
    void loadFile(game::map::Universe& univ, int playerNr, afl::base::ConstBytes_t data)
    {
        afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
        afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("");
        dir->addStream(afl::string::Format("fleet%d.cc", playerNr), *new afl::io::ConstMemoryStream(data));

        game::db::FleetLoader(cs).load(*dir, univ, playerNr);
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
void
TestGameDbFleetLoader::testEmpty()
{
    game::map::Universe univ;
    afl::charset::Utf8Charset cs;
    afl::base::Ref<afl::io::Directory> dir = afl::io::InternalDirectory::create("");

    TS_ASSERT_THROWS_NOTHING(game::db::FleetLoader(cs).load(*dir, univ, 1));
}

/** Test loading a broken file: zero-length. */
void
TestGameDbFleetLoader::testBroken()
{
    game::map::Universe univ;
    TS_ASSERT_THROWS(loadFile(univ, 1, afl::base::Nothing), afl::except::FileFormatException);
}

/** Test loading a broken file: bad signature. */
void
TestGameDbFleetLoader::testBrokenBadSig()
{
    game::map::Universe univ;
    static const uint8_t DATA[] = {'x','x','x','x','x','x','x','x','x','x','x','x','x','x'};
    TS_ASSERT_THROWS(loadFile(univ, 1, DATA), afl::except::FileFormatException);
}

/** Test loading a broken file: bad version. */
void
TestGameDbFleetLoader::testBrokenBadVersion()
{
    game::map::Universe univ;
    static const uint8_t DATA[] = {'C','C','f','l','e','e','t',26,7,'x','x','x','x','x'};
    TS_ASSERT_THROWS(loadFile(univ, 1, DATA), afl::except::FileFormatException);
}

/** Test loading a broken file: truncated file. */
void
TestGameDbFleetLoader::testBrokenTruncated()
{
    game::map::Universe univ;
    static const uint8_t DATA[] = {'C','C','f','l','e','e','t',26,0,0,0,0,0,0,0,0,0,0,0};
    TS_ASSERT_THROWS(loadFile(univ, 1, DATA), afl::except::FileFormatException);
}

/** Test loading a correct file: simple case. */
void
TestGameDbFleetLoader::testSimple()
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
    TS_ASSERT_THROWS_NOTHING(loadFile(univ, 1, DATA));

    TS_ASSERT_EQUALS(univ.ships().get(1)->getFleetNumber(), 3);
    TS_ASSERT_EQUALS(univ.ships().get(2)->getFleetNumber(), 3);
    TS_ASSERT_EQUALS(univ.ships().get(3)->getFleetNumber(), 3);
    TS_ASSERT_EQUALS(univ.ships().get(4)->getFleetNumber(), 5);
    TS_ASSERT_EQUALS(univ.ships().get(5)->getFleetNumber(), 5);
    TS_ASSERT_EQUALS(univ.ships().get(6)->getFleetNumber(), 5);
    TS_ASSERT_EQUALS(univ.ships().get(7)->getFleetNumber(), 9);
    TS_ASSERT_EQUALS(univ.ships().get(8)->getFleetNumber(), 9);
    TS_ASSERT_EQUALS(univ.ships().get(9)->getFleetNumber(), 9);
    TS_ASSERT_EQUALS(univ.ships().get(10)->getFleetNumber(), 0);
}

/** Test moved fleet.
    If the file contains a fleet whose leader no longer exists, this must be fixed. */
void
TestGameDbFleetLoader::testMoved()
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
    TS_ASSERT_THROWS_NOTHING(loadFile(univ, 1, DATA));

    TS_ASSERT_EQUALS(univ.ships().get(1)->getFleetNumber(), 1);
    TS_ASSERT_EQUALS(univ.ships().get(2)->getFleetNumber(), 1);
    TS_ASSERT_EQUALS(univ.ships().get(3)->getFleetNumber(), 0);
}

/** Test moved fleet, out-of-range case. */
void
TestGameDbFleetLoader::testMovedRange()
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
    TS_ASSERT_THROWS_NOTHING(loadFile(univ, 1, DATA));

    TS_ASSERT_EQUALS(univ.ships().get(1)->getFleetNumber(), 1);
    TS_ASSERT_EQUALS(univ.ships().get(2)->getFleetNumber(), 1);
    TS_ASSERT_EQUALS(univ.ships().get(3)->getFleetNumber(), 0);
}

/** Test moved fleet, moving the name. */
void
TestGameDbFleetLoader::testMovedName()
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
    TS_ASSERT_THROWS_NOTHING(loadFile(univ, 1, DATA));

    TS_ASSERT_EQUALS(univ.ships().get(1)->getFleetNumber(), 1);
    TS_ASSERT_EQUALS(univ.ships().get(2)->getFleetNumber(), 1);
    TS_ASSERT_EQUALS(univ.ships().get(3)->getFleetNumber(), 0);

    TS_ASSERT_EQUALS(univ.ships().get(1)->getFleetName(), "hi");
    TS_ASSERT_EQUALS(univ.ships().get(2)->getFleetName(), "");
    TS_ASSERT_EQUALS(univ.ships().get(3)->getFleetName(), "");
}

/** Test deleted fleet. */
void
TestGameDbFleetLoader::testDeleted()
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
    TS_ASSERT_THROWS_NOTHING(loadFile(univ, 1, DATA));

    TS_ASSERT_EQUALS(univ.ships().get(2)->getFleetNumber(), 3);
    TS_ASSERT_EQUALS(univ.ships().get(3)->getFleetNumber(), 3);

    TS_ASSERT_EQUALS(univ.ships().get(2)->getFleetName(), "");
    TS_ASSERT_EQUALS(univ.ships().get(3)->getFleetName(), "ho");
}

/** Test comment handling.
    A comment attached to a non-leader must be ignored.
    Comments must be charset-translated. */
void
TestGameDbFleetLoader::testComments()
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
    TS_ASSERT_THROWS_NOTHING(loadFile(univ, 1, DATA));

    TS_ASSERT_EQUALS(univ.ships().get(1)->getFleetNumber(), 2);
    TS_ASSERT_EQUALS(univ.ships().get(2)->getFleetNumber(), 2);
    TS_ASSERT_EQUALS(univ.ships().get(3)->getFleetNumber(), 3);
    TS_ASSERT_EQUALS(univ.ships().get(4)->getFleetNumber(), 3);

    TS_ASSERT_EQUALS(univ.ships().get(1)->getFleetName(), "");
    TS_ASSERT_EQUALS(univ.ships().get(2)->getFleetName(), "");
    TS_ASSERT_EQUALS(univ.ships().get(3)->getFleetName(), "h\xC3\xB6");
    TS_ASSERT_EQUALS(univ.ships().get(4)->getFleetName(), "");
}

/** Test loading conflicting fleets.
    If a ship has changed owners and is member of a new fleet, loading must not overwrite this. */
void
TestGameDbFleetLoader::testConflict()
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

    TS_ASSERT_THROWS_NOTHING(loadFile(univ, 1, DATA));

    TS_ASSERT_EQUALS(univ.ships().get(1)->getFleetNumber(), 0);
    TS_ASSERT_EQUALS(univ.ships().get(2)->getFleetNumber(), 3);
    TS_ASSERT_EQUALS(univ.ships().get(3)->getFleetNumber(), 3);
}
