/**
  *  \file game/turnloader.cpp
  *  \brief Base class game::TurnLoader
  */

#include "game/turnloader.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/db/loader.hpp"
#include "game/limits.hpp"
#include "game/root.hpp"
#include "game/score/loader.hpp"
#include "game/session.hpp"

namespace {
    const char*const LOG_NAME = "game.db";
}

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
game::TurnLoader::loadCurrentDatabases(Turn& turn, Game& game, int player, Root& root, Session& session)
{
    afl::charset::Charset& charset = root.charset();
    afl::string::Translator& tx = session.translator();
    afl::sys::LogListener& log = session.log();

    // Starchart DB
    afl::base::Ptr<afl::io::Stream> file = root.gameDirectory().openFileNT(afl::string::Format("chart%d.cc", player), afl::io::FileSystem::OpenRead);
    if (file.get() != 0) {
        game::db::Loader(charset, session.world(), session.translator()).load(*file, turn, game, true);
    }

    // Score DB
    // ex game/stat.h:initStatisticsFile
    try {
        file = root.gameDirectory().openFileNT("score.cc", afl::io::FileSystem::OpenRead);
        if (file.get() != 0) {
            // We have a score.cc file, load it
            game::score::Loader(session.translator(), charset).load(game.scores(), *file);
        } else if ((file = root.gameDirectory().openFileNT("stat.cc", afl::io::FileSystem::OpenRead)).get() != 0) {
            // We have a stat.cc file, load it
            game::score::Loader(session.translator(), charset).loadOldFile(game.scores(), *file);
        } else {
            // No score file
        }
    }
    catch (afl::except::FileProblemException& e) {
        log.write(afl::sys::LogListener::Error, LOG_NAME, tx("File has been ignored"), e);
    }

    // Message configuration
    game.messageConfiguration().load(root.gameDirectory(), player);

    // Teams
    game.teamSettings().load(root.gameDirectory(), player, charset, tx);
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

void
game::TurnLoader::saveCurrentDatabases(const Turn& turn, const Game& game, int player, const Root& root, Session& session, afl::charset::Charset& charset)
{
    // Save starchart
    if (game::spec::ShipList* shipList = session.getShipList().get()) {
        afl::base::Ref<afl::io::Stream> out = root.gameDirectory().openFile(afl::string::Format("chart%d.cc", player), afl::io::FileSystem::Create);
        game::db::Loader(charset, session.world(), session.translator()).save(*out, turn, game, *shipList);
    }

    // Save scores
    // ex saveStatisticsFile
    if (!game.scores().hasFutureFeatures()) {
        afl::base::Ref<afl::io::Stream> out = root.gameDirectory().openFile("score.cc", afl::io::FileSystem::Create);
        game::score::Loader(session.translator(), charset).save(game.scores(), *out);
    } else {
        session.log().write(afl::sys::LogListener::Warn, LOG_NAME, session.translator()("The statistics file in game directory was written by a newer version of PCC2; changes not written."));
    }

    // Save message configuration
    game.messageConfiguration().save(root.gameDirectory(), player);

    // Teams
    game.teamSettings().save(root.gameDirectory(), player, charset);
}
