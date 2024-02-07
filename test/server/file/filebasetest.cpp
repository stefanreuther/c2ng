/**
  *  \file test/server/file/filebasetest.cpp
  *  \brief Test for server::file::FileBase
  */

#include "server/file/filebase.hpp"

#include "afl/data/access.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/test/testrunner.hpp"
#include "server/file/ca/root.hpp"
#include "server/file/directoryhandler.hpp"
#include "server/file/directoryitem.hpp"
#include "server/file/internaldirectoryhandler.hpp"
#include "server/file/root.hpp"
#include "server/file/session.hpp"
#include <stdexcept>

#define AFL_CHECK_THROWS_CODE(a, call, code)                            \
                              do {                                      \
                                  bool threw = false;                   \
                                  try {                                 \
                                      call;                             \
                                  }                                     \
                                  catch (std::exception& e) {           \
                                      a.checkEqual("01. what", String_t(e.what()).substr(0, 3), code); \
                                      threw = true;                     \
                                  }                                     \
                                  catch (...) {                         \
                                      a.fail("02. Wrong exception");    \
                                      threw = true;                     \
                                  }                                     \
                                  a.check("03. threw", threw);          \
                              } while (0)

using server::file::InternalDirectoryHandler;

namespace {
    struct Testbench {
        InternalDirectoryHandler::Directory dir;
        server::file::DirectoryItem item;
        server::file::Root root;
        server::file::Session session;

        Testbench()
            : dir(""),
              item("(root)", 0, std::auto_ptr<server::file::DirectoryHandler>(new InternalDirectoryHandler("(root)", dir))),
              root(item, afl::io::InternalDirectory::create("(spec)")),
              session()
            { }
    };
}

/** Some simple tests. */
AFL_TEST("server.file.FileBase:basics", a)
{
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    AFL_CHECK_SUCCEEDS(a("01. createDirectory"), testee.createDirectory("d"));
    AFL_CHECK_SUCCEEDS(a("02. createDirectory"), testee.createDirectory("d/sd"));
    AFL_CHECK_SUCCEEDS(a("03. putFile"),         testee.putFile("d/f", "content..."));
    a.checkEqual("04. getFile", testee.getFile("d/f"), "content...");

    server::interface::FileBase::Info i;
    AFL_CHECK_SUCCEEDS(a("11. getFileInformation"), i = testee.getFileInformation("d"));
    a.checkEqual("12. type", i.type, server::interface::FileBase::IsDirectory);

    AFL_CHECK_SUCCEEDS(a("21. getFileInformation"), i = testee.getFileInformation("d/f"));
    a.checkEqual("22. type", i.type, server::interface::FileBase::IsFile);
    a.checkEqual("23. size", i.size.orElse(-1), 10);

    AFL_CHECK_THROWS(a("31. createDirectory"), testee.createDirectory("d"), std::exception);
    AFL_CHECK_THROWS(a("32. createDirectory"), testee.createDirectory("d/f"), std::exception);
    AFL_CHECK_THROWS(a("33. putFile"), testee.putFile("d/sd", "xx"), std::exception);
}

/** Test createDirectory variants. */
AFL_TEST("server.file.FileBase:createDirectory", a)
{
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    // Create a file in root
    AFL_CHECK_SUCCEEDS(a("01. putFile"), testee.putFile("f", ""));

    // Admin context: create directories
    // - success case
    AFL_CHECK_SUCCEEDS(a("11. createDirectory"), testee.createDirectoryAsUser("u", "1001"));
    AFL_CHECK_SUCCEEDS(a("12. createDirectory"), testee.createDirectoryAsUser("w", "1002"));
    AFL_CHECK_SUCCEEDS(a("13. createDirectory"), testee.createDirectory("u/sub"));

    // - failure case: missing user name
    AFL_CHECK_THROWS_CODE(a("21. createDirectoryAsUser"), testee.createDirectoryAsUser("v", ""), "400");

    // - failure case: already exists
    AFL_CHECK_THROWS_CODE(a("31. createDirectoryAsUser"), testee.createDirectoryAsUser("u", "1001"), "409");
    AFL_CHECK_THROWS_CODE(a("32. createDirectory"),       testee.createDirectory("u"),               "409");
    AFL_CHECK_THROWS_CODE(a("33. createDirectory"),       testee.createDirectory("f"),               "409");

    // - failure case: bad file names
    AFL_CHECK_THROWS_CODE(a("41. createDirectory"), testee.createDirectory(""),                    "400");
    AFL_CHECK_THROWS_CODE(a("42. createDirectory"), testee.createDirectory("/a"),                  "400");
    AFL_CHECK_THROWS_CODE(a("43. createDirectory"), testee.createDirectory("u//a"),                "400");
    AFL_CHECK_THROWS_CODE(a("44. createDirectory"), testee.createDirectory("u/a:b"),               "400");
    AFL_CHECK_THROWS_CODE(a("45. createDirectory"), testee.createDirectory("u/a\\b"),              "400");
    AFL_CHECK_THROWS_CODE(a("46. createDirectory"), testee.createDirectory("u/.dot"),              "400");
    AFL_CHECK_THROWS_CODE(a("47. createDirectory"), testee.createDirectory(String_t("u/a\0b", 5)), "400");

    // User context
    tb.session.setUser("1001");

    // - success case
    AFL_CHECK_SUCCEEDS(a("51. createDirectory"), testee.createDirectory("u/sub2"));

    // - failure case: missing permissions
    AFL_CHECK_THROWS_CODE(a("61. createDirectoryAsUser"), testee.createDirectoryAsUser("u/sub3", "1002"), "403");
    AFL_CHECK_THROWS_CODE(a("62. createDirectory"),       testee.createDirectory("v"),                    "403");
    AFL_CHECK_THROWS_CODE(a("63. createDirectory"),       testee.createDirectory("w/x"),                  "403");

    // - failure case: already exists (but also missing permissions), so reports missing permissions
    AFL_CHECK_THROWS_CODE(a("71. createDirectory"), testee.createDirectory("u"), "403");
    AFL_CHECK_THROWS_CODE(a("72. createDirectory"), testee.createDirectory("f"), "403");

    // - failure case: already exists
    AFL_CHECK_THROWS_CODE(a("81. createDirectory"), testee.createDirectory("u/sub"), "409");
}

/** Test getFile() and copyFile(). */
AFL_TEST("server.file.FileBase:getFile", a)
{
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    // Create test setup
    testee.createDirectoryAsUser("u1", "1001");
    testee.createDirectory("u1/sub");
    testee.putFile("u1/f", "u1 f");
    testee.putFile("u1/sub/f", "u1 sub f");

    testee.createDirectoryAsUser("u2", "1002");
    testee.putFile("u2/f", "u2 f");

    testee.setDirectoryPermissions("u2", "1003", "r");
    testee.setDirectoryPermissions("u2", "1004", "l");

    testee.createDirectory("tmp");
    testee.setDirectoryPermissions("tmp", "*", "w");

    // Some file name stuff
    AFL_CHECK_THROWS_CODE(a("01. getFile"), testee.getFile("/"), "400");
    AFL_CHECK_THROWS_CODE(a("02. getFile"), testee.getFile("u1//a"), "400");
    AFL_CHECK_THROWS_CODE(a("03. getFile"), testee.getFile("u1/x:y/a"), "400");
    AFL_CHECK_THROWS_CODE(a("04. getFile"), testee.getFile("u1/x:y"), "400");
    AFL_CHECK_THROWS_CODE(a("05. getFile"), testee.getFile("u1//"), "400");

    // User 1
    // - getFile
    tb.session.setUser("1001");
    AFL_CHECK_THROWS_CODE(a("11. getFile"), testee.getFile("u1"), "403");                         // access a directory we can read
    AFL_CHECK_THROWS_CODE(a("12. getFile"), testee.getFile("u1/g"), "404");                       // access nonexistant file in a directory we can read
    a.checkEqual           ("13. getFile",  testee.getFile("u1/f"), "u1 f");                      // ok
    a.checkEqual           ("14. getFile",  testee.getFile("u1/sub/f"), "u1 sub f");              // ok
    AFL_CHECK_THROWS_CODE(a("15. getFile"), testee.getFile("u2/f"), "403");                       // access existing file in a directory we cannot read
    AFL_CHECK_THROWS_CODE(a("16. getFile"), testee.getFile("u2/g"), "403");                       // access nonexistant file in a directory we cannot read
    AFL_CHECK_THROWS_CODE(a("17. getFile"), testee.getFile("u2/g/g"), "403");                     // access nonexistant file in a directory we cannot read

    // - copyFile
    AFL_CHECK_THROWS_CODE(a("21. copyFile"), testee.copyFile("u1", "tmp/x"), "403");          // access a directory we can read
    AFL_CHECK_THROWS_CODE(a("22. copyFile"), testee.copyFile("u1/g", "tmp/x"), "404");        // access nonexistant file in a directory we can read
    AFL_CHECK_SUCCEEDS(a   ("23. copyFile"), testee.copyFile("u1/f", "tmp/x"));               // ok
    AFL_CHECK_SUCCEEDS(a   ("24. copyFile"), testee.copyFile("u1/sub/f", "tmp/x"));           // ok
    AFL_CHECK_THROWS_CODE(a("25. copyFile"), testee.copyFile("u2/f", "tmp/x"), "403");        // access existing file in a directory we cannot read
    AFL_CHECK_THROWS_CODE(a("26. copyFile"), testee.copyFile("u2/g", "tmp/x"), "403");        // access nonexistant file in a directory we cannot read
    AFL_CHECK_THROWS_CODE(a("27. copyFile"), testee.copyFile("u2/g/g", "tmp/x"), "403");      // access nonexistant file in a directory we cannot read

    // User 2
    // - getFile
    tb.session.setUser("1002");
    AFL_CHECK_THROWS_CODE(a("31. getFile"), testee.getFile("u1"), "403");                        // access a directory
    AFL_CHECK_THROWS_CODE(a("32. getFile"), testee.getFile("u1/g"), "403");                      // access nonexistant file in a directory we cannot read
    AFL_CHECK_THROWS_CODE(a("34. getFile"), testee.getFile("u1/f"), "403");                      // access existing file in a directory we cannot read
    AFL_CHECK_THROWS_CODE(a("34. getFile"), testee.getFile("u1/sub/f"), "403");                  // ditto
    a.checkEqual           ("35. getFile", testee.getFile("u2/f"), "u2 f");                      // ok
    AFL_CHECK_THROWS_CODE(a("36. getFile"), testee.getFile("u2/g"), "404");                      // access nonexistant file in a directory we can read
    AFL_CHECK_THROWS_CODE(a("37. getFile"), testee.getFile("u2/g/g"), "404");                    // access nonexistant file in a directory we can read

    // - copyFile
    AFL_CHECK_THROWS_CODE(a("41. copyFile"), testee.copyFile("u1", "tmp/x"), "403");          // access a directory
    AFL_CHECK_THROWS_CODE(a("42. copyFile"), testee.copyFile("u1/g", "tmp/x"), "403");        // access nonexistant file in a directory we cannot read
    AFL_CHECK_THROWS_CODE(a("43. copyFile"), testee.copyFile("u1/f", "tmp/x"), "403");        // access existing file in a directory we cannot read
    AFL_CHECK_THROWS_CODE(a("44. copyFile"), testee.copyFile("u1/sub/f", "tmp/x"), "403");    // ditto
    AFL_CHECK_SUCCEEDS(a   ("45. copyFile"), testee.copyFile("u2/f", "tmp/x"));               // ok
    AFL_CHECK_THROWS_CODE(a("46. copyFile"), testee.copyFile("u2/g", "tmp/x"), "404");        // access nonexistant file in a directory we can read
    AFL_CHECK_THROWS_CODE(a("47. copyFile"), testee.copyFile("u2/g/g", "tmp/x"), "404");      // access nonexistant file in a directory we can read

    // User 3
    // - getFile
    tb.session.setUser("1003");
    AFL_CHECK_THROWS_CODE(a("51. getFile"), testee.getFile("u1"), "403");                        // access a directory
    AFL_CHECK_THROWS_CODE(a("52. getFile"), testee.getFile("u1/g"), "403");                      // access nonexistant file in a directory we cannot read
    AFL_CHECK_THROWS_CODE(a("53. getFile"), testee.getFile("u1/f"), "403");                      // access existing file in a directory we cannot read
    AFL_CHECK_THROWS_CODE(a("54. getFile"), testee.getFile("u1/sub/f"), "403");                  // ditto
    a.checkEqual           ("55. getFile", testee.getFile("u2/f"), "u2 f");                      // ok, user got explicit permissions to read
    AFL_CHECK_THROWS_CODE(a("56. getFile"), testee.getFile("u2/g"), "403");                      // user did not get permissions to read the directory, so this is 403
    AFL_CHECK_THROWS_CODE(a("57. getFile"), testee.getFile("u2/g/g"), "403");                    // access nonexistant file in a directory we cannot read

    // - copyFile
    AFL_CHECK_THROWS_CODE(a("61. copyFile"), testee.copyFile("u1", "tmp/x"), "403");          // access a directory
    AFL_CHECK_THROWS_CODE(a("62. copyFile"), testee.copyFile("u1/g", "tmp/x"), "403");        // access nonexistant file in a directory we cannot read
    AFL_CHECK_THROWS_CODE(a("63. copyFile"), testee.copyFile("u1/f", "tmp/x"), "403");        // access existing file in a directory we cannot read
    AFL_CHECK_THROWS_CODE(a("64. copyFile"), testee.copyFile("u1/sub/f", "tmp/x"), "403");    // ditto
    AFL_CHECK_SUCCEEDS(a   ("65. copyFile"), testee.copyFile("u2/f", "tmp/x"));               // ok, user got explicit permissions to read
    AFL_CHECK_THROWS_CODE(a("66. copyFile"), testee.copyFile("u2/g", "tmp/x"), "403");        // user did not get permissions to read the directory, so this is 403

    // User 4
    // - getFile
    tb.session.setUser("1004");
    AFL_CHECK_THROWS_CODE(a("71. getFile"), testee.getFile("u1"), "403");                    // access a directory
    AFL_CHECK_THROWS_CODE(a("72. getFile"), testee.getFile("u1/g"), "403");                  // access nonexistant file in a directory we cannot read
    AFL_CHECK_THROWS_CODE(a("73. getFile"), testee.getFile("u1/f"), "403");                  // access existing file in a directory we cannot read
    AFL_CHECK_THROWS_CODE(a("74. getFile"), testee.getFile("u1/sub/f"), "403");              // ditto
    AFL_CHECK_THROWS_CODE(a("75. getFile"), testee.getFile("u2/f"), "403");                  // user got permissions to read the directory but not the file
    AFL_CHECK_THROWS_CODE(a("76. getFile"), testee.getFile("u2/g"), "404");                  // user got permissions to know that this file does not exist
    AFL_CHECK_THROWS_CODE(a("77. getFile"), testee.getFile("u2/g/g"), "404");                // user got permissions to know that this file does not exist

    // - copyFile
    AFL_CHECK_THROWS_CODE(a("81. copyFile"), testee.copyFile("u1", "tmp/x"), "403");          // access a directory
    AFL_CHECK_THROWS_CODE(a("82. copyFile"), testee.copyFile("u1/g", "tmp/x"), "403");        // access nonexistant file in a directory we cannot read
    AFL_CHECK_THROWS_CODE(a("83. copyFile"), testee.copyFile("u1/f", "tmp/x"), "403");        // access existing file in a directory we cannot read
    AFL_CHECK_THROWS_CODE(a("84. copyFile"), testee.copyFile("u1/sub/f", "tmp/x"), "403");    // ditto
    AFL_CHECK_THROWS_CODE(a("85. copyFile"), testee.copyFile("u2/f", "tmp/x"), "403");        // user got permissions to read the directory but not the file
    AFL_CHECK_THROWS_CODE(a("86. copyFile"), testee.copyFile("u2/g", "tmp/x"), "404");        // user got permissions to know that this file does not exist
}

/** Test testFiles(). */
AFL_TEST("server.file.FileBase:testFiles", a)
{
    // Set up test bench. This is similar to the testGet() testbench.
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    // Create test setup
    testee.createDirectoryAsUser("u1", "1001");
    testee.createDirectory("u1/sub");
    testee.putFile("u1/f", "u1 f");
    testee.putFile("u1/sub/f", "u1 sub f");

    testee.createDirectoryAsUser("u2", "1002");
    testee.putFile("u2/f", "u2 f");

    testee.setDirectoryPermissions("u2", "1003", "r");
    testee.setDirectoryPermissions("u2", "1004", "l");

    const String_t FILE_NAMES[] = { "u1", "u1/g", "u1/f", "u1/sub/f", "u2/f", "u2/g", "u2/g/g" };

    // Empty
    {
        afl::data::IntegerList_t result;
        testee.testFiles(afl::base::Nothing, result);
        a.checkEqual("01. size", result.size(), 0U);
    }

    // Root
    {
        afl::data::IntegerList_t result;
        testee.testFiles(FILE_NAMES, result);
        a.checkEqual("11. size", result.size(), 7U);
        a.checkEqual("12. result", result[0], 0);
        a.checkEqual("13. result", result[1], 0);
        a.checkEqual("14. result", result[2], 1);
        a.checkEqual("15. result", result[3], 1);
        a.checkEqual("16. result", result[4], 1);
        a.checkEqual("17. result", result[5], 0);
        a.checkEqual("18. result", result[6], 0);
    }

    // User 1
    tb.session.setUser("1001");
    {
        afl::data::IntegerList_t result;
        testee.testFiles(FILE_NAMES, result);
        a.checkEqual("21. size", result.size(), 7U);
        a.checkEqual("22. result", result[0], 0);
        a.checkEqual("23. result", result[1], 0);
        a.checkEqual("24. result", result[2], 1);
        a.checkEqual("25. result", result[3], 1);
        a.checkEqual("26. result", result[4], 0);
        a.checkEqual("27. result", result[5], 0);
        a.checkEqual("28. result", result[6], 0);
    }

    // User 2
    tb.session.setUser("1002");
    {
        afl::data::IntegerList_t result;
        testee.testFiles(FILE_NAMES, result);
        a.checkEqual("31. size", result.size(), 7U);
        a.checkEqual("32. result", result[0], 0);
        a.checkEqual("33. result", result[1], 0);
        a.checkEqual("34. result", result[2], 0);
        a.checkEqual("35. result", result[3], 0);
        a.checkEqual("36. result", result[4], 1);
        a.checkEqual("37. result", result[5], 0);
        a.checkEqual("38. result", result[6], 0);
    }

    // User 3
    tb.session.setUser("1003");
    {
        afl::data::IntegerList_t result;
        testee.testFiles(FILE_NAMES, result);
        a.checkEqual("41. size", result.size(), 7U);
        a.checkEqual("42. result", result[0], 0);
        a.checkEqual("43. result", result[1], 0);
        a.checkEqual("44. result", result[2], 0);
        a.checkEqual("45. result", result[3], 0);
        a.checkEqual("46. result", result[4], 1);
        a.checkEqual("47. result", result[5], 0);
        a.checkEqual("48. result", result[6], 0);
    }

    // User 4
    tb.session.setUser("1004");
    {
        afl::data::IntegerList_t result;
        testee.testFiles(FILE_NAMES, result);
        a.checkEqual("51. size", result.size(), 7U);
        a.checkEqual("52. result", result[0], 0);
        a.checkEqual("53. result", result[1], 0);
        a.checkEqual("54. result", result[2], 0);
        a.checkEqual("55. result", result[3], 0);
        a.checkEqual("56. result", result[4], 0);
        a.checkEqual("57. result", result[5], 0);
        a.checkEqual("58. result", result[6], 0);
    }
}

/** Test getDirectoryProperty(), setDirectoryProperty(). */
AFL_TEST("server.file.FileBase:directory-properties", a)
{
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);
    testee.createDirectory("u");

    // Set and get properties
    testee.setDirectoryProperty("u", "name", "foo");
    testee.setDirectoryProperty("u", "count", "3");
    testee.setDirectoryProperty("u", "a", "e=mc2");
    a.checkEqual("01. getDirectoryStringProperty", testee.getDirectoryStringProperty("u", "name"), "foo");
    a.checkEqual("02. getDirectoryStringProperty", testee.getDirectoryStringProperty("u", "count"), "3");
    AFL_CHECK_THROWS(a("03. getDirectoryIntegerProperty"), testee.getDirectoryIntegerProperty("u", "name"), std::exception);
    a.checkEqual("04. getDirectoryIntegerProperty", testee.getDirectoryIntegerProperty("u", "count"), 3);

    // Error cases
    // - not found
    AFL_CHECK_THROWS_CODE(a("11. setDirectoryProperty"), testee.setDirectoryProperty("v", "x", "y"), "404");
    AFL_CHECK_THROWS_CODE(a("12. setDirectoryProperty"), testee.setDirectoryProperty("u/v", "x", "y"), "404");

    // - bad file name
    AFL_CHECK_THROWS_CODE(a("21. setDirectoryProperty"), testee.setDirectoryProperty("u/", "x", "y"), "400");
    AFL_CHECK_THROWS_CODE(a("22. setDirectoryProperty"), testee.setDirectoryProperty("a:b", "x", "y"), "400");
    AFL_CHECK_THROWS_CODE(a("23. setDirectoryProperty"), testee.setDirectoryProperty("u/a:b", "x", "y"), "400");

    // - bad property name
    // AFL_CHECK_THROWS_CODE(a(". setDirectoryProperty"), testee.setDirectoryProperty("u", "", "y"), "400"); <- allowed, but not sensible
    AFL_CHECK_THROWS_CODE(a("31. setDirectoryProperty"), testee.setDirectoryProperty("u", "a=b", "y"), "400");
    AFL_CHECK_THROWS_CODE(a("32. setDirectoryProperty"), testee.setDirectoryProperty("u", "=b", "y"), "400");
    AFL_CHECK_THROWS_CODE(a("33. setDirectoryProperty"), testee.setDirectoryProperty("u", "a=", "y"), "400");
    AFL_CHECK_THROWS_CODE(a("34. setDirectoryProperty"), testee.setDirectoryProperty("u", "a\nb", "y"), "400");

    // - bad property value
    AFL_CHECK_THROWS_CODE(a("41. setDirectoryProperty"), testee.setDirectoryProperty("u", "a", "y\n"), "400");

    // Forget & reload
    testee.forgetDirectory("u");
    a.checkEqual("51. getDirectoryStringProperty", testee.getDirectoryStringProperty("u", "name"), "foo");
    a.checkEqual("52. getDirectoryStringProperty", testee.getDirectoryStringProperty("u", "count"), "3");
    a.checkEqual("53. getDirectoryStringProperty", testee.getDirectoryStringProperty("u", "a"), "e=mc2");
}

/** Test getDirectoryProperty(), setDirectoryProperty() vs.\ permissions. */
AFL_TEST("server.file.FileBase:directory-properties:permissions", a)
{
    // Test setup
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);
    testee.createDirectory("writable");
    testee.createDirectory("readable");
    testee.createDirectory("both");
    testee.createDirectory("none");
    testee.createDirectory("none/readable");
    testee.createDirectory("none/writable");
    testee.createDirectory("none/none");
    testee.createDirectory("listable");

    testee.setDirectoryPermissions("writable", "1001", "w");
    testee.setDirectoryPermissions("readable", "1001", "r");
    testee.setDirectoryPermissions("both", "1001", "rw");
    testee.setDirectoryPermissions("none/readable", "1001", "r");
    testee.setDirectoryPermissions("none/writable", "1001", "w");
    testee.setDirectoryPermissions("listable", "1001", "l");

    testee.setDirectoryProperty("writable", "p", "w");
    testee.setDirectoryProperty("readable", "p", "r");
    testee.setDirectoryProperty("both", "p", "b");
    testee.setDirectoryProperty("none", "p", "n");
    testee.setDirectoryProperty("none/readable", "p", "nr");
    testee.setDirectoryProperty("none/writable", "p", "nw");
    testee.setDirectoryProperty("none/none", "p", "nn");
    testee.setDirectoryProperty("listable", "p", "l");

    // Test reading in user context
    tb.session.setUser("1001");
    AFL_CHECK_THROWS_CODE(a("01. getDirectoryStringProperty"), testee.getDirectoryStringProperty("writable", "p"),         "403");
    a.checkEqual           ("02. getDirectoryStringProperty",  testee.getDirectoryStringProperty("readable", "p"),         "r");
    a.checkEqual           ("03. getDirectoryStringProperty",  testee.getDirectoryStringProperty("both", "p"),             "b");
    AFL_CHECK_THROWS_CODE(a("04. getDirectoryStringProperty"), testee.getDirectoryStringProperty("none", "p"),             "403");
    a.checkEqual           ("05. getDirectoryStringProperty",  testee.getDirectoryStringProperty("none/readable", "p"),    "nr");
    AFL_CHECK_THROWS_CODE(a("06. getDirectoryStringProperty"), testee.getDirectoryStringProperty("none/writable", "p"),    "403");
    AFL_CHECK_THROWS_CODE(a("07. getDirectoryStringProperty"), testee.getDirectoryStringProperty("none/none", "p"),        "403");
    AFL_CHECK_THROWS_CODE(a("08. getDirectoryStringProperty"), testee.getDirectoryStringProperty("none/missing", "p"),     "403");
    AFL_CHECK_THROWS_CODE(a("09. getDirectoryStringProperty"), testee.getDirectoryStringProperty("listable", "p"),         "403");
    AFL_CHECK_THROWS_CODE(a("10. getDirectoryStringProperty"), testee.getDirectoryStringProperty("readable/missing", "p"), "403");
    AFL_CHECK_THROWS_CODE(a("11. getDirectoryStringProperty"), testee.getDirectoryStringProperty("listable/missing", "p"), "404");

    // Test writing in user context [bug #338]
    tb.session.setUser("1001");
    AFL_CHECK_SUCCEEDS(a   ("21. setDirectoryProperty"), testee.setDirectoryProperty("writable", "p", "v"));
    AFL_CHECK_THROWS_CODE(a("22. setDirectoryProperty"), testee.setDirectoryProperty("readable", "p", "v"),         "403");
    AFL_CHECK_SUCCEEDS(a   ("23. setDirectoryProperty"), testee.setDirectoryProperty("both", "p", "v"));
    AFL_CHECK_THROWS_CODE(a("24. setDirectoryProperty"), testee.setDirectoryProperty("none", "p", "v"),             "403");
    AFL_CHECK_THROWS_CODE(a("25. setDirectoryProperty"), testee.setDirectoryProperty("none/readable", "p", "v"),    "403");
    AFL_CHECK_SUCCEEDS(a   ("26. setDirectoryProperty"), testee.setDirectoryProperty("none/writable", "p", "v"));
    AFL_CHECK_THROWS_CODE(a("27. setDirectoryProperty"), testee.setDirectoryProperty("none/none", "p", "v"),        "403");
    AFL_CHECK_THROWS_CODE(a("28. setDirectoryProperty"), testee.setDirectoryProperty("none/missing", "p", "v"),     "403");
    AFL_CHECK_THROWS_CODE(a("29. setDirectoryProperty"), testee.setDirectoryProperty("listable", "p", "v"),         "403");
    AFL_CHECK_THROWS_CODE(a("30. setDirectoryProperty"), testee.setDirectoryProperty("readable/missing", "p", "v"), "403");
    AFL_CHECK_THROWS_CODE(a("31. setDirectoryProperty"), testee.setDirectoryProperty("listable/missing", "p", "v"), "404");
}

/** Test property access vs. file */
AFL_TEST("server.file.FileBase:directory-properties:file", a)
{
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    testee.putFile("f", "c");
    testee.createDirectory("d");
    testee.putFile("d/ff", "cc");

    AFL_CHECK_THROWS_CODE(a("01. getDirectoryStringProperty"), testee.getDirectoryStringProperty("f", "p"), "405");
    AFL_CHECK_THROWS_CODE(a("02. getDirectoryStringProperty"), testee.getDirectoryStringProperty("dd/ff", "p"), "404");
    AFL_CHECK_THROWS_CODE(a("03. setDirectoryProperty"), testee.setDirectoryProperty("f", "p", "v"), "405");
    AFL_CHECK_THROWS_CODE(a("04. setDirectoryProperty"), testee.setDirectoryProperty("dd/ff", "p", "v"), "404");
}

/** Test createDirectoryTree. */
AFL_TEST("server.file.FileBase:createDirectoryTree", a)
{
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    // Success case
    AFL_CHECK_SUCCEEDS(a("01. createDirectoryTree"), testee.createDirectoryTree("0/a/b/c/d/e/f/g"));

    // Repeating is ok, also with shorter and longer path
    AFL_CHECK_SUCCEEDS(a("11. createDirectoryTree"), testee.createDirectoryTree("0/a/b/c/d/e/f/g"));
    AFL_CHECK_SUCCEEDS(a("12. createDirectoryTree"), testee.createDirectoryTree("0/a/b/c/d/e"));
    AFL_CHECK_SUCCEEDS(a("13. createDirectoryTree"), testee.createDirectoryTree("0/a/b/c/d/e/f/g/h/i"));

    // Attempt to overwrite a file
    // FIXME: 409 should only be produced if we have read access!
    testee.putFile("1", "");
    AFL_CHECK_THROWS_CODE(a("21. createDirectoryTree"), testee.createDirectoryTree("1/a/b/c/d/e"), "409");

    // Attempt to overwrite a nested file
    testee.createDirectoryTree("2/a/b/c/d");
    testee.putFile("2/a/b/c/d/e", "");
    AFL_CHECK_THROWS_CODE(a("31. createDirectoryTree"), testee.createDirectoryTree("2/a/b/c/d/e/f/g/h"), "409");

    // Attempt to create without write permissions
    testee.createDirectory("3");
    testee.createDirectory("4");
    testee.setDirectoryPermissions("3", "1009", "r");
    testee.setDirectoryPermissions("4", "1009", "w");
    tb.session.setUser("1009");
    AFL_CHECK_THROWS_CODE(a("41. createDirectoryTree"), testee.createDirectoryTree("3/a/b"), "403");
    AFL_CHECK_SUCCEEDS(a("42. createDirectoryTree"), testee.createDirectoryTree("4/a"));

    // FIXME: fails, because the user has no permissions to the newly-created 4/a directory!
    // AFL_CHECK_SUCCEEDS(a("51. createDirectoryTree"), testee.createDirectoryTree("4/a/b"));
}

/** Test getFileInformation(). */
AFL_TEST("server.file.FileBase:getFileInformation", a)
{
    using server::interface::FileBase;

    // Test setup
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);
    testee.createDirectory("writable");
    testee.createDirectory("readable");
    testee.createDirectory("both");
    testee.createDirectory("none");
    testee.createDirectory("listable");

    testee.setDirectoryPermissions("writable", "1001", "w");
    testee.setDirectoryPermissions("readable", "1001", "r");
    testee.setDirectoryPermissions("both", "1001", "rw");
    testee.setDirectoryPermissions("listable", "1001", "l");

    testee.putFile("writable/f", "ww");
    testee.putFile("readable/f", "r");
    testee.putFile("both/f", "");
    testee.putFile("none/f", "");
    testee.putFile("listable/f", "");

    // Some generic tests
    // - invalid file names
    AFL_CHECK_THROWS_CODE(a("01. getFileInformation"), testee.getFileInformation(""), "400");
    AFL_CHECK_THROWS_CODE(a("02. getFileInformation"), testee.getFileInformation("/"), "400");
    AFL_CHECK_THROWS_CODE(a("03. getFileInformation"), testee.getFileInformation("readable/"), "400");
    AFL_CHECK_THROWS_CODE(a("04. getFileInformation"), testee.getFileInformation("/x"), "400");
    AFL_CHECK_THROWS_CODE(a("05. getFileInformation"), testee.getFileInformation("a:b"), "400");
    AFL_CHECK_THROWS_CODE(a("06. getFileInformation"), testee.getFileInformation("readable/a:b"), "400");

    // - non existant
    AFL_CHECK_THROWS_CODE(a("11. getFileInformation"), testee.getFileInformation("foo"), "404");
    AFL_CHECK_THROWS_CODE(a("12. getFileInformation"), testee.getFileInformation("readable/foo"), "404");

    // - Content
    FileBase::Info i;
    AFL_CHECK_SUCCEEDS(a("21. getFileInformation"), i = testee.getFileInformation("writable"));
    a.checkEqual("22. type",       i.type, FileBase::IsDirectory);
    a.checkEqual("23. visibility", i.visibility.orElse(99), 1);                    // 1 because it has some permissions

    AFL_CHECK_SUCCEEDS(a("31. getFileInformation"), i = testee.getFileInformation("none"));
    a.checkEqual("32. type",       i.type, FileBase::IsDirectory);
    a.checkEqual("33. visibility", i.visibility.orElse(99), 0);

    AFL_CHECK_SUCCEEDS(a("41. getFileInformation"), i = testee.getFileInformation("readable/f"));
    a.checkEqual("42. type",    i.type, FileBase::IsFile);
    a.checkEqual("43. isValid", i.visibility.isValid(), false);
    a.checkEqual("44. size",    i.size.orElse(99), 1);

    // Test as user 1001
    tb.session.setUser("1001");
    AFL_CHECK_THROWS_CODE(a("51. getFileInformation"), testee.getFileInformation("writable"), "403");
    AFL_CHECK_THROWS_CODE(a("52. getFileInformation"), testee.getFileInformation("writable/f"), "403");
    AFL_CHECK_THROWS_CODE(a("53. getFileInformation"), testee.getFileInformation("readable"), "403");
    AFL_CHECK_THROWS_CODE(a("54. getFileInformation"), testee.getFileInformation("readable/f"), "403");  // FIXME: should this be allowed?
    AFL_CHECK_THROWS_CODE(a("55. getFileInformation"), testee.getFileInformation("readable/foo"), "403");
    AFL_CHECK_THROWS_CODE(a("56. getFileInformation"), testee.getFileInformation("both"), "403");
    AFL_CHECK_THROWS_CODE(a("57. getFileInformation"), testee.getFileInformation("both/f"), "403");
    AFL_CHECK_THROWS_CODE(a("58. getFileInformation"), testee.getFileInformation("none"), "403");
    AFL_CHECK_THROWS_CODE(a("59. getFileInformation"), testee.getFileInformation("none/f"), "403");

    // STAT(listable) is allowed: this is the same usecase as /file.cgi/user, i.e. get information
    // about an item whose parent is not listable. See #390.
    AFL_CHECK_SUCCEEDS(a("61. getFileInformation"), i = testee.getFileInformation("listable"));
    a.checkEqual("62. type", i.type, FileBase::IsDirectory);

    AFL_CHECK_SUCCEEDS(a("71. getFileInformation"), i = testee.getFileInformation("listable/f"));
    a.checkEqual("72. type", i.type, FileBase::IsFile);
    a.checkEqual("73", i.visibility.isValid(), false);
    a.checkEqual("74. size", i.size.orElse(99), 0);

    AFL_CHECK_THROWS_CODE(a("81. getFileInformation"), testee.getFileInformation("listable/foo"), "404");
}

/** Test getDirectoryPermission(). */
AFL_TEST("server.file.FileBase:getDirectoryPermission", a)
{
    using server::interface::FileBase;

    // Test setup
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    testee.createDirectory("root");
    testee.createDirectoryAsUser("normal", "1001");
    testee.createDirectoryAsUser("accessible", "1001");
    testee.setDirectoryPermissions("normal", "1002", "r");
    testee.setDirectoryPermissions("accessible", "1002", "a");
    testee.putFile("normal/f", "");
    testee.putFile("accessible/f", "");

    // Test as root
    {
        String_t user;
        std::vector<FileBase::Permission> perm;
        AFL_CHECK_THROWS_CODE(a("01. getDirectoryPermission"), testee.getDirectoryPermission("bad", user, perm), "404");
    }
    {
        String_t user;
        std::vector<FileBase::Permission> perm;
        testee.getDirectoryPermission("root", user, perm);
        a.checkEqual("11. user", user, "");
        a.checkEqual("12. size", perm.size(), 0U);
    }
    {
        String_t user;
        std::vector<FileBase::Permission> perm;
        testee.getDirectoryPermission("normal", user, perm);
        a.checkEqual("21. user",       user, "1001");
        a.checkEqual("22. size",       perm.size(), 1U);
        a.checkEqual("23. userId",     perm[0].userId, "1002");
        a.checkEqual("24. permission", perm[0].permission, "r");
    }
    {
        String_t user;
        std::vector<FileBase::Permission> perm;
        testee.getDirectoryPermission("accessible", user, perm);
        a.checkEqual("31. user",       user, "1001");
        a.checkEqual("32. size",       perm.size(), 1U);
        a.checkEqual("33. userId",     perm[0].userId, "1002");
        a.checkEqual("34. permission", perm[0].permission, "a");
    }

    // Test as owner
    tb.session.setUser("1001");
    {
        String_t user;
        std::vector<FileBase::Permission> perm;
        AFL_CHECK_THROWS_CODE(a("41. getDirectoryPermission"), testee.getDirectoryPermission("bad", user, perm), "403");
    }
    {
        String_t user;
        std::vector<FileBase::Permission> perm;
        AFL_CHECK_THROWS_CODE(a("51. getDirectoryPermission"), testee.getDirectoryPermission("root", user, perm), "403");
    }
    {
        String_t user;
        std::vector<FileBase::Permission> perm;
        testee.getDirectoryPermission("normal", user, perm);
        a.checkEqual("61. user",       user, "1001");
        a.checkEqual("62. size",       perm.size(), 1U);
        a.checkEqual("63. userId",     perm[0].userId, "1002");
        a.checkEqual("64. permission", perm[0].permission, "r");
    }
    {
        String_t user;
        std::vector<FileBase::Permission> perm;
        testee.getDirectoryPermission("accessible", user, perm);
        a.checkEqual("71. user",       user, "1001");
        a.checkEqual("72. size",       perm.size(), 1U);
        a.checkEqual("73. userId",     perm[0].userId, "1002");
        a.checkEqual("74. permission", perm[0].permission, "a");
    }

    // Test as other
    tb.session.setUser("1002");
    {
        String_t user;
        std::vector<FileBase::Permission> perm;
        AFL_CHECK_THROWS_CODE(a("81. getDirectoryPermission"), testee.getDirectoryPermission("bad", user, perm), "403");
    }
    {
        String_t user;
        std::vector<FileBase::Permission> perm;
        AFL_CHECK_THROWS_CODE(a("91. getDirectoryPermission"), testee.getDirectoryPermission("root", user, perm), "403");
    }
    {
        String_t user;
        std::vector<FileBase::Permission> perm;
        AFL_CHECK_THROWS_CODE(a("101. getDirectoryPermission"), testee.getDirectoryPermission("normal", user, perm), "403");
    }
    {
        String_t user;
        std::vector<FileBase::Permission> perm;
        testee.getDirectoryPermission("accessible", user, perm);
        a.checkEqual("111. user",       user, "1001");
        a.checkEqual("112. size",       perm.size(), 1U);
        a.checkEqual("113. userId",     perm[0].userId, "1002");
        a.checkEqual("114. permission", perm[0].permission, "a");
    }
}

/** Test getDirectoryContent. */
AFL_TEST("server.file.FileBase:getDirectoryContent", a)
{
    using server::interface::FileBase;

    // Test setup
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);
    testee.createDirectory("writable");
    testee.createDirectory("readable");
    testee.createDirectory("both");
    testee.createDirectory("none");
    testee.createDirectory("listable");

    testee.setDirectoryPermissions("writable", "1001", "w");
    testee.setDirectoryPermissions("readable", "1001", "r");
    testee.setDirectoryPermissions("both", "1001", "rw");
    testee.setDirectoryPermissions("listable", "1001", "l");

    testee.putFile("writable/f", "ww");
    testee.putFile("readable/f", "r");
    testee.putFile("both/f", "");
    testee.putFile("none/f", "");
    testee.putFile("listable/f", "");

    // Some generic tests
    // - invalid file names
    {
        FileBase::ContentInfoMap_t result;
        AFL_CHECK_THROWS_CODE(a("01. getDirectoryContent"), testee.getDirectoryContent("", result),             "400");
        AFL_CHECK_THROWS_CODE(a("02. getDirectoryContent"), testee.getDirectoryContent("/", result),            "400");
        AFL_CHECK_THROWS_CODE(a("03. getDirectoryContent"), testee.getDirectoryContent("readable/", result),    "400");
        AFL_CHECK_THROWS_CODE(a("04. getDirectoryContent"), testee.getDirectoryContent("/x", result),           "400");
        AFL_CHECK_THROWS_CODE(a("05. getDirectoryContent"), testee.getDirectoryContent("a:b", result),          "400");
        AFL_CHECK_THROWS_CODE(a("06. getDirectoryContent"), testee.getDirectoryContent("readable/a:b", result), "400");

        // - non existant
        AFL_CHECK_THROWS_CODE(a("11. getDirectoryContent"), testee.getDirectoryContent("foo", result),          "404");
        AFL_CHECK_THROWS_CODE(a("12. getDirectoryContent"), testee.getDirectoryContent("readable/foo", result), "404");
        AFL_CHECK_THROWS_CODE(a("13. getDirectoryContent"), testee.getDirectoryContent("readable/f", result),   "405");

        // - Content
        AFL_CHECK_SUCCEEDS(a("21. getDirectoryContent"), testee.getDirectoryContent("writable", result));
        a.checkEqual("22. size",     result.size(), 1U);
        a.checkNonNull("23. result", result["f"]);
        a.checkEqual("24. type",     result["f"]->type, FileBase::IsFile);
        a.checkEqual("25. size",     result["f"]->size.orElse(99), 2);
    }

    // Test as user 1001
    tb.session.setUser("1001");
    {
        FileBase::ContentInfoMap_t result;
        AFL_CHECK_THROWS_CODE(a("31. getDirectoryContent"), testee.getDirectoryContent("writable", result), "403");
        AFL_CHECK_THROWS_CODE(a("32. getDirectoryContent"), testee.getDirectoryContent("writable/f", result), "403");
        AFL_CHECK_THROWS_CODE(a("33. getDirectoryContent"), testee.getDirectoryContent("readable", result), "403");
        AFL_CHECK_THROWS_CODE(a("34. getDirectoryContent"), testee.getDirectoryContent("readable/f", result), "403");
        AFL_CHECK_THROWS_CODE(a("35. getDirectoryContent"), testee.getDirectoryContent("readable/foo", result), "403");
        AFL_CHECK_THROWS_CODE(a("36. getDirectoryContent"), testee.getDirectoryContent("both", result), "403");
        AFL_CHECK_THROWS_CODE(a("37. getDirectoryContent"), testee.getDirectoryContent("both/f", result), "403");
        AFL_CHECK_THROWS_CODE(a("38. getDirectoryContent"), testee.getDirectoryContent("none", result), "403");
        AFL_CHECK_THROWS_CODE(a("39. getDirectoryContent"), testee.getDirectoryContent("none/f", result), "403");

        AFL_CHECK_SUCCEEDS(a("41. getDirectoryContent"), testee.getDirectoryContent("listable", result));

        AFL_CHECK_THROWS_CODE(a("51. getDirectoryContent"), testee.getDirectoryContent("listable/foo", result), "404");
        AFL_CHECK_THROWS_CODE(a("52. getDirectoryContent"), testee.getDirectoryContent("listable/f", result), "405");
    }
}

/** Test getDirectoryContent, 2nd round. */
AFL_TEST("server.file.FileBase:getDirectoryContent:2", a)
{
    using server::interface::FileBase;

    // Test setup
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);
    testee.createDirectoryTree("a/b/c/d");
    testee.createDirectory("a/b/e");
    testee.putFile("a/b/f", "hi!");

    // Why not....
    testee.forgetDirectory("a");

    // Read content
    FileBase::ContentInfoMap_t result;
    AFL_CHECK_SUCCEEDS(a("01. getDirectoryContent"), testee.getDirectoryContent("a/b", result));
    a.checkEqual("02. size", result.size(), 3U);
    a.checkNonNull("03. result", result["f"]);
    a.checkEqual("04. type",     result["f"]->type, FileBase::IsFile);
    a.checkEqual("05. size",     result["f"]->size.orElse(99), 3);

    a.checkNonNull("11. result", result["c"]);
    a.checkEqual("12. type",     result["c"]->type, FileBase::IsDirectory);
    a.checkEqual("13. size",     result["c"]->size.isValid(), false);
    a.checkEqual("14. visi",     result["c"]->visibility.orElse(99), 0);

    a.checkNonNull("21. result", result["e"]);
    a.checkEqual("22. type",     result["e"]->type, FileBase::IsDirectory);
    a.checkEqual("23. size",     result["e"]->size.isValid(), false);
    a.checkEqual("24. visi",     result["e"]->visibility.orElse(99), 0);
}

/** Test removeFile(). */
AFL_TEST("server.file.FileBase:removeFile", a)
{
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    // Create stuff
    testee.createDirectory("readable");
    testee.createDirectory("listable");
    testee.createDirectory("writable");
    testee.setDirectoryPermissions("writable", "1009", "w");
    testee.setDirectoryPermissions("listable", "1009", "l");
    testee.setDirectoryPermissions("readable", "1009", "r");
    testee.putFile("readable/f", "");
    testee.putFile("writable/f", "");
    testee.putFile("listable/f", "");
    testee.createDirectory("readable/d");
    testee.createDirectory("writable/d");
    testee.createDirectory("listable/d");

    // Remove as user
    tb.session.setUser("1009");
    AFL_CHECK_THROWS_CODE(a("01. removeFile"), testee.removeFile("readable/f"),  "403");
    AFL_CHECK_THROWS_CODE(a("02. removeFile"), testee.removeFile("readable/d"),  "403");
    AFL_CHECK_THROWS_CODE(a("03. removeFile"), testee.removeFile("readable/nx"), "403");
    AFL_CHECK_THROWS_CODE(a("04. removeFile"), testee.removeFile("readable/nx/nx"), "403");

    AFL_CHECK_SUCCEEDS   (a("11. removeFile"), testee.removeFile("writable/f"));
    AFL_CHECK_SUCCEEDS   (a("12. removeFile"), testee.removeFile("writable/d"));
    AFL_CHECK_THROWS_CODE(a("13. removeFile"), testee.removeFile("writable/nx"), "403");
    AFL_CHECK_THROWS_CODE(a("14. removeFile"), testee.removeFile("writable/nx/nx"), "403");

    AFL_CHECK_THROWS_CODE(a("21. removeFile"), testee.removeFile("listable/f"),  "403");
    AFL_CHECK_THROWS_CODE(a("22. removeFile"), testee.removeFile("listable/d"),  "403");
    AFL_CHECK_THROWS_CODE(a("23. removeFile"), testee.removeFile("listable/nx"), "404");

    AFL_CHECK_THROWS_CODE(a("31. removeFile"), testee.removeFile("listable/nx/nx"), "404");
}

/** Test removal of non-empty directory. */
AFL_TEST("server.file.FileBase:removeFile:non-empty-dir", a)
{
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    // Create stuff
    testee.createDirectory("a");
    testee.createDirectory("a/b");
    testee.putFile("a/b/zz", "");

    // Erase
    AFL_CHECK_THROWS_CODE(a("01. removeFile"), testee.removeFile("a/b"), "403");

    AFL_CHECK_SUCCEEDS(a("11. removeFile"), testee.removeFile("a/b/zz"));
    AFL_CHECK_SUCCEEDS(a("12. removeFile"), testee.removeFile("a/b"));
}

/** Test removal of non-empty directory, with a permission file. */
AFL_TEST("server.file.FileBase:removeFile:non-empty-dir:permission-file", a)
{
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    // Create stuff
    testee.createDirectory("a");
    testee.createDirectory("a/b");
    testee.putFile("a/b/zz", "");
    testee.setDirectoryPermissions("a/b", "1020", "rwl");

    // Verify internal structure
    {
        InternalDirectoryHandler::Directory* da = InternalDirectoryHandler("/", tb.dir).findDirectory("a");
        a.checkNonNull("01. a", da);
        InternalDirectoryHandler::Directory* db = InternalDirectoryHandler("a", *da).findDirectory("b");
        a.checkNonNull("02. a/b", db);
        InternalDirectoryHandler::File* c2file = InternalDirectoryHandler("b", *db).findFile(".c2file");
        a.checkNonNull("03. a/b/.c2file", c2file);
    }

    // Erase
    AFL_CHECK_THROWS_CODE(a("11. removeFile"), testee.removeFile("a/b"), "403");

    AFL_CHECK_SUCCEEDS(a("21. removeFile"), testee.removeFile("a/b/zz"));
    AFL_CHECK_SUCCEEDS(a("22. removeFile"), testee.removeFile("a/b"));
}

/** Test removal of non-empty directory, with an extra file. */
AFL_TEST("server.file.FileBase:removeFile:non-empty-dir:extra-file", a)
{
    using server::interface::FileBase;

    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    // Create stuff
    testee.createDirectory("a");
    testee.createDirectory("a/b");

    // Verify internal structure
    {
        InternalDirectoryHandler::Directory* da = InternalDirectoryHandler("/", tb.dir).findDirectory("a");
        a.checkNonNull("01. a", da);
        InternalDirectoryHandler::Directory* db = InternalDirectoryHandler("a", *da).findDirectory("b");
        a.checkNonNull("02. a/b", db);
        InternalDirectoryHandler("b", *db).createFile(".block", afl::base::Nothing);
    }

    // Verify that a/b appears empty
    {
        FileBase::ContentInfoMap_t result;
        testee.getDirectoryContent("a/b", result);
        a.check("11. empty", result.empty());
    }

    // Erase
    // This fails because the ".block" file is not recognized and therefore cannot be removed.
    AFL_CHECK_THROWS_CODE(a("21. removeFile"), testee.removeFile("a/b"), "403");
}

/** Test removal of a directory tree, base case. */
AFL_TEST("server.file.FileBase:removeDirectory", a)
{
    using server::interface::FileBase;

    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    testee.createDirectoryTree("a/b/c/d/e");
    testee.createDirectoryTree("a/b/c/x/y");
    testee.putFile("a/f", "");

    // Some failures
    AFL_CHECK_THROWS_CODE(a("01. removeDirectory"), testee.removeDirectory("a/f"), "405");
    AFL_CHECK_THROWS_CODE(a("02. removeDirectory"), testee.removeDirectory("a/x"), "404");

    // Success
    AFL_CHECK_SUCCEEDS(a("11. removeDirectory"),       testee.removeDirectory("a/b/c/x"));
    AFL_CHECK_SUCCEEDS(a("12. getFileInformation"),    testee.getFileInformation("a/b/c/d"));
    AFL_CHECK_SUCCEEDS(a("13. removeDirectory"),       testee.removeDirectory("a/b"));
    AFL_CHECK_THROWS_CODE(a("14. getFileInformation"), testee.getFileInformation("a/b"), "404");
}

/** Test removal of a directory tree, user case 1. */
AFL_TEST("server.file.FileBase:removeDirectory:user", a)
{
    using server::interface::FileBase;

    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    testee.createDirectoryTree("a/b/c/d/e");
    testee.createDirectoryTree("a/b/c/x/y");
    testee.putFile("a/b/c/d/e/f", "");

    // User has access to children, but not root
    testee.setDirectoryPermissions("a/b/c/d/e", "1001", "w");
    testee.setDirectoryPermissions("a/b/c/d",   "1001", "w");
    testee.setDirectoryPermissions("a/b/c/x/y", "1001", "w");
    testee.setDirectoryPermissions("a/b/c/x",   "1001", "w");

    tb.session.setUser("1001");
    AFL_CHECK_THROWS_CODE(a("01. removeDirectory"), testee.removeDirectory("a/b"), "403");

    // Verify it's still there
    tb.session.setUser("");
    AFL_CHECK_SUCCEEDS(a("11. getFileInformation"), testee.getFileInformation("a/b"));
}

/** Test removal of a directory tree, user case 2. */
AFL_TEST("server.file.FileBase:removeDirectory:user:2", a)
{
    using server::interface::FileBase;

    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    testee.createDirectoryTree("a/b/c/d/e");
    testee.createDirectoryTree("a/b/c/x/y");
    testee.putFile("a/b/c/d/e/f", "");

    // User has access to root, but not all children
    testee.setDirectoryPermissions("a",         "1001", "w");
    testee.setDirectoryPermissions("a/b",       "1001", "w");
    testee.setDirectoryPermissions("a/b/c",     "1001", "w");
    testee.setDirectoryPermissions("a/b/c/d",   "1001", "w");
    testee.setDirectoryPermissions("a/b/c/d/e", "1001", "w");
    testee.setDirectoryPermissions("a/b/c/x",   "1001", "w");

    tb.session.setUser("1001");
    AFL_CHECK_THROWS_CODE(a("01. removeDirectory"), testee.removeDirectory("a/b"), "403");
    AFL_CHECK_THROWS_CODE(a("02. removeDirectory"), testee.removeDirectory("a/b/c"), "403");
    AFL_CHECK_SUCCEEDS(a("03. removeDirectory"), testee.removeDirectory("a/b/c/d"));

    // Verify it's still there
    tb.session.setUser("");
    AFL_CHECK_SUCCEEDS(a("11. getFileInformation"), testee.getFileInformation("a/b"));
}

/** Test removal of a directory tree, user case 3. */
AFL_TEST("server.file.FileBase:removeDirectory:user:3", a)
{
    using server::interface::FileBase;

    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    testee.createDirectoryTree("a/b/c/d/e");
    testee.createDirectoryTree("a/b/c/x/y");
    testee.putFile("a/b/c/d/e/f", "");

    // User has full access
    testee.setDirectoryPermissions("a",         "1001", "w");
    testee.setDirectoryPermissions("a/b",       "1001", "w");
    testee.setDirectoryPermissions("a/b/c",     "1001", "w");
    testee.setDirectoryPermissions("a/b/c/d",   "1001", "w");
    testee.setDirectoryPermissions("a/b/c/d/e", "1001", "w");
    testee.setDirectoryPermissions("a/b/c/x",   "1001", "w");
    testee.setDirectoryPermissions("a/b/c/x/y", "1001", "w");

    tb.session.setUser("1001");
    AFL_CHECK_SUCCEEDS(a("01. removeDirectory"), testee.removeDirectory("a/b"));

    // Verify it's gone
    tb.session.setUser("");
    AFL_CHECK_THROWS_CODE(a("11. getFileInformation"), testee.getFileInformation("a/b"), "404");
    AFL_CHECK_SUCCEEDS(a("12. getFileInformation"), testee.getFileInformation("a"));
}

/** Test removal of directory tree, with an extra file. */
AFL_TEST("server.file.FileBase:removeDirectory:block", a)
{
    using server::interface::FileBase;

    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    // Create stuff
    testee.createDirectoryTree("a/b/c/d/e");
    testee.createDirectoryTree("a/b/x/y/z");

    // Verify internal structure
    {
        InternalDirectoryHandler::Directory* da = InternalDirectoryHandler("/", tb.dir).findDirectory("a");
        a.checkNonNull("01. a", da);
        InternalDirectoryHandler::Directory* db = InternalDirectoryHandler("a", *da).findDirectory("b");
        a.checkNonNull("02. a/b", db);
        InternalDirectoryHandler::Directory* dx = InternalDirectoryHandler("x", *db).findDirectory("x");
        a.checkNonNull("03. a/b/x", dx);
        InternalDirectoryHandler("x", *dx).createFile(".block", afl::base::Nothing);
    }

    // Erase
    // This fails because the ".block" file is not recognized and therefore cannot be removed.
    // Note that the directory might have still be cleared partially here.
    AFL_CHECK_THROWS_CODE(a("11. removeDirectory"), testee.removeDirectory("a/b"), "403");
}

/** Test removeDirectory(), permission test. */
AFL_TEST("server.file.FileBase:removeDirectory:permissions", a)
{
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    // Create stuff
    testee.createDirectory("readable");
    testee.createDirectory("listable");
    testee.createDirectory("writable");
    testee.setDirectoryPermissions("writable", "1009", "w");
    testee.setDirectoryPermissions("listable", "1009", "l");
    testee.setDirectoryPermissions("readable", "1009", "r");
    testee.putFile("readable/f", "");
    testee.putFile("writable/f", "");
    testee.putFile("listable/f", "");
    testee.createDirectory("readable/d");
    testee.createDirectory("writable/d");
    testee.createDirectory("listable/d");

    // Remove as user
    tb.session.setUser("1009");
    AFL_CHECK_THROWS_CODE(a("01. removeDirectory"), testee.removeDirectory("readable/f"),  "403");
    AFL_CHECK_THROWS_CODE(a("02. removeDirectory"), testee.removeDirectory("readable/d"),  "403");
    AFL_CHECK_THROWS_CODE(a("03. removeDirectory"), testee.removeDirectory("readable/nx"), "403");
    AFL_CHECK_THROWS_CODE(a("04. removeDirectory"), testee.removeDirectory("readable/nx/nx"), "403");

    AFL_CHECK_THROWS_CODE(a("11. removeDirectory"), testee.removeDirectory("writable/f"),  "403");
    // FIXME: the following should probably be permitted.
    // It fails because of missing permissions on 'd', but removeFile(d) would be accepted.
    AFL_CHECK_THROWS_CODE(a("12. removeDirectory"), testee.removeDirectory("writable/nx"), "403");
    AFL_CHECK_THROWS_CODE(a("13. removeDirectory"), testee.removeDirectory("writable/nx/nx"), "403");

    AFL_CHECK_THROWS_CODE(a("21. removeDirectory"), testee.removeDirectory("listable/f"),  "405");
    AFL_CHECK_THROWS_CODE(a("22. removeDirectory"), testee.removeDirectory("listable/d"),  "403");
    AFL_CHECK_THROWS_CODE(a("23. removeDirectory"), testee.removeDirectory("listable/nx"), "404");
    AFL_CHECK_THROWS_CODE(a("24. removeDirectory"), testee.removeDirectory("listable/nx/nx"), "404");
}

/** Test getDiskUsage(). */
AFL_TEST("server.file.FileBase:getDiskUsage", a)
{
    using server::interface::FileBase;

    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    // Create stuff
    testee.createDirectory("readable");
    testee.createDirectory("listable");
    testee.createDirectory("writable");
    testee.setDirectoryPermissions("writable", "1009", "w");
    testee.setDirectoryPermissions("listable", "1009", "l");
    testee.setDirectoryPermissions("readable", "1009", "r");
    testee.putFile("readable/f", "");
    testee.putFile("writable/f", "w");
    testee.putFile("listable/f", String_t(10240, 'x'));
    testee.createDirectory("readable/d");
    testee.createDirectory("writable/d");
    testee.createDirectory("listable/d");

    // Test as root
    FileBase::Usage u;
    AFL_CHECK_SUCCEEDS(a("01. getDiskUsage"), u = testee.getDiskUsage("readable"));
    a.checkEqual("02. numItems", u.numItems, 3);            // 1 per directory, 1 per file
    a.checkEqual("03. totalKBytes", u.totalKBytes, 2);      // 1 per directory, 0 for the empty file

    AFL_CHECK_SUCCEEDS(a("11. getDiskUsage"), u = testee.getDiskUsage("writable"));
    a.checkEqual("12. numItems", u.numItems, 3);            // 1 per directory, 1 per file
    a.checkEqual("13. totalKBytes", u.totalKBytes, 3);      // 1 per directory, 1 for the nonempty file

    AFL_CHECK_SUCCEEDS(a("21. getDiskUsage"), u = testee.getDiskUsage("listable"));
    a.checkEqual("22. numItems", u.numItems, 3);            // 1 per directory, 1 per file
    a.checkEqual("23. totalKBytes", u.totalKBytes, 12);     // 1 per directory, 10 for the file file

    AFL_CHECK_THROWS_CODE(a("31. getDiskUsage"), testee.getDiskUsage("nx"), "404");
    AFL_CHECK_THROWS_CODE(a("32. getDiskUsage"), testee.getDiskUsage("readable/nx"), "404");
    AFL_CHECK_THROWS_CODE(a("33. getDiskUsage"), testee.getDiskUsage("readable/nx/nx"), "404");

    AFL_CHECK_THROWS_CODE(a("41. getDiskUsage"), testee.getDiskUsage("readable/f"), "405");

    // Test as user
    tb.session.setUser("1009");
    AFL_CHECK_THROWS_CODE(a("51. getDiskUsage"), testee.getDiskUsage("readable"), "403");
    AFL_CHECK_THROWS_CODE(a("52. getDiskUsage"), testee.getDiskUsage("writable"), "403");

    AFL_CHECK_SUCCEEDS(a("61. getDiskUsage"), u = testee.getDiskUsage("listable"));
    a.checkEqual("62. numItems", u.numItems, 3);            // 1 per directory, 1 per file
    a.checkEqual("63. totalKBytes", u.totalKBytes, 12);     // 1 per directory, 10 for the file file

    AFL_CHECK_THROWS_CODE(a("71. getDiskUsage"), testee.getDiskUsage("nx"), "403");
    AFL_CHECK_THROWS_CODE(a("72. getDiskUsage"), testee.getDiskUsage("readable/nx"), "403");
    AFL_CHECK_THROWS_CODE(a("73. getDiskUsage"), testee.getDiskUsage("readable/nx/nx"), "403");
    AFL_CHECK_THROWS_CODE(a("74. getDiskUsage"), testee.getDiskUsage("readable/f"), "403");

    AFL_CHECK_THROWS_CODE(a("81. getDiskUsage"), testee.getDiskUsage("listable/nx"), "404");
    AFL_CHECK_THROWS_CODE(a("82. getDiskUsage"), testee.getDiskUsage("listable/nx/nx"), "404");
    AFL_CHECK_THROWS_CODE(a("83. getDiskUsage"), testee.getDiskUsage("listable/f"), "405");
}

/** Test putFile. */
AFL_TEST("server.file.FileBase:putFile", a)
{
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    // Create stuff
    testee.createDirectory("readable");
    testee.createDirectory("listable");
    testee.createDirectory("writable");
    testee.setDirectoryPermissions("writable", "1009", "w");
    testee.setDirectoryPermissions("listable", "1009", "l");
    testee.setDirectoryPermissions("readable", "1009", "r");
    testee.createDirectory("readable/d");
    testee.createDirectory("writable/d");
    testee.createDirectory("listable/d");

    // Test as user
    tb.session.setUser("1009");
    AFL_CHECK_THROWS_CODE(a("01. putFile"), testee.putFile("rootfile", ""), "403");
    AFL_CHECK_THROWS_CODE(a("02. putFile"), testee.putFile("readable/f", ""), "403");
    AFL_CHECK_SUCCEEDS   (a("03. putFile. putFile"), testee.putFile("writable/f", ""));
    AFL_CHECK_THROWS_CODE(a("04. putFile"), testee.putFile("writable/nx/f", ""), "403");
    AFL_CHECK_THROWS_CODE(a("05. putFile"), testee.putFile("listable/f", ""), "403");
    AFL_CHECK_THROWS_CODE(a("06. putFile"), testee.putFile("listable/d/f", ""), "403");
    AFL_CHECK_THROWS_CODE(a("07. putFile"), testee.putFile("listable/nx/f", ""), "404");

    // Attempt to overwrite a directory
    AFL_CHECK_THROWS_CODE(a("11. putFile"), testee.putFile("writable/d", ""), "409");
}

/** Test limits. */
AFL_TEST("server.file.FileBase:putFile:limit", a)
{
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    // Put some files
    testee.putFile("ten",    String_t(10, 'x'));
    testee.putFile("eleven", String_t(11, 'x'));

    // Enable limit
    tb.root.setMaxFileSize(10);

    // get
    AFL_CHECK_SUCCEEDS(a("01. getFile"), testee.getFile("ten"));
    AFL_CHECK_THROWS_CODE(a("02. getFile"), testee.getFile("eleven"), "413");

    // put
    AFL_CHECK_SUCCEEDS(a("11. putFile"), testee.putFile("ten2", String_t(10, 'y')));
    AFL_CHECK_THROWS_CODE(a("12. putFile"), testee.putFile("eleven2", String_t(11, 'y')), "413");

    // copy
    AFL_CHECK_SUCCEEDS(a("21. copyFile"), testee.copyFile("ten", "ten3"));
    AFL_CHECK_THROWS_CODE(a("22. copyFile"), testee.copyFile("eleven", "eleven3"), "413");
}

/** Test some copyFile() border cases. */
AFL_TEST("server.file.FileBase:copyFile:errors", a)
{
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    testee.createDirectory("a");
    testee.createDirectory("a/b");
    testee.putFile("a/f", "");

    // Attempt to overwrite a directory
    AFL_CHECK_THROWS_CODE(a("copyFile to dir"), testee.copyFile("a/f", "a/b"), "409");

    // Copy from nonexistant path
    AFL_CHECK_THROWS_CODE(a("copyFile missing"), testee.copyFile("a/x/y", "a/f"), "404");

    // Test to copy a directory
    AFL_CHECK_THROWS_CODE(a("copyFile from dir"), testee.copyFile("a/b", "a/y"), "404");
}

/** Test copyFile() implemented in underlay. */
AFL_TEST("server.file.FileBase:copyFile:underlay", a)
{
    // CA backend allows underlay copies, so build one.
    server::file::InternalDirectoryHandler::Directory underDir("");
    server::file::InternalDirectoryHandler underHandler("underlay", underDir);
    server::file::ca::Root underRoot(underHandler);
    server::file::DirectoryItem rootDirectory("root", 0, std::auto_ptr<server::file::DirectoryHandler>(underRoot.createRootHandler()));

    // Server root
    server::file::Root root(rootDirectory, afl::io::InternalDirectory::create("(spec)"));
    server::file::Session session;
    server::file::FileBase testee(session, root);

    // Create, copy and verify a file
    // (We cannot sensibly determine from the outside that this actually is an underlay copy.
    // But it can be seen in the coverage report.)
    testee.putFile("a", "content");
    testee.copyFile("a", "b");
    a.checkEqual("01. getFile", testee.getFile("b"), "content");

    a.checkEqual("11. getFileInformation", testee.getFileInformation("a").size.orElse(-1), 7);
    a.checkEqual("12. getFileInformation", testee.getFileInformation("b").size.orElse(-1), 7);
}

/** Test file upload content snooping. */
AFL_TEST("server.file.FileBase:putFile:snoop", a)
{
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    testee.createDirectory("a");
    testee.createDirectory("a/b");
    testee.putFile("a/b/pconfig.src", "GAMENAME = Hi There");

    std::auto_ptr<afl::data::Value> p(testee.getDirectoryProperty("a/b", "name"));
    a.checkEqual("01. getDirectoryProperty", afl::data::Access(p).toString(), "Hi There");
}

/** Test file upload content snooping on copy. */
AFL_TEST("server.file.FileBase:putFile:snoop:copy", a)
{
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    testee.createDirectory("a");
    testee.createDirectory("a/b");
    testee.createDirectory("a/c");
    testee.putFile("a/b/file.txt", "GAMENAME = Hi There");
    testee.copyFile("a/b/file.txt", "a/c/pconfig.src");

    std::auto_ptr<afl::data::Value> p(testee.getDirectoryProperty("a/c", "name"));
    a.checkEqual("01. getDirectoryProperty", afl::data::Access(p).toString(), "Hi There");
}
