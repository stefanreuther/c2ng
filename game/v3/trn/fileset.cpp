/**
  *  \file game/v3/trn/fileset.cpp
  *  \brief Class game::v3::trn::FileSet
  */

#include "game/v3/trn/fileset.hpp"
#include "afl/string/format.hpp"
#include "game/config/stringoption.hpp"
#include "util/backupfile.hpp"
#include "util/translation.hpp"

namespace {
    const char*const LOG_NAME = "game.v3.trn";

    // FIXME: where to define this? ex opt_BackupTurn, opt_MaketurnTarget
    // ConfigStringOption opt_BackupTurn    (getUserPreferences(), "Backup.Turn");
    // ConfigStringOption opt_MaketurnTarget(getUserPreferences(), "Maketurn.Target");
    const game::config::StringOptionDescriptor opt_BackupTurn     = { "Backup.Turn" };
    const game::config::StringOptionDescriptor opt_MaketurnTarget = { "Maketurn.Target" };
}


// Constructor.
game::v3::trn::FileSet::FileSet(afl::io::Directory& dir, afl::charset::Charset& charset)
    : m_directory(dir),
      m_charset(charset),
      m_turnFiles(),
      m_turnNumbers()
{ }

// Destructor.
game::v3::trn::FileSet::~FileSet()
{ }

// Create a turn file in memory.
game::v3::TurnFile&
game::v3::trn::FileSet::create(int playerNr,
                               const Timestamp& timestamp,
                               int turnNumber)
{
    TurnFile& result = *m_turnFiles.pushBackNew(new TurnFile(m_charset, playerNr, timestamp));
    m_turnNumbers.push_back(turnNumber);
    return result;
}

// Update turn file trailers.
void
game::v3::trn::FileSet::updateTrailers()
{
    if (!m_turnFiles.empty()) {
        structures::TurnPlayerSecret data;
        bool haveData = false;
        const Timestamp t = m_turnFiles[0]->getTimestamp();
    
        // Try to find existing block
        for (int i = 1; i <= NUM_PLAYERS; ++i) {
            // FIXME: only check files that we will not rewrite?
            try {
                afl::base::Ref<afl::io::Stream> file = m_directory.openFile(afl::string::Format("player%d.trn", i), afl::io::FileSystem::OpenRead);
                TurnFile turnFile(m_charset, *file, false);
                if (turnFile.getTimestamp() == t) {
                    data = turnFile.getDosTrailer().playerSecret;
                    haveData = true;
                    break;
                }
            }
            catch(...) { }
        }

        // If we didn't find one, generate anew (DOS maketurn rules)
        if (!haveData) {
            afl::base::fromObject(data).fill(0);
            for (size_t i = 0, n = m_turnFiles.size(); i < n; ++i) {
                int playerNr = m_turnFiles[i]->getPlayer();
                if (playerNr > 0 && playerNr <= NUM_PLAYERS) {
                    data.data[playerNr-1] = m_turnFiles[i]->getDosTrailer().checksum;
                }
            }
        }

        // Update turns
        for (size_t i = 0, n = m_turnFiles.size(); i < n; ++i) {
            m_turnFiles[i]->setPlayerSecret(data);
            m_turnFiles[i]->updateTrailer();
        }
    }
}

// Save all turn files.
void
game::v3::trn::FileSet::saveAll(afl::sys::LogListener& log, const PlayerList& players, afl::io::FileSystem& fs, const game::config::UserConfiguration& config)
{
    for (size_t i = 0, n = m_turnFiles.size(); i < n; ++i) {
        const int player = m_turnFiles[i]->getPlayer();
        afl::base::Ref<afl::io::Stream> file = m_directory.openFile(afl::string::Format("player%d.trn", player), afl::io::FileSystem::Create);
        log.write(afl::sys::LogListener::Info, LOG_NAME,
                  afl::string::Format(_("Writing %s turn file (%d command%!1{s%})...").c_str(),
                                      players.getPlayerName(player, Player::AdjectiveName),
                                      m_turnFiles[i]->getNumCommands()));
        m_turnFiles[i]->write(*file);

        // Write backup copies
        util::BackupFile tpl;
        tpl.setTurnNumber(m_turnNumbers[i]);
        tpl.setPlayerNumber(m_turnFiles[i]->getPlayer());
        tpl.setGameDirectoryName(m_directory.getDirectoryName());

        file->setPos(0);
        tpl.copyFile(fs, config[opt_BackupTurn](), *file);
        file->setPos(0);
        tpl.copyFile(fs, config[opt_MaketurnTarget](), *file);
    }
}

// Get number of turn files.
size_t
game::v3::trn::FileSet::getNumFiles() const
{
    return m_turnFiles.size();
}
