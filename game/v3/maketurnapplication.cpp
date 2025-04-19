/**
  *  \file game/v3/maketurnapplication.cpp
  *  \brief Class game::v3::MaketurnApplication
  */

#include "game/v3/maketurnapplication.hpp"
#include "afl/base/optional.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/multidirectory.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "game/playerlist.hpp"
#include "game/v3/directoryscanner.hpp"
#include "game/v3/maketurn.hpp"
#include "game/v3/utils.hpp"
#include "util/charsetfactory.hpp"
#include "util/string.hpp"
#include "version.hpp"

using afl::base::Ref;
using afl::base::Optional;
using afl::io::Directory;
using afl::io::FileSystem;

namespace {
    Ref<Directory> openGameDirectory(FileSystem& fs, const Optional<String_t>& value)
    {
        const String_t* p = value.get();
        return fs.openDirectory(p != 0 ? *p : fs.getWorkingDirectoryName());
    }

    Ref<Directory> openSpecDirectory(FileSystem& fs, const Optional<String_t>& value, const Ref<Directory>& gameDir, afl::sys::Environment& env)
    {
        const String_t* p = value.get();
        Ref<Directory> specDir = fs.openDirectory(p != 0 ? *p : fs.makePathName(fs.makePathName(env.getInstallationDirectoryName(), "share"), "specs"));

        Ref<afl::io::MultiDirectory> result(afl::io::MultiDirectory::create());
        result->addDirectory(gameDir);
        result->addDirectory(specDir);
        return result;
    }
}

game::v3::MaketurnApplication::MaketurnApplication(afl::sys::Environment& env, afl::io::FileSystem& fs)
    : Application(env, fs)
{
    consoleLogger().setConfiguration("*=raw", translator());
}

void
game::v3::MaketurnApplication::appMain()
{
    afl::base::Optional<String_t> gameDir;
    afl::base::Optional<String_t> rootDir;
    bool optForce = false;

    afl::sys::StandardCommandLineParser parser(environment().getCommandLine());
    afl::string::Translator& tx = translator();
    String_t text;
    bool isOption;
    while (parser.getNext(isOption, text)) {
        if (isOption) {
            if (text == "h" || text == "help") {
                help();
            } else if (text == "f") {
                optForce = true;
            } else if (text == "log") {
                consoleLogger().setConfiguration(parser.getRequiredParameter("log"), tx);
            } else {
                errorExit(afl::string::Format(tx("invalid option specified. Use \"%s -h\" for help"), environment().getInvocationName()));
            }
        } else if (!gameDir.isValid()) {
            gameDir = text;
        } else if (!rootDir.isValid()) {
            rootDir = text;
        } else {
            errorExit(tx("too many arguments"));
        }
    }

    // Set up directories
    Ref<Directory> gameDirObj = openGameDirectory(fileSystem(), gameDir);
    Ref<Directory> specDirObj = openSpecDirectory(fileSystem(), rootDir, gameDirObj, environment());

    // Configuration
    game::config::UserConfiguration config;
    config.loadGameConfiguration(*gameDirObj, log(), translator());

    // Character set
    std::auto_ptr<afl::charset::Charset> charset(util::CharsetFactory().createCharset(config[config.Game_Charset]()));
    if (charset.get() == 0) {
        charset.reset(new afl::charset::CodepageCharset(afl::charset::g_codepageLatin1));
    }

    // Check
    DirectoryScanner scanner(*specDirObj, translator(), log());
    scanner.scan(*gameDirObj, *charset, DirectoryScanner::UnpackedOnly);

    if (!scanner.getDirectoryFlags().contains(DirectoryScanner::HaveUnpacked)) {
        errorExit(afl::string::Format(tx("directory '%s' does not contain unpacked game data"), gameDirObj->getDirectoryName()));
    }
    if (scanner.getDirectoryFlags().contains(DirectoryScanner::HaveConflict) && !optForce) {
        errorExit(afl::string::Format(tx("directory '%s' contains data from different games.\n"
                                         "NOTE: use '-f' to force compilation of turn files anyway"), gameDirObj->getDirectoryName()));
    }

    // Race names (needed for log messages and multi-player messages)
    PlayerList players;
    loadRaceNames(players, *specDirObj, *charset);

    // Maketurn
    Maketurn theMaketurn(*gameDirObj, players, *charset, translator());
    for (int pl = 1; pl <= DirectoryScanner::NUM_PLAYERS; ++pl) {
        if (scanner.getPlayerFlags(pl).contains(DirectoryScanner::HaveUnpacked)) {
            // @change PCC2 would write some entertaining message here; we have that in saveAll()
            // size_t numCommands =
            theMaketurn.makeTurn(pl, log());
            // standardOutput().writeLine(afl::string::Format(tx("%s: %d command%!1{s%}"), players.getPlayerName(pl, Player::ShortName), numCommands));
        }
    }

    // Write them out
    // @change PCC2 would write some entertaining message here; we have that in saveAll()
    // standardOutput().writeLine(afl::string::Format(tx("Writing %d turn file%!1{s%}..."), theMaketurn.getNumFiles()));
    theMaketurn.saveAll(log(), fileSystem(), config);
}

void
game::v3::MaketurnApplication::help()
{
    afl::io::TextWriter& out = standardOutput();
    afl::string::Translator& tx = translator();
    out.writeLine(afl::string::Format(tx("PCC2 Turn File Compiler v%s - (c) 2010-2025 Stefan Reuther"), PCC2_VERSION));
    out.writeLine();
    out.writeLine(afl::string::Format(tx("Usage:\n"
                                         "  %s [-h]\n"
                                         "  %$0s [-f] [GAMEDIR]\n\n"
                                         "%s\n"
                                         "Report bugs to <Streu@gmx.de>").c_str(),
                                      environment().getInvocationName(),
                                      util::formatOptions(tx("Options:\n"
                                                             "-f\tForce operation even on file conflicts\n"
                                                             "--log=CONFIG\tSet logger configuration\n"))));
    exit(0);
}
