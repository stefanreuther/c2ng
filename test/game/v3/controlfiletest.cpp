/**
  *  \file test/game/v3/controlfiletest.cpp
  *  \brief Test for game::v3::ControlFile
  */

#include "game/v3/controlfile.hpp"

#include "afl/base/ref.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"

namespace {
    const uint8_t TEST_PATTERN[] = {1,2,3,4,5,6,7,8,1,2,3,4,5,6,7,8};
}

/** Save with no configured owner. This is a no-op. */
AFL_TEST("game.v3.ControlFile:save:empty", a)
{
    game::v3::ControlFile testee;
    afl::string::NullTranslator tx;
    afl::sys::Log log;

    // Empty directory
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("foo");
    a.checkEqual("01. getFileType", dir->getDirectoryEntryByName("control.dat")->getFileType(), afl::io::DirectoryEntry::tUnknown);

    // Save it. Still empty.
    testee.set(game::v3::structures::ShipSection, 500, 1);
    testee.set(game::v3::structures::PlanetSection, 500, 1);
    testee.set(game::v3::structures::BaseSection, 500, 1);
    testee.save(*dir, tx, log);
    a.checkEqual("11. getFileType", dir->getDirectoryEntryByName("control.dat")->getFileType(), afl::io::DirectoryEntry::tUnknown);

    afl::base::Ptr<afl::io::DirectoryEntry> entry;
    a.check("21. dir empty", !dir->getDirectoryEntries()->getNextElement(entry));
}

/** Save in DOS format (owner 0). */
AFL_TEST("game.v3.ControlFile:save:dos", a)
{
    game::v3::ControlFile testee;
    afl::string::NullTranslator tx;
    afl::sys::Log log;

    // Empty directory
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("foo");
    a.checkEqual("01. getFileType", dir->getDirectoryEntryByName("control.dat")->getFileType(), afl::io::DirectoryEntry::tUnknown);

    // Save it with data in slot 500.
    testee.setFileOwner(0);
    testee.set(game::v3::structures::ShipSection, 500, 1);
    testee.set(game::v3::structures::PlanetSection, 500, 1);
    testee.set(game::v3::structures::BaseSection, 500, 1);
    testee.save(*dir, tx, log);
    a.checkEqual("11. getFileType", dir->getDirectoryEntryByName("control.dat")->getFileType(), afl::io::DirectoryEntry::tFile);
    a.checkEqual("12. getFileSize", dir->getDirectoryEntryByName("control.dat")->getFileSize(), 6002U);
}

/** Save in Windows format (nonzero owner). */
AFL_TEST("game.v3.ControlFile:save:win", a)
{
    game::v3::ControlFile testee;
    afl::string::NullTranslator tx;
    afl::sys::Log log;

    // Empty directory
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("foo");
    a.checkEqual("01. getFileType", dir->getDirectoryEntryByName("contrl6.dat")->getFileType(), afl::io::DirectoryEntry::tUnknown);

    // Save it with data in slot 500.
    testee.setFileOwner(6);
    testee.set(game::v3::structures::ShipSection, 500, 1);
    testee.set(game::v3::structures::PlanetSection, 500, 1);
    testee.set(game::v3::structures::BaseSection, 500, 1);
    testee.save(*dir, tx, log);
    a.checkEqual("11. getFileType", dir->getDirectoryEntryByName("contrl6.dat")->getFileType(), afl::io::DirectoryEntry::tFile);
    a.checkEqual("12. getFileSize", dir->getDirectoryEntryByName("contrl6.dat")->getFileSize(), 6002U);
}

/** Save in Host999 format. */
AFL_TEST("game.v3.ControlFile:save:host999", a)
{
    game::v3::ControlFile testee;
    afl::string::NullTranslator tx;
    afl::sys::Log log;

    // Empty directory
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("foo");
    a.checkEqual("01. getFileType", dir->getDirectoryEntryByName("contrl6.dat")->getFileType(), afl::io::DirectoryEntry::tUnknown);

    // Save it with data in slot 501.
    testee.setFileOwner(6);
    testee.set(game::v3::structures::ShipSection, 501, 1);
    testee.set(game::v3::structures::PlanetSection, 500, 1);
    testee.set(game::v3::structures::BaseSection, 500, 1);
    testee.save(*dir, tx, log);
    a.checkEqual("11. getFileType", dir->getDirectoryEntryByName("contrl6.dat")->getFileType(), afl::io::DirectoryEntry::tFile);
    a.checkEqual("12. getFileSize", dir->getDirectoryEntryByName("contrl6.dat")->getFileSize(), 9996U);
}

/** Test loading of a DOS file. */
AFL_TEST("game.v3.ControlFile:load:dos", a)
{
    game::v3::ControlFile testee;
    afl::string::NullTranslator tx;
    afl::sys::Log log;

    // Create a DOS file and load it
    {
        afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("foo");
        dir->openFile("control.dat", afl::io::FileSystem::Create)->fullWrite(TEST_PATTERN);
        testee.load(*dir, 3, tx, log);
    }

    // Write again into a new directory and verify it's there
    {
        afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("foo");
        testee.save(*dir, tx, log);
        a.checkEqual("01. getFileType", dir->getDirectoryEntryByName("control.dat")->getFileType(), afl::io::DirectoryEntry::tFile);

        afl::base::Ref<afl::io::FileMapping> map(dir->openFile("control.dat", afl::io::FileSystem::OpenRead)->createVirtualMapping());
        a.checkEqualContent("02. content", map->get().trim(sizeof(TEST_PATTERN)), afl::base::ConstBytes_t(TEST_PATTERN));
    }
}

/** Test loading of a Windows file. */
AFL_TEST("game.v3.ControlFile:load:win", a)
{
    game::v3::ControlFile testee;
    afl::string::NullTranslator tx;
    afl::sys::Log log;

    // Create a Windows file and load it
    {
        afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("foo");
        dir->openFile("contrl3.dat", afl::io::FileSystem::Create)->fullWrite(TEST_PATTERN);
        testee.load(*dir, 3, tx, log);
    }

    // Write again into a new directory and verify it's there
    {
        afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("foo");
        testee.save(*dir, tx, log);
        a.checkEqual("01. getFileType", dir->getDirectoryEntryByName("contrl3.dat")->getFileType(), afl::io::DirectoryEntry::tFile);

        afl::base::Ref<afl::io::FileMapping> map(dir->openFile("contrl3.dat", afl::io::FileSystem::OpenRead)->createVirtualMapping());
        a.checkEqualContent("02. content", map->get().trim(sizeof(TEST_PATTERN)), afl::base::ConstBytes_t(TEST_PATTERN));
    }
}

/** Test loading empty directory. */
AFL_TEST("game.v3.ControlFile:load:empty", a)
{
    game::v3::ControlFile testee;
    afl::string::NullTranslator tx;
    afl::sys::Log log;

    // Load empty directory
    {
        afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("foo");
        testee.load(*dir, 3, tx, log);
    }

    // Save again
    {
        afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("foo");
        testee.save(*dir, tx, log);

        afl::base::Ptr<afl::io::DirectoryEntry> entry;
        a.check("01. dir empty", !dir->getDirectoryEntries()->getNextElement(entry));
    }
}

/** Test out-of-range access. */
AFL_TEST("game.v3.ControlFile:range", a)
{
    game::v3::ControlFile testee;
    afl::string::NullTranslator tx;
    afl::sys::Log log;

    // These accesses are out-of-range and should be ignored
    testee.set(game::v3::structures::ShipSection, 9999, 1);
    testee.set(game::v3::structures::PlanetSection, 9999, 1);
    testee.set(game::v3::structures::BaseSection, 9999, 1);

    // Save and verify that it's empty
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("foo");
    testee.setFileOwner(0);
    testee.save(*dir, tx, log);
    a.checkEqual("01. getFileType", dir->getDirectoryEntryByName("control.dat")->getFileType(), afl::io::DirectoryEntry::tFile);

    afl::base::Ref<afl::io::FileMapping> map(dir->openFile("control.dat", afl::io::FileSystem::OpenRead)->createVirtualMapping());
    afl::base::ConstBytes_t bytes(map->get());
    a.checkEqual("11. size", bytes.size(), 6002U);
    while (const uint8_t* p = bytes.eat()) {
        a.checkEqual("12. content", *p, 0U);
    }
}
