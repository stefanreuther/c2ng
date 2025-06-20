/**
  *  \file game/v3/scannerapplet.cpp
  *  \brief Class game::v3::ScannerApplet
  */

#include <stdexcept>

#include "game/v3/scannerapplet.hpp"

#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/environment.hpp"
#include "game/v3/directoryscanner.hpp"

using afl::base::Ref;
using afl::charset::CodepageCharset;
using afl::io::Directory;
using afl::io::FileSystem;
using afl::io::TextWriter;
using afl::string::Format;
using afl::sys::Environment;
using game::v3::DirectoryScanner;

namespace {
    String_t formatFlags(DirectoryScanner::PlayerFlags_t flags)
    {
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

int
game::v3::ScannerApplet::run(util::Application& app, afl::sys::Environment::CommandLine_t& cmdl)
{
    CodepageCharset charset(afl::charset::g_codepageLatin1);
    Environment& env = app.environment();
    FileSystem& fs = app.fileSystem();
    TextWriter& out = app.standardOutput();
    Ref<Directory> specDir = fs.openDirectory(fs.makePathName(fs.makePathName(env.getInstallationDirectoryName(), "share"), "specs"));

    String_t dirName;
    while (cmdl.getNextElement(dirName)) {
        DirectoryScanner scanner(*specDir, app.translator(), app.log());
        try {
            Ref<Directory> dir = fs.openDirectory(dirName);
            scanner.scan(*dir, charset, DirectoryScanner::UnpackedThenResult);
            out.writeLine(dirName + ":");
            out.writeLine("  directory flags = " + formatFlags(scanner.getDirectoryFlags()));
            out.writeLine("  host version = " + scanner.getDirectoryHostVersion().toString());
            for (int i = 1; i <= DirectoryScanner::NUM_PLAYERS; ++i) {
                if (!scanner.getPlayerFlags(i).empty()) {
                    out.writeLine(Format("  player %d: %s", i, formatFlags(scanner.getPlayerFlags(i))));
                }
            }
        }
        catch (std::exception& e) {
            out.writeLine(Format("%s: %s", dirName, e.what()));
        }
    }
    return 0;
}
