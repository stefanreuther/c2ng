/**
  *  \file u/t_util_directorybrowser.cpp
  *  \brief Test for util::DirectoryBrowser
  */

#include "util/directorybrowser.hpp"

#include "t_util.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"

/** Generic browsing sequence test. */
void
TestUtilDirectoryBrowser::testIt()
{
    // Environment
    afl::io::InternalFileSystem fs;
    fs.createDirectory("/dir");
    fs.createDirectory("/dir/subdir");
    fs.openFile("/dir/file1.c", afl::io::FileSystem::Create);
    fs.openFile("/dir/file2.h", afl::io::FileSystem::Create);
    fs.openFile("/dir/file3.d", afl::io::FileSystem::Create);
    fs.openFile("/dir/file4.c", afl::io::FileSystem::Create);

    util::DirectoryBrowser testee(fs);
    testee.addFileNamePattern(util::FileNamePattern("*.c"));
    testee.addFileNamePattern(util::FileNamePattern("*.h"));

    // Browse root
    testee.openDirectory("/");
    TS_ASSERT_EQUALS(testee.getErrorText(), "");

    // Verify root
    TS_ASSERT_EQUALS(testee.path().size(), 1U);
    TS_ASSERT_EQUALS(testee.path()[0]->getDirectoryName(), "/");
    TS_ASSERT_EQUALS(testee.directories().size(), 1U);
    TS_ASSERT_EQUALS(testee.directories()[0].title, "dir");
    TS_ASSERT_EQUALS(testee.files().size(), 0U);
    TS_ASSERT_EQUALS(testee.getSelectedChild().isValid(), false);
    TS_ASSERT_EQUALS(testee.getCurrentDirectory()->getDirectoryName(), "/");

    // Enter child
    testee.openChild(0);
    TS_ASSERT_EQUALS(testee.path().size(), 2U);
    TS_ASSERT_EQUALS(testee.path()[0]->getDirectoryName(), "/");
    TS_ASSERT_EQUALS(testee.path()[1]->getDirectoryName(), "/dir");
    TS_ASSERT_EQUALS(testee.directories().size(), 1U);
    TS_ASSERT_EQUALS(testee.directories()[0].title, "subdir");
    TS_ASSERT_EQUALS(testee.files().size(), 3U);
    TS_ASSERT_EQUALS(testee.files()[0]->getTitle(), "file1.c");
    TS_ASSERT_EQUALS(testee.files()[1]->getTitle(), "file2.h");
    TS_ASSERT_EQUALS(testee.files()[2]->getTitle(), "file4.c");
    TS_ASSERT_EQUALS(testee.getSelectedChild().isValid(), false);
    TS_ASSERT_EQUALS(testee.getCurrentDirectory()->getDirectoryName(), "/dir");

    // Go back up
    testee.openParent();
    TS_ASSERT_EQUALS(testee.path().size(), 1U);
    TS_ASSERT_EQUALS(testee.path()[0]->getDirectoryName(), "/");
    TS_ASSERT_EQUALS(testee.directories().size(), 1U);
    TS_ASSERT_EQUALS(testee.directories()[0].title, "dir");
    TS_ASSERT_EQUALS(testee.files().size(), 0U);
    TS_ASSERT_EQUALS(testee.getSelectedChild().isValid(), true);
    TS_ASSERT_EQUALS(testee.getSelectedChild().orElse(777), 0U);
    TS_ASSERT_EQUALS(testee.getCurrentDirectory()->getDirectoryName(), "/");
}

/** Test createDirectory(). */
void
TestUtilDirectoryBrowser::testCreateDirectory()
{
    // Environment
    afl::io::InternalFileSystem fs;
    afl::string::NullTranslator tx;
    fs.createDirectory("/dir");
    fs.createDirectory("/dir/a");
    fs.createDirectory("/dir/e");
    fs.createDirectory("/dir/i");

    // Browse /dir
    util::DirectoryBrowser testee(fs);
    testee.openDirectory("/dir");
    TS_ASSERT_EQUALS(testee.directories().size(), 3U);
    TS_ASSERT_EQUALS(testee.directories()[0].title, "a");
    TS_ASSERT_EQUALS(testee.directories()[1].title, "e");
    TS_ASSERT_EQUALS(testee.directories()[2].title, "i");

    // Create
    TS_ASSERT_EQUALS(testee.createDirectory("f", tx), "");

    // Verify
    TS_ASSERT_EQUALS(testee.directories().size(), 4U);
    TS_ASSERT_EQUALS(testee.directories()[0].title, "a");
    TS_ASSERT_EQUALS(testee.directories()[1].title, "e");
    TS_ASSERT_EQUALS(testee.directories()[2].title, "f");
    TS_ASSERT_EQUALS(testee.directories()[3].title, "i");
    TS_ASSERT_EQUALS(testee.getSelectedChild().orElse(777), 2U);

    TS_ASSERT_THROWS_NOTHING(fs.openDirectory("/dir/f")->getDirectoryEntries());
}

/** Test selectChild(), getSelectedChild(). */
void
TestUtilDirectoryBrowser::testSelect()
{
    // Environment
    afl::io::InternalFileSystem fs;
    fs.createDirectory("/dir");
    fs.createDirectory("/dir/a");
    fs.createDirectory("/dir/e");
    fs.createDirectory("/dir/i");

    // Browse /dir
    util::DirectoryBrowser testee(fs);
    testee.openDirectory("/dir");
    TS_ASSERT_EQUALS(testee.getSelectedChild().isValid(), false);

    testee.selectChild(2);
    TS_ASSERT_EQUALS(testee.getSelectedChild().isValid(), true);
    TS_ASSERT_EQUALS(testee.getSelectedChild().orElse(777), 2U);
}

/** Test wildcard handling. */
void
TestUtilDirectoryBrowser::testWildcard()
{
    // Environment
    afl::io::InternalFileSystem fs;
    fs.createDirectory("/dir");
    fs.createDirectory("/dir/subdir");
    fs.openFile("/dir/file1.c", afl::io::FileSystem::Create);
    fs.openFile("/dir/file2.h", afl::io::FileSystem::Create);
    fs.openFile("/dir/file3.d", afl::io::FileSystem::Create);
    fs.openFile("/dir/file4.c", afl::io::FileSystem::Create);

    util::DirectoryBrowser testee(fs);
    testee.addFileNamePattern(util::FileNamePattern("*.c"));

    // Browse root
    testee.openDirectory("/dir");
    TS_ASSERT_EQUALS(testee.getErrorText(), "");
    TS_ASSERT_EQUALS(testee.files().size(), 2U);
    TS_ASSERT_EQUALS(testee.files()[0]->getTitle(), "file1.c");
    TS_ASSERT_EQUALS(testee.files()[1]->getTitle(), "file4.c");

    // Add more wildcards
    testee.addFileNamePattern(util::FileNamePattern("*.h"));
    testee.loadContent();
    TS_ASSERT_EQUALS(testee.files().size(), 3U);
    TS_ASSERT_EQUALS(testee.files()[0]->getTitle(), "file1.c");
    TS_ASSERT_EQUALS(testee.files()[1]->getTitle(), "file2.h");
    TS_ASSERT_EQUALS(testee.files()[2]->getTitle(), "file4.c");

    // Reset wildcards
    testee.clearFileNamePatterns();
    testee.loadContent();
    TS_ASSERT_EQUALS(testee.files().size(), 0U);
}

/** Test openRoot(). */
void
TestUtilDirectoryBrowser::testRoot()
{
    // Environment
    afl::io::InternalFileSystem fs;

    // Test root
    util::DirectoryBrowser testee(fs);
    testee.openRoot();
    testee.loadContent();    // If first call is openRoot(), it must be followed by loadContent()
    TS_ASSERT_EQUALS(testee.path().size(), 0U);
    TS_ASSERT_DIFFERS(testee.directories().size(), 0U);
    TS_ASSERT_EQUALS(testee.files().size(), 0U);
}

