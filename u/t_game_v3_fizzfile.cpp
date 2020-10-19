/**
  *  \file u/t_game_v3_fizzfile.cpp
  *  \brief Test for game::v3::FizzFile
  */

#include "game/v3/fizzfile.hpp"

#include "t_game_v3.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/internalstream.hpp"

using afl::base::Ref;
using afl::io::InternalDirectory;
using afl::io::InternalStream;
namespace gt = game::v3::structures;

/** Test missing file.
    A: create empty directory. Perform read/modify/write cycle.
    E: directory still empty. */
void
TestGameV3FizzFile::testMissing()
{
    // Environment
    game::v3::FizzFile testee;
    Ref<InternalDirectory> dir = InternalDirectory::create("testMissing");

    // Read/modify/write cycle
    TS_ASSERT_THROWS_NOTHING(testee.load(*dir));
    TS_ASSERT_EQUALS(testee.isValid(), false);
    TS_ASSERT_THROWS_NOTHING(testee.set(gt::ShipSection, 3, 1000));
    TS_ASSERT_THROWS_NOTHING(testee.save(*dir));

    // Directory has no file
    TS_ASSERT(dir->getStream("fizz.bin").get() == 0);
}

/** Test truncated file.
    A: create directory with truncated file. Perform read/modify/write cycle.
    E: file not modified. */
void
TestGameV3FizzFile::testShort()
{
    // Environment
    game::v3::FizzFile testee;
    Ref<InternalDirectory> dir = InternalDirectory::create("testShort");
    Ref<InternalStream> s = *new InternalStream();
    dir->addStream("fizz.bin", s);
    TS_ASSERT_EQUALS(s->getSize(), 0U);

    // Read/modify/write cycle
    TS_ASSERT_THROWS_NOTHING(testee.load(*dir));
    TS_ASSERT_EQUALS(testee.isValid(), false);
    TS_ASSERT_THROWS_NOTHING(testee.set(gt::ShipSection, 3, 1000));
    TS_ASSERT_THROWS_NOTHING(testee.save(*dir));

    // Stream is unmodified
    TS_ASSERT_EQUALS(s->getSize(), 0U);
}

/** Test normal case file.
    A: create directory with regular file. Perform read/modify/write cycle.
    E: file modified as expected. */
void
TestGameV3FizzFile::testNormal()
{
    // Environment
    std::vector<uint8_t> content(200);
    game::v3::FizzFile testee;
    Ref<InternalDirectory> dir = InternalDirectory::create("testShort");
    Ref<InternalStream> s = *new InternalStream();
    s->fullWrite(content);
    s->setPos(0);

    dir->addStream("fizz.bin", s);
    TS_ASSERT_EQUALS(s->getSize(), 200U);

    // Read/modify/write cycle
    TS_ASSERT_THROWS_NOTHING(testee.load(*dir));
    TS_ASSERT_EQUALS(testee.isValid(), true);
    TS_ASSERT_THROWS_NOTHING(testee.set(gt::ShipSection, 3, 1000));
    TS_ASSERT_THROWS_NOTHING(testee.save(*dir));

    // Stream is modified
    TS_ASSERT_EQUALS(s->getSize(), 200U);

    s->setPos(0);
    TS_ASSERT_THROWS_NOTHING(s->fullRead(content));
    TS_ASSERT_EQUALS(content[0], 0);
    TS_ASSERT_EQUALS(content[199], 0);

    // Modified data is 83 06 (1000 + 667) at position 24
    TS_ASSERT_EQUALS(content[24], 0x83);
    TS_ASSERT_EQUALS(content[25], 0x06);
}

