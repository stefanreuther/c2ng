/**
  *  \file test/game/maint/dump/applicationtest.cpp
  *  \brief Test for game::maint::dump::Application
  */

#include "game/maint/dump/application.hpp"

#include "afl/test/testrunner.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "game/test/files.hpp"
#include "util/io.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::io::FileSystem;
using afl::io::InternalFileSystem;
using afl::io::InternalStream;
using afl::sys::Environment;
using afl::sys::InternalEnvironment;

/* Baseline: dump a file */
AFL_TEST("game.maint.dump.Application", a)
{
    InternalFileSystem fs;
    InternalEnvironment env;

    fs.openFile("/planet.nm", FileSystem::Create)
        ->fullWrite(game::test::getDefaultPlanetNames().subrange(0, 40));

    afl::data::StringList_t args;
    args.push_back("/planet.nm");
    env.setCommandLine(args);

    Ptr<InternalStream> out = new InternalStream();
    env.setChannelStream(Environment::Output, out);

    int ret = game::maint::dump::Application(env, fs).run();
    a.checkEqual("expect success return", ret, 0);
    a.checkEqual("expect correct output",
                 util::normalizeLinefeeds(out->getContent()),
                 "=== Dump of file '/planet.nm' using type 'names' ===\n"
                 "Names:\n"
                 "  Name(1)                        = 'Ceti Alpha one'\n"
                 "  Name(2)                        = 'Orionis I'\n"
                 "\n");
}

/* Parameterless invocation */
AFL_TEST("game.maint.dump.Application:no-args", a)
{
    InternalFileSystem fs;
    InternalEnvironment env;

    Ptr<InternalStream> out = new InternalStream();
    env.setChannelStream(Environment::Error, out);

    int ret = game::maint::dump::Application(env, fs).run();
    a.checkDifferent("expect error return", ret, 0);
    a.checkDifferent("expect nonempty output", out->getContent().size(), 0U);
}

/* Invoke help screen */
AFL_TEST("game.maint.dump.Application:help", a)
{
    InternalFileSystem fs;
    InternalEnvironment env;

    Ptr<InternalStream> out = new InternalStream();
    env.setChannelStream(Environment::Output, out);

    afl::data::StringList_t args;
    args.push_back("--help");
    env.setCommandLine(args);

    int ret = game::maint::dump::Application(env, fs).run();
    a.checkEqual("expect error return", ret, 0);
    a.checkDifferent("expect nonempty output", out->getContent().size(), 0U);
}

/* Dump multiple files, including missing ones */
AFL_TEST("game.maint.dump.Application:multiple", a)
{
    InternalFileSystem fs;
    InternalEnvironment env;

    fs.openFile("/planet.nm", FileSystem::Create)
        ->fullWrite(game::test::getDefaultPlanetNames().subrange(0, 40));
    fs.openFile("/storm.nm", FileSystem::Create)
        ->fullWrite(game::test::getDefaultIonStormNames().subrange(0, 40));

    afl::data::StringList_t args;
    args.push_back("/planet.nm");
    args.push_back("/missing.nm");  // does not exist, unknown type
    args.push_back("/ship8.dat");   // does not exist, known type
    args.push_back("/storm.nm");
    env.setCommandLine(args);

    Ptr<InternalStream> out = new InternalStream();
    env.setChannelStream(Environment::Output, out);

    Ptr<InternalStream> err = new InternalStream();
    env.setChannelStream(Environment::Error, err);

    int ret = game::maint::dump::Application(env, fs).run();
    a.checkDifferent("expect error return", ret, 0);
    a.checkEqual("expect correct output",
                 util::normalizeLinefeeds(out->getContent()),
                 "=== Dump of file '/planet.nm' using type 'names' ===\n"
                 "Names:\n"
                 "  Name(1)                        = 'Ceti Alpha one'\n"
                 "  Name(2)                        = 'Orionis I'\n"
                 "\n"
                 "=== Dump of file '/storm.nm' using type 'names' ===\n"
                 "Names:\n"
                 "  Name(1)                        = 'Hillery'\n"
                 "  Name(2)                        = 'Leah'\n"
                 "\n");
    a.checkDifferent("expect error output", err->getContent().size(), 0U);
}

/* Explicitly specified type */
AFL_TEST("game.maint.dump.Application:type-specified", a)
{
    InternalFileSystem fs;
    InternalEnvironment env;

    fs.openFile("/ship7.dat", FileSystem::Create)
        ->fullWrite(game::test::getDefaultPlanetNames().subrange(0, 40));

    afl::data::StringList_t args;
    args.push_back("-t");
    args.push_back("names");
    args.push_back("/ship7.dat");
    env.setCommandLine(args);

    Ptr<InternalStream> out = new InternalStream();
    env.setChannelStream(Environment::Output, out);

    int ret = game::maint::dump::Application(env, fs).run();
    a.checkEqual("expect success return", ret, 0);
    a.checkEqual("expect correct output",
                 util::normalizeLinefeeds(out->getContent()),
                 "=== Dump of file '/ship7.dat' using type 'names' ===\n"
                 "Names:\n"
                 "  Name(1)                        = 'Ceti Alpha one'\n"
                 "  Name(2)                        = 'Orionis I'\n"
                 "\n");
}
