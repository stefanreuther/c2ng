/**
  *  \file u/t_game_msg_configuration.cpp
  *  \brief Test for game::msg::Configuration
  */

#include "game/msg/configuration.hpp"

#include "t_game_msg.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/filemapping.hpp"

/** Basic functionality test.
    A: Call toggleHeadingFiltered, setHeadingFiltered, clear.
    E: isHeadingFiltered must return correct value */
void
TestGameMsgConfiguration::testIt()
{
    game::msg::Configuration testee;

    // Toggle
    TS_ASSERT(!testee.isHeadingFiltered("h"));
    testee.toggleHeadingFiltered("h");
    TS_ASSERT(testee.isHeadingFiltered("h"));
    testee.toggleHeadingFiltered("h");
    TS_ASSERT(!testee.isHeadingFiltered("h"));

    // Add
    TS_ASSERT(!testee.isHeadingFiltered("a"));
    testee.setHeadingFiltered("a", true);
    TS_ASSERT(testee.isHeadingFiltered("a"));
    testee.setHeadingFiltered("a", true);
    TS_ASSERT(testee.isHeadingFiltered("a"));

    // Remove
    testee.setHeadingFiltered("a", false);
    TS_ASSERT(!testee.isHeadingFiltered("a"));
    testee.setHeadingFiltered("a", false);
    TS_ASSERT(!testee.isHeadingFiltered("a"));

    // clear
    testee.setHeadingFiltered("c", true);
    TS_ASSERT(testee.isHeadingFiltered("c"));
    testee.clear();
    TS_ASSERT(!testee.isHeadingFiltered("c"));
}

/** Test load().
    A: create internal directory with sample file. Call load().
    E: isHeadingFiltered must return correct value */
void
TestGameMsgConfiguration::testLoad()
{
    const char*const FILE_CONTENT =
        "# PCC2 Message Configuration File\n"
        "Filter=(-9) Sub Space Message\n";

    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("dir");
    dir->addStream("msg3.ini", *new afl::io::ConstMemoryStream(afl::string::toBytes(FILE_CONTENT)));

    game::msg::Configuration testee;
    testee.load(*dir, 3);

    TS_ASSERT(testee.isHeadingFiltered("(-9) Sub Space Message"));
}

/** Test save().
    A: call setHeadingFiltered(), then save().
    E: file must be created, containing the filtered heading */
void
TestGameMsgConfiguration::testSave()
{
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("dir");
    game::msg::Configuration testee;
    testee.setHeadingFiltered("(f)", true);
    testee.save(*dir, 7);

    afl::base::Ptr<afl::io::Stream> s = dir->getStream("msg7.ini");
    TS_ASSERT(s.get() != 0);
    TS_ASSERT_DIFFERS(s->getSize(), 0U);
    s->setPos(0);                           // getStream will return the file pointer wherever save() left off, i.e. at the end

    String_t fileContent = afl::string::fromBytes(s->createVirtualMapping()->get());
    TS_ASSERT_DIFFERS(fileContent, "");
    TS_ASSERT_DIFFERS(fileContent.find("(f)"), String_t::npos);
}

/** Test save(), empty case.
    A: create internal directory with sample file. Create empty Configuration. Call save().
    E: file must be deleted */
void
TestGameMsgConfiguration::testSaveEmpty()
{
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("dir");
    dir->addStream("msg5.ini", *new afl::io::ConstMemoryStream(afl::string::toBytes("whatever")));

    game::msg::Configuration testee;
    testee.save(*dir, 5);

    afl::base::Ptr<afl::io::Stream> s = dir->getStream("msg5.ini");
    TS_ASSERT(s.get() == 0);
}

