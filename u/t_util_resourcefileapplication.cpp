/**
  *  \file u/t_util_resourcefileapplication.cpp
  *  \brief Test for util::ResourceFileApplication
  */

#include "util/resourcefileapplication.hpp"

#include "t_util.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/sys/internalenvironment.hpp"

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

    void testFailingCreateScript(const char* name, const char* script)
    {
        Environment env;
        env.fs.openFile("script", FileSystem::Create)->fullWrite(afl::string::toBytes(script));

        String_t args[] = { "create", "out.res", "script" };
        setCommandLine(env, args);

        TSM_ASSERT_DIFFERS(name, runApplication(env), 0);
        TSM_ASSERT_DIFFERS(name, getOutput(env), "");
    }

    static const uint8_t TEST_FILE[] = {
        0x52, 0x5a, 0x21, 0x00, 0x00, 0x00, 0x03, 0x00, 0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x77,
        0x6f, 0x72, 0x6c, 0x64, 0x0d, 0x0a, 0x6d, 0x6f, 0x72, 0x65, 0x20, 0x74, 0x65, 0x78, 0x74, 0x0d,
        0x0a, 0x64, 0x00, 0x08, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x65, 0x00, 0x16, 0x00, 0x00,
        0x00, 0x0b, 0x00, 0x00, 0x00, 0xc8, 0x00, 0x08, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00
    };

}

/** Invocation without parameters. */
void
TestUtilResourceFileApplication::testNoArgs()
{
    Environment env;
    TS_ASSERT_EQUALS(runApplication(env), 1);
    TS_ASSERT_DIFFERS(getOutput(env), "");
}

/** Test "list" command. */
void
TestUtilResourceFileApplication::testList()
{
    Environment env;
    env.fs.openFile("x.res", FileSystem::Create)->fullWrite(TEST_FILE);

    String_t args[] = { "list", "x.res" };
    setCommandLine(env, args);

    TS_ASSERT_EQUALS(runApplication(env), 0);
    TS_ASSERT_EQUALS(getOutput(env),
                     "  100        14\n"
                     "  101        11\n"
                     "  200        14\n");
}

/** Test "list" command, error case: file not found. */
void
TestUtilResourceFileApplication::testListFileNotFound()
{
    Environment env;

    String_t args[] = { "list", "x.res" };
    setCommandLine(env, args);

    TS_ASSERT_DIFFERS(runApplication(env), 0);
    TS_ASSERT_DIFFERS(getOutput(env), "");
}

/** Test "list" command, error case: bad file. */
void
TestUtilResourceFileApplication::testListBadFile()
{
    Environment env;
    env.fs.openFile("x.res", FileSystem::Create);

    String_t args[] = { "list", "x.res" };
    setCommandLine(env, args);

    TS_ASSERT_DIFFERS(runApplication(env), 0);
    TS_ASSERT_DIFFERS(getOutput(env), "");
}

/** Test "extract" command, success case. */
void
TestUtilResourceFileApplication::testExtract()
{
    Environment env;
    env.fs.openFile("x.res", FileSystem::Create)->fullWrite(TEST_FILE);

    String_t args[] = { "extract", "x.res", "101", "f.out" };
    setCommandLine(env, args);

    TS_ASSERT_EQUALS(runApplication(env), 0);
    TS_ASSERT_EQUALS(getOutput(env), "");

    uint8_t buf[30];
    size_t n = env.fs.openFile("f.out", FileSystem::OpenRead)->read(buf);
    TS_ASSERT_EQUALS(n, 11U);
    TS_ASSERT_SAME_DATA(buf, "more text\r\n", 11);
}

/** Test "extract" command, failure case. */
void
TestUtilResourceFileApplication::testExtractFail()
{
    Environment env;
    env.fs.openFile("x.res", FileSystem::Create)->fullWrite(TEST_FILE);

    String_t args[] = { "extract", "x.res", "201", "f.out" };
    setCommandLine(env, args);

    TS_ASSERT_DIFFERS(runApplication(env), 0);
    TS_ASSERT_DIFFERS(getOutput(env), "");
    TS_ASSERT_THROWS(env.fs.openFile("f.out", FileSystem::OpenRead), afl::except::FileProblemException);
}


/** Test "extract" command, file not found case. */
void
TestUtilResourceFileApplication::testExtractFileNotFound()
{
    Environment env;

    String_t args[] = { "extract", "x.res", "201", "f.out" };
    setCommandLine(env, args);

    TS_ASSERT_DIFFERS(runApplication(env), 0);
    TS_ASSERT_DIFFERS(getOutput(env), "");
    TS_ASSERT_THROWS(env.fs.openFile("f.out", FileSystem::OpenRead), afl::except::FileProblemException);
}

/** Test "extract" command, syntax error. */
void
TestUtilResourceFileApplication::testExtractSyntax()
{
    Environment env;
    env.fs.openFile("x.res", FileSystem::Create)->fullWrite(TEST_FILE);

    String_t args[] = { "extract", "x.res", "qqq", "f.out" };
    setCommandLine(env, args);

    TS_ASSERT_DIFFERS(runApplication(env), 0);
    TS_ASSERT_DIFFERS(getOutput(env), "");
    TS_ASSERT_THROWS(env.fs.openFile("f.out", FileSystem::OpenRead), afl::except::FileProblemException);
}


/** Test "extract" command, syntax error: too many args. */
void
TestUtilResourceFileApplication::testExtractSyntax2()
{
    Environment env;
    env.fs.openFile("x.res", FileSystem::Create)->fullWrite(TEST_FILE);

    String_t args[] = { "extract", "x.res", "201", "f.out", "extra" };
    setCommandLine(env, args);

    TS_ASSERT_DIFFERS(runApplication(env), 0);
    TS_ASSERT_DIFFERS(getOutput(env), "");
    TS_ASSERT_THROWS(env.fs.openFile("f.out", FileSystem::OpenRead), afl::except::FileProblemException);
}

/** Test "extract" command, syntax error: option. */
void
TestUtilResourceFileApplication::testExtractSyntax3()
{
    Environment env;
    env.fs.openFile("x.res", FileSystem::Create)->fullWrite(TEST_FILE);

    String_t args[] = { "extract", "x.res", "201", "f.out", "--extra" };
    setCommandLine(env, args);

    TS_ASSERT_DIFFERS(runApplication(env), 0);
    TS_ASSERT_DIFFERS(getOutput(env), "");
    TS_ASSERT_THROWS(env.fs.openFile("f.out", FileSystem::OpenRead), afl::except::FileProblemException);
}

/** Test "extract-all" command, one-argument version. */
void
TestUtilResourceFileApplication::testExtractAllOne()
{
    Environment env;
    env.fs.openFile("x.res", FileSystem::Create)->fullWrite(TEST_FILE);

    String_t args[] = { "extract-all", "x.res" };
    setCommandLine(env, args);

    TS_ASSERT_EQUALS(runApplication(env), 0);
    TS_ASSERT_EQUALS(getOutput(env), "");

    uint8_t buf[30];
    size_t n = env.fs.openFile("00100.dat", FileSystem::OpenRead)->read(buf);
    TS_ASSERT_EQUALS(n, 14U);
    TS_ASSERT_SAME_DATA(buf, "hello, world\r\n", 14);
}

/** Test "extract-all" command, two-argument version. */
void
TestUtilResourceFileApplication::testExtractAllTwo()
{
    Environment env;
    env.fs.openFile("x.res", FileSystem::Create)->fullWrite(TEST_FILE);

    String_t args[] = { "extract-all", "x.res", "list.rc" };
    setCommandLine(env, args);

    TS_ASSERT_EQUALS(runApplication(env), 0);
    TS_ASSERT_EQUALS(getOutput(env), "");

    uint8_t buf[30];
    size_t n = env.fs.openFile("00100.dat", FileSystem::OpenRead)->read(buf);
    TS_ASSERT_EQUALS(n, 14U);
    TS_ASSERT_SAME_DATA(buf, "hello, world\r\n", 14);

    TS_ASSERT_EQUALS(getFileContent(env, "list.rc"),
                     "100 00100.dat\n"
                     "101 00101.dat\n"
                     "200 eq 100\n");
}

/** Test "extract-all" command, syntax error. */
void
TestUtilResourceFileApplication::testExtractAllError()
{
    Environment env;
    env.fs.openFile("x.res", FileSystem::Create)->fullWrite(TEST_FILE);

    String_t args[] = { "extract-all", "x.res", "list.rc", "whatever" };
    setCommandLine(env, args);

    TS_ASSERT_DIFFERS(runApplication(env), 0);
    TS_ASSERT_DIFFERS(getOutput(env), "");
}

/** Test "create" command, full version. */
void
TestUtilResourceFileApplication::testCreate()
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

    TS_ASSERT_EQUALS(runApplication(env), 0);
    TS_ASSERT_EQUALS(getOutput(env), "");

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
    TS_ASSERT_EQUALS(file->getSize(), sizeof(EXPECTED));

    uint8_t data[sizeof(EXPECTED)];
    TS_ASSERT_EQUALS(file->read(data), sizeof(EXPECTED));
    TS_ASSERT_SAME_DATA(EXPECTED, data, sizeof(EXPECTED));

    // Verify list file
    TS_ASSERT_EQUALS(getFileContent(env, "file.lst"),
                     "first=100\n"
                     "second=101\n"
                     "last=202\n");
}

/** Test "create" command, with CRLF option. */
void
TestUtilResourceFileApplication::testCreateCRLF()
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

    TS_ASSERT_EQUALS(runApplication(env), 0);
    TS_ASSERT_EQUALS(getOutput(env), "");

    // Verify resource file
    static const uint8_t EXPECTED[] = {
        'R','Z',14,0,0,0,1,0,          // 0-7
        'a','\r','\n','b','\r','\n',   // 8-13
        100,0,8,0,0,0,6,0,0,0,
    };
    afl::base::Ref<afl::io::Stream> file = env.fs.openFile("out.res", FileSystem::OpenRead);
    TS_ASSERT_EQUALS(file->getSize(), sizeof(EXPECTED));

    uint8_t data[sizeof(EXPECTED)];
    TS_ASSERT_EQUALS(file->read(data), sizeof(EXPECTED));
    TS_ASSERT_SAME_DATA(EXPECTED, data, sizeof(EXPECTED));
}

/** Test "create" command, script error cases. */
void
TestUtilResourceFileApplication::testCreateErrors()
{
    testFailingCreateScript("next on first",   "next .text\n.endtext\n");
    testFailingCreateScript("bad id",          "foobar .text\n.endtext\n");
    testFailingCreateScript("big id",          "100000 .text\n.endtext\n");
    testFailingCreateScript("missing file",    "100\n");
    testFailingCreateScript("missing endtext", "100 .text\n");
    testFailingCreateScript("missing file",    "100 file\n");
    testFailingCreateScript("bad link",        "100 eq 101\n");
}

/** Test "create" command, command line syntax error case. */
void
TestUtilResourceFileApplication::testCreateSyntaxError()
{
    Environment env;

    String_t args[] = { "create", "out.res" };
    setCommandLine(env, args);

    TS_ASSERT_DIFFERS(runApplication(env), 0);
    TS_ASSERT_DIFFERS(getOutput(env), "");
}

/** Test "create" command, command line syntax error case. */
void
TestUtilResourceFileApplication::testCreateSyntaxError2()
{
    Environment env;
    env.fs.openFile("script", FileSystem::Create)->fullWrite(afl::string::toBytes(""));

    String_t args[] = { "create", "out.res", "--unknown", "script"  };
    setCommandLine(env, args);

    TS_ASSERT_DIFFERS(runApplication(env), 0);
    TS_ASSERT_DIFFERS(getOutput(env), "");
}

/** Test "create" command, search path. */
void
TestUtilResourceFileApplication::testCreateSearch()
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

    TS_ASSERT_EQUALS(runApplication(env), 0);
    TS_ASSERT_EQUALS(getOutput(env), "");

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
    TS_ASSERT_EQUALS(file->getSize(), sizeof(EXPECTED));

    uint8_t data[sizeof(EXPECTED)];
    TS_ASSERT_EQUALS(file->read(data), sizeof(EXPECTED));
    TS_ASSERT_SAME_DATA(EXPECTED, data, sizeof(EXPECTED));

    // Verify dependency file
    TS_ASSERT_EQUALS(getFileContent(env, "x.d"),
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
void
TestUtilResourceFileApplication::testHelp()
{
    Environment env;

    String_t args[] = { "--help" };
    setCommandLine(env, args);

    TS_ASSERT_EQUALS(runApplication(env), 0);
    TS_ASSERT_DIFFERS(getOutput(env), "");

    // Some keywords
    TS_ASSERT_DIFFERS(getOutput(env).find("--list"), String_t::npos);
    TS_ASSERT_DIFFERS(getOutput(env).find("extract-all"), String_t::npos);
}

