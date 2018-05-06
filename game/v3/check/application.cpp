/**
  *  \file game/v3/check/application.cpp
  *  \brief Class game::v3::check::Application
  */

#include "game/v3/check/application.hpp"
#include "afl/base/optional.hpp"
#include "afl/except/filetooshortexception.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/stream.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "game/v3/check/checker.hpp"
#include "game/v3/check/configuration.hpp"
#include "util/string.hpp"
#include "version.hpp"

using afl::base::Optional;
using afl::string::Format;

game::v3::check::Application::Application(afl::sys::Environment& env, afl::io::FileSystem& fs)
    : util::Application(env, fs)
{ }

void
game::v3::check::Application::appMain()
{
    // Command-line parser
    int player = 0;
    Optional<String_t> gamedir;
    Optional<String_t> rootdir;
    Configuration config;

    // ex check.pas:ParseArgs
    afl::sys::StandardCommandLineParser parser(environment().getCommandLine());
    bool opt;
    String_t text;
    while (parser.getNext(opt, text)) {
        if (opt) {
            if (text == "r") {
                config.setResultMode(true);
            } else if (text == "H") {
                config.setHtmlMode(true);
            } else if (text == "c") {
                config.setChecksumsMode(true);
            } else if (text == "p") {
                config.setPickyMode(true);
            } else if (text == "z") {
                config.setHandleMinus1Special(true);
            } else if (text == "h" || text == "help") {
                help();
            } else {
                // FIXME: text...
                errorExit(Format("Invalid option \"%s\"", text));
            }
        } else {
            int n;
            if (player == 0 && afl::string::strToInteger(text, n) && n > 0 && n <= 11) {
                player = n;
            } else if (!gamedir.isValid()) {
                gamedir = text;
            } else if (!rootdir.isValid()) {
                rootdir = text;
            } else {
                // FIXME: text...
                errorExit("Command line syntax error");
            }
        }
    }

    // Validate
    if (player == 0) {
        errorExit("Missing player number");
    }

    // Prepare environment
    afl::io::FileSystem& fs = fileSystem();
    afl::sys::Environment& env = environment();
    if (!gamedir.isValid()) {
        gamedir = ".";
    }
    if (!rootdir.isValid()) {
        rootdir = fs.makePathName(fs.makePathName(env.getInstallationDirectoryName(), "share"), "specs");
    }
    afl::base::Ref<afl::io::Directory> gamedirObj = fs.openDirectory(*gamedir.get());
    afl::base::Ref<afl::io::Directory> rootdirObj = fs.openDirectory(*rootdir.get());

    afl::base::Ref<afl::io::Stream> logFile = gamedirObj->openFile(config.isHtmlMode() ? "check.htm" : "check.log", fs.Create);
    afl::io::TextFile log(*logFile);

    // Prepare checker
    Checker checker(*gamedirObj, *rootdirObj, player, log, standardOutput(), errorOutput());
    checker.config() = config;

    // Operate!
    checker.run();

    // Log
    if (checker.hadAnyError()) {
        exit(2);
    }
}

void
game::v3::check::Application::help()
{
    afl::io::TextWriter& out = standardOutput();
    out.writeLine(Format("Turn Checker v%s - (c) 2005-2018 Stefan Reuther", PCC2_VERSION));
    out.writeLine();
    out.writeLine(Format("Usage:\n"
                         "  %s -h\n"
                         "  %0$s [-rHcpz] PLAYER [GAMEDIR [ROOTDIR]]\n\n"
                         "%s\n"
                         "Report bugs to <Streu@gmx.de>",
                         environment().getInvocationName(),
                         util::formatOptions("Parameters:\n"
                                             "PLAYER\tplayer number, 1..11\n"
                                             "GAMEDIR\tgame directory, defaults to current directory\n"
                                             "ROOTDIR\troot directory, defaults to builtin defaults\n"
                                             "\n"
                                             "Options:\n"
                                             "-h\tHelp\n"
                                             "-r\tCheck result + turn file. Default: validate unpacked\n"
                                             "-H\tWrite log file in HTML format (check.htm). Default: text file (check.log)\n"
                                             "-c\tValidate checksums.\n"
                                             "-p\tBe extra picky.\n"
                                             "-z\tDo not warn about '-1' values\n")));
    exit(0);
}
