/**
  *  \file test/game/parser/testapplettest.cpp
  *  \brief Test for game::parser::TestApplet
  */

#include "game/parser/testapplet.hpp"

#include "afl/io/internalfilesystem.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/test/testrunner.hpp"
#include "util/io.hpp"

using afl::io::FileSystem;
using afl::io::InternalFileSystem;
using afl::io::InternalStream;
using afl::sys::Environment;
using afl::sys::InternalEnvironment;

AFL_TEST("game.parser.TestApplet", a)
{
    const char* INI =
        "config,GroundKillFactor\n"
        "  kind     = g\n"
        "  check    = Ground Attack Kill Ratio\n"
        "  array    = +1,$ $ : 1\n"
        "  assign   = Index:Race.Adj, GroundKillFactor\n"
        "  continue = y\n"
        "\n"
        "config,ScanRange\n"
        "  kind     = g\n"
        "  parse    = Ships are visible at $\n"
        "  assign   = ScanRange\n"
        "  continue = y\n"
        "config,AllowWebMines\n"
        "  kind     = g\n"
        "  parse    = Web mines $\n"
        "  assign   = AllowWebMines\n"
        "  continue = y\n"
        "explosion,THost\n"
        "  kind   = x\n"
        "  parse  = ($,$)\n"
        "  assign = X, Y\n"
        "  check  = The name of the ship\n"
        "  parse  = +1,$\n"
        "  assign = Name";

    const char* MSG =
        "--- Message ---\n"
        "(-g0000)< Message from your Host >\n"
        "Ground Attack Kill Ratio\n"
        "  Fed          1  : 1\n"
        "  Lizard       20 : 1\n"
        "  Bird         1  : 1\n"
        "  Fascist      10 : 1\n"
        "  Crystal      1  : 1\n"
        "Ships are visible at  300\n"
        "--- Message ---\n"
        "(-g0000)< Message from your Host >\n"
        "Web mines  YES\n"
        "--- Message ---\n"
        "TURN: 33\n"
        "(-x0005)<< Long Range Sensors >>\n"
        "Distress call and explosion\n"
        "detected from a starship at:\n"
        "( 1930 , 2728 )\n"
        "The name of the ship was the: \n"
        "C.S.S. War03\n";

    InternalEnvironment env;
    InternalFileSystem fs;

    afl::base::Ptr<InternalStream> out = new InternalStream();
    env.setChannelStream(Environment::Output, out);
    env.setChannelStream(Environment::Error, out);

    fs.openFile("/p.ini", FileSystem::Create)->fullWrite(afl::string::toBytes(INI));
    fs.openFile("/m.txt", FileSystem::Create)->fullWrite(afl::string::toBytes(MSG));

    afl::data::StringList_t args;
    args.push_back("app");
    args.push_back("-load=/p.ini");
    args.push_back("/m.txt");
    env.setCommandLine(args);

    util::Applet::Runner("", env, fs)
        .addNew("app", "", new game::parser::TestApplet())
        .run();

    String_t output = util::normalizeLinefeeds(out->getContent());
    a.checkEqual("", output,
                 "--- Parsed Message:\n"
                 "(-g0000)< Message from your Host >\n"
                 "Ground Attack Kill Ratio\n"
                 "  Fed          1  : 1\n"
                 "  Lizard       20 : 1\n"
                 "  Bird         1  : 1\n"
                 "  Fascist      10 : 1\n"
                 "  Crystal      1  : 1\n"
                 "Ships are visible at  300\n"
                 "| Configuration #0, turn 1\n"
                 "|    Config: GROUNDKILLFACTOR = 1,20,,10,,,,,,,\n"
                 "|    Config: SCANRANGE = 300\n"
                 "--- Parsed Message:\n"
                 "(-g0000)< Message from your Host >\n"
                 "Web mines  YES\n"
                 "| Configuration #0, turn 1\n"
                 "|    Config: ALLOWWEBMINES = YES\n"
                 "--- Parsed Message:\n"
                 "(-x0005)<< Long Range Sensors >>\n"
                 "Distress call and explosion\n"
                 "detected from a starship at:\n"
                 "( 1930 , 2728 )\n"
                 "The name of the ship was the: \n"
                 "C.S.S. War03\n"
                 "| Explosion #0, turn 33\n"
                 "|    X: 1930\n"
                 "|    Y: 2728\n"
                 "|    Name: C.S.S. War03\n");
}
