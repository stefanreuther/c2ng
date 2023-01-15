/**
  *  \file server/file/gamestatus.cpp
  */

#include "server/file/gamestatus.hpp"
#include "afl/base/countof.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/v3/registrationkey.hpp"
#include "server/common/racenames.hpp"
#include "server/file/directoryitem.hpp"
#include "server/file/directorywrapper.hpp"
#include "server/file/root.hpp"

using afl::base::Ref;

namespace {
    const char KEYFILE_NAME[] = "fizz.bin";

    void createSlotList(server::file::GameStatus::GameInfo& out,
                        const game::v3::DirectoryScanner& scanner,
                        const server::common::RaceNames& raceNames)
    {
        for (int i = 1; i <= scanner.NUM_PLAYERS; ++i) {
            if (scanner.getPlayerFlags(i).contains(scanner.HaveResult)) {
                if (const String_t* pName = raceNames.longNames().at(i)) {
                    out.slots.push_back(server::file::GameStatus::Slot_t(i, pName->empty() ? String_t(afl::string::Format("Player %d", i)) : *pName));
                }
            }
        }
    }
}

server::file::GameStatus::GameStatus()
    : m_game(),
      m_key()
{ }

server::file::GameStatus::~GameStatus()
{ }

void
server::file::GameStatus::load(Root& root, DirectoryItem& dir)
{
    // ex DirInfo::init (sort-of)
    dir.readContent(root);
    Ref<DirectoryWrapper> dirWrapper(DirectoryWrapper::create(dir));

    try {
        // Step 1: Load registration
        // FIXME: this uses a-priori knowledge that the key parser will use the file KEYFILE_NAME.
        if (/*FileItem* it =*/ dir.findFile(KEYFILE_NAME)) {
            game::v3::RegistrationKey key(std::auto_ptr<afl::charset::Charset>(root.defaultCharacterSet().clone()));
            afl::string::NullTranslator tx;
            key.initFromDirectory(*dirWrapper, root.log(), tx);

            std::auto_ptr<KeyInfo> k(new KeyInfo());
            k->fileName = KEYFILE_NAME;
            k->isRegistered = (key.getStatus() == key.Registered);
            k->label1 = key.getLine(key.Line1);
            k->label2 = key.getLine(key.Line2);
            k->keyId  = key.getKeyId();

            // Commit
            m_key = k;
        }

        // Step 2: Load overview
        game::v3::DirectoryScanner& scanner = root.directoryScanner();
        scanner.scan(*dirWrapper, root.defaultCharacterSet(), game::v3::DirectoryScanner::ResultOnly);
        if (!scanner.getDirectoryFlags().empty()) {
            // Create new GameInfo object.
            // Start with a local object in case anything below throws.
            // (Could be loadRaceNames.)
            std::auto_ptr<GameInfo> g(new GameInfo());
            g->hostVersion = scanner.getDirectoryHostVersion().toString();

            // Load race names and generate slot list
            if (FileItem* it = dir.findFile("race.nm")) {
                server::common::RaceNames raceNames;
                raceNames.load(dir.getFileContent(*it)->get(), root.defaultCharacterSet());
                createSlotList(*g, scanner, raceNames);
            } else {
                g->missingFiles.push_back("race.nm");
                createSlotList(*g, scanner, root.defaultRaceNames());
            }

            // Missing files
            static const char*const FILES[] = {
                "beamspec.dat",
                "engspec.dat",
                "hullspec.dat",
                "pconfig.src",
                "planet.nm",
                "torpspec.dat",
                "truehull.dat",
            };
            for (size_t i = 0; i < countof(FILES); ++i) {
                if (dir.findFile(FILES[i]) == 0) {
                    g->missingFiles.push_back(FILES[i]);
                }
            }

            if (dir.findFile("xyplan.dat") == 0) {
                bool haveXY = true;
                for (int i = 1; i <= scanner.NUM_PLAYERS; ++i) {
                    if (scanner.getPlayerFlags(i).contains(scanner.HaveResult)) {
                        if (dir.findFile(afl::string::Format("xyplan%d.dat", i)) == 0) {
                            haveXY = false;
                            break;
                        }
                    }
                }
                if (!haveXY) {
                    g->missingFiles.push_back("xyplan.dat");
                }
            }

            // Save data
            m_game = g;
        }
    }
    catch (...) {
        // Ignore all errors.
        // This would mean the directory is not recognized as a proper game directory.
    }
}

const server::file::GameStatus::GameInfo*
server::file::GameStatus::getGameInfo() const
{
    return m_game.get();
}

const server::file::GameStatus::KeyInfo*
server::file::GameStatus::getKeyInfo() const
{
    return m_key.get();
}
