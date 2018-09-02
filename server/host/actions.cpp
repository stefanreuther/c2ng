/**
  *  \file server/host/actions.cpp
  *  \brief Actions on games
  */

#include "server/host/actions.hpp"
#include "afl/bits/int16le.hpp"
#include "afl/bits/int32le.hpp"
#include "afl/bits/pack.hpp"
#include "afl/bits/value.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/string/format.hpp"
#include "game/playerset.hpp"
#include "server/host/game.hpp"
#include "server/host/root.hpp"
#include "server/interface/hostgame.hpp"

using afl::string::Format;
using server::interface::FileBase;

namespace {
    const char*const LOG_NAME = "host.action";

    void importSingleFileHistory(FileBase& hostFile,
                                 const String_t& outDir,
                                 const FileBase::ContentInfoMap_t& backupContent,
                                 afl::net::redis::StringSetKey fileKey)
    {
        // Find content of "out" directory
        FileBase::ContentInfoMap_t outContent;
        try {
            hostFile.getDirectoryContent(outDir, outContent);
        }
        catch (...) { }
             
        // Clear old value, if any
        fileKey.remove();

        // Add to database
        for (FileBase::ContentInfoMap_t::const_iterator it = outContent.begin(); it != outContent.end(); ++it) {
            if (it->second->type == FileBase::IsFile && backupContent.find(it->first) != backupContent.end()) {
                fileKey.add(it->first);
            }
        }
    }

}

// Drop slot if it is dead.
bool
server::host::dropSlotIfDead(Game& game, int slot)
{
    bool dead = false;
    if (game.getState() == server::interface::HostGame::Running) {
        String_t packedScore = game.turn(game.turnNumber().get()).scores().stringField("timscore").get();
        if (slot <= 0
            || packedScore.size() < slot*4U
            || afl::bits::Int32LE::unpack(*reinterpret_cast<const afl::bits::Int32LE::Bytes_t*>(packedScore.data() + 4*(slot-1))) <= 0)
        {
            dead = true;
        }
    }
    if (dead) {
        game.getSlot(slot).slotStatus().set(0);
    }
    return dead;
}

// Kick inactive players.
void
server::host::processInactivityKicks(Root& root, int32_t gameId)
{
    Game game(root, gameId);

    // Get configured limit
    int32_t turnLimit = game.numMissedTurnsForKick().getOptional().orElse(root.config().numMissedTurnsForKick);
    if (turnLimit <= 0) {
        return;
    }

    // Get current turn
    int32_t turnNumber = game.turnNumber().get();
    if (turnNumber < turnLimit + 1) {
        return;
    }

    // Determine players to kick
    // - determine existing players
    game::PlayerSet_t players;
    game::PlayerArray<String_t> primaryPlayers;
    for (int32_t slot = 1; slot <= Game::NUM_PLAYERS; ++slot) {
        if (game.getSlot(slot).slotStatus().get() != 0) {
            String_t playerId = game.getSlot(slot).players()[0];
            if (!playerId.empty()) {
                players += slot;
                primaryPlayers.set(slot, playerId);
            }
        }
    }

    // - remove players that submitted a turn
    for (int32_t turn = turnNumber - turnLimit+1; turn <= turnNumber; ++turn) {
        int16_t turnStatus[Game::NUM_PLAYERS];
        afl::bits::unpackArray<afl::bits::Int16LE>(turnStatus, afl::string::toBytes(game.turn(turn).info().turnStatus().get()));
        for (int32_t i = 1; i <= Game::NUM_PLAYERS; ++i) {
            if (players.contains(i) && turnStatus[i-1] != Game::TurnMissing) {
                players -= i;
            } else {
                if (primaryPlayers.get(i) != game.turn(turn).playerId().stringField(Format("%d", i)).get()) {
                    players -= i;
                }
            }
        }
        if (players.empty()) {
            break;
        }
    }

    // Kick these players
    for (int32_t slot = 1; slot <= Game::NUM_PLAYERS; ++slot) {
        if (players.contains(slot)) {
            root.log().write(afl::sys::Log::Info, LOG_NAME, Format("game %d: kicking slot %d for inactivity", gameId, slot));

            int numPlayers = game.getSlot(slot).players().size();
            String_t userId;
            while (numPlayers > 0) {
                // Unsubscribe and save the player
                userId = game.popPlayerSlot(slot, root);
                --numPlayers;
            }

            // Is this slot dead now? If so, drop it. */
            bool dead = dropSlotIfDead(game, slot);

            // History
            game.addUserHistoryItem(root, dead ? "game-resign-dead" : "game-kick", Format("%s:%d", userId, slot), userId);
        }
    }

    // Notify scheduler
    if (!players.empty()) {
        root.handleGameChange(gameId);
    }
}

// Import file history for one turn.
void
server::host::importFileHistory(server::interface::FileBase& hostFile,
                                const String_t& gameDir,
                                const int turnNumber,
                                Game::TurnFiles fileHistory)
{
    // Find content of "pre" backup
    FileBase::ContentInfoMap_t backupContent;
    hostFile.getDirectoryContent(Format("%s/backup/pre-%03d", gameDir, turnNumber), backupContent);

    // Import into file history
    importSingleFileHistory(hostFile, Format("%s/out/all", gameDir), backupContent, fileHistory.globalFiles());
    for (int i = 1; i <= Game::NUM_PLAYERS; ++i) {
        importSingleFileHistory(hostFile, Format("%s/out/%d", gameDir, i), backupContent, fileHistory.playerFiles(i));
    }
}

// Import file history for all turns.
void
server::host::importAllFileHistory(server::interface::FileBase& hostFile, Game& game)
{
    int turnNr = game.turnNumber().get();
    String_t gameDir = game.getDirectory();

    for (int turn = 1; turn <= turnNr; ++turn) {
        // Try to import every turn that has no globalFiles.
        // Every imported game will have globalFiles, so this seems a good indicator.
        Game::TurnFiles files = game.turn(turn).files();
        if (files.globalFiles().empty()) {
            // Ignore errors.
            try {
                importFileHistory(hostFile, gameDir, turn, files);
            }
            catch (...) { }
        }
    }
}
