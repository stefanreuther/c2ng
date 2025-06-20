/**
  *  \file test/game/browser/testapplettest.cpp
  *  \brief Test for game::browser::TestApplet
  */

#include "game/browser/testapplet.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/test/testrunner.hpp"
#include "util/io.hpp"
#include "afl/net/nullnetworkstack.hpp"
#include "game/test/files.hpp"

using afl::io::ConstMemoryStream;
using afl::io::FileSystem;
using afl::io::InternalFileSystem;
using afl::io::InternalStream;
using afl::sys::Environment;
using afl::sys::InternalEnvironment;
using afl::net::NullNetworkStack;

namespace {
    String_t runSequence(InternalFileSystem& fs, const char* seq)
    {
        NullNetworkStack net;
        InternalEnvironment env;
        afl::base::Ptr<InternalStream> out = new InternalStream();
        env.setChannelStream(Environment::Input, new ConstMemoryStream(afl::string::toBytes(seq)));
        env.setChannelStream(Environment::Output, out);
        env.setChannelStream(Environment::Error, out);
        env.setSettingsDirectoryName("/settings");
        env.setInstallationDirectoryName("/install");

        fs.createDirectory("/install");
        fs.createDirectory("/install/share");
        fs.createDirectory("/install/share/specs");
        fs.openFile("/install/share/specs/race.nm", FileSystem::Create)
            ->fullWrite(game::test::getDefaultRaceNames());

        afl::data::StringList_t args;
        args.push_back("app");
        env.setCommandLine(args);

        util::Applet::Runner("", env, fs)
            .addNew("app", "", new game::browser::TestApplet(net))
            .run();

        return util::normalizeLinefeeds(out->getContent());
    }
}

AFL_TEST("game.browser.TestApplet:ls-cd-pwd", a)
{
    InternalFileSystem fs;
    fs.createDirectory("/a");
    fs.createDirectory("/b");
    fs.createDirectory("/b/c");

    String_t out = runSequence(fs,
                               "ls\n"          // lists "My Computer"
                               "cd 0\n"
                               "ls\n"          // lists "Root Directory"
                               "cd 0\n"
                               "ls\n"          // lists "a", "b", "install"
                               "cd 1\n"
                               "ls\n"          // lists "c"
                               "pwd\n");

    // The output is intermixed with log messages.
    // Thus, don't check for entire result; check for substrings only.
    a.checkContains("01. ls",  out, "  0. My Computer\n<Root>>");
    a.checkContains("02. ls",  out, "  0. Root Directory\nMy Computer>");
    a.checkContains("03. ls",  out, "  0. a\n  1. b\n  2. install\nRoot Directory>");
    a.checkContains("04. ls",  out, "  0. c\nb>");
    a.checkContains("05. pwd", out, "  0. My Computer\n  1. Root Directory\n  2. b\nb>");
}

AFL_TEST("game.browser.TestApplet:open-info", a)
{
    InternalFileSystem fs;
    fs.createDirectory("/a");
    fs.createDirectory("/a/b");

    String_t out = runSequence(fs,
                               "open /a/b\n"
                               "ls\n"
                               "info\n");
    a.checkContains("01. info",  out, "No game.\nb>");
}

AFL_TEST("game.browser.TestApplet:open-info-game", a)
{
    InternalFileSystem fs;
    fs.createDirectory("/a");
    fs.createDirectory("/a/b");
    fs.openFile("/a/b/player7.rst", FileSystem::Create)
        ->fullWrite(game::test::getResultFile30());

    String_t out = runSequence(fs,
                               "open /a/b\n"
                               "ls\n"
                               "info\n");
    a.checkContains("01. info", out,
                    "Turn loader present.\n"
                    "Player 7, The Crystal People, available, playable, primary, RST\n"
                    "Unknown registration key.\n"
                    "Host version: unknown\n"
                    "b>");
}
