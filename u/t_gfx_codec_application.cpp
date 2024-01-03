/**
  *  \file u/t_gfx_codec_application.cpp
  *  \brief Test for gfx::codec::Application
  */

#include "gfx/codec/application.hpp"

#include "t_gfx_codec.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/internalenvironment.hpp"
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
void
TestGfxCodecApplication::testNoArgs()
{
    Environment env;
    TS_ASSERT_EQUALS(runApplication(env), 1);
    TS_ASSERT_DIFFERS(getOutput(env), "");
}

/** Test successful invocation of "convert" subcommant. */
void
TestGfxCodecApplication::testConvert()
{
    Environment env;
    env.fs.openFile("in.cc", FileSystem::Create)->fullWrite(FOUR_BIT_FILE);

    String_t args[] = { "convert", "custom:in.cc", "bmp:out.bmp" };
    setCommandLine(env, args);

    TS_ASSERT_EQUALS(runApplication(env), 0);
    TS_ASSERT_EQUALS(getOutput(env), "");
    TS_ASSERT_EQUALS(getFileContent(env, "out.bmp").substr(0, 2), "BM");
}

/** Test failing invocation of "convert" subcommand: input file not found.
    This must generate an error message, exit code 1, and not create the output file. */
void
TestGfxCodecApplication::testConvertFileNotFound()
{
    Environment env;

    String_t args[] = { "convert", "custom:in.cc", "bmp:out.bmp" };
    setCommandLine(env, args);

    TS_ASSERT_EQUALS(runApplication(env), 1);
    TS_ASSERT_DIFFERS(getOutput(env), "");
    TS_ASSERT(env.fs.openFileNT("out.bmp", FileSystem::OpenRead).get() == 0);
}

/** Test failing invocation of "convert" subcommand: bad input syntax.
    This must generate an error message, exit code 1, and not create the output file. */
void
TestGfxCodecApplication::testConvertBadSyntax()
{
    Environment env;
    env.fs.openFile("in.cc", FileSystem::Create)->fullWrite(FOUR_BIT_FILE);

    String_t args[] = { "convert", "whatever:in.cc", "bmp:out.bmp" };
    setCommandLine(env, args);

    TS_ASSERT_EQUALS(runApplication(env), 1);
    TS_ASSERT_DIFFERS(getOutput(env), "");
    TS_ASSERT(env.fs.openFileNT("out.bmp", FileSystem::OpenRead).get() == 0);
}

/** Test conversion to plain 4-bit. */
void
TestGfxCodecApplication::testConvertToPlain4()
{
    Environment env;
    env.fs.openFile("in.cc", FileSystem::Create)->fullWrite(FOUR_BIT_FILE);

    String_t args[] = { "convert", "custom:in.cc", "plain4:out.cc" };
    setCommandLine(env, args);

    TS_ASSERT_EQUALS(runApplication(env), 0);
    TS_ASSERT_EQUALS(getOutput(env), "");
    TS_ASSERT_EQUALS(getFileContent(env, "out.cc").substr(0, 2), "CC");
}

/** Test conversion to plain 8-bit. */
void
TestGfxCodecApplication::testConvertToPlain8()
{
    Environment env;
    env.fs.openFile("in.cc", FileSystem::Create)->fullWrite(FOUR_BIT_FILE);

    String_t args[] = { "convert", "custom:in.cc", "plain8:out.cd" };
    setCommandLine(env, args);

    TS_ASSERT_EQUALS(runApplication(env), 0);
    TS_ASSERT_EQUALS(getOutput(env), "");
    TS_ASSERT_EQUALS(getFileContent(env, "out.cd").substr(0, 2), "CD");
}

/** Test conversion to packed 4-bit.
    The compressed data stream will contain our signature at position 7. */
void
TestGfxCodecApplication::testConvertToPacked4()
{
    Environment env;
    env.fs.openFile("in.cc", FileSystem::Create)->fullWrite(FOUR_BIT_FILE);

    String_t args[] = { "convert", "custom:in.cc", "packed4:out.cc" };
    setCommandLine(env, args);

    TS_ASSERT_EQUALS(runApplication(env), 0);
    TS_ASSERT_EQUALS(getOutput(env), "");
    TS_ASSERT_LESS_THAN(7U, getFileContent(env, "out.cc").size());
    TS_ASSERT_EQUALS(getFileContent(env, "out.cc").substr(7, 2), "CC");
}

/** Test conversion to packed 8-bit. */
void
TestGfxCodecApplication::testConvertToPacked8()
{
    Environment env;
    env.fs.openFile("in.cc", FileSystem::Create)->fullWrite(FOUR_BIT_FILE);

    String_t args[] = { "convert", "custom:in.cc", "packed8:out.cd" };
    setCommandLine(env, args);

    TS_ASSERT_EQUALS(runApplication(env), 0);
    TS_ASSERT_EQUALS(getOutput(env), "");
    TS_ASSERT_LESS_THAN(7U, getFileContent(env, "out.cd").size());
    TS_ASSERT_EQUALS(getFileContent(env, "out.cd").substr(7, 2), "CD");
}

/** Test unsuccessful "convert" subcommand invocation: too few args. */
void
TestGfxCodecApplication::testConvertTooFew()
{
    Environment env;
    env.fs.openFile("in.cc", FileSystem::Create)->fullWrite(FOUR_BIT_FILE);

    String_t args[] = { "convert", "custom:in.cc" };
    setCommandLine(env, args);

    TS_ASSERT_EQUALS(runApplication(env), 1);
    TS_ASSERT_DIFFERS(getOutput(env), "");
}

/** Test unsuccessful "convert" subcommand invocation: too many args. */
void
TestGfxCodecApplication::testConvertTooMany()
{
    Environment env;
    env.fs.openFile("in.cc", FileSystem::Create)->fullWrite(FOUR_BIT_FILE);

    String_t args[] = { "convert", "custom:in.cc", "packed8:out.cd", "--foobar" };
    setCommandLine(env, args);

    TS_ASSERT_EQUALS(runApplication(env), 1);
    TS_ASSERT_DIFFERS(getOutput(env), "");
}

/** Test successful invocation of "create" command. */
void
TestGfxCodecApplication::testCreate()
{
    Environment env;
    env.fs.openFile("in.cc", FileSystem::Create)->fullWrite(FOUR_BIT_FILE);

    String_t args[] = { "create", "out.res", "100=custom:in.cc", "200=custom:in.cc" };
    setCommandLine(env, args);

    TS_ASSERT_EQUALS(runApplication(env), 0);
    TS_ASSERT_EQUALS(getOutput(env), "");

    afl::base::Ref<afl::io::Stream> res = env.fs.openFile("out.res", FileSystem::OpenRead);
    afl::string::NullTranslator tx;
    util::ResourceFileReader rdr(*res, tx);
    TS_ASSERT_EQUALS(rdr.getNumMembers(), 4U);
    TS_ASSERT_THROWS_NOTHING(rdr.openMember(100));
    TS_ASSERT_THROWS_NOTHING(rdr.openMember(200));
    TS_ASSERT_THROWS_NOTHING(rdr.openMember(20100));
    TS_ASSERT_THROWS_NOTHING(rdr.openMember(20200));
}

/** Test unsuccessful "create" subcommand invocation: too few args. */
void
TestGfxCodecApplication::testCreateTooFew()
{
    Environment env;

    String_t args[] = { "create" };
    setCommandLine(env, args);

    TS_ASSERT_EQUALS(runApplication(env), 1);
    TS_ASSERT_DIFFERS(getOutput(env), "");
}

/** Test unsuccessful "create" subcommand invocation: syntax error. */
void
TestGfxCodecApplication::testCreateSyntax()
{
    Environment env;
    env.fs.openFile("in.cc", FileSystem::Create)->fullWrite(FOUR_BIT_FILE);

    String_t args[] = { "create", "out.res", "100=whatever:in.cc" };
    setCommandLine(env, args);

    TS_ASSERT_EQUALS(runApplication(env), 1);
    TS_ASSERT_DIFFERS(getOutput(env), "");
}

/** Test invocation with bad command. */
void
TestGfxCodecApplication::testBadCommand()
{
    Environment env;

    String_t args[] = { "frobnicate" };
    setCommandLine(env, args);

    TS_ASSERT_EQUALS(runApplication(env), 1);
    TS_ASSERT_DIFFERS(getOutput(env), "");
}

/** Test help invocation. */
void
TestGfxCodecApplication::testHelp()
{
    Environment env;

    String_t args[] = { "--help" };
    setCommandLine(env, args);

    TS_ASSERT_EQUALS(runApplication(env), 0);
    TS_ASSERT_DIFFERS(getOutput(env), "");
    TS_ASSERT_DIFFERS(getOutput(env).find("create"), String_t::npos);
}

