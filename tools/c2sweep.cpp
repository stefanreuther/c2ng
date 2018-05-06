/**
  *  \file tools/c2sweep.cpp
  *  \brief "Sweep" utility - Main Function
  */

#include "afl/base/optional.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/textwriter.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/sys/environment.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "game/limits.hpp"
#include "game/maint/directorywrapper.hpp"
#include "game/maint/sweeper.hpp"
#include "game/playerset.hpp"
#include "util/application.hpp"
#include "util/translation.hpp"
#include "version.hpp"

using game::maint::DirectoryWrapper;

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

    class ConsoleSweepApplication : public util::Application {
     public:
        ConsoleSweepApplication(afl::sys::Environment& env, afl::io::FileSystem& fs)
            : Application(env, fs)
            { }

        void appMain();

     private:
        void help(afl::io::TextWriter& out);
    };
}

void
ConsoleSweepApplication::appMain()
{
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
                errorExit(afl::string::Format(_("invalid option specified. Use \"%s -h\" for help").c_str(), environment().getInvocationName()));
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
                errorExit(_("too many arguments"));
            }
        }
    }

    if (params.selectedPlayers.empty()) {
        errorExit(afl::string::Format(_("No player number specified. Use \"%s -h\" for help").c_str(), environment().getInvocationName()));
    }

    // Set up a directory
    // FIXME: pedantery: "orElse(".")" violates the FileSystem abstraction
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
    game::maint::Sweeper sweeper;
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
            
        standardOutput().writeLine(afl::string::Format(_("%d file%!1{s%} removed.").c_str(), wrap->getNumRemovedFiles()));
        if (numPlayers == 0) {
            standardOutput().writeLine(_("No player data remains."));
        } else {
            standardOutput().writeLine(afl::string::Format(_("%d player%1{'s%|s'%} data remains.").c_str(), numPlayers));
        }
    }
}

void
ConsoleSweepApplication::help(afl::io::TextWriter& out)
{
    out.writeLine(afl::string::Format(_("PCC2 Game Directory Cleaner v%s - (c) 2010-2018 Stefan Reuther").c_str(), PCC2_VERSION));
    out.writeLine();
    out.writeLine(afl::string::Format(_("Usage:\n"
                                        "  %s [-h]\n"
                                        "  %$0s [-nlx] [-a|player numbers] [gamedir [rootdir]]\n\n"
                                        "Options:\n"
                                        "  -n       Dry run, list files that would be deleted\n"
                                        "  -l       Also erase database/log files usually kept several turns\n"
                                        "  -a       Erase all players' files\n"
                                        "  -x       Increase verbosity\n"
                                        "\n"
                                        "Report bugs to <Streu@gmx.de>").c_str(), environment().getInvocationName()));
    exit(0);
}

int main(int /*argc*/, char** argv)
{
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    return ConsoleSweepApplication(env, fs).run();
}
