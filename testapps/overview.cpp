/**
  *  \file test_apps/overview.cpp
  */

#include <iostream>
#include "game/v3/directoryscanner.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/loglistener.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/directory.hpp"
#include "util/consolelogger.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/sys/environment.hpp"

namespace {
    String_t formatFlags(game::v3::DirectoryScanner::PlayerFlags_t flags)
    {
        using game::v3::DirectoryScanner;
        String_t result = "{";
        if (flags.contains(DirectoryScanner::HaveResult)) { result += " Result"; }
        if (flags.contains(DirectoryScanner::HaveTurn)) { result += " Turn"; }
        if (flags.contains(DirectoryScanner::HaveUnpacked)) { result += " Unpacked"; }
        if (flags.contains(DirectoryScanner::HaveNewResult)) { result += " NewResult"; }
        if (flags.contains(DirectoryScanner::HaveConflict)) { result += " Conflict"; }
        if (flags.contains(DirectoryScanner::HaveOtherResult)) { result += " OtherResult"; }
        result += " }";
        return result;
    }
}

int main(int /*argc*/, char** argv)
{
    afl::string::NullTranslator tx;
    util::ConsoleLogger logger;
    afl::charset::CodepageCharset charset(afl::charset::g_codepageLatin1);
    afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    afl::base::Ref<afl::io::Directory> specDir = fs.openDirectory(fs.makePathName(fs.makePathName(env.getInstallationDirectoryName(), "share"), "specs"));
    while (const char* dirName = *++argv) {
        game::v3::DirectoryScanner scanner(*specDir, tx, logger);
        try {
            afl::base::Ref<afl::io::Directory> dir = fs.openDirectory(dirName);
            scanner.scan(*dir, charset);
            std::cout << dirName << ":\n"
                      << "  directory flags = " << formatFlags(scanner.getDirectoryFlags()) << "\n"
                      << "  host version = " << scanner.getDirectoryHostVersion().toString(tx) << "\n";
            for (int i = 1; i <= scanner.NUM_PLAYERS; ++i) {
                if (!scanner.getPlayerFlags(i).empty()) {
                    std::cout << "  player " << i << ": " << formatFlags(scanner.getPlayerFlags(i)) << "\n";
                }
            }
        }
        catch (std::exception& e) {
            std::cout << dirName << ": " << e.what() << "\n";
        }
    }
}
