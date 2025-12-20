/**
  *  \file test/interpreter/consoleapplicationtest.cpp
  *  \brief Test for interpreter::ConsoleApplication
  */

#include "interpreter/consoleapplication.hpp"

#include "afl/io/filemapping.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/stream.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/test/testrunner.hpp"
#include "util/io.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::io::FileSystem;
using afl::io::InternalFileSystem;
using afl::io::InternalStream;
using afl::sys::Environment;
using afl::sys::InternalEnvironment;

namespace {
    // Sample object file, created from code
    //   Struct narf
    //      x, y, z
    //   EndStruct
    // Generates three objects: the structure type, constructor function, main.
    const uint8_t OBJECT_FILE[] = {
        0x43, 0x43, 0x6f, 0x62, 0x6a, 0x1a, 0x64, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x07, 0x00,
        0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x58,
        0x01, 0x59, 0x01, 0x5a, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x79, 0x00, 0x00, 0x00,
        0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
        0x16, 0x0b, 0x01, 0x00, 0x01, 0x0b, 0x4e, 0x41, 0x52, 0x46, 0x73, 0x2e, 0x71, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x00, 0x00, 0x6e, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
        0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
        0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
        0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x02,
        0x00, 0x00, 0x00, 0x04, 0x4e, 0x41, 0x52, 0x46, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x09, 0x0b,
        0x73, 0x2e, 0x71, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00
    };

    // Stripped version of OBJECT_FILE
    const uint8_t STRIPPED_FILE[] = {
        0x43, 0x43, 0x6f, 0x62, 0x6a, 0x1a, 0x64, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x07, 0x00,
        0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x58,
        0x01, 0x59, 0x01, 0x5a, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x56, 0x00, 0x00, 0x00,
        0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
        0x16, 0x0b, 0x01, 0x00, 0x01, 0x0b, 0x4e, 0x41, 0x52, 0x46, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x43, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x08, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x02, 0x00, 0x00, 0x00,
        0x04, 0x4e, 0x41, 0x52, 0x46, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x09, 0x0b
    };
}

/*
 *  Basic invocation
 */

/* Parameterless invocation */
AFL_TEST("interpreter.ConsoleApplication:no-args", a)
{
    InternalFileSystem fs;
    InternalEnvironment env;

    Ptr<InternalStream> out = new InternalStream();
    env.setChannelStream(Environment::Error, out);

    int ret = interpreter::ConsoleApplication(env, fs).run();
    a.checkDifferent("expect error return", ret, 0);
    a.checkDifferent("expect nonempty output", out->getContent().size(), 0U);
}

/* Invoke help screen */
AFL_TEST("interpreter.ConsoleApplication:help", a)
{
    InternalFileSystem fs;
    InternalEnvironment env;

    Ptr<InternalStream> out = new InternalStream();
    env.setChannelStream(Environment::Output, out);

    afl::data::StringList_t args;
    args.push_back("--help");
    env.setCommandLine(args);

    int ret = interpreter::ConsoleApplication(env, fs).run();
    a.checkEqual("expect success return", ret, 0);
    a.checkDifferent("expect nonempty output", out->getContent().size(), 0U);
}

/*
 *  Compile
 */

/* Basic test */
AFL_TEST("interpreter.ConsoleApplication:compile", a)
{
    InternalFileSystem fs;
    InternalEnvironment env;

    afl::data::StringList_t args;
    args.push_back("-k");
    args.push_back("print 5");
    args.push_back("-o");
    args.push_back("x.qc");
    env.setCommandLine(args);

    int ret = interpreter::ConsoleApplication(env, fs).run();
    a.checkEqual("expect success return", ret, 0);

    AFL_CHECK_SUCCEEDS(a("expect output to exist"), fs.openFile("/x.qc", FileSystem::OpenRead));
}

/* Basic test, file to file */
AFL_TEST("interpreter.ConsoleApplication:compile:file-to-file", a)
{
    InternalFileSystem fs;
    InternalEnvironment env;

    fs.openFile("/x.q", FileSystem::Create)
        ->fullWrite(afl::string::toBytes("print 5\n"));

    afl::data::StringList_t args;
    args.push_back("x.q");
    env.setCommandLine(args);

    int ret = interpreter::ConsoleApplication(env, fs).run();
    a.checkEqual("expect success return", ret, 0);

    AFL_CHECK_SUCCEEDS(a("expect output to exist"), fs.openFile("/x.qc", FileSystem::OpenRead));
}

/* Error case: commands given, but no output file name */
AFL_TEST("interpreter.ConsoleApplication:compile:error:no-output", a)
{
    InternalFileSystem fs;
    InternalEnvironment env;

    Ptr<InternalStream> out = new InternalStream();
    env.setChannelStream(Environment::Error, out);

    afl::data::StringList_t args;
    args.push_back("-k");
    args.push_back("print 5");
    env.setCommandLine(args);

    int ret = interpreter::ConsoleApplication(env, fs).run();
    a.checkDifferent("expect success return", ret, 0);
    a.checkDifferent("expect nonempty error output", out->getContent().size(), 0U);
}

/*
 *  Disassemble
 */

/* Basic test, disassemble to standard output */
AFL_TEST("interpreter.ConsoleApplication:disassemble", a)
{
    InternalFileSystem fs;
    InternalEnvironment env;

    Ptr<InternalStream> out = new InternalStream();
    env.setChannelStream(Environment::Output, out);

    afl::data::StringList_t args;
    args.push_back("-k");
    args.push_back("print 5");
    args.push_back("-S");
    env.setCommandLine(args);

    int ret = interpreter::ConsoleApplication(env, fs).run();
    a.checkEqual("expect success return", ret, 0);

    String_t output = util::normalizeLinefeeds(out->getContent());
    String_t expect =
        "Sub BCO1\n"
        "  .name -\n"
        "    .line 1\n"
        "    pushint         5\n"
        "    sprint\n"
        "EndSub\n"
        "\n";
    a.checkEqual("expected output", output, expect);
}

/* Basic test, disassemble to file */
AFL_TEST("interpreter.ConsoleApplication:disassemble:to-file", a)
{
    InternalFileSystem fs;
    InternalEnvironment env;

    Ptr<InternalStream> out = new InternalStream();
    env.setChannelStream(Environment::Output, out);

    afl::data::StringList_t args;
    args.push_back("-k");
    args.push_back("print 5");
    args.push_back("-S");
    args.push_back("-o");
    args.push_back("x.txt");
    env.setCommandLine(args);

    int ret = interpreter::ConsoleApplication(env, fs).run();
    a.checkEqual("expect success return", ret, 0);

    Ref<afl::io::Stream> result = fs.openFile("/x.txt", FileSystem::OpenRead);
    Ref<afl::io::FileMapping> resultContent = result->createVirtualMapping();
    String_t output = util::normalizeLinefeeds(resultContent->get());
    String_t expect =
        "Sub BCO1\n"
        "  .name -\n"
        "    .line 1\n"
        "    pushint         5\n"
        "    sprint\n"
        "EndSub\n"
        "\n";
    a.checkEqual("expected output", output, expect);
}

/* Basic test, disassemble from file to file */
AFL_TEST("interpreter.ConsoleApplication:disassemble:file-to-file", a)
{
    InternalFileSystem fs;
    InternalEnvironment env;

    fs.openFile("/f.q", FileSystem::Create)
        ->fullWrite(afl::string::toBytes("print 5\n"));

    Ptr<InternalStream> out = new InternalStream();
    env.setChannelStream(Environment::Output, out);

    afl::data::StringList_t args;
    args.push_back("f.q");
    args.push_back("-S");
    args.push_back("-o");
    args.push_back("y.txt");
    env.setCommandLine(args);

    int ret = interpreter::ConsoleApplication(env, fs).run();
    a.checkEqual("expect success return", ret, 0);

    Ref<afl::io::Stream> result = fs.openFile("/y.txt", FileSystem::OpenRead);
    Ref<afl::io::FileMapping> resultContent = result->createVirtualMapping();
    String_t output = util::normalizeLinefeeds(resultContent->get());
    String_t expect =
        "Sub BCO1\n"
        "  .name -\n"
        "  .file f.q\n"
        "    .line 1\n"
        "    pushint         5\n"
        "    sprint\n"
        "EndSub\n"
        "\n";
    a.checkEqual("expected output", output, expect);
}

/* Basic test, disassemble from file to file, disabled debug info */
AFL_TEST("interpreter.ConsoleApplication:disassemble:file-to-file:nondebug", a)
{
    InternalFileSystem fs;
    InternalEnvironment env;

    fs.openFile("/f.q", FileSystem::Create)
        ->fullWrite(afl::string::toBytes("print 5\n"));

    Ptr<InternalStream> out = new InternalStream();
    env.setChannelStream(Environment::Output, out);

    afl::data::StringList_t args;
    args.push_back("f.q");
    args.push_back("-S");
    args.push_back("-o");
    args.push_back("y.txt");
    args.push_back("-s");
    env.setCommandLine(args);

    int ret = interpreter::ConsoleApplication(env, fs).run();
    a.checkEqual("expect success return", ret, 0);

    Ref<afl::io::Stream> result = fs.openFile("/y.txt", FileSystem::OpenRead);
    Ref<afl::io::FileMapping> resultContent = result->createVirtualMapping();
    String_t output = util::normalizeLinefeeds(resultContent->get());
    String_t expect =
        "Sub BCO1\n"
        "  .name -\n"
        "    pushint         5\n"
        "    sprint\n"
        "EndSub\n"
        "\n";
    a.checkEqual("expected output", output, expect);
}


/*
 *  Size
 */

/* Basic test */
AFL_TEST("interpreter.ConsoleApplication:size", a)
{
    InternalFileSystem fs;
    InternalEnvironment env;

    fs.openFile("/f.qc", FileSystem::Create)
        ->fullWrite(OBJECT_FILE);

    Ptr<InternalStream> out = new InternalStream();
    env.setChannelStream(Environment::Output, out);

    afl::data::StringList_t args;
    args.push_back("--size");
    args.push_back("f.qc");
    env.setCommandLine(args);

    int ret = interpreter::ConsoleApplication(env, fs).run();
    a.checkEqual("expect success return", ret, 0);

    String_t output = util::normalizeLinefeeds(out->getContent());
    String_t expect =
        "Content of f.qc:\n"
        "  Code    Literals    Total   Routine\n"
        "      12         6        49  NARF\n"         // constructor function (3 insns, pushlit/sinstance/sreturn)
        "       8         6        38  (entry)\n"      // main (2 insns, pushlit/sdefsub)
        "      20        12        87  -> Total\n"
        "30 bytes debug information\n"
        "6 bytes data\n";                              // structure definition
    a.checkEqual("expected output", output, expect);
}

/* Error case */
AFL_TEST("interpreter.ConsoleApplication:size:error:not-found", a)
{
    InternalFileSystem fs;
    InternalEnvironment env;

    Ptr<InternalStream> out = new InternalStream();
    env.setChannelStream(Environment::Output, out);

    Ptr<InternalStream> err = new InternalStream();
    env.setChannelStream(Environment::Error, err);

    afl::data::StringList_t args;
    args.push_back("--size");
    args.push_back("f.qc");
    env.setCommandLine(args);

    int ret = interpreter::ConsoleApplication(env, fs).run();
    a.checkDifferent("expect success return", ret, 0);
    a.checkDifferent("expect error output", err->getContent().size(), 0U);
    a.checkEqual("expect no standard output", out->getContent().size(), 0U);
}

/*
 *  Strip
 */

/* Default update-in-place mode */
AFL_TEST("interpreter.ConsoleApplication:strip", a)
{
    InternalFileSystem fs;
    InternalEnvironment env;

    fs.openFile("/f.qc", FileSystem::Create)
        ->fullWrite(OBJECT_FILE);

    afl::data::StringList_t args;
    args.push_back("--strip");
    args.push_back("f.qc");
    env.setCommandLine(args);

    int ret = interpreter::ConsoleApplication(env, fs).run();
    a.checkEqual("expect success return", ret, 0);

    Ref<afl::io::Stream> result = fs.openFile("/f.qc", FileSystem::OpenRead);
    Ref<afl::io::FileMapping> resultContent = result->createVirtualMapping();
    a.checkEqualContent("expect correct output", resultContent->get(), afl::base::ConstBytes_t(STRIPPED_FILE));
}

/* With output file name */
AFL_TEST("interpreter.ConsoleApplication:strip-o", a)
{
    InternalFileSystem fs;
    InternalEnvironment env;

    fs.openFile("/f.qc", FileSystem::Create)
        ->fullWrite(OBJECT_FILE);

    afl::data::StringList_t args;
    args.push_back("--strip");
    args.push_back("f.qc");
    args.push_back("-o");
    args.push_back("a.qc");
    env.setCommandLine(args);

    int ret = interpreter::ConsoleApplication(env, fs).run();
    a.checkEqual("expect success return", ret, 0);

    Ref<afl::io::Stream> orig = fs.openFile("/f.qc", FileSystem::OpenRead);
    Ref<afl::io::FileMapping> origContent = orig->createVirtualMapping();
    a.checkEqualContent("expect correct output", origContent->get(), afl::base::ConstBytes_t(OBJECT_FILE));

    Ref<afl::io::Stream> result = fs.openFile("/a.qc", FileSystem::OpenRead);
    Ref<afl::io::FileMapping> resultContent = result->createVirtualMapping();
    a.checkEqualContent("expect correct output", resultContent->get(), afl::base::ConstBytes_t(STRIPPED_FILE));
}

/* Error case: multiple inputs, one output */
AFL_TEST("interpreter.ConsoleApplication:error:strip-o-multi", a)
{
    InternalFileSystem fs;
    InternalEnvironment env;

    fs.openFile("/f.qc", FileSystem::Create)
        ->fullWrite(OBJECT_FILE);
    fs.openFile("/g.qc", FileSystem::Create)
        ->fullWrite(OBJECT_FILE);

    Ptr<InternalStream> out = new InternalStream();
    env.setChannelStream(Environment::Error, out);

    afl::data::StringList_t args;
    args.push_back("--strip");
    args.push_back("f.qc");
    args.push_back("g.qc");
    args.push_back("-o");
    args.push_back("a.qc");
    env.setCommandLine(args);

    int ret = interpreter::ConsoleApplication(env, fs).run();
    a.checkDifferent("expect error return", ret, 0);
    a.checkDifferent("expect nonempty error output", out->getContent().size(), 0U);
}

