/**
  *  \file test/util/backupfiletest.cpp
  *  \brief Test for util::BackupFile
  */

#include "util/backupfile.hpp"

#include "afl/except/fileproblemexception.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

/** Test the expandFileName() function. */
AFL_TEST("util.BackupFile:expandFileName", a)
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
    a.checkEqual("01", t.expandFileName(fs, ""), "");
    a.checkEqual("02", t.expandFileName(fs, "%%"), "%");
    a.checkEqual("03", t.expandFileName(fs, "a%%p"), "a%p");
    a.checkEqual("04", t.expandFileName(fs, "%'"), "'");
    a.checkEqual("05", t.expandFileName(fs, "%"), "%");

    // - no directory
    a.checkEqual("11", t.expandFileName(fs, "player%p.%t"), "player3.042");

    // - referencing full game directory name
    a.checkEqual("21", t.expandFileName(fs, "%d/player%p.%t"), "game/dir/player3.042");
    a.checkEqual("22", t.expandFileName(fs, "%dplayer%p.%t"), "game/dir/player3.042");

    // - referencing game directory base name
    a.checkEqual("31", t.expandFileName(fs, "backup/%d/player%p.%t"), "backup/dir/player3.042");

    // Game directory test with trailing slash
    t.setGameDirectoryName("game/dir/");
    a.checkEqual("41", t.expandFileName(fs, "%d/player%p.%t"), "game/dir/player3.042");
    a.checkEqual("42", t.expandFileName(fs, "%dplayer%p.%t"), "game/dir/player3.042");
}

/** Test file operations. */
AFL_TEST("util.BackupFile:file-operations", a)
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
    a.checkEqual        ("01. hasFile",    t.hasFile(fs, TPL), false);
    AFL_CHECK_THROWS(a  ("02. openFile"),  t.openFile(fs, TPL, tx), afl::except::FileProblemException);
    AFL_CHECK_SUCCEEDS(a("03. eraseFile"), t.eraseFile(fs, TPL));

    // Create a file
    afl::io::ConstMemoryStream ms(afl::string::toBytes("content"));
    AFL_CHECK_SUCCEEDS(a("11. copyFile"), t.copyFile(fs, TPL, ms));

    // Verify
    a.checkEqual("21. hasFile", t.hasFile(fs, TPL), true);
    afl::base::Ref<afl::io::Stream> in = t.openFile(fs, TPL, tx);
    a.checkEqual("22. content", afl::string::fromBytes(in->createVirtualMapping()->get()), "content");

    AFL_CHECK_SUCCEEDS(a("31. eraseFile"), t.eraseFile(fs, TPL));
    a.checkEqual        ("32. hasFile",    t.hasFile(fs, TPL), false);
}

/** Test file operations with empty template. */
AFL_TEST("util.BackupFile:file-operations:empty-template", a)
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
    a.checkEqual        ("01. hasFile",    t.hasFile(fs, TPL), false);
    AFL_CHECK_THROWS(a  ("02. openFile"),  t.openFile(fs, TPL, tx), afl::except::FileProblemException);
    AFL_CHECK_SUCCEEDS(a("03. eraseFile"), t.eraseFile(fs, TPL));

    // Create a file
    afl::io::ConstMemoryStream ms(afl::string::toBytes("content"));
    AFL_CHECK_SUCCEEDS(a("11. copyFile"), t.copyFile(fs, TPL, ms));

    // Verify: file is not created
    a.checkEqual("21. hasFile", t.hasFile(fs, TPL), false);
}
