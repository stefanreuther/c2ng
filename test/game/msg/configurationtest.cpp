/**
  *  \file test/game/msg/configurationtest.cpp
  *  \brief Test for game::msg::Configuration
  */

#include "game/msg/configuration.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/test/testrunner.hpp"

/** Basic functionality test.
    A: Call toggleHeadingFiltered, setHeadingFiltered, clear.
    E: isHeadingFiltered must return correct value */
AFL_TEST("game.msg.Configuration:filter", a)
{
    game::msg::Configuration testee;

    // Toggle
    a.check("01. isHeadingFiltered", !testee.isHeadingFiltered("h"));
    testee.toggleHeadingFiltered("h");
    a.check("02. isHeadingFiltered", testee.isHeadingFiltered("h"));
    testee.toggleHeadingFiltered("h");
    a.check("03. isHeadingFiltered", !testee.isHeadingFiltered("h"));

    // Add
    a.check("11. isHeadingFiltered", !testee.isHeadingFiltered("a"));
    testee.setHeadingFiltered("a", true);
    a.check("12. isHeadingFiltered", testee.isHeadingFiltered("a"));
    testee.setHeadingFiltered("a", true);
    a.check("13. isHeadingFiltered", testee.isHeadingFiltered("a"));

    // Remove
    testee.setHeadingFiltered("a", false);
    a.check("21. isHeadingFiltered", !testee.isHeadingFiltered("a"));
    testee.setHeadingFiltered("a", false);
    a.check("22. isHeadingFiltered", !testee.isHeadingFiltered("a"));

    // clear
    testee.setHeadingFiltered("c", true);
    a.check("31. isHeadingFiltered", testee.isHeadingFiltered("c"));
    testee.clear();
    a.check("32. isHeadingFiltered", !testee.isHeadingFiltered("c"));
}

/** Test load().
    A: create internal directory with sample file. Call load().
    E: isHeadingFiltered must return correct value */
AFL_TEST("game.msg.Configuration:load", a)
{
    const char*const FILE_CONTENT =
        "# PCC2 Message Configuration File\n"
        "Filter=(-9) Sub Space Message\n";

    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("dir");
    dir->addStream("msg3.ini", *new afl::io::ConstMemoryStream(afl::string::toBytes(FILE_CONTENT)));

    afl::charset::Utf8Charset cs;
    game::msg::Configuration testee;
    testee.load(*dir, 3, cs);

    a.check("01. isHeadingFiltered", testee.isHeadingFiltered("(-9) Sub Space Message"));
}

/** Test save().
    A: call setHeadingFiltered(), then save().
    E: file must be created, containing the filtered heading */
AFL_TEST("game.msg.Configuration:save", a)
{
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("dir");
    afl::charset::Utf8Charset cs;
    game::msg::Configuration testee;
    testee.setHeadingFiltered("(f)", true);
    testee.save(*dir, 7, cs);

    afl::base::Ptr<afl::io::Stream> s = dir->getStream("msg7.ini");
    a.checkNonNull("01. stream", s.get());
    a.checkDifferent("02. stream size", s->getSize(), 0U);
    s->setPos(0);                           // getStream will return the file pointer wherever save() left off, i.e. at the end

    String_t fileContent = afl::string::fromBytes(s->createVirtualMapping()->get());
    a.checkDifferent("11. fileContent", fileContent, "");
    a.checkDifferent("12. fileContent", fileContent.find("(f)"), String_t::npos);
}

/** Test save(), empty case.
    A: create internal directory with sample file. Create empty Configuration. Call save().
    E: file must be deleted */
AFL_TEST("game.msg.Configuration:save:empty", a)
{
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("dir");
    dir->addStream("msg5.ini", *new afl::io::ConstMemoryStream(afl::string::toBytes("whatever")));

    afl::charset::Utf8Charset cs;
    game::msg::Configuration testee;
    testee.save(*dir, 5, cs);

    afl::base::Ptr<afl::io::Stream> s = dir->getStream("msg5.ini");
    a.checkNull("01. stream", s.get());
}
