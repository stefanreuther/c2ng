/**
  *  \file u/t_util_backupfile.cpp
  *  \brief Test for util::BackupFile
  */

#include "util/backupfile.hpp"

#include "t_util.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/filemapping.hpp"

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
    TS_ASSERT_EQUALS(t.expandFileName(fs, "%%"), "%");
    TS_ASSERT_EQUALS(t.expandFileName(fs, "a%%p"), "a%p");
    TS_ASSERT_EQUALS(t.expandFileName(fs, "%'"), "'");
    TS_ASSERT_EQUALS(t.expandFileName(fs, "%"), "%");

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

/** Test file operations. */
void
TestUtilBackupFile::testFileOperations()
{
    // Preconditions
    afl::io::InternalFileSystem fs;
    afl::string::NullTranslator tx;

    // Configure an object
    util::BackupFile t;
    t.setGameDirectoryName("game/dir");
    t.setPlayerNumber(3);
    t.setTurnNumber(42);

    const String_t TPL = "%d/player%p.%t";

    // Verify
    TS_ASSERT_EQUALS(t.hasFile(fs, TPL), false);
    TS_ASSERT_THROWS(t.openFile(fs, TPL, tx), afl::except::FileProblemException);
    TS_ASSERT_THROWS_NOTHING(t.eraseFile(fs, TPL));

    // Create a file
    afl::io::ConstMemoryStream ms(afl::string::toBytes("content"));
    TS_ASSERT_THROWS_NOTHING(t.copyFile(fs, TPL, ms));

    // Verify
    TS_ASSERT_EQUALS(t.hasFile(fs, TPL), true);
    afl::base::Ref<afl::io::Stream> in = t.openFile(fs, TPL, tx);
    TS_ASSERT_EQUALS(afl::string::fromBytes(in->createVirtualMapping()->get()), "content");

    TS_ASSERT_THROWS_NOTHING(t.eraseFile(fs, TPL));
    TS_ASSERT_EQUALS(t.hasFile(fs, TPL), false);
}

/** Test file operations with empty template. */
void
TestUtilBackupFile::testFileOperationsEmpty()
{
    // Preconditions
    afl::io::InternalFileSystem fs;
    afl::string::NullTranslator tx;

    // Configure an object
    util::BackupFile t;
    t.setGameDirectoryName("game/dir");
    t.setPlayerNumber(3);
    t.setTurnNumber(42);

    const String_t TPL = "";

    // Verify
    TS_ASSERT_EQUALS(t.hasFile(fs, TPL), false);
    TS_ASSERT_THROWS(t.openFile(fs, TPL, tx), afl::except::FileProblemException);
    TS_ASSERT_THROWS_NOTHING(t.eraseFile(fs, TPL));

    // Create a file
    afl::io::ConstMemoryStream ms(afl::string::toBytes("content"));
    TS_ASSERT_THROWS_NOTHING(t.copyFile(fs, TPL, ms));

    // Verify: file is not created
    TS_ASSERT_EQUALS(t.hasFile(fs, TPL), false);
}

