/**
  *  \file test/util/resourcefileapplicationtest.cpp
  *  \brief Test for util::ResourceFileApplication
  */

#include "util/resourcefileapplication.hpp"

#include "afl/except/fileproblemexception.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/test/testrunner.hpp"

using afl::io::FileSystem;

namespace {
    String_t normalizeLinefeeds(afl::base::ConstBytes_t bytes)
    {
        String_t result;
        while (const uint8_t* p = bytes.eat()) {
            if (*p != '\r') {
                result.append(1, char(*p));
            }
        }
        return result;
    }

    struct Environment {
        afl::io::InternalFileSystem fs;
        afl::sys::InternalEnvironment env;
        afl::base::Ref<afl::io::InternalStream> output;

        Environment()
            : fs(), env(),
              output(*new afl::io::InternalStream())
            {
                env.setChannelStream(afl::sys::Environment::Output, output.asPtr());
                env.setChannelStream(afl::sys::Environment::Error, output.asPtr());
            }
    };

    void setCommandLine(Environment& env, afl::base::Memory<const String_t> argv)
    {
        afl::data::StringList_t args;
        while (const String_t* p = argv.eat()) {
            args.push_back(*p);
        }
        env.env.setCommandLine(args);
    }

    int runApplication(Environment& env)
    {
        return util::ResourceFileApplication(env.env, env.fs).run();
    }

    String_t getOutput(Environment& env)
    {
        return normalizeLinefeeds(env.output->getContent());
    }

    String_t getFileContent(Environment& env, String_t fileName)
    {
        return normalizeLinefeeds(env.fs.openFile(fileName, FileSystem::OpenRead)
                                  ->createVirtualMapping()
                                  ->get());
    }

    void testFailingCreateScript(afl::test::Assert a, const char* script)
    {
        Environment env;
        env.fs.openFile("script", FileSystem::Create)->fullWrite(afl::string::toBytes(script));

        String_t args[] = { "create", "out.res", "script" };
        setCommandLine(env, args);

        a.checkDifferent("runApplication", runApplication(env), 0);
        a.checkDifferent("getOutput", getOutput(env), "");
    }

    static const uint8_t TEST_FILE[] = {
        0x52, 0x5a, 0x21, 0x00, 0x00, 0x00, 0x03, 0x00, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x77,
        0x6f, 0x72, 0x6c, 0x64, 0x0d, 0x0a, 0x6d, 0x6f, 0x72, 0x65, 0x20, 0x74, 0x65, 0x78, 0x74, 0x0d,
        0x0a, 0x64, 0x00, 0x08, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x65, 0x00, 0x16, 0x00, 0x00,
        0x00, 0x0b, 0x00, 0x00, 0x00, 0xc8, 0x00, 0x08, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00
    };
}

/** Invocation without parameters. */
AFL_TEST("util.ResourceFileApplication:no-args", a)
{
    Environment env;
    a.checkEqual("01. runApplication", runApplication(env), 1);
    a.checkDifferent("02. getOutput", getOutput(env), "");
}

/** Test "list" command. */
AFL_TEST("util.ResourceFileApplication:list", a)
{
    Environment env;
    env.fs.openFile("x.res", FileSystem::Create)->fullWrite(TEST_FILE);

    String_t args[] = { "list", "x.res" };
    setCommandLine(env, args);

    a.checkEqual("01. runApplication", runApplication(env), 0);
    a.checkEqual("02. getOutput", getOutput(env),
                 "  100        14\n"
                 "  101        11\n"
                 "  200        14\n");
}

/** Test "list" command, error case: file not found. */
AFL_TEST("util.ResourceFileApplication:list:error:file-not-found", a)
{
    Environment env;

    String_t args[] = { "list", "x.res" };
    setCommandLine(env, args);

    a.checkDifferent("01. runApplication", runApplication(env), 0);
    a.checkDifferent("02. getOutput", getOutput(env), "");
}

/** Test "list" command, error case: bad file. */
AFL_TEST("util.ResourceFileApplication:list:error:bad-file", a)
{
    Environment env;
    env.fs.openFile("x.res", FileSystem::Create);

    String_t args[] = { "list", "x.res" };
    setCommandLine(env, args);

    a.checkDifferent("01. runApplication", runApplication(env), 0);
    a.checkDifferent("02. getOutput", getOutput(env), "");
}

/** Test "extract" command, success case. */
AFL_TEST("util.ResourceFileApplication:extract", a)
{
    Environment env;
    env.fs.openFile("x.res", FileSystem::Create)->fullWrite(TEST_FILE);

    String_t args[] = { "extract", "x.res", "101", "f.out" };
    setCommandLine(env, args);

    a.checkEqual("01. runApplication", runApplication(env), 0);
    a.checkEqual("02. getOutput", getOutput(env), "");

    uint8_t buf[30];
    size_t n = env.fs.openFile("f.out", FileSystem::OpenRead)->read(buf);
    a.checkEqual("11. result file", n, 11U);
    a.checkEqualContent("12. content", afl::base::ConstBytes_t(buf).trim(11), afl::string::toBytes("more text\r\n"));
}

/** Test "extract" command, failure case. */
AFL_TEST("util.ResourceFileApplication:extract:error:bad-id", a)
{
    Environment env;
    env.fs.openFile("x.res", FileSystem::Create)->fullWrite(TEST_FILE);

    String_t args[] = { "extract", "x.res", "201", "f.out" };
    setCommandLine(env, args);

    a.checkDifferent("01. runApplication", runApplication(env), 0);
    a.checkDifferent("02. getOutput", getOutput(env), "");
    AFL_CHECK_THROWS(a("03. result file"), env.fs.openFile("f.out", FileSystem::OpenRead), afl::except::FileProblemException);
}


/** Test "extract" command, file not found case. */
AFL_TEST("util.ResourceFileApplication:extract:error:file-not-found", a)
{
    Environment env;

    String_t args[] = { "extract", "x.res", "201", "f.out" };
    setCommandLine(env, args);

    a.checkDifferent("01. runApplication", runApplication(env), 0);
    a.checkDifferent("02. getOutput", getOutput(env), "");
    AFL_CHECK_THROWS(a("03. result file"), env.fs.openFile("f.out", FileSystem::OpenRead), afl::except::FileProblemException);
}

/** Test "extract" command, syntax error. */
AFL_TEST("util.ResourceFileApplication:extract:error:bad-number", a)
{
    Environment env;
    env.fs.openFile("x.res", FileSystem::Create)->fullWrite(TEST_FILE);

    String_t args[] = { "extract", "x.res", "qqq", "f.out" };
    setCommandLine(env, args);

    a.checkDifferent("01. runApplication", runApplication(env), 0);
    a.checkDifferent("02. getOutput", getOutput(env), "");
    AFL_CHECK_THROWS(a("03. result file"), env.fs.openFile("f.out", FileSystem::OpenRead), afl::except::FileProblemException);
}


/** Test "extract" command, syntax error: too many args. */
AFL_TEST("util.ResourceFileApplication:extract:error:too-many-args", a)
{
    Environment env;
    env.fs.openFile("x.res", FileSystem::Create)->fullWrite(TEST_FILE);

    String_t args[] = { "extract", "x.res", "201", "f.out", "extra" };
    setCommandLine(env, args);

    a.checkDifferent("01. runApplication", runApplication(env), 0);
    a.checkDifferent("02. getOutput", getOutput(env), "");
    AFL_CHECK_THROWS(a("03. result file"), env.fs.openFile("f.out", FileSystem::OpenRead), afl::except::FileProblemException);
}

/** Test "extract" command, syntax error: option. */
AFL_TEST("util.ResourceFileApplication:extract:error:unknown-option", a)
{
    Environment env;
    env.fs.openFile("x.res", FileSystem::Create)->fullWrite(TEST_FILE);

    String_t args[] = { "extract", "x.res", "201", "f.out", "--extra" };
    setCommandLine(env, args);

    a.checkDifferent("01. runApplication", runApplication(env), 0);
    a.checkDifferent("02. getOutput", getOutput(env), "");
    AFL_CHECK_THROWS(a("03. result file"), env.fs.openFile("f.out", FileSystem::OpenRead), afl::except::FileProblemException);
}

/** Test "extract-all" command, one-argument version. */
AFL_TEST("util.ResourceFileApplication:extract-all:one-arg", a)
{
    Environment env;
    env.fs.openFile("x.res", FileSystem::Create)->fullWrite(TEST_FILE);

    String_t args[] = { "extract-all", "x.res" };
    setCommandLine(env, args);

    a.checkEqual("01. runApplication", runApplication(env), 0);
    a.checkEqual("02. getOutput", getOutput(env), "");

    uint8_t buf[30];
    size_t n = env.fs.openFile("00100.dat", FileSystem::OpenRead)->read(buf);
    a.checkEqual("11. result file", n, 14U);
    a.checkEqualContent("12. content", afl::base::ConstBytes_t(buf).trim(14), afl::string::toBytes("hello, world\r\n"));
}

/** Test "extract-all" command, two-argument version. */
AFL_TEST("util.ResourceFileApplication:extract-all:two-args", a)
{
    Environment env;
    env.fs.openFile("x.res", FileSystem::Create)->fullWrite(TEST_FILE);

    String_t args[] = { "extract-all", "x.res", "list.rc" };
    setCommandLine(env, args);

    a.checkEqual("01. runApplication", runApplication(env), 0);
    a.checkEqual("02. getOutput", getOutput(env), "");

    uint8_t buf[30];
    size_t n = env.fs.openFile("00100.dat", FileSystem::OpenRead)->read(buf);
    a.checkEqual("11. result file", n, 14U);
    a.checkEqualContent("12. content", afl::base::ConstBytes_t(buf).trim(14), afl::string::toBytes("hello, world\r\n"));

    a.checkEqual("21. script file", getFileContent(env, "list.rc"),
                 "100 00100.dat\n"
                 "101 00101.dat\n"
                 "200 eq 100\n");
}

/** Test "extract-all" command, syntax error. */
AFL_TEST("util.ResourceFileApplication:extract-all:error:too-many-args", a)
{
    Environment env;
    env.fs.openFile("x.res", FileSystem::Create)->fullWrite(TEST_FILE);

    String_t args[] = { "extract-all", "x.res", "list.rc", "whatever" };
    setCommandLine(env, args);

    a.checkDifferent("01. runApplication", runApplication(env), 0);
    a.checkDifferent("02. getOutput", getOutput(env), "");
}

/** Test "create" command, full version. */
AFL_TEST("util.ResourceFileApplication:create", a)
{
    const char*const SCRIPT =
        " ;the script\n\n"
        "100=first in1\n"
        " next = second in2\n"
        "200 .text\n"
        "hello\n"
        ".endtext\n"
        "201 eq 100\n"
        "202=last .nul\n";
    Environment env;
    env.fs.openFile("in1", FileSystem::Create)->fullWrite(afl::string::toBytes("one"));
    env.fs.openFile("in2", FileSystem::Create)->fullWrite(afl::string::toBytes("two"));
    env.fs.openFile("script", FileSystem::Create)->fullWrite(afl::string::toBytes(SCRIPT));

    String_t args[] = { "create", "out.res", "script", "--list=file.lst", "--list-format=%s=%d" };
    setCommandLine(env, args);

    a.checkEqual("01. runApplication", runApplication(env), 0);
    a.checkEqual("02. getOutput", getOutput(env), "");

    // Verify resource file
    static const uint8_t EXPECTED[] = {
        'R','Z',20,0,0,0,5,0,          // 0-7
        'o','n','e',                   // 8-10
        't','w','o',                   // 11-13
        'h','e','l','l','o','\n',      // 14-19
        100,0,8,0,0,0,3,0,0,0,
        101,0,11,0,0,0,3,0,0,0,
        200,0,14,0,0,0,6,0,0,0,
        201,0,8,0,0,0,3,0,0,0,
        202,0,20,0,0,0,0,0,0,0,
    };
    afl::base::Ref<afl::io::Stream> file = env.fs.openFile("out.res", FileSystem::OpenRead);
    a.checkEqual("11. getSize", file->getSize(), sizeof(EXPECTED));

    uint8_t data[sizeof(EXPECTED)];
    a.checkEqual("21. read", file->read(data), sizeof(EXPECTED));
    a.checkEqualContent<uint8_t>("22. content", data, EXPECTED);

    // Verify list file
    a.checkEqual("31. list file", getFileContent(env, "file.lst"),
                 "first=100\n"
                 "second=101\n"
                 "last=202\n");
}

/** Test "create" command, with CRLF option. */
AFL_TEST("util.ResourceFileApplication:create:crlf", a)
{
    const char*const SCRIPT =
        "100 .text\n"
        "a\n"
        "b\n"
        ".endtext";
    Environment env;
    env.fs.openFile("script", FileSystem::Create)->fullWrite(afl::string::toBytes(SCRIPT));

    String_t args[] = { "create", "--crlf", "out.res", "script" };
    setCommandLine(env, args);

    a.checkEqual("01. runApplication", runApplication(env), 0);
    a.checkEqual("02. getOutput", getOutput(env), "");

    // Verify resource file
    static const uint8_t EXPECTED[] = {
        'R','Z',14,0,0,0,1,0,          // 0-7
        'a','\r','\n','b','\r','\n',   // 8-13
        100,0,8,0,0,0,6,0,0,0,
    };
    afl::base::Ref<afl::io::Stream> file = env.fs.openFile("out.res", FileSystem::OpenRead);
    a.checkEqual("11. getSize", file->getSize(), sizeof(EXPECTED));

    uint8_t data[sizeof(EXPECTED)];
    a.checkEqual("21", file->read(data), sizeof(EXPECTED));
    a.checkEqualContent<uint8_t>("22. content", data, EXPECTED);
}

/** Test "create" command, script error cases. */
AFL_TEST("util.ResourceFileApplication:create:error:script", a)
{
    testFailingCreateScript(a("next on first"),   "next .text\n.endtext\n");
    testFailingCreateScript(a("bad id"),          "foobar .text\n.endtext\n");
    testFailingCreateScript(a("big id"),          "100000 .text\n.endtext\n");
    testFailingCreateScript(a("missing file"),    "100\n");
    testFailingCreateScript(a("missing endtext"), "100 .text\n");
    testFailingCreateScript(a("missing file"),    "100 file\n");
    testFailingCreateScript(a("bad link"),        "100 eq 101\n");
}

/** Test "create" command, command line syntax error case. */
AFL_TEST("util.ResourceFileApplication:create:error:missing-arg", a)
{
    Environment env;

    String_t args[] = { "create", "out.res" };
    setCommandLine(env, args);

    a.checkDifferent("01. runApplication", runApplication(env), 0);
    a.checkDifferent("02. getOutput", getOutput(env), "");
}

/** Test "create" command, command line syntax error case. */
AFL_TEST("util.ResourceFileApplication:create:error:unknown-option", a)
{
    Environment env;
    env.fs.openFile("script", FileSystem::Create)->fullWrite(afl::string::toBytes(""));

    String_t args[] = { "create", "out.res", "--unknown", "script"  };
    setCommandLine(env, args);

    a.checkDifferent("01. runApplication", runApplication(env), 0);
    a.checkDifferent("02. getOutput", getOutput(env), "");
}

/** Test "create" command, search path. */
AFL_TEST("util.ResourceFileApplication:create:search-path", a)
{
    const char*const SCRIPT =
        "100 a\n"                      // Path search (implicit because no delimiter)
        "101 */b\n"                    // Path search (explicit)
        "102 ex/c\n"                   // No path search
        "103 */ex/c\n";                // Path search (explicit)
    Environment env;
    env.fs.createDirectory("sub");
    env.fs.createDirectory("sub/ex");
    env.fs.createDirectory("ex");
    env.fs.openFile("sub/a",    FileSystem::Create)->fullWrite(afl::string::toBytes("x"));
    env.fs.openFile("sub/b",    FileSystem::Create)->fullWrite(afl::string::toBytes("y"));
    env.fs.openFile("sub/ex/c", FileSystem::Create)->fullWrite(afl::string::toBytes("q"));
    env.fs.openFile("ex/c",     FileSystem::Create)->fullWrite(afl::string::toBytes("z"));
    env.fs.openFile("script", FileSystem::Create)->fullWrite(afl::string::toBytes(SCRIPT));

    String_t args[] = { "create", "out.res", "script", "-Lsub", "--dep=x.d" };
    setCommandLine(env, args);

    a.checkEqual("01. runApplication", runApplication(env), 0);
    a.checkEqual("02. getOutput", getOutput(env), "");

    // Verify resource file
    static const uint8_t EXPECTED[] = {
        'R','Z',12,0,0,0,4,0,          // 0-7
        'x','y','z','q',               // 8-11
        100,0,8,0,0,0,1,0,0,0,
        101,0,9,0,0,0,1,0,0,0,
        102,0,10,0,0,0,1,0,0,0,
        103,0,11,0,0,0,1,0,0,0,
    };
    afl::base::Ref<afl::io::Stream> file = env.fs.openFile("out.res", FileSystem::OpenRead);
    a.checkEqual("11. getSize", file->getSize(), sizeof(EXPECTED));

    uint8_t data[sizeof(EXPECTED)];
    a.checkEqual("21. read", file->read(data), sizeof(EXPECTED));
    a.checkEqualContent<uint8_t>("22. content", data, EXPECTED);

    // Verify dependency file
    a.checkEqual("31. dep file", getFileContent(env, "x.d"),
                 "out.res: \\\n"
                 "\tscript \\\n"
                 "\tsub/a \\\n"
                 "\tsub/b \\\n"
                 "\tex/c \\\n"
                 "\tsub/ex/c\n"
                 "script:\n"
                 "sub/a:\n"
                 "sub/b:\n"
                 "ex/c:\n"
                 "sub/ex/c:\n");
}

/** Test help invocation. */
AFL_TEST("util.ResourceFileApplication:help", a)
{
    Environment env;

    String_t args[] = { "--help" };
    setCommandLine(env, args);

    a.checkEqual("01. runApplication", runApplication(env), 0);
    a.checkDifferent("02. getOutput", getOutput(env), "");

    // Some keywords
    a.checkDifferent("11. getOutput", getOutput(env).find("--list"), String_t::npos);
    a.checkDifferent("12. getOutput", getOutput(env).find("extract-all"), String_t::npos);
}
