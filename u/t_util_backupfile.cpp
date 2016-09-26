/**
  *  \file u/t_util_backupfile.cpp
  *  \brief Test for util::BackupFile
  */

#include "util/backupfile.hpp"

#include "t_util.hpp"
#include "afl/io/nullfilesystem.hpp"

/** Test the expandFileName() function. */
void
TestUtilBackupFile::testExpand()
{
    // Preconditions
    afl::io::NullFileSystem fs;

    // Configure an object
    util::BackupFile t;
    t.setGameDirectoryName("game/dir");
    t.setPlayerNumber(3);
    t.setTurnNumber(42);

    // Test
    // - trivial case
    TS_ASSERT_EQUALS(t.expandFileName(fs, ""), "");

    // - no directory
    TS_ASSERT_EQUALS(t.expandFileName(fs, "player%p.%t"), "player3.042");

    // - referencing full game directory name
    TS_ASSERT_EQUALS(t.expandFileName(fs, "%d/player%p.%t"), "game/dir/player3.042");
    TS_ASSERT_EQUALS(t.expandFileName(fs, "%dplayer%p.%t"), "game/dir/player3.042");

    // - referencing game directory base name
    TS_ASSERT_EQUALS(t.expandFileName(fs, "backup/%d/player%p.%t"), "backup/dir/player3.042");

    // Game directory test with trailing slash
    t.setGameDirectoryName("game/dir/");
    TS_ASSERT_EQUALS(t.expandFileName(fs, "%d/player%p.%t"), "game/dir/player3.042");
    TS_ASSERT_EQUALS(t.expandFileName(fs, "%dplayer%p.%t"), "game/dir/player3.042");
}
