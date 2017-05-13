/**
  *  \file u/t_server_file_filebase.cpp
  *  \brief Test for server::file::FileBase
  */

#include <stdexcept>
#include "server/file/filebase.hpp"

#include "t_server_file.hpp"
#include "server/file/directoryhandler.hpp"
#include "server/file/root.hpp"
#include "server/file/session.hpp"
#include "server/file/directoryitem.hpp"
#include "afl/io/internaldirectory.hpp"
#include "server/file/internaldirectoryhandler.hpp"
#include "server/file/ca/root.hpp"

#define TS_ASSERT_THROWS_CODE(call, code)                               \
                              do {                                      \
                                  bool threw = false;                   \
                                  try {                                 \
                                      call;                             \
                                  }                                     \
                                  catch (std::exception& e) {           \
                                      TS_ASSERT_EQUALS(String_t(e.what()).substr(0, 3), code); \
                                      threw = true;                     \
                                  }                                     \
                                  catch (...) {                         \
                                      TS_ASSERT(!"Wrong exception");    \
                                      threw = true;                     \
                                  }                                     \
                                  TS_ASSERT(threw);                     \
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
void
TestServerFileFileBase::testSimple()
{
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    TS_ASSERT_THROWS_NOTHING(testee.createDirectory("d"));
    TS_ASSERT_THROWS_NOTHING(testee.createDirectory("d/sd"));
    TS_ASSERT_THROWS_NOTHING(testee.putFile("d/f", "content..."));
    TS_ASSERT_EQUALS(testee.getFile("d/f"), "content...");

    server::interface::FileBase::Info i;
    TS_ASSERT_THROWS_NOTHING(i = testee.getFileInformation("d"));
    TS_ASSERT_EQUALS(i.type, server::interface::FileBase::IsDirectory);

    TS_ASSERT_THROWS_NOTHING(i = testee.getFileInformation("d/f"));
    TS_ASSERT_EQUALS(i.type, server::interface::FileBase::IsFile);
    TS_ASSERT_EQUALS(i.size.orElse(-1), 10);

    TS_ASSERT_THROWS(testee.createDirectory("d"), std::exception);
    TS_ASSERT_THROWS(testee.createDirectory("d/f"), std::exception);
    TS_ASSERT_THROWS(testee.putFile("d/sd", "xx"), std::exception);
}

/** Test createDirectory variants. */
void
TestServerFileFileBase::testCreateDirectory()
{
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    // Create a file in root
    TS_ASSERT_THROWS_NOTHING(testee.putFile("f", ""));

    // Admin context: create directories
    // - success case
    TS_ASSERT_THROWS_NOTHING(testee.createDirectoryAsUser("u", "1001"));
    TS_ASSERT_THROWS_NOTHING(testee.createDirectoryAsUser("w", "1002"));
    TS_ASSERT_THROWS_NOTHING(testee.createDirectory("u/sub"));

    // - failure case: missing user name
    TS_ASSERT_THROWS_CODE(testee.createDirectoryAsUser("v", ""), "400");

    // - failure case: already exists
    TS_ASSERT_THROWS_CODE(testee.createDirectoryAsUser("u", "1001"), "409");
    TS_ASSERT_THROWS_CODE(testee.createDirectory("u"),               "409");
    TS_ASSERT_THROWS_CODE(testee.createDirectory("f"),               "409");

    // - failure case: bad file names
    TS_ASSERT_THROWS_CODE(testee.createDirectory(""),                    "400");
    TS_ASSERT_THROWS_CODE(testee.createDirectory("/a"),                  "400");
    TS_ASSERT_THROWS_CODE(testee.createDirectory("u//a"),                "400");
    TS_ASSERT_THROWS_CODE(testee.createDirectory("u/a:b"),               "400");
    TS_ASSERT_THROWS_CODE(testee.createDirectory("u/a\\b"),              "400");
    TS_ASSERT_THROWS_CODE(testee.createDirectory("u/.dot"),              "400");
    TS_ASSERT_THROWS_CODE(testee.createDirectory(String_t("u/a\0b", 5)), "400");

    // User context
    tb.session.setUser("1001");

    // - success case
    TS_ASSERT_THROWS_NOTHING(testee.createDirectory("u/sub2"));

    // - failure case: missing permissions
    TS_ASSERT_THROWS_CODE(testee.createDirectoryAsUser("u/sub3", "1002"), "403");
    TS_ASSERT_THROWS_CODE(testee.createDirectory("v"),                    "403");
    TS_ASSERT_THROWS_CODE(testee.createDirectory("w/x"),                  "403");

    // - failure case: already exists (but also missing permissions), so reports missing permissions
    TS_ASSERT_THROWS_CODE(testee.createDirectory("u"), "403");
    TS_ASSERT_THROWS_CODE(testee.createDirectory("f"), "403");

    // - failure case: already exists
    TS_ASSERT_THROWS_CODE(testee.createDirectory("u/sub"), "409");
}

/** Test getFile() and copyFile(). */
void
TestServerFileFileBase::testGet()
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
    TS_ASSERT_THROWS_CODE(testee.getFile("/"), "400");
    TS_ASSERT_THROWS_CODE(testee.getFile("u1//a"), "400");
    TS_ASSERT_THROWS_CODE(testee.getFile("u1/x:y/a"), "400");
    TS_ASSERT_THROWS_CODE(testee.getFile("u1/x:y"), "400");
    TS_ASSERT_THROWS_CODE(testee.getFile("u1//"), "400");

    // User 1
    // - getFile
    tb.session.setUser("1001");
    TS_ASSERT_THROWS_CODE(testee.getFile("u1"), "403");                    // access a directory we can read
    TS_ASSERT_THROWS_CODE(testee.getFile("u1/g"), "404");                  // access nonexistant file in a directory we can read
    TS_ASSERT_EQUALS(testee.getFile("u1/f"), "u1 f");                      // ok
    TS_ASSERT_EQUALS(testee.getFile("u1/sub/f"), "u1 sub f");              // ok
    TS_ASSERT_THROWS_CODE(testee.getFile("u2/f"), "403");                  // access existing file in a directory we cannot read
    TS_ASSERT_THROWS_CODE(testee.getFile("u2/g"), "403");                  // access nonexistant file in a directory we cannot read
    TS_ASSERT_THROWS_CODE(testee.getFile("u2/g/g"), "403");                // access nonexistant file in a directory we cannot read

    // - copyFile
    TS_ASSERT_THROWS_CODE(testee.copyFile("u1", "tmp/x"), "403");          // access a directory we can read
    TS_ASSERT_THROWS_CODE(testee.copyFile("u1/g", "tmp/x"), "404");        // access nonexistant file in a directory we can read
    TS_ASSERT_THROWS_NOTHING(testee.copyFile("u1/f", "tmp/x"));            // ok
    TS_ASSERT_THROWS_NOTHING(testee.copyFile("u1/sub/f", "tmp/x"));        // ok
    TS_ASSERT_THROWS_CODE(testee.copyFile("u2/f", "tmp/x"), "403");        // access existing file in a directory we cannot read
    TS_ASSERT_THROWS_CODE(testee.copyFile("u2/g", "tmp/x"), "403");        // access nonexistant file in a directory we cannot read
    TS_ASSERT_THROWS_CODE(testee.copyFile("u2/g/g", "tmp/x"), "403");      // access nonexistant file in a directory we cannot read

    // User 2
    // - getFile
    tb.session.setUser("1002");
    TS_ASSERT_THROWS_CODE(testee.getFile("u1"), "403");                    // access a directory
    TS_ASSERT_THROWS_CODE(testee.getFile("u1/g"), "403");                  // access nonexistant file in a directory we cannot read
    TS_ASSERT_THROWS_CODE(testee.getFile("u1/f"), "403");                  // access existing file in a directory we cannot read
    TS_ASSERT_THROWS_CODE(testee.getFile("u1/sub/f"), "403");              // ditto
    TS_ASSERT_EQUALS(testee.getFile("u2/f"), "u2 f");                      // ok
    TS_ASSERT_THROWS_CODE(testee.getFile("u2/g"), "404");                  // access nonexistant file in a directory we can read
    TS_ASSERT_THROWS_CODE(testee.getFile("u2/g/g"), "404");                // access nonexistant file in a directory we can read

    // - copyFile
    TS_ASSERT_THROWS_CODE(testee.copyFile("u1", "tmp/x"), "403");          // access a directory
    TS_ASSERT_THROWS_CODE(testee.copyFile("u1/g", "tmp/x"), "403");        // access nonexistant file in a directory we cannot read
    TS_ASSERT_THROWS_CODE(testee.copyFile("u1/f", "tmp/x"), "403");        // access existing file in a directory we cannot read
    TS_ASSERT_THROWS_CODE(testee.copyFile("u1/sub/f", "tmp/x"), "403");    // ditto
    TS_ASSERT_THROWS_NOTHING(testee.copyFile("u2/f", "tmp/x"));            // ok
    TS_ASSERT_THROWS_CODE(testee.copyFile("u2/g", "tmp/x"), "404");        // access nonexistant file in a directory we can read
    TS_ASSERT_THROWS_CODE(testee.copyFile("u2/g/g", "tmp/x"), "404");      // access nonexistant file in a directory we can read

    // User 3
    // - getFile
    tb.session.setUser("1003");
    TS_ASSERT_THROWS_CODE(testee.getFile("u1"), "403");                    // access a directory
    TS_ASSERT_THROWS_CODE(testee.getFile("u1/g"), "403");                  // access nonexistant file in a directory we cannot read
    TS_ASSERT_THROWS_CODE(testee.getFile("u1/f"), "403");                  // access existing file in a directory we cannot read
    TS_ASSERT_THROWS_CODE(testee.getFile("u1/sub/f"), "403");              // ditto
    TS_ASSERT_EQUALS(testee.getFile("u2/f"), "u2 f");                      // ok, user got explicit permissions to read
    TS_ASSERT_THROWS_CODE(testee.getFile("u2/g"), "403");                  // user did not get permissions to read the directory, so this is 403
    TS_ASSERT_THROWS_CODE(testee.getFile("u2/g/g"), "403");                // access nonexistant file in a directory we cannot read

    // - copyFile
    TS_ASSERT_THROWS_CODE(testee.copyFile("u1", "tmp/x"), "403");          // access a directory
    TS_ASSERT_THROWS_CODE(testee.copyFile("u1/g", "tmp/x"), "403");        // access nonexistant file in a directory we cannot read
    TS_ASSERT_THROWS_CODE(testee.copyFile("u1/f", "tmp/x"), "403");        // access existing file in a directory we cannot read
    TS_ASSERT_THROWS_CODE(testee.copyFile("u1/sub/f", "tmp/x"), "403");    // ditto
    TS_ASSERT_THROWS_NOTHING(testee.copyFile("u2/f", "tmp/x"));            // ok, user got explicit permissions to read
    TS_ASSERT_THROWS_CODE(testee.copyFile("u2/g", "tmp/x"), "403");        // user did not get permissions to read the directory, so this is 403

    // User 4
    // - getFile
    tb.session.setUser("1004");
    TS_ASSERT_THROWS_CODE(testee.getFile("u1"), "403");                    // access a directory
    TS_ASSERT_THROWS_CODE(testee.getFile("u1/g"), "403");                  // access nonexistant file in a directory we cannot read
    TS_ASSERT_THROWS_CODE(testee.getFile("u1/f"), "403");                  // access existing file in a directory we cannot read
    TS_ASSERT_THROWS_CODE(testee.getFile("u1/sub/f"), "403");              // ditto
    TS_ASSERT_THROWS_CODE(testee.getFile("u2/f"), "403");                  // user got permissions to read the directory but not the file
    TS_ASSERT_THROWS_CODE(testee.getFile("u2/g"), "404");                  // user got permissions to know that this file does not exist
    TS_ASSERT_THROWS_CODE(testee.getFile("u2/g/g"), "404");                // user got permissions to know that this file does not exist

    // - copyFile
    TS_ASSERT_THROWS_CODE(testee.copyFile("u1", "tmp/x"), "403");          // access a directory
    TS_ASSERT_THROWS_CODE(testee.copyFile("u1/g", "tmp/x"), "403");        // access nonexistant file in a directory we cannot read
    TS_ASSERT_THROWS_CODE(testee.copyFile("u1/f", "tmp/x"), "403");        // access existing file in a directory we cannot read
    TS_ASSERT_THROWS_CODE(testee.copyFile("u1/sub/f", "tmp/x"), "403");    // ditto
    TS_ASSERT_THROWS_CODE(testee.copyFile("u2/f", "tmp/x"), "403");        // user got permissions to read the directory but not the file
    TS_ASSERT_THROWS_CODE(testee.copyFile("u2/g", "tmp/x"), "404");        // user got permissions to know that this file does not exist
}

/** Test testFiles(). */
void
TestServerFileFileBase::testTestFiles()
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
        TS_ASSERT_EQUALS(result.size(), 0U);
    }

    // Root
    {
        afl::data::IntegerList_t result;
        testee.testFiles(FILE_NAMES, result);
        TS_ASSERT_EQUALS(result.size(), 7U);
        TS_ASSERT_EQUALS(result[0], 0);
        TS_ASSERT_EQUALS(result[1], 0);
        TS_ASSERT_EQUALS(result[2], 1);
        TS_ASSERT_EQUALS(result[3], 1);
        TS_ASSERT_EQUALS(result[4], 1);
        TS_ASSERT_EQUALS(result[5], 0);
        TS_ASSERT_EQUALS(result[6], 0);
    }

    // User 1
    tb.session.setUser("1001");
    {
        afl::data::IntegerList_t result;
        testee.testFiles(FILE_NAMES, result);
        TS_ASSERT_EQUALS(result.size(), 7U);
        TS_ASSERT_EQUALS(result[0], 0);
        TS_ASSERT_EQUALS(result[1], 0);
        TS_ASSERT_EQUALS(result[2], 1);
        TS_ASSERT_EQUALS(result[3], 1);
        TS_ASSERT_EQUALS(result[4], 0);
        TS_ASSERT_EQUALS(result[5], 0);
        TS_ASSERT_EQUALS(result[6], 0);
    }

    // User 2
    tb.session.setUser("1002");
    {
        afl::data::IntegerList_t result;
        testee.testFiles(FILE_NAMES, result);
        TS_ASSERT_EQUALS(result.size(), 7U);
        TS_ASSERT_EQUALS(result[0], 0);
        TS_ASSERT_EQUALS(result[1], 0);
        TS_ASSERT_EQUALS(result[2], 0);
        TS_ASSERT_EQUALS(result[3], 0);
        TS_ASSERT_EQUALS(result[4], 1);
        TS_ASSERT_EQUALS(result[5], 0);
        TS_ASSERT_EQUALS(result[6], 0);
    }

    // User 3
    tb.session.setUser("1003");
    {
        afl::data::IntegerList_t result;
        testee.testFiles(FILE_NAMES, result);
        TS_ASSERT_EQUALS(result.size(), 7U);
        TS_ASSERT_EQUALS(result[0], 0);
        TS_ASSERT_EQUALS(result[1], 0);
        TS_ASSERT_EQUALS(result[2], 0);
        TS_ASSERT_EQUALS(result[3], 0);
        TS_ASSERT_EQUALS(result[4], 1);
        TS_ASSERT_EQUALS(result[5], 0);
        TS_ASSERT_EQUALS(result[6], 0);
    }

    // User 4
    tb.session.setUser("1004");
    {
        afl::data::IntegerList_t result;
        testee.testFiles(FILE_NAMES, result);
        TS_ASSERT_EQUALS(result.size(), 7U);
        TS_ASSERT_EQUALS(result[0], 0);
        TS_ASSERT_EQUALS(result[1], 0);
        TS_ASSERT_EQUALS(result[2], 0);
        TS_ASSERT_EQUALS(result[3], 0);
        TS_ASSERT_EQUALS(result[4], 0);
        TS_ASSERT_EQUALS(result[5], 0);
        TS_ASSERT_EQUALS(result[6], 0);
    }
}

/** Test getDirectoryProperty(), setDirectoryProperty(). */
void
TestServerFileFileBase::testProperty()
{
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);
    testee.createDirectory("u");

    // Set and get properties
    testee.setDirectoryProperty("u", "name", "foo");
    testee.setDirectoryProperty("u", "count", "3");
    testee.setDirectoryProperty("u", "a", "e=mc2");
    TS_ASSERT_EQUALS(testee.getDirectoryStringProperty("u", "name"), "foo");
    TS_ASSERT_EQUALS(testee.getDirectoryStringProperty("u", "count"), "3");
    TS_ASSERT_THROWS(testee.getDirectoryIntegerProperty("u", "name"), std::exception);
    TS_ASSERT_EQUALS(testee.getDirectoryIntegerProperty("u", "count"), 3);

    // Error cases
    // - not found
    TS_ASSERT_THROWS_CODE(testee.setDirectoryProperty("v", "x", "y"), "404");
    TS_ASSERT_THROWS_CODE(testee.setDirectoryProperty("u/v", "x", "y"), "404");

    // - bad file name
    TS_ASSERT_THROWS_CODE(testee.setDirectoryProperty("u/", "x", "y"), "400");
    TS_ASSERT_THROWS_CODE(testee.setDirectoryProperty("a:b", "x", "y"), "400");
    TS_ASSERT_THROWS_CODE(testee.setDirectoryProperty("u/a:b", "x", "y"), "400");

    // - bad property name
    // TS_ASSERT_THROWS_CODE(testee.setDirectoryProperty("u", "", "y"), "400"); <- allowed, but not sensible
    TS_ASSERT_THROWS_CODE(testee.setDirectoryProperty("u", "a=b", "y"), "400");
    TS_ASSERT_THROWS_CODE(testee.setDirectoryProperty("u", "=b", "y"), "400");
    TS_ASSERT_THROWS_CODE(testee.setDirectoryProperty("u", "a=", "y"), "400");
    TS_ASSERT_THROWS_CODE(testee.setDirectoryProperty("u", "a\nb", "y"), "400");

    // - bad property value
    TS_ASSERT_THROWS_CODE(testee.setDirectoryProperty("u", "a", "y\n"), "400");

    // Forget & reload
    testee.forgetDirectory("u");
    TS_ASSERT_EQUALS(testee.getDirectoryStringProperty("u", "name"), "foo");
    TS_ASSERT_EQUALS(testee.getDirectoryStringProperty("u", "count"), "3");
    TS_ASSERT_EQUALS(testee.getDirectoryStringProperty("u", "a"), "e=mc2");
}

/** Test getDirectoryProperty(), setDirectoryProperty() vs.\ permissions. */
void
TestServerFileFileBase::testPropertyPermissions()
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
    TS_ASSERT_THROWS_CODE(testee.getDirectoryStringProperty("writable", "p"),         "403");
    TS_ASSERT_EQUALS     (testee.getDirectoryStringProperty("readable", "p"),         "r");
    TS_ASSERT_EQUALS     (testee.getDirectoryStringProperty("both", "p"),             "b");
    TS_ASSERT_THROWS_CODE(testee.getDirectoryStringProperty("none", "p"),             "403");
    TS_ASSERT_EQUALS     (testee.getDirectoryStringProperty("none/readable", "p"),    "nr");
    TS_ASSERT_THROWS_CODE(testee.getDirectoryStringProperty("none/writable", "p"),    "403");
    TS_ASSERT_THROWS_CODE(testee.getDirectoryStringProperty("none/none", "p"),        "403");
    TS_ASSERT_THROWS_CODE(testee.getDirectoryStringProperty("none/missing", "p"),     "403");
    TS_ASSERT_THROWS_CODE(testee.getDirectoryStringProperty("listable", "p"),         "403");
    TS_ASSERT_THROWS_CODE(testee.getDirectoryStringProperty("readable/missing", "p"), "403");
    TS_ASSERT_THROWS_CODE(testee.getDirectoryStringProperty("listable/missing", "p"), "404");

    // Test writing in user context [bug #338]
    tb.session.setUser("1001");
    TS_ASSERT_THROWS_NOTHING(testee.setDirectoryProperty("writable", "p", "v"));
    TS_ASSERT_THROWS_CODE   (testee.setDirectoryProperty("readable", "p", "v"),         "403");
    TS_ASSERT_THROWS_NOTHING(testee.setDirectoryProperty("both", "p", "v"));
    TS_ASSERT_THROWS_CODE   (testee.setDirectoryProperty("none", "p", "v"),             "403");
    TS_ASSERT_THROWS_CODE   (testee.setDirectoryProperty("none/readable", "p", "v"),    "403");
    TS_ASSERT_THROWS_NOTHING(testee.setDirectoryProperty("none/writable", "p", "v"));
    TS_ASSERT_THROWS_CODE   (testee.setDirectoryProperty("none/none", "p", "v"),        "403");
    TS_ASSERT_THROWS_CODE   (testee.setDirectoryProperty("none/missing", "p", "v"),     "403");
    TS_ASSERT_THROWS_CODE   (testee.setDirectoryProperty("listable", "p", "v"),         "403");
    TS_ASSERT_THROWS_CODE   (testee.setDirectoryProperty("readable/missing", "p", "v"), "403");
    TS_ASSERT_THROWS_CODE   (testee.setDirectoryProperty("listable/missing", "p", "v"), "404");
}

/** Test property access vs. file */
void
TestServerFileFileBase::testPropertyFile()
{
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    testee.putFile("f", "c");
    testee.createDirectory("d");
    testee.putFile("d/ff", "cc");

    TS_ASSERT_THROWS_CODE(testee.getDirectoryStringProperty("f", "p"), "405");
    TS_ASSERT_THROWS_CODE(testee.getDirectoryStringProperty("dd/ff", "p"), "404");
    TS_ASSERT_THROWS_CODE(testee.setDirectoryProperty("f", "p", "v"), "405");
    TS_ASSERT_THROWS_CODE(testee.setDirectoryProperty("dd/ff", "p", "v"), "404");
}

/** Test createDirectoryTree. */
void
TestServerFileFileBase::testCreateDirectoryTree()
{
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    // Success case
    TS_ASSERT_THROWS_NOTHING(testee.createDirectoryTree("0/a/b/c/d/e/f/g"));

    // Repeating is ok, also with shorter and longer path
    TS_ASSERT_THROWS_NOTHING(testee.createDirectoryTree("0/a/b/c/d/e/f/g"));
    TS_ASSERT_THROWS_NOTHING(testee.createDirectoryTree("0/a/b/c/d/e"));
    TS_ASSERT_THROWS_NOTHING(testee.createDirectoryTree("0/a/b/c/d/e/f/g/h/i"));

    // Attempt to overwrite a file
    // FIXME: 409 should only be produced if we have read access!
    testee.putFile("1", "");
    TS_ASSERT_THROWS_CODE(testee.createDirectoryTree("1/a/b/c/d/e"), "409");

    // Attempt to overwrite a nested file
    testee.createDirectoryTree("2/a/b/c/d");
    testee.putFile("2/a/b/c/d/e", "");
    TS_ASSERT_THROWS_CODE(testee.createDirectoryTree("2/a/b/c/d/e/f/g/h"), "409");

    // Attempt to create without write permissions
    testee.createDirectory("3");
    testee.createDirectory("4");
    testee.setDirectoryPermissions("3", "1009", "r");
    testee.setDirectoryPermissions("4", "1009", "w");
    tb.session.setUser("1009");
    TS_ASSERT_THROWS_CODE(testee.createDirectoryTree("3/a/b"), "403");
    TS_ASSERT_THROWS_NOTHING(testee.createDirectoryTree("4/a"));

    // FIXME: fails, because the user has no permissions to the newly-created 4/a directory!
    // TS_ASSERT_THROWS_NOTHING(testee.createDirectoryTree("4/a/b"));
}

/** Test getFileInformation(). */
void
TestServerFileFileBase::testStat()
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
    TS_ASSERT_THROWS_CODE(testee.getFileInformation(""), "400");
    TS_ASSERT_THROWS_CODE(testee.getFileInformation("/"), "400");
    TS_ASSERT_THROWS_CODE(testee.getFileInformation("readable/"), "400");
    TS_ASSERT_THROWS_CODE(testee.getFileInformation("/x"), "400");
    TS_ASSERT_THROWS_CODE(testee.getFileInformation("a:b"), "400");
    TS_ASSERT_THROWS_CODE(testee.getFileInformation("readable/a:b"), "400");

    // - non existant
    TS_ASSERT_THROWS_CODE(testee.getFileInformation("foo"), "404");
    TS_ASSERT_THROWS_CODE(testee.getFileInformation("readable/foo"), "404");

    // - Content
    FileBase::Info i;
    TS_ASSERT_THROWS_NOTHING(i = testee.getFileInformation("writable"));
    TS_ASSERT_EQUALS(i.type, FileBase::IsDirectory);
    TS_ASSERT_EQUALS(i.visibility.orElse(99), 1);                    // 1 because it has some permissions

    TS_ASSERT_THROWS_NOTHING(i = testee.getFileInformation("none"));
    TS_ASSERT_EQUALS(i.type, FileBase::IsDirectory);
    TS_ASSERT_EQUALS(i.visibility.orElse(99), 0);

    TS_ASSERT_THROWS_NOTHING(i = testee.getFileInformation("readable/f"));
    TS_ASSERT_EQUALS(i.type, FileBase::IsFile);
    TS_ASSERT_EQUALS(i.visibility.isValid(), false);
    TS_ASSERT_EQUALS(i.size.orElse(99), 1);

    // Test as user 1001
    tb.session.setUser("1001");
    TS_ASSERT_THROWS_CODE(testee.getFileInformation("writable"), "403");
    TS_ASSERT_THROWS_CODE(testee.getFileInformation("writable/f"), "403");
    TS_ASSERT_THROWS_CODE(testee.getFileInformation("readable"), "403");
    TS_ASSERT_THROWS_CODE(testee.getFileInformation("readable/f"), "403");  // FIXME: should this be allowed?
    TS_ASSERT_THROWS_CODE(testee.getFileInformation("readable/foo"), "403");
    TS_ASSERT_THROWS_CODE(testee.getFileInformation("both"), "403");
    TS_ASSERT_THROWS_CODE(testee.getFileInformation("both/f"), "403");
    TS_ASSERT_THROWS_CODE(testee.getFileInformation("none"), "403");
    TS_ASSERT_THROWS_CODE(testee.getFileInformation("none/f"), "403");
    TS_ASSERT_THROWS_CODE(testee.getFileInformation("listable"), "403");

    TS_ASSERT_THROWS_NOTHING(i = testee.getFileInformation("listable/f"));
    TS_ASSERT_EQUALS(i.type, FileBase::IsFile);
    TS_ASSERT_EQUALS(i.visibility.isValid(), false);
    TS_ASSERT_EQUALS(i.size.orElse(99), 0);

    TS_ASSERT_THROWS_CODE(testee.getFileInformation("listable/foo"), "404");
}

/** Test getDirectoryPermission(). */
void
TestServerFileFileBase::testGetDirPermission()
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
        TS_ASSERT_THROWS_CODE(testee.getDirectoryPermission("bad", user, perm), "404");
    }
    {
        String_t user;
        std::vector<FileBase::Permission> perm;
        testee.getDirectoryPermission("root", user, perm);
        TS_ASSERT_EQUALS(user, "");
        TS_ASSERT_EQUALS(perm.size(), 0U);
    }
    {
        String_t user;
        std::vector<FileBase::Permission> perm;
        testee.getDirectoryPermission("normal", user, perm);
        TS_ASSERT_EQUALS(user, "1001");
        TS_ASSERT_EQUALS(perm.size(), 1U);
        TS_ASSERT_EQUALS(perm[0].userId, "1002");
        TS_ASSERT_EQUALS(perm[0].permission, "r");
    }
    {
        String_t user;
        std::vector<FileBase::Permission> perm;
        testee.getDirectoryPermission("accessible", user, perm);
        TS_ASSERT_EQUALS(user, "1001");
        TS_ASSERT_EQUALS(perm.size(), 1U);
        TS_ASSERT_EQUALS(perm[0].userId, "1002");
        TS_ASSERT_EQUALS(perm[0].permission, "a");
    }

    // Test as owner
    tb.session.setUser("1001");
    {
        String_t user;
        std::vector<FileBase::Permission> perm;
        TS_ASSERT_THROWS_CODE(testee.getDirectoryPermission("bad", user, perm), "403");
    }
    {
        String_t user;
        std::vector<FileBase::Permission> perm;
        TS_ASSERT_THROWS_CODE(testee.getDirectoryPermission("root", user, perm), "403");
    }
    {
        String_t user;
        std::vector<FileBase::Permission> perm;
        testee.getDirectoryPermission("normal", user, perm);
        TS_ASSERT_EQUALS(user, "1001");
        TS_ASSERT_EQUALS(perm.size(), 1U);
        TS_ASSERT_EQUALS(perm[0].userId, "1002");
        TS_ASSERT_EQUALS(perm[0].permission, "r");
    }
    {
        String_t user;
        std::vector<FileBase::Permission> perm;
        testee.getDirectoryPermission("accessible", user, perm);
        TS_ASSERT_EQUALS(user, "1001");
        TS_ASSERT_EQUALS(perm.size(), 1U);
        TS_ASSERT_EQUALS(perm[0].userId, "1002");
        TS_ASSERT_EQUALS(perm[0].permission, "a");
    }

    // Test as other
    tb.session.setUser("1002");
    {
        String_t user;
        std::vector<FileBase::Permission> perm;
        TS_ASSERT_THROWS_CODE(testee.getDirectoryPermission("bad", user, perm), "403");
    }
    {
        String_t user;
        std::vector<FileBase::Permission> perm;
        TS_ASSERT_THROWS_CODE(testee.getDirectoryPermission("root", user, perm), "403");
    }
    {
        String_t user;
        std::vector<FileBase::Permission> perm;
        TS_ASSERT_THROWS_CODE(testee.getDirectoryPermission("normal", user, perm), "403");
    }
    {
        String_t user;
        std::vector<FileBase::Permission> perm;
        testee.getDirectoryPermission("accessible", user, perm);
        TS_ASSERT_EQUALS(user, "1001");
        TS_ASSERT_EQUALS(perm.size(), 1U);
        TS_ASSERT_EQUALS(perm[0].userId, "1002");
        TS_ASSERT_EQUALS(perm[0].permission, "a");
    }
}

/** Test getDirectoryContent. */
void
TestServerFileFileBase::testGetDirContent()
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
        TS_ASSERT_THROWS_CODE(testee.getDirectoryContent("", result), "400");
        TS_ASSERT_THROWS_CODE(testee.getDirectoryContent("/", result), "400");
        TS_ASSERT_THROWS_CODE(testee.getDirectoryContent("readable/", result), "400");
        TS_ASSERT_THROWS_CODE(testee.getDirectoryContent("/x", result), "400");
        TS_ASSERT_THROWS_CODE(testee.getDirectoryContent("a:b", result), "400");
        TS_ASSERT_THROWS_CODE(testee.getDirectoryContent("readable/a:b", result), "400");

        // - non existant
        TS_ASSERT_THROWS_CODE(testee.getDirectoryContent("foo", result), "404");
        TS_ASSERT_THROWS_CODE(testee.getDirectoryContent("readable/foo", result), "404");
        TS_ASSERT_THROWS_CODE(testee.getDirectoryContent("readable/f", result), "405");

        // - Content
        TS_ASSERT_THROWS_NOTHING(testee.getDirectoryContent("writable", result));
        TS_ASSERT_EQUALS(result.size(), 1U);
        TS_ASSERT(result["f"] != 0);
        TS_ASSERT_EQUALS(result["f"]->type, FileBase::IsFile);
        TS_ASSERT_EQUALS(result["f"]->size.orElse(99), 2);
    }

    // Test as user 1001
    tb.session.setUser("1001");
    {
        FileBase::ContentInfoMap_t result;
        TS_ASSERT_THROWS_CODE(testee.getDirectoryContent("writable", result), "403");
        TS_ASSERT_THROWS_CODE(testee.getDirectoryContent("writable/f", result), "403");
        TS_ASSERT_THROWS_CODE(testee.getDirectoryContent("readable", result), "403");
        TS_ASSERT_THROWS_CODE(testee.getDirectoryContent("readable/f", result), "403");
        TS_ASSERT_THROWS_CODE(testee.getDirectoryContent("readable/foo", result), "403");
        TS_ASSERT_THROWS_CODE(testee.getDirectoryContent("both", result), "403");
        TS_ASSERT_THROWS_CODE(testee.getDirectoryContent("both/f", result), "403");
        TS_ASSERT_THROWS_CODE(testee.getDirectoryContent("none", result), "403");
        TS_ASSERT_THROWS_CODE(testee.getDirectoryContent("none/f", result), "403");

        TS_ASSERT_THROWS_NOTHING(testee.getDirectoryContent("listable", result));

        TS_ASSERT_THROWS_CODE(testee.getDirectoryContent("listable/foo", result), "404");
        TS_ASSERT_THROWS_CODE(testee.getDirectoryContent("listable/f", result), "405");
    }
}

/** Test getDirectoryContent, 2nd round. */
void
TestServerFileFileBase::testGetDirContent2()
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
    TS_ASSERT_THROWS_NOTHING(testee.getDirectoryContent("a/b", result));
    TS_ASSERT_EQUALS(result.size(), 3U);
    TS_ASSERT(result["f"] != 0);
    TS_ASSERT_EQUALS(result["f"]->type, FileBase::IsFile);
    TS_ASSERT_EQUALS(result["f"]->size.orElse(99), 3);

    TS_ASSERT(result["c"] != 0);
    TS_ASSERT_EQUALS(result["c"]->type, FileBase::IsDirectory);
    TS_ASSERT_EQUALS(result["c"]->size.isValid(), false);
    TS_ASSERT_EQUALS(result["c"]->visibility.orElse(99), 0);

    TS_ASSERT(result["e"] != 0);
    TS_ASSERT_EQUALS(result["e"]->type, FileBase::IsDirectory);
    TS_ASSERT_EQUALS(result["e"]->size.isValid(), false);
    TS_ASSERT_EQUALS(result["e"]->visibility.orElse(99), 0);
}

/** Test removeFile(). */
void
TestServerFileFileBase::testRemove()
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
    TS_ASSERT_THROWS_CODE(testee.removeFile("readable/f"),  "403");
    TS_ASSERT_THROWS_CODE(testee.removeFile("readable/d"),  "403");
    TS_ASSERT_THROWS_CODE(testee.removeFile("readable/nx"), "403");
    TS_ASSERT_THROWS_CODE(testee.removeFile("readable/nx/nx"), "403");

    TS_ASSERT_THROWS_NOTHING(testee.removeFile("writable/f"));
    TS_ASSERT_THROWS_NOTHING(testee.removeFile("writable/d"));
    TS_ASSERT_THROWS_CODE(testee.removeFile("writable/nx"), "403");
    TS_ASSERT_THROWS_CODE(testee.removeFile("writable/nx/nx"), "403");

    TS_ASSERT_THROWS_CODE(testee.removeFile("listable/f"),  "403");
    TS_ASSERT_THROWS_CODE(testee.removeFile("listable/d"),  "403");
    TS_ASSERT_THROWS_CODE(testee.removeFile("listable/nx"), "404");

    TS_ASSERT_THROWS_CODE(testee.removeFile("listable/nx/nx"), "404");
}

/** Test removal of non-empty directory. */
void
TestServerFileFileBase::testRemoveNonemptyDir()
{
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    // Create stuff
    testee.createDirectory("a");
    testee.createDirectory("a/b");
    testee.putFile("a/b/zz", "");

    // Erase
    TS_ASSERT_THROWS_CODE(testee.removeFile("a/b"), "403");

    TS_ASSERT_THROWS_NOTHING(testee.removeFile("a/b/zz"));
    TS_ASSERT_THROWS_NOTHING(testee.removeFile("a/b"));
}

/** Test removal of non-empty directory, with a permission file. */
void
TestServerFileFileBase::testRemoveNonemptyDir2()
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
        InternalDirectoryHandler::Directory* a = InternalDirectoryHandler("/", tb.dir).findDirectory("a");
        TS_ASSERT(a != 0);
        InternalDirectoryHandler::Directory* b = InternalDirectoryHandler("a", *a).findDirectory("b");
        TS_ASSERT(b != 0);
        InternalDirectoryHandler::File* c2file = InternalDirectoryHandler("b", *b).findFile(".c2file");
        TS_ASSERT(c2file != 0);
    }

    // Erase
    TS_ASSERT_THROWS_CODE(testee.removeFile("a/b"), "403");

    TS_ASSERT_THROWS_NOTHING(testee.removeFile("a/b/zz"));
    TS_ASSERT_THROWS_NOTHING(testee.removeFile("a/b"));
}

/** Test removal of non-empty directory, with an extra file. */
void
TestServerFileFileBase::testRemoveNonemptyDir3()
{
    using server::interface::FileBase;

    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    // Create stuff
    testee.createDirectory("a");
    testee.createDirectory("a/b");

    // Verify internal structure
    {
        InternalDirectoryHandler::Directory* a = InternalDirectoryHandler("/", tb.dir).findDirectory("a");
        TS_ASSERT(a != 0);
        InternalDirectoryHandler::Directory* b = InternalDirectoryHandler("a", *a).findDirectory("b");
        TS_ASSERT(b != 0);
        InternalDirectoryHandler("b", *b).createFile(".block", afl::base::Nothing);
    }

    // Verify that a/b appears empty
    {
        FileBase::ContentInfoMap_t result;
        testee.getDirectoryContent("a/b", result);
        TS_ASSERT(result.empty());
    }

    // Erase
    // This fails because the ".block" file is not recognized and therefore cannot be removed.
    TS_ASSERT_THROWS_CODE(testee.removeFile("a/b"), "403");
}

/** Test removal of a directory tree, base case. */
void
TestServerFileFileBase::testRemoveTree()
{
    using server::interface::FileBase;

    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    testee.createDirectoryTree("a/b/c/d/e");
    testee.createDirectoryTree("a/b/c/x/y");
    testee.putFile("a/f", "");

    // Some failures
    TS_ASSERT_THROWS_CODE(testee.removeDirectory("a/f"), "405");
    TS_ASSERT_THROWS_CODE(testee.removeDirectory("a/x"), "404");

    // Success
    TS_ASSERT_THROWS_NOTHING(testee.removeDirectory("a/b/c/x"));
    TS_ASSERT_THROWS_NOTHING(testee.getFileInformation("a/b/c/d"));
    TS_ASSERT_THROWS_NOTHING(testee.removeDirectory("a/b"));
    TS_ASSERT_THROWS_CODE(testee.getFileInformation("a/b"), "404");
}

/** Test removal of a directory tree, user case 1. */
void
TestServerFileFileBase::testRemoveTree1()
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
    TS_ASSERT_THROWS_CODE(testee.removeDirectory("a/b"), "403");

    // Verify it's still there
    tb.session.setUser("");
    TS_ASSERT_THROWS_NOTHING(testee.getFileInformation("a/b"));
}

/** Test removal of a directory tree, user case 2. */
void
TestServerFileFileBase::testRemoveTree2()
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
    TS_ASSERT_THROWS_CODE(testee.removeDirectory("a/b"), "403");
    TS_ASSERT_THROWS_CODE(testee.removeDirectory("a/b/c"), "403");
    TS_ASSERT_THROWS_NOTHING(testee.removeDirectory("a/b/c/d"));

    // Verify it's still there
    tb.session.setUser("");
    TS_ASSERT_THROWS_NOTHING(testee.getFileInformation("a/b"));
}

/** Test removal of a directory tree, user case 3. */
void
TestServerFileFileBase::testRemoveTree3()
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
    TS_ASSERT_THROWS_NOTHING(testee.removeDirectory("a/b"));

    // Verify it's gone
    tb.session.setUser("");
    TS_ASSERT_THROWS_CODE(testee.getFileInformation("a/b"), "404");
    TS_ASSERT_THROWS_NOTHING(testee.getFileInformation("a"));
}

/** Test removal of directory tree, with an extra file. */
void
TestServerFileFileBase::testRemoveTreeFail()
{
    using server::interface::FileBase;

    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    // Create stuff
    testee.createDirectoryTree("a/b/c/d/e");
    testee.createDirectoryTree("a/b/x/y/z");

    // Verify internal structure
    {
        InternalDirectoryHandler::Directory* a = InternalDirectoryHandler("/", tb.dir).findDirectory("a");
        TS_ASSERT(a != 0);
        InternalDirectoryHandler::Directory* b = InternalDirectoryHandler("a", *a).findDirectory("b");
        TS_ASSERT(b != 0);
        InternalDirectoryHandler::Directory* x = InternalDirectoryHandler("x", *b).findDirectory("x");
        TS_ASSERT(x != 0);
        InternalDirectoryHandler("x", *x).createFile(".block", afl::base::Nothing);
    }

    // Erase
    // This fails because the ".block" file is not recognized and therefore cannot be removed.
    // Note that the directory might have still be cleared partially here.
    TS_ASSERT_THROWS_CODE(testee.removeDirectory("a/b"), "403");
}

/** Test removeDirectory(), permission test. */
void
TestServerFileFileBase::testRemoveTreePerm()
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
    TS_ASSERT_THROWS_CODE(testee.removeDirectory("readable/f"),  "403");
    TS_ASSERT_THROWS_CODE(testee.removeDirectory("readable/d"),  "403");
    TS_ASSERT_THROWS_CODE(testee.removeDirectory("readable/nx"), "403");
    TS_ASSERT_THROWS_CODE(testee.removeDirectory("readable/nx/nx"), "403");

    TS_ASSERT_THROWS_CODE(testee.removeDirectory("writable/f"),  "403");
    // FIXME: the following should probably be permitted.
    // It fails because of missing permissions on 'd', but removeFile(d) would be accepted.
    TS_ASSERT_THROWS_CODE(testee.removeDirectory("writable/nx"), "403");
    TS_ASSERT_THROWS_CODE(testee.removeDirectory("writable/nx/nx"), "403");

    TS_ASSERT_THROWS_CODE(testee.removeDirectory("listable/f"),  "405");
    TS_ASSERT_THROWS_CODE(testee.removeDirectory("listable/d"),  "403");
    TS_ASSERT_THROWS_CODE(testee.removeDirectory("listable/nx"), "404");
    TS_ASSERT_THROWS_CODE(testee.removeDirectory("listable/nx/nx"), "404");
}

/** Test getDiskUsage(). */
void
TestServerFileFileBase::testUsage()
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
    TS_ASSERT_THROWS_NOTHING(u = testee.getDiskUsage("readable"));
    TS_ASSERT_EQUALS(u.numItems, 3);         // 1 per directory, 1 per file
    TS_ASSERT_EQUALS(u.totalKBytes, 2);      // 1 per directory, 0 for the empty file

    TS_ASSERT_THROWS_NOTHING(u = testee.getDiskUsage("writable"));
    TS_ASSERT_EQUALS(u.numItems, 3);         // 1 per directory, 1 per file
    TS_ASSERT_EQUALS(u.totalKBytes, 3);      // 1 per directory, 1 for the nonempty file

    TS_ASSERT_THROWS_NOTHING(u = testee.getDiskUsage("listable"));
    TS_ASSERT_EQUALS(u.numItems, 3);         // 1 per directory, 1 per file
    TS_ASSERT_EQUALS(u.totalKBytes, 12);     // 1 per directory, 10 for the file file

    TS_ASSERT_THROWS_CODE(testee.getDiskUsage("nx"), "404");
    TS_ASSERT_THROWS_CODE(testee.getDiskUsage("readable/nx"), "404");
    TS_ASSERT_THROWS_CODE(testee.getDiskUsage("readable/nx/nx"), "404");

    TS_ASSERT_THROWS_CODE(testee.getDiskUsage("readable/f"), "405");

    // Test as user
    tb.session.setUser("1009");
    TS_ASSERT_THROWS_CODE(testee.getDiskUsage("readable"), "403");
    TS_ASSERT_THROWS_CODE(testee.getDiskUsage("writable"), "403");

    TS_ASSERT_THROWS_NOTHING(u = testee.getDiskUsage("listable"));
    TS_ASSERT_EQUALS(u.numItems, 3);         // 1 per directory, 1 per file
    TS_ASSERT_EQUALS(u.totalKBytes, 12);     // 1 per directory, 10 for the file file

    TS_ASSERT_THROWS_CODE(testee.getDiskUsage("nx"), "403");
    TS_ASSERT_THROWS_CODE(testee.getDiskUsage("readable/nx"), "403");
    TS_ASSERT_THROWS_CODE(testee.getDiskUsage("readable/nx/nx"), "403");
    TS_ASSERT_THROWS_CODE(testee.getDiskUsage("readable/f"), "403");

    TS_ASSERT_THROWS_CODE(testee.getDiskUsage("listable/nx"), "404");
    TS_ASSERT_THROWS_CODE(testee.getDiskUsage("listable/nx/nx"), "404");
    TS_ASSERT_THROWS_CODE(testee.getDiskUsage("listable/f"), "405");
}

/** Test putFile. */
void
TestServerFileFileBase::testPut()
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
    TS_ASSERT_THROWS_CODE(testee.putFile("rootfile", ""), "403");
    TS_ASSERT_THROWS_CODE(testee.putFile("readable/f", ""), "403");
    TS_ASSERT_THROWS_NOTHING(testee.putFile("writable/f", ""));
    TS_ASSERT_THROWS_CODE(testee.putFile("writable/nx/f", ""), "403");
    TS_ASSERT_THROWS_CODE(testee.putFile("listable/f", ""), "403");
    TS_ASSERT_THROWS_CODE(testee.putFile("listable/d/f", ""), "403");
    TS_ASSERT_THROWS_CODE(testee.putFile("listable/nx/f", ""), "404");

    // Attempt to overwrite a directory
    TS_ASSERT_THROWS_CODE(testee.putFile("writable/d", ""), "409");
}

/** Test limits. */
void
TestServerFileFileBase::testLimits()
{
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    // Put some files
    testee.putFile("ten",    String_t(10, 'x'));
    testee.putFile("eleven", String_t(11, 'x'));

    // Enable limit
    tb.root.setMaxFileSize(10);

    // get
    TS_ASSERT_THROWS_NOTHING(testee.getFile("ten"));
    TS_ASSERT_THROWS_CODE(testee.getFile("eleven"), "413");

    // put
    TS_ASSERT_THROWS_NOTHING(testee.putFile("ten2", String_t(10, 'y')));
    TS_ASSERT_THROWS_CODE(testee.putFile("eleven2", String_t(11, 'y')), "413");

    // copy
    TS_ASSERT_THROWS_NOTHING(testee.copyFile("ten", "ten3"));
    TS_ASSERT_THROWS_CODE(testee.copyFile("eleven", "eleven3"), "413");
}

/** Test some copyFile() border cases. */
void
TestServerFileFileBase::testCopy()
{
    Testbench tb;
    server::file::FileBase testee(tb.session, tb.root);

    testee.createDirectory("a");
    testee.createDirectory("a/b");
    testee.putFile("a/f", "");

    // Attempt to overwrite a directory
    TS_ASSERT_THROWS_CODE(testee.copyFile("a/f", "a/b"), "409");

    // Copy from nonexistant path
    TS_ASSERT_THROWS_CODE(testee.copyFile("a/x/y", "a/f"), "404");

    // Test to copy a directory
    TS_ASSERT_THROWS_CODE(testee.copyFile("a/b", "a/y"), "404");
}

/** Test copyFile() implemented in underlay. */
void
TestServerFileFileBase::testCopyUnderlay()
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
    TS_ASSERT_EQUALS(testee.getFile("b"), "content");

    TS_ASSERT_EQUALS(testee.getFileInformation("a").size.orElse(-1), 7);
    TS_ASSERT_EQUALS(testee.getFileInformation("b").size.orElse(-1), 7);
}
