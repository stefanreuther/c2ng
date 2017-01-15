/**
  *  \file u/t_game_v3_controlfile.cpp
  *  \brief Test for game::v3::ControlFile
  */

#include "game/v3/controlfile.hpp"

#include "t_game_v3.hpp"
#include "afl/base/ref.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/io/directoryentry.hpp"

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
