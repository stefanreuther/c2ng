/**
  *  \file test/game/v3/fizzfiletest.cpp
  *  \brief Test for game::v3::FizzFile
  */

#include "game/v3/fizzfile.hpp"

#include "afl/io/internaldirectory.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/test/testrunner.hpp"

using afl::base::Ref;
using afl::io::InternalDirectory;
using afl::io::InternalStream;
namespace gt = game::v3::structures;

/** Test missing file.
    A: create empty directory. Perform read/modify/write cycle.
    E: directory still empty. */
AFL_TEST("game.v3.FizzFile:missing", a)
{
    // Environment
    game::v3::FizzFile testee;
    Ref<InternalDirectory> dir = InternalDirectory::create("testMissing");

    // Read/modify/write cycle
    AFL_CHECK_SUCCEEDS(a("01. load"), testee.load(*dir));
    a.checkEqual("02. isValid", testee.isValid(), false);
    AFL_CHECK_SUCCEEDS(a("03. set"), testee.set(gt::ShipSection, 3, 1000));
    AFL_CHECK_SUCCEEDS(a("04. save"), testee.save(*dir));

    // Directory has no file
    a.checkNull("11. open", dir->getStream("fizz.bin").get());
}

/** Test truncated file.
    A: create directory with truncated file. Perform read/modify/write cycle.
    E: file not modified. */
AFL_TEST("game.v3.FizzFile:truncated", a)
{
    // Environment
    game::v3::FizzFile testee;
    Ref<InternalDirectory> dir = InternalDirectory::create("testShort");
    Ref<InternalStream> s = *new InternalStream();
    dir->addStream("fizz.bin", s);
    a.checkEqual("01. getSize", s->getSize(), 0U);

    // Read/modify/write cycle
    AFL_CHECK_SUCCEEDS(a("11. load"), testee.load(*dir));
    a.checkEqual("12. isValid", testee.isValid(), false);
    AFL_CHECK_SUCCEEDS(a("13. set"), testee.set(gt::ShipSection, 3, 1000));
    AFL_CHECK_SUCCEEDS(a("14. save"), testee.save(*dir));

    // Stream is unmodified
    a.checkEqual("21. getSize", s->getSize(), 0U);
}

/** Test normal case file.
    A: create directory with regular file. Perform read/modify/write cycle.
    E: file modified as expected. */
AFL_TEST("game.v3.FizzFile:normal", a)
{
    // Environment
    std::vector<uint8_t> content(200);
    game::v3::FizzFile testee;
    Ref<InternalDirectory> dir = InternalDirectory::create("testShort");
    Ref<InternalStream> s = *new InternalStream();
    s->fullWrite(content);
    s->setPos(0);

    dir->addStream("fizz.bin", s);
    a.checkEqual("01. getSize", s->getSize(), 200U);

    // Read/modify/write cycle
    AFL_CHECK_SUCCEEDS(a("11. load"), testee.load(*dir));
    a.checkEqual("12. isValid", testee.isValid(), true);
    AFL_CHECK_SUCCEEDS(a("13. set"), testee.set(gt::ShipSection, 3, 1000));
    AFL_CHECK_SUCCEEDS(a("14. save"), testee.save(*dir));

    // Stream is modified
    a.checkEqual("21. getSize", s->getSize(), 200U);

    s->setPos(0);
    AFL_CHECK_SUCCEEDS(a("31. fullRead"), s->fullRead(content));
    a.checkEqual("32. content", content[0], 0);
    a.checkEqual("33. content", content[199], 0);

    // Modified data is 83 06 (1000 + 667) at position 24
    a.checkEqual("41. content", content[24], 0x83);
    a.checkEqual("42. content", content[25], 0x06);
}
