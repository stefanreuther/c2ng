/**
  *  \file test/util/directorybrowsertest.cpp
  *  \brief Test for util::DirectoryBrowser
  */

#include "util/directorybrowser.hpp"

#include "afl/io/internalfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

/** Generic browsing sequence test. */
AFL_TEST("util.DirectoryBrowser:basics", a)
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
    a.checkEqual("01. getErrorText", testee.getErrorText(), "");

    // Verify root
    a.checkEqual("11. path size",           testee.path().size(), 1U);
    a.checkEqual("12. getDirectoryName",    testee.path()[0]->getDirectoryName(), "/");
    a.checkEqual("13. dir size",            testee.directories().size(), 1U);
    a.checkEqual("14. dir title",           testee.directories()[0].title, "dir");
    a.checkEqual("15. files size",          testee.files().size(), 0U);
    a.checkEqual("16. getSelectedChild",    testee.getSelectedChild().isValid(), false);
    a.checkEqual("17. getCurrentDirectory", testee.getCurrentDirectory()->getDirectoryName(), "/");

    // Enter child
    testee.openChild(0);
    a.checkEqual("21. path size",           testee.path().size(), 2U);
    a.checkEqual("22. getDirectoryName",    testee.path()[0]->getDirectoryName(), "/");
    a.checkEqual("23. getDirectoryName",    testee.path()[1]->getDirectoryName(), "/dir");
    a.checkEqual("24. dir size",            testee.directories().size(), 1U);
    a.checkEqual("25. dir title",           testee.directories()[0].title, "subdir");
    a.checkEqual("26. files size",          testee.files().size(), 3U);
    a.checkEqual("27. file title",          testee.files()[0]->getTitle(), "file1.c");
    a.checkEqual("28. file title",          testee.files()[1]->getTitle(), "file2.h");
    a.checkEqual("29. file title",          testee.files()[2]->getTitle(), "file4.c");
    a.checkEqual("30. getSelectedChild",    testee.getSelectedChild().isValid(), false);
    a.checkEqual("31. getCurrentDirectory", testee.getCurrentDirectory()->getDirectoryName(), "/dir");

    // Go back up
    testee.openParent();
    a.checkEqual("41. path size",           testee.path().size(), 1U);
    a.checkEqual("42. getDirectoryName",    testee.path()[0]->getDirectoryName(), "/");
    a.checkEqual("43. dir size",            testee.directories().size(), 1U);
    a.checkEqual("44. dir title",           testee.directories()[0].title, "dir");
    a.checkEqual("45. files size",          testee.files().size(), 0U);
    a.checkEqual("46. getSelectedChild",    testee.getSelectedChild().isValid(), true);
    a.checkEqual("47. getSelectedChild",    testee.getSelectedChild().orElse(777), 0U);
    a.checkEqual("48. getCurrentDirectory", testee.getCurrentDirectory()->getDirectoryName(), "/");
}

/** Test openDirectory, go up 2 levels. */
AFL_TEST("util.DirectoryBrowser:openDirectory:go-up-2", a)
{
    // Environment
    afl::io::InternalFileSystem fs;
    afl::string::NullTranslator tx;
    fs.createDirectory("/dir");
    fs.createDirectory("/dir/a");
    fs.createDirectory("/dir/b");
    fs.createDirectory("/dir/c");
    fs.createDirectory("/dir/b/1");
    fs.createDirectory("/dir/b/2");
    fs.createDirectory("/dir/b/2/x");

    // Browse /dir/b/2
    util::DirectoryBrowser testee(fs);
    testee.openDirectory("/dir/b/2");
    a.checkEqual("01. dir size",  testee.directories().size(), 1U);
    a.checkEqual("02. dir title", testee.directories()[0].title, "x");

    // Go up two levels
    testee.openDirectory("/dir");
    a.checkEqual("11. dir size",  testee.directories().size(), 3U);
    a.checkEqual("12. dir title", testee.directories()[0].title, "a");
    a.checkEqual("13. dir title", testee.directories()[1].title, "b");
    a.checkEqual("14. dir title", testee.directories()[2].title, "c");
    a.checkEqual("15. cursor", testee.getSelectedChild().orElse(99), 1U);
}

/** Test openDirectory, go up 1 level. */
AFL_TEST("util.DirectoryBrowser:openDirectory:go-up-1", a)
{
    // Environment
    afl::io::InternalFileSystem fs;
    afl::string::NullTranslator tx;
    fs.createDirectory("/dir");
    fs.createDirectory("/dir/a");
    fs.createDirectory("/dir/b");
    fs.createDirectory("/dir/c");
    fs.createDirectory("/dir/b/1");
    fs.createDirectory("/dir/b/2");
    fs.createDirectory("/dir/b/2/x");

    // Browse /dir/b/2
    util::DirectoryBrowser testee(fs);
    testee.openDirectory("/dir/b/2");
    a.checkEqual("01. dir size",  testee.directories().size(), 1U);
    a.checkEqual("02. dir title", testee.directories()[0].title, "x");

    // Go up oen level
    testee.openDirectory("/dir/b");
    a.checkEqual("11. dir size",  testee.directories().size(), 2U);
    a.checkEqual("12. dir title", testee.directories()[0].title, "1");
    a.checkEqual("13. dir title", testee.directories()[1].title, "2");
    a.checkEqual("14. cursor", testee.getSelectedChild().orElse(99), 1U);
}

/** Test openDirectory, stay. */
AFL_TEST("util.DirectoryBrowser:openDirectory:stay", a)
{
    // Environment
    afl::io::InternalFileSystem fs;
    afl::string::NullTranslator tx;
    fs.createDirectory("/dir");
    fs.createDirectory("/dir/a");
    fs.createDirectory("/dir/b");
    fs.createDirectory("/dir/c");
    fs.createDirectory("/dir/b/1");
    fs.createDirectory("/dir/b/2");
    fs.createDirectory("/dir/b/2/x");

    // Browse /dir/b/2
    util::DirectoryBrowser testee(fs);
    testee.openDirectory("/dir/b/2");
    a.checkEqual("01. dir size",  testee.directories().size(), 1U);
    a.checkEqual("02. dir title", testee.directories()[0].title, "x");

    // Reload; this resets the cursor
    testee.openDirectory("/dir/b/2");
    a.checkEqual("11. dir size",  testee.directories().size(), 1U);
    a.checkEqual("12. dir title", testee.directories()[0].title, "x");
    a.checkEqual("13. cursor", testee.getSelectedChild().orElse(99), 99U);
}

/** Test createDirectory(). */
AFL_TEST("util.DirectoryBrowser:createDirectory", a)
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
    a.checkEqual("01. dir size",  testee.directories().size(), 3U);
    a.checkEqual("02. dir title", testee.directories()[0].title, "a");
    a.checkEqual("03. dir title", testee.directories()[1].title, "e");
    a.checkEqual("04. dir title", testee.directories()[2].title, "i");

    // Create
    a.checkEqual("11. createDirectory", testee.createDirectory("f", tx), "");

    // Verify
    a.checkEqual("21. dir size",         testee.directories().size(), 4U);
    a.checkEqual("22. dir title",        testee.directories()[0].title, "a");
    a.checkEqual("23. dir title",        testee.directories()[1].title, "e");
    a.checkEqual("24. dir title",        testee.directories()[2].title, "f");
    a.checkEqual("25. dir title",        testee.directories()[3].title, "i");
    a.checkEqual("26. getSelectedChild", testee.getSelectedChild().orElse(777), 2U);

    AFL_CHECK_SUCCEEDS(a("31. openDirectory"), fs.openDirectory("/dir/f")->getDirectoryEntries());
}

/** Test selectChild(), getSelectedChild(). */
AFL_TEST("util.DirectoryBrowser:selectChild", a)
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
    a.checkEqual("01. getSelectedChild", testee.getSelectedChild().isValid(), false);

    testee.selectChild(2);
    a.checkEqual("11. getSelectedChild", testee.getSelectedChild().isValid(), true);
    a.checkEqual("12. getSelectedChild", testee.getSelectedChild().orElse(777), 2U);
}

/** Test wildcard handling. */
AFL_TEST("util.DirectoryBrowser:addFileNamePattern", a)
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
    a.checkEqual("01. getErrorText", testee.getErrorText(), "");
    a.checkEqual("02. files size", testee.files().size(), 2U);
    a.checkEqual("03. file title", testee.files()[0]->getTitle(), "file1.c");
    a.checkEqual("04. file title", testee.files()[1]->getTitle(), "file4.c");

    // Add more wildcards
    testee.addFileNamePattern(util::FileNamePattern("*.h"));
    testee.loadContent();
    a.checkEqual("11. files size", testee.files().size(), 3U);
    a.checkEqual("12. file title", testee.files()[0]->getTitle(), "file1.c");
    a.checkEqual("13. file title", testee.files()[1]->getTitle(), "file2.h");
    a.checkEqual("14. file title", testee.files()[2]->getTitle(), "file4.c");

    // Reset wildcards
    testee.clearFileNamePatterns();
    testee.loadContent();
    a.checkEqual("21. files size", testee.files().size(), 0U);
}

/** Test openRoot(). */
AFL_TEST("util.DirectoryBrowser:openRoot", a)
{
    // Environment
    afl::io::InternalFileSystem fs;

    // Test root
    util::DirectoryBrowser testee(fs);
    testee.openRoot();
    testee.loadContent();    // If first call is openRoot(), it must be followed by loadContent()
    a.checkEqual    ("01. path size",  testee.path().size(), 0U);
    a.checkDifferent("02. dir size",   testee.directories().size(), 0U);
    a.checkEqual    ("03. files size", testee.files().size(), 0U);
}

