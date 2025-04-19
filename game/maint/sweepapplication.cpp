/**
  *  \file game/maint/sweepapplication.cpp
  */

#include "game/maint/sweepapplication.hpp"
#include "afl/base/optional.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "game/limits.hpp"
#include "game/maint/directorywrapper.hpp"
#include "game/maint/sweeper.hpp"
#include "game/playerset.hpp"
#include "util/string.hpp"
#include "version.hpp"

using afl::string::Format;

namespace {
    // Limit
    const int MAX_PLAYERS = game::MAX_PLAYERS;

    struct Parameters {
        afl::base::Optional<String_t> gameDir;
        afl::base::Optional<String_t> rootDir;
        bool opt_dryRun;
        bool opt_eraseDatabase;
        bool opt_verbose;
        game::PlayerSet_t selectedPlayers;

        Parameters()
            : gameDir(),
              rootDir(),
              opt_dryRun(false),
              opt_eraseDatabase(false),
              opt_verbose(false),
              selectedPlayers()
            { }
    };
}

void
game::maint::SweepApplication::appMain()
{
    afl::string::Translator& tx = translator();
    Parameters params;

    // Parser
    afl::sys::StandardCommandLineParser commandLine(environment().getCommandLine());
    String_t p;
    bool opt;
    while (commandLine.getNext(opt, p)) {
        if (opt) {
            if (p == "n") {
                params.opt_dryRun = true;
            } else if (p == "l") {
                params.opt_eraseDatabase = true;
            } else if (p == "x") {
                params.opt_verbose = true;
            } else if (p == "a") {
                params.selectedPlayers = game::PlayerSet_t::allUpTo(MAX_PLAYERS);
            } else if (p == "h" || p == "help") {
                help(standardOutput());
            } else {
                errorExit(Format(tx("invalid option specified. Use \"%s -h\" for help").c_str(), environment().getInvocationName()));
            }
        } else {
            int n;
            if (afl::string::strToInteger(p, n) && n > 0 && n <= MAX_PLAYERS) {
                params.selectedPlayers += n;
            } else if (!params.gameDir.isValid()) {
                params.gameDir = p;
            } else if (!params.rootDir.isValid()) {
                // PCC2 accepts a rootDir parameter because it needs it for framework initialisation.
                // We do not need it; accept it anyway for compatibility.
                params.rootDir = p;
            } else {
                errorExit(tx("too many arguments"));
            }
        }
    }

    if (params.selectedPlayers.empty()) {
        errorExit(Format(tx("No player number specified. Use \"%s -h\" for help").c_str(), environment().getInvocationName()));
    }

    // Set up a directory
    afl::base::Ref<afl::io::Directory> dir = fileSystem().openDirectory(params.gameDir.orElse("."));
    afl::base::Ref<DirectoryWrapper> wrap = DirectoryWrapper::create(dir, standardOutput(), translator());
    if (params.opt_dryRun) {
        wrap->setWriteMode(DirectoryWrapper::IgnoreWrites);
        wrap->setEraseMode(DirectoryWrapper::IgnoreAndLogErase);
    } else if (params.opt_verbose) {
        wrap->setWriteMode(DirectoryWrapper::PassThroughWrites);
        wrap->setEraseMode(DirectoryWrapper::LogErase);
    } else {
        wrap->setWriteMode(DirectoryWrapper::PassThroughWrites);
        wrap->setEraseMode(DirectoryWrapper::PassThroughErase);
    }

    // Set up a sweeper
    Sweeper sweeper;
    sweeper.setEraseDatabase(params.opt_eraseDatabase);
    sweeper.setPlayers(params.selectedPlayers);
    sweeper.execute(*wrap);

    // Final words
    if (!params.opt_dryRun) {
        int numPlayers = 0;
        for (int i = 1; i <= MAX_PLAYERS; ++i) {
            if (sweeper.getRemainingPlayers().contains(i)) {
                ++numPlayers;
            }
        }

        standardOutput().writeLine(Format(tx("%d file%!1{s%} removed.").c_str(), wrap->getNumRemovedFiles()));
        if (numPlayers == 0) {
            standardOutput().writeLine(tx("No player data remains."));
        } else {
            standardOutput().writeLine(Format(tx("%d player%1{'s%|s'%} data remains.").c_str(), numPlayers));
        }
    }
}

void
game::maint::SweepApplication::help(afl::io::TextWriter& out)
{
    afl::string::Translator& tx = translator();
    out.writeLine(Format(tx("PCC2 Game Directory Cleaner v%s - (c) 2010-2025 Stefan Reuther").c_str(), PCC2_VERSION));
    out.writeLine();
    out.writeLine(Format(tx("Usage:\n"
                            "  %s [-h]\n"
                            "  %$0s [-nlx] [-a|player numbers] [gamedir [rootdir]]\n\n"
                            "%s"
                            "\n"
                            "Report bugs to <Streu@gmx.de>").c_str(), environment().getInvocationName(),
                         util::formatOptions(tx("Options:\n"
                                                "-n\tDry run, list files that would be deleted\n"
                                                "-l\tAlso erase database/log files usually kept several turns\n"
                                                "-a\tErase all players' files\n"
                                                "-x\tIncrease verbosity\n"))));
    exit(0);
}
