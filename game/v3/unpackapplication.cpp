/**
  *  \file game/v3/unpackapplication.cpp
  *  \brief Class game::v3::UnpackApplication
  */

#include "game/v3/unpackapplication.hpp"
#include "afl/base/optional.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/multidirectory.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "afl/sys/standardcommandlineparser.hpp"
#include "game/playerset.hpp"
#include "game/v3/attachmentunpacker.hpp"
#include "game/v3/genfile.hpp"
#include "game/v3/resultfile.hpp"
#include "game/v3/turnfile.hpp"
#include "game/v3/unpacker.hpp"
#include "util/string.hpp"
#include "version.hpp"

using afl::base::Ref;
using afl::base::Optional;
using afl::io::Directory;
using afl::io::FileSystem;
using afl::string::Format;

namespace {
    const char*const LOG_NAME = "game.v3.unpack";

    Ref<Directory> openSpecDirectory(FileSystem& fs, const Optional<String_t>& value, afl::sys::Environment& env)
    {
        const String_t* p = value.get();
        return fs.openDirectory(p != 0 ? *p : fs.makePathName(fs.makePathName(env.getInstallationDirectoryName(), "share"), "specs"));
    }
}

game::v3::UnpackApplication::UnpackApplication(afl::sys::Environment& env, afl::io::FileSystem& fs)
    : Application(env, fs)
{
    consoleLogger().setConfiguration("*=raw");
}

void
game::v3::UnpackApplication::appMain()
{
    Ref<afl::io::MultiDirectory> specDir(afl::io::MultiDirectory::create());
    Unpacker theUnpacker(translator(), *specDir);
    theUnpacker.log().addListener(log());

    AttachmentUnpacker detacher;

    Optional<String_t> gameDirName;
    Optional<String_t> rootDirName;
    bool playerSetUsed = false;
    bool uncompileTurns = false;
    bool receiveAttachments = true;
    PlayerSet_t players;

    afl::sys::StandardCommandLineParser parser(environment().getCommandLine());
    afl::string::Translator& tx = translator();
    String_t text;
    bool option;
    while (parser.getNext(option, text)) {
        if (option) {
            if (text == "w") {
                theUnpacker.setFormat(theUnpacker.WindowsFormat);
            } else if (text == "d") {
                theUnpacker.setFormat(theUnpacker.DosFormat);
            } else if (text == "a") {
                theUnpacker.setIgnore35Part(true);
            } else if (text == "t") {
                theUnpacker.setCreateTargetExt(true);
            } else if (text == "n") {
                theUnpacker.setFixErrors(false);
            } else if (text == "f") {
                theUnpacker.setForceIgnoreErrors(true);
            } else if (text == "x" || text == "v") {
                theUnpacker.setVerbose(true);
            } else if (text == "R") {
                detacher.setAcceptableKind(AttachmentUnpacker::RaceNameFile, false);
            } else if (text == "K") {
                detacher.setAcceptableKind(AttachmentUnpacker::ConfigurationFile, false);
            } else if (text == "A") {
                receiveAttachments = false;
            } else if (text == "u") {
                uncompileTurns = true;
            } else if (text == "log") {
                consoleLogger().setConfiguration(parser.getRequiredParameter("log"));
            } else if (text == "h" || text == "help") {
                help();
            } else {
                errorExit(Format(tx("invalid option specified. Use \"%s -h\" for help"), environment().getInvocationName()));
            }
        } else {
            int n;
            if (afl::string::strToInteger(text, n) && n > 0 && n <= game::v3::structures::NUM_PLAYERS) {
                // Player number
                players += n;
                playerSetUsed = true;
            } else if (!gameDirName.isValid()) {
                // Game directory
                gameDirName = text;
            } else if (!rootDirName.isValid()) {
                // Root directory - has no meaning in c2ng, accepted for compatibility
                rootDirName = text;
            } else {
                errorExit(tx("too many arguments"));
            }
        }
    }

    Ref<Directory> gameDir = fileSystem().openDirectory(gameDirName.orElse("."));
    specDir->addDirectory(gameDir);
    specDir->addDirectory(openSpecDirectory(fileSystem(), rootDirName, environment()));

    int retval = 0;
    int count = 0;
    for (int i = 1; i <= game::v3::structures::NUM_PLAYERS; ++i) {
        if (!playerSetUsed || players.contains(i)) {
            bool opened = false;
            String_t fileName = Format("player%d.rst", i);

            try {
                // Open and unpack the file
                Ref<afl::io::Stream> rst = gameDir->openFile(fileName, FileSystem::OpenRead);
                opened = true;

                ResultFile rstFile(*rst, translator());
                log().write(afl::sys::Log::Info, LOG_NAME, Format(tx("=== Unpacking player %d... ==="), i));

                theUnpacker.prepare(rstFile, i);

                // Check turn file
                if (uncompileTurns) {
                    String_t trnName = Format("player%d.trn", i);
                    afl::base::Ptr<afl::io::Stream> trn = gameDir->openFileNT(trnName, FileSystem::OpenRead);
                    if (trn.get() != 0) {
                        TurnFile trnFile(theUnpacker.charset(), tx, *trn);
                        if (validateTurn(i, rstFile, trnFile)) {
                            log().write(afl::sys::Log::Info, LOG_NAME, Format(tx("Using turn file %s."), trnName));
                            theUnpacker.turnProcessor().handleTurnFile(trnFile, theUnpacker.charset());
                        }
                    }
                }

                theUnpacker.finish(*gameDir, rstFile);
                ++count;

                // Load attachments
                if (receiveAttachments) {
                    detacher.loadDirectory(*gameDir, i, log(), tx);
                }
            }
            catch (afl::except::FileProblemException& e) {
                // If no player set was given, and the exception occurs during opening the RST, it is not fatal.
                if (playerSetUsed || opened) {
                    log().write(afl::sys::Log::Error, LOG_NAME, e.getFileName() + ": " + e.what());
                    retval = 1;
                }
            }
            catch (std::exception& e) {
                log().write(afl::sys::Log::Error, LOG_NAME, fileName + ": " + e.what());
                retval = 1;
            }
        }
    }

    detacher.dropUnselectedAttachments();
    detacher.dropUnchangedFiles(*gameDir, log(), tx);
    if (detacher.getNumAttachments() != 0) {
        detacher.saveFiles(*gameDir, log(), tx);
        log().write(afl::sys::Log::Info, LOG_NAME, Format(tx("Unpacked %d new attachment%!1{s%}."), detacher.getNumAttachments()));
    }

    if (!count) {
        errorExit(Format(tx("no result files found. Use \"%s -h\" for help"), environment().getInvocationName()));
    }
    exit(retval);
}

bool
game::v3::UnpackApplication::validateTurn(int player, ResultFile& rst, TurnFile& trn)
{
    // Check player
    if (trn.getPlayer() != player) {
        return false;
    }

    // Check timestamp
    rst.seekToSection(ResultFile::GenSection);
    GenFile gen;
    gen.loadFromResult(rst.getFile());
    if (trn.getTimestamp() != gen.getTimestamp()) {
        return false;
    }

    return true;
}

void
game::v3::UnpackApplication::help()
{
    afl::io::TextWriter& out = standardOutput();
    afl::string::Translator& tx = translator();
    out.writeLine(Format(tx("PCC2 Result File Unpacker v%s - (c) 2010-2021 Stefan Reuther"), PCC2_VERSION));
    out.writeLine();
    out.writeLine(Format(tx("Usage:\n"
                            "  %s [-h]\n"
                            "  %$0s [-OPTIONS] [PLAYER] [GAMEDIR]\n\n"
                            "%s\n"
                            "Report bugs to <Streu@gmx.de>"),
                         environment().getInvocationName(),
                         util::formatOptions(tx("Options:\n"
                                                "-w\tCreate Windows (3.5) format [default]\n"
                                                "-d\tCreate DOS (3.0) format\n"
                                                "-a\tIgnore version 3.5 part of RST\n"
                                                "-t\tCreate TARGETx.EXT files\n"
                                                "-n\tDo not attempt to fix host-side errors\n"
                                                "-f\tForce unpack of files with failing checksums\n"
                                                "-x\tIncrease verbosity\n"
                                                "-R\tRefuse race name updates\n"
                                                "-K\tRefuse configuration file updates\n"
                                                "-A\tDo not receive any attachments\n"
                                                "-u\tUnpack turn files as well\n"
                                                "--log=CONFIG\tSet logger configuration\n"))));
    exit(0);
}
