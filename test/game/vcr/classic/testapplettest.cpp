/**
  *  \file test/game/vcr/classic/testapplettest.cpp
  *  \brief Test for game::vcr::classic::TestApplet
  */

#include "game/vcr/classic/testapplet.hpp"

#include "afl/io/internalfilesystem.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/files.hpp"
#include "util/io.hpp"

using afl::io::FileSystem;
using afl::io::InternalFileSystem;
using afl::io::InternalStream;
using afl::sys::Environment;
using afl::sys::InternalEnvironment;

namespace {
    const uint8_t VCR[] = {
        0x01, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x96, 0x00, 0xe9, 0x00, 0x4b, 0x6f,
        0x74, 0x53, 0x43, 0x48, 0x61, 0x20, 0x50, 0x6f, 0x58, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x00, 0x00, 0x02, 0x00, 0x0e, 0x00, 0x02, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x53, 0x44, 0x52, 0x20, 0x44, 0x61, 0x75, 0x74,
        0x68, 0x69, 0x20, 0x53, 0x68, 0x61, 0x64, 0x6f, 0x77, 0x20, 0x20, 0x20, 0x00, 0x00, 0xf0, 0x00,
        0xb2, 0x01, 0x03, 0x00, 0x3d, 0x00, 0x05, 0x00, 0x06, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
        0x04, 0x00, 0x64, 0x00, 0x64, 0x00
    };

    void prepareFileSystem(InternalFileSystem& fs)
    {
        fs.createDirectory("/install");
        fs.createDirectory("/install/share");
        fs.createDirectory("/install/share/specs");
        fs.openFile("/install/share/specs/race.nm", FileSystem::Create)->fullWrite(game::test::getDefaultRaceNames());
        fs.openFile("/install/share/specs/torpspec.dat", FileSystem::Create)->fullWrite(game::test::getDefaultTorpedoes());
        fs.openFile("/install/share/specs/engspec.dat",  FileSystem::Create)->fullWrite(game::test::getDefaultEngines());
        fs.openFile("/install/share/specs/hullspec.dat", FileSystem::Create)->fullWrite(game::test::getDefaultHulls());
        fs.openFile("/install/share/specs/truehull.dat", FileSystem::Create)->fullWrite(game::test::getDefaultHullAssignments());

        fs.createDirectory("/game");
        fs.openFile("/game/test.vcr", FileSystem::Create)->fullWrite(VCR);
        fs.openFile("/game/beamspec.dat", FileSystem::Create)->fullWrite(game::test::getDefaultBeams());  // in game directory instead of default, to exercise spec file lookup
    }
}

AFL_TEST("game.vcr.classic.TestApplet", a)
{
    InternalEnvironment env;
    InternalFileSystem fs;

    afl::base::Ptr<InternalStream> out = new InternalStream();
    env.setChannelStream(Environment::Output, out);
    env.setChannelStream(Environment::Error, out);
    env.setInstallationDirectoryName("/install");
    prepareFileSystem(fs);

    afl::data::StringList_t args;
    args.push_back("app");
    args.push_back("/game/test.vcr"); // file name parameter
    args.push_back("/game");          // directory parameter
    env.setCommandLine(args);

    util::Applet::Runner("", env, fs)
        .addNew("app", "", new game::vcr::classic::TestApplet())
        .run();

    String_t output = util::normalizeLinefeeds(out->getContent());

    // Output contains spurious log messages, so check for "contains"
    a.checkContains("", output,
                    "VCR file contains 1 entries\n"
                    "--- Starting Playback ---\n"
                    "Record #1:\n"
                    "\tEnding time 193 (3:13)\n"
                    "\tleft-captured\n"
                    "  S:  0  D:  9  C:  0  A:  0   |     S:100  D:  0  C:240  A:  0\n");
}

AFL_TEST("game.vcr.classic.TestApplet:help", a)
{
    InternalEnvironment env;
    InternalFileSystem fs;

    afl::base::Ptr<InternalStream> out = new InternalStream();
    env.setChannelStream(Environment::Output, out);
    env.setChannelStream(Environment::Error, out);
    env.setInstallationDirectoryName("/install");
    prepareFileSystem(fs);

    afl::data::StringList_t args;
    args.push_back("app");
    env.setCommandLine(args);

    util::Applet::Runner("", env, fs)
        .addNew("app", "", new game::vcr::classic::TestApplet())
        .run();

    String_t output = util::normalizeLinefeeds(out->getContent());
    a.checkContains("", output, "Usage:");
}
