/**
  *  \file test/game/v3/scannerapplettest.cpp
  *  \brief Test for game::v3::ScannerApplet
  */

#include "game/v3/scannerapplet.hpp"

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

AFL_TEST("game.v3.ScannerApplet", a)
{
    InternalEnvironment env;
    InternalFileSystem fs;

    afl::base::Ptr<InternalStream> out = new InternalStream();
    env.setChannelStream(Environment::Output, out);
    env.setChannelStream(Environment::Error, out);
    env.setInstallationDirectoryName("/install");

    fs.createDirectory("/install");
    fs.createDirectory("/install/share");
    fs.createDirectory("/install/share/specs");
    fs.openFile("/install/share/specs/race.nm", FileSystem::Create)->fullWrite(game::test::getDefaultRaceNames());
    fs.openFile("/install/share/specs/hostver.ini", FileSystem::Create)
        ->fullWrite(afl::string::toBytes(
                        "config,PHost Version Message\n"
                        "  kind   = h\n"
                        "  check  = HUL=\n"
                        "  check  = PXY=\n"
                        "  parse  = =1,PHost $\n"
                        "  assign = HostVersion\n"
                        "  value  = PHost\n"
                        "  assign = HostType\n"));

    fs.createDirectory("/game");
    fs.openFile("/game/player7.rst", FileSystem::Create)->fullWrite(game::test::getResultFile30());

    afl::data::StringList_t args;
    args.push_back("app");
    args.push_back("/game");
    env.setCommandLine(args);

    util::Applet::Runner("", env, fs)
        .addNew("app", "", new game::v3::ScannerApplet())
        .run();

    String_t output = util::normalizeLinefeeds(out->getContent());
    a.checkEqual("", output,
                 "/game:\n"
                 "  directory flags = { Result }\n"
                 "  host version = PHost 4.1h\n"
                 "  player 7: { Result }\n");
}

