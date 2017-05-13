/**
  *  \file u/t_game_v3_controlfile.cpp
  *  \brief Test for game::v3::ControlFile
  */

#include "game/v3/controlfile.hpp"

#include "t_game_v3.hpp"
#include "afl/base/ref.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/io/filemapping.hpp"

namespace {
    const uint8_t TEST_PATTERN[] = {1,2,3,4,5,6,7,8,1,2,3,4,5,6,7,8};
}

/** Save with no configured owner. This is a no-op. */
void
TestGameV3ControlFile::testSave()
{
    game::v3::ControlFile testee;

    // Empty directory
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("foo");
    TS_ASSERT_EQUALS(dir->getDirectoryEntryByName("control.dat")->getFileType(), afl::io::DirectoryEntry::tUnknown);

    // Save it. Still empty.
    testee.set(game::v3::structures::ShipSection, 500, 1);
    testee.set(game::v3::structures::PlanetSection, 500, 1);
    testee.set(game::v3::structures::BaseSection, 500, 1);
    testee.save(*dir);
    TS_ASSERT_EQUALS(dir->getDirectoryEntryByName("control.dat")->getFileType(), afl::io::DirectoryEntry::tUnknown);
    
    afl::base::Ptr<afl::io::DirectoryEntry> entry;
    TS_ASSERT(!dir->getDirectoryEntries()->getNextElement(entry));
}

/** Save in DOS format (owner 0). */
void
TestGameV3ControlFile::testSaveDOS()
{
    game::v3::ControlFile testee;

    // Empty directory
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("foo");
    TS_ASSERT_EQUALS(dir->getDirectoryEntryByName("control.dat")->getFileType(), afl::io::DirectoryEntry::tUnknown);

    // Save it with data in slot 500.
    testee.setFileOwner(0);
    testee.set(game::v3::structures::ShipSection, 500, 1);
    testee.set(game::v3::structures::PlanetSection, 500, 1);
    testee.set(game::v3::structures::BaseSection, 500, 1);
    testee.save(*dir);
    TS_ASSERT_EQUALS(dir->getDirectoryEntryByName("control.dat")->getFileType(), afl::io::DirectoryEntry::tFile);
    TS_ASSERT_EQUALS(dir->getDirectoryEntryByName("control.dat")->getFileSize(), 6002U);
}

/** Save in Windows format (nonzero owner). */
void
TestGameV3ControlFile::testSaveWin()
{
    game::v3::ControlFile testee;

    // Empty directory
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("foo");
    TS_ASSERT_EQUALS(dir->getDirectoryEntryByName("contrl6.dat")->getFileType(), afl::io::DirectoryEntry::tUnknown);

    // Save it with data in slot 500.
    testee.setFileOwner(6);
    testee.set(game::v3::structures::ShipSection, 500, 1);
    testee.set(game::v3::structures::PlanetSection, 500, 1);
    testee.set(game::v3::structures::BaseSection, 500, 1);
    testee.save(*dir);
    TS_ASSERT_EQUALS(dir->getDirectoryEntryByName("contrl6.dat")->getFileType(), afl::io::DirectoryEntry::tFile);
    TS_ASSERT_EQUALS(dir->getDirectoryEntryByName("contrl6.dat")->getFileSize(), 6002U);
}

/** Save in Host999 format. */
void
TestGameV3ControlFile::testSaveBig()
{
    game::v3::ControlFile testee;

    // Empty directory
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("foo");
    TS_ASSERT_EQUALS(dir->getDirectoryEntryByName("contrl6.dat")->getFileType(), afl::io::DirectoryEntry::tUnknown);

    // Save it with data in slot 501.
    testee.setFileOwner(6);
    testee.set(game::v3::structures::ShipSection, 501, 1);
    testee.set(game::v3::structures::PlanetSection, 500, 1);
    testee.set(game::v3::structures::BaseSection, 500, 1);
    testee.save(*dir);
    TS_ASSERT_EQUALS(dir->getDirectoryEntryByName("contrl6.dat")->getFileType(), afl::io::DirectoryEntry::tFile);
    TS_ASSERT_EQUALS(dir->getDirectoryEntryByName("contrl6.dat")->getFileSize(), 9996U);
}

/** Test loading of a DOS file. */
void
TestGameV3ControlFile::testLoadDOS()
{
    game::v3::ControlFile testee;

    // Create a DOS file and load it
    {
        afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("foo");
        dir->openFile("control.dat", afl::io::FileSystem::Create)->fullWrite(TEST_PATTERN);
        testee.load(*dir, 3);
    }

    // Write again into a new directory and verify it's there
    {
        afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("foo");
        testee.save(*dir);
        TS_ASSERT_EQUALS(dir->getDirectoryEntryByName("control.dat")->getFileType(), afl::io::DirectoryEntry::tFile);

        afl::base::Ref<afl::io::FileMapping> map(dir->openFile("control.dat", afl::io::FileSystem::OpenRead)->createVirtualMapping());
        TS_ASSERT_SAME_DATA(map->get().unsafeData(), TEST_PATTERN, sizeof(TEST_PATTERN));
    }
}

/** Test loading of a Windows file. */
void
TestGameV3ControlFile::testLoadWindows()
{
    game::v3::ControlFile testee;

    // Create a Windows file and load it
    {
        afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("foo");
        dir->openFile("contrl3.dat", afl::io::FileSystem::Create)->fullWrite(TEST_PATTERN);
        testee.load(*dir, 3);
    }

    // Write again into a new directory and verify it's there
    {
        afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("foo");
        testee.save(*dir);
        TS_ASSERT_EQUALS(dir->getDirectoryEntryByName("contrl3.dat")->getFileType(), afl::io::DirectoryEntry::tFile);

        afl::base::Ref<afl::io::FileMapping> map(dir->openFile("contrl3.dat", afl::io::FileSystem::OpenRead)->createVirtualMapping());
        TS_ASSERT_SAME_DATA(map->get().unsafeData(), TEST_PATTERN, sizeof(TEST_PATTERN));
    }
}

/** Test loading empty directory. */
void
TestGameV3ControlFile::testLoadEmpty()
{
    game::v3::ControlFile testee;

    // Load empty directory
    {
        afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("foo");
        testee.load(*dir, 3);
    }

    // Save again
    {
        afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("foo");
        testee.save(*dir);

        afl::base::Ptr<afl::io::DirectoryEntry> entry;
        TS_ASSERT(!dir->getDirectoryEntries()->getNextElement(entry));
    }
}

/** Test out-of-range access. */
void
TestGameV3ControlFile::testRange()
{
    game::v3::ControlFile testee;

    // These accesses are out-of-range and should be ignored
    testee.set(game::v3::structures::ShipSection, 9999, 1);
    testee.set(game::v3::structures::PlanetSection, 9999, 1);
    testee.set(game::v3::structures::BaseSection, 9999, 1);

    // Save and verify that it's empty
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("foo");
    testee.setFileOwner(0);
    testee.save(*dir);
    TS_ASSERT_EQUALS(dir->getDirectoryEntryByName("control.dat")->getFileType(), afl::io::DirectoryEntry::tFile);

    afl::base::Ref<afl::io::FileMapping> map(dir->openFile("control.dat", afl::io::FileSystem::OpenRead)->createVirtualMapping());
    afl::base::ConstBytes_t bytes(map->get());
    TS_ASSERT_EQUALS(bytes.size(), 6002U);
    while (const uint8_t* p = bytes.eat()) {
        TS_ASSERT_EQUALS(*p, 0U);
    }
}
