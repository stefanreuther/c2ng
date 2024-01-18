/**
  *  \file test/gfx/codec/applicationtest.cpp
  *  \brief Test for gfx::codec::Application
  */

#include "gfx/codec/application.hpp"

#include "afl/io/filemapping.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/test/testrunner.hpp"
#include "util/resourcefilereader.hpp"

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
        return gfx::codec::Application(env.env, env.fs).run();
    }

    String_t getOutput(Environment& env)
    {
        return normalizeLinefeeds(env.output->getContent());
    }

    String_t getFileContent(Environment& env, String_t fileName)
    {
        return afl::string::fromBytes(env.fs.openFile(fileName, FileSystem::OpenRead)
                                      ->createVirtualMapping()
                                      ->get());
    }

    static const uint8_t FOUR_BIT_FILE[] = {
        'C','C',3,0,5,0,
        16,0,16, 0,0,16, 0,12,0, 0,0,0, 0,0,0,
        0xAA, 0x0A,
        0xCA, 0x0A,
        0xAA, 0x0A,
        0xBA, 0x0A,
        0xBA, 0x0A,
    };
}

/** Test invocation with no arguments.
    This is an error and should generate exit code 1. */
AFL_TEST("gfx.codec.Application:no-args", a)
{
    Environment env;
    a.checkEqual("01. run", runApplication(env), 1);
    a.checkDifferent("02. output", getOutput(env), "");
}

/** Test successful invocation of "convert" subcommant. */
AFL_TEST("gfx.codec.Application:convert:bmp", a)
{
    Environment env;
    env.fs.openFile("in.cc", FileSystem::Create)->fullWrite(FOUR_BIT_FILE);

    String_t args[] = { "convert", "custom:in.cc", "bmp:out.bmp" };
    setCommandLine(env, args);

    a.checkEqual("01. run",    runApplication(env), 0);
    a.checkEqual("02. output", getOutput(env), "");
    a.checkEqual("03. file",   getFileContent(env, "out.bmp").substr(0, 2), "BM");
}

/** Test failing invocation of "convert" subcommand: input file not found.
    This must generate an error message, exit code 1, and not create the output file. */
AFL_TEST("gfx.codec.Application:convert:error:not-found", a)
{
    Environment env;

    String_t args[] = { "convert", "custom:in.cc", "bmp:out.bmp" };
    setCommandLine(env, args);

    a.checkEqual    ("01. run",               runApplication(env), 1);
    a.checkDifferent("02. output",            getOutput(env), "");
    a.checkNull     ("03. no output created", env.fs.openFileNT("out.bmp", FileSystem::OpenRead).get());
}

/** Test failing invocation of "convert" subcommand: bad input syntax.
    This must generate an error message, exit code 1, and not create the output file. */
AFL_TEST("gfx.codec.Application:convert:error:bad-syntax", a)
{
    Environment env;
    env.fs.openFile("in.cc", FileSystem::Create)->fullWrite(FOUR_BIT_FILE);

    String_t args[] = { "convert", "whatever:in.cc", "bmp:out.bmp" };
    setCommandLine(env, args);

    a.checkEqual    ("01. run",               runApplication(env), 1);
    a.checkDifferent("02. output",            getOutput(env), "");
    a.checkNull     ("03. no output created", env.fs.openFileNT("out.bmp", FileSystem::OpenRead).get());
}

/** Test conversion to plain 4-bit. */
AFL_TEST("gfx.codec.Application:convert:plain4", a)
{
    Environment env;
    env.fs.openFile("in.cc", FileSystem::Create)->fullWrite(FOUR_BIT_FILE);

    String_t args[] = { "convert", "custom:in.cc", "plain4:out.cc" };
    setCommandLine(env, args);

    a.checkEqual("01. run",          runApplication(env), 0);
    a.checkEqual("02. output",       getOutput(env), "");
    a.checkEqual("03. file content", getFileContent(env, "out.cc").substr(0, 2), "CC");
}

/** Test conversion to plain 8-bit. */
AFL_TEST("gfx.codec.Application:convert:plain8", a)
{
    Environment env;
    env.fs.openFile("in.cc", FileSystem::Create)->fullWrite(FOUR_BIT_FILE);

    String_t args[] = { "convert", "custom:in.cc", "plain8:out.cd" };
    setCommandLine(env, args);

    a.checkEqual("01. run",          runApplication(env), 0);
    a.checkEqual("02. output",       getOutput(env), "");
    a.checkEqual("03. file content", getFileContent(env, "out.cd").substr(0, 2), "CD");
}

/** Test conversion to packed 4-bit.
    The compressed data stream will contain our signature at position 7. */
AFL_TEST("gfx.codec.Application:convert:packed4", a)
{
    Environment env;
    env.fs.openFile("in.cc", FileSystem::Create)->fullWrite(FOUR_BIT_FILE);

    String_t args[] = { "convert", "custom:in.cc", "packed4:out.cc" };
    setCommandLine(env, args);

    a.checkEqual       ("01. run",          runApplication(env), 0);
    a.checkEqual       ("02. output",       getOutput(env), "");
    a.checkGreaterEqual("03. file size",    getFileContent(env, "out.cc").size(), 7U);
    a.checkEqual       ("04. file content", getFileContent(env, "out.cc").substr(7, 2), "CC");
}

/** Test conversion to packed 8-bit. */
AFL_TEST("gfx.codec.Application:convert:packed8", a)
{
    Environment env;
    env.fs.openFile("in.cc", FileSystem::Create)->fullWrite(FOUR_BIT_FILE);

    String_t args[] = { "convert", "custom:in.cc", "packed8:out.cd" };
    setCommandLine(env, args);

    a.checkEqual       ("01. run",          runApplication(env), 0);
    a.checkEqual       ("02. output",       getOutput(env), "");
    a.checkGreaterEqual("03. file size",    getFileContent(env, "out.cd").size(), 7U);
    a.checkEqual       ("04. file content", getFileContent(env, "out.cd").substr(7, 2), "CD");
}

/** Test unsuccessful "convert" subcommand invocation: too few args. */
AFL_TEST("gfx.codec.Application:convert:error:too-few-args", a)
{
    Environment env;
    env.fs.openFile("in.cc", FileSystem::Create)->fullWrite(FOUR_BIT_FILE);

    String_t args[] = { "convert", "custom:in.cc" };
    setCommandLine(env, args);

    a.checkEqual    ("01. run",    runApplication(env), 1);
    a.checkDifferent("02. output", getOutput(env), "");
}

/** Test unsuccessful "convert" subcommand invocation: too many args. */
AFL_TEST("gfx.codec.Application:convert:error:too-many-args", a)
{
    Environment env;
    env.fs.openFile("in.cc", FileSystem::Create)->fullWrite(FOUR_BIT_FILE);

    String_t args[] = { "convert", "custom:in.cc", "packed8:out.cd", "--foobar" };
    setCommandLine(env, args);

    a.checkEqual    ("01. run",    runApplication(env), 1);
    a.checkDifferent("02. output", getOutput(env), "");
}

/** Test successful invocation of "create" command. */
AFL_TEST("gfx.codec.Application:create", a)
{
    Environment env;
    env.fs.openFile("in.cc", FileSystem::Create)->fullWrite(FOUR_BIT_FILE);

    String_t args[] = { "create", "out.res", "100=custom:in.cc", "200=custom:in.cc" };
    setCommandLine(env, args);

    a.checkEqual("01. run", runApplication(env), 0);
    a.checkEqual("02. output", getOutput(env), "");

    afl::base::Ref<afl::io::Stream> res = env.fs.openFile("out.res", FileSystem::OpenRead);
    afl::string::NullTranslator tx;
    util::ResourceFileReader rdr(*res, tx);
    a.checkEqual("11. getNumMembers", rdr.getNumMembers(), 4U);
    AFL_CHECK_SUCCEEDS(a("12. openMember"), rdr.openMember(100));
    AFL_CHECK_SUCCEEDS(a("13. openMember"), rdr.openMember(200));
    AFL_CHECK_SUCCEEDS(a("14. openMember"), rdr.openMember(20100));
    AFL_CHECK_SUCCEEDS(a("15. openMember"), rdr.openMember(20200));
}

/** Test unsuccessful "create" subcommand invocation: too few args. */
AFL_TEST("gfx.codec.Application:create:error:too-few-args", a)
{
    Environment env;

    String_t args[] = { "create" };
    setCommandLine(env, args);

    a.checkEqual    ("01. run",    runApplication(env), 1);
    a.checkDifferent("02. output", getOutput(env), "");
}

/** Test unsuccessful "create" subcommand invocation: syntax error. */
AFL_TEST("gfx.codec.Application:create:error:syntax", a)
{
    Environment env;
    env.fs.openFile("in.cc", FileSystem::Create)->fullWrite(FOUR_BIT_FILE);

    String_t args[] = { "create", "out.res", "100=whatever:in.cc" };
    setCommandLine(env, args);

    a.checkEqual    ("01. run",    runApplication(env), 1);
    a.checkDifferent("02. output", getOutput(env), "");
}

/** Test invocation with bad command. */
AFL_TEST("gfx.codec.Application:error:bad-command", a)
{
    Environment env;

    String_t args[] = { "frobnicate" };
    setCommandLine(env, args);

    a.checkEqual    ("01. run",    runApplication(env), 1);
    a.checkDifferent("02. output", getOutput(env), "");
}

/** Test help invocation. */
AFL_TEST("gfx.codec.Application:help", a)
{
    Environment env;

    String_t args[] = { "--help" };
    setCommandLine(env, args);

    a.checkEqual("01. run", runApplication(env), 0);
    a.checkDifferent("02. output not empty", getOutput(env), "");
    a.checkDifferent("03. output mentions command", getOutput(env).find("create"), String_t::npos);
}
