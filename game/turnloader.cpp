/**
  *  \file game/turnloader.cpp
  */

#include "game/turnloader.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/limits.hpp"
#include "game/db/loader.hpp"
#include "game/root.hpp"
#include "afl/string/format.hpp"
#include "game/session.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "game/score/loader.hpp"

int
game::TurnLoader::getDefaultPlayer(PlayerSet_t baseSet) const
{
    // We don't care for the string
    afl::string::NullTranslator tx;
    String_t tmp;

    PlayerStatus foundStatus = Available;
    int foundPlayer = 0;
    for (int i = 1; i <= MAX_PLAYERS; ++i) {
        if (baseSet.contains(i)) {
            PlayerStatusSet_t thisSet = getPlayerStatus(i, tmp, tx);
            if (thisSet.contains(Available)) {
                PlayerStatus thisStatus = thisSet.contains(Primary) ? Primary : thisSet.contains(Playable) ? Playable : Available;
                if (thisStatus > foundStatus || foundPlayer == 0) {
                    // Better status or first finding: keep it
                    foundPlayer = i;
                    foundStatus = thisStatus;
                } else if (thisStatus == foundStatus && foundPlayer != 0) {
                    // Same status and second finding: ambiguous
                    foundPlayer = -1;
                } else {
                    // Worse status: skip
                }
            }
        }
    }
    return foundPlayer > 0 ? foundPlayer : 0;
}


void
game::TurnLoader::loadCurrentDatabases(Turn& turn, Game& game, int player, Root& root, Session& session, afl::charset::Charset& charset)
{
    // Starchart DB
    afl::base::Ptr<afl::io::Stream> file = root.gameDirectory().openFileNT(afl::string::Format("chart%d.cc", player), afl::io::FileSystem::OpenRead);
    if (file.get() != 0) {
        game::db::Loader(charset, session.world()).load(*file, turn, game, true);
    }

    // Score DB
    // ex game/stat.h:initStatisticsFile
    try {
        file = root.gameDirectory().openFileNT("score.cc", afl::io::FileSystem::OpenRead);
        if (file.get() != 0) {
            // We have a score.cc file, load it
            game::score::Loader(session.translator(), charset).load(game.scores(), *file);
            // } else if ((s = game_file_dir->openFileNT("stat.cc", Stream::C_READ)) != 0) {
            //     /* we have a stat.cc file, load it */
            //     statistics_file->loadOldFile(*s);
        } else {
            // No score file
        }
    }
    catch (afl::except::FileProblemException& e) {
        // FIXME: port this: console.write(LOG_ERROR, format(_("%s: %s, file has been ignored"), e.getFileName(), e.what()));
    }

}

void
game::TurnLoader::loadHistoryDatabases(Turn& turn, Game& game, int player, int turnNumber, Root& root, afl::charset::Charset& charset)
{
    // FIXME
    (void) turn;
    (void) game;
    (void) player;
    (void) turnNumber;
    (void) root;
    (void) charset;
}
