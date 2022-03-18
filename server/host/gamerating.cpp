/**
  *  \file server/host/gamerating.cpp
  *  \brief Game Rating
  *
  *  PCC2 Comment:
  *
  *  These functions rate a game or tool. Unlike the standalone rating tool, c2rater,
  *  this has to deal with two obstacles:
  *  - tools have potentially totally incomplete configuration. We still must produce
  *    a guess how it will affect the rating. Therefore, DifficultyRater is able to
  *    produce individual ratings when given enough configuration to compute them.
  *    We therefore feed the DifficultyRater with partial configuration.
  *  - for a game, tools can override the game difficulty. For unmastered games, this
  *    is pretty easy: if the tool specifies a difficulty, we exclude it from the
  *    rating computation. For mastered games, this is hard because we don't know
  *    where the actual config values come from. A tool that configures a NativeRating
  *    of 150 and specifies an override of 180 would be rated as 150%*180% = 270%.
  *    Overrides therefore only work reliably for two cases:
  *    - ship lists. Here we know how to exclude values: if a ship list specifies a
  *      rating, we don't rate the ship list files.
  *    - tools that affect configuration which is not rated by the normal formula.
  */

#include "server/host/gamerating.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/string/format.hpp"
#include "game/maint/difficultyrater.hpp"
#include "server/file/clientdirectory.hpp"
#include "server/file/root.hpp"
#include "server/host/game.hpp"
#include "server/host/root.hpp"
#include "util/math.hpp"
#include "server/interface/baseclient.hpp"

namespace {
    const char LOG_NAME[] = "host.rating";

    enum {
        HaveAMaster = 1,
        HavePMaster = 2
    };

    bool processDirectoryFile(game::maint::DifficultyRater& rater, afl::io::Directory& dir, const char* name)
    {
        afl::base::Ptr<afl::io::Stream> s = dir.openFileNT(name, afl::io::FileSystem::OpenRead);
        if (s.get() != 0) {
            rater.addConfigurationFile(*s);
            return true;
        } else {
            return false;
        }
    }

    void processDirectory(game::maint::DifficultyRater& rater, uint32_t& state, afl::net::CommandHandler& filer, String_t dir)
    {
        // No directory (could be tool without directory): ignore
        if (dir.empty()) {
            return;
        }

        // Access directory
        afl::base::Ref<server::file::ClientDirectory> dirWrapper = server::file::ClientDirectory::create(filer, dir);

        // Read ship list unless we already have one
        if (!rater.isRatingKnown(game::maint::DifficultyRater::ShiplistRating)) {
            rater.addShipList(*dirWrapper);
        }

        // Read configuration files
        processDirectoryFile(rater, *dirWrapper, "pconfig.src");
        processDirectoryFile(rater, *dirWrapper, "pconfig.src.frag");
        processDirectoryFile(rater, *dirWrapper, "shiplist.txt");
        processDirectoryFile(rater, *dirWrapper, "shiplist.txt.frag");

        // Conditionally process master config files. Do not process
        // AMaster config if we have a PMaster config and vice versa;
        // only process matching fragments.
        if ((state & (HaveAMaster|HavePMaster)) == 0 && processDirectoryFile(rater, *dirWrapper, "amaster.src")) {
            state |= HaveAMaster;
        }
        if ((state & (HaveAMaster|HavePMaster)) == 0 && processDirectoryFile(rater, *dirWrapper, "pmaster.cfg")) {
            state |= HavePMaster;
        }
        if ((state & HaveAMaster) != 0) {
            processDirectoryFile(rater, *dirWrapper, "amaster.src.frag");
        }
        if ((state & HavePMaster) != 0) {
            processDirectoryFile(rater, *dirWrapper, "pmaster.cfg.frag");
        }
    }

    bool processRatingForTool(double& modifier, server::host::Root::ToolTree tree, String_t toolName)
    {
        afl::net::redis::HashKey tool(tree.byName(toolName));
        bool result = false;
        if (tool.intField("useDifficulty").get()) {
            modifier *= tool.intField("difficulty").get();
            modifier /= 100.0;
            result = true;
        }
        return result;
    }

    void processDirectoryForTool(double& modifier, game::maint::DifficultyRater& rater, uint32_t& state, server::host::Root::ToolTree tree, String_t toolName, afl::net::CommandHandler& filer)
    {
        if (!processRatingForTool(modifier, tree, toolName)) {
            String_t toolDir = tree.byName(toolName).stringField("path").get();
            processDirectory(rater, state, filer, toolDir);
        }
    }

    /** Format computed rating to external format.
        \param rating Rating, floating point, 1.0 for standard rating
        \return [1,1000], 100 for standard rating */
    int formatRating(double rating)
    {
        int result = util::roundToInt(100 * rating);
        if (result < 1) {
            result = 1;
        }
        if (result > 1000) {
            result = 1000;
        }
        return result;
    }
}

// /** Compute rating of a game.
//     \param root Server root
//     \param g Game */
int
server::host::computeGameRating(Root& root, Game& g)
{
    // ex planetscentral/host/rating.cc:computeGameRating
    try {
        server::interface::BaseClient(root.hostFile()).setUserContext(String_t());

        String_t gameDir(g.getDirectory() + "/data");
        game::maint::DifficultyRater rater;

        afl::data::StringList_t tools;
        g.toolsByKind().getAll(tools);

        double modifier = 1.0;
        if (g.getConfigInt("masterHasRun")) {
            // Master has run, so the game directory is fully-populated.
            // We still have to check whether any component enforces a particular rating modifier.
            afl::base::Ref<server::file::ClientDirectory> dirWrapper = server::file::ClientDirectory::create(root.hostFile(), gameDir);
            processRatingForTool(modifier, root.hostRoot(),     g.getConfig("host"));
            processRatingForTool(modifier, root.masterRoot(),   g.getConfig("master"));
            bool slmod = processRatingForTool(modifier, root.shipListRoot(), g.getConfig("shiplist"));
            for (size_t i = 0; i+1 < tools.size(); i += 2) {
                processRatingForTool(modifier, root.toolRoot(), tools[i+1]);
            }
            if (!slmod) {
                rater.addShipList(*dirWrapper);
            }
            rater.addConfigurationDirectory(*dirWrapper);
        } else {
            // Master has not run, so gather data from individual directories
            uint32_t state = 0;
            processDirectory(rater, state, root.hostFile(), gameDir);
            processDirectoryForTool(modifier, rater, state, root.hostRoot(),     g.getConfig("host"),     root.hostFile());
            processDirectoryForTool(modifier, rater, state, root.masterRoot(),   g.getConfig("master"),   root.hostFile());
            processDirectoryForTool(modifier, rater, state, root.shipListRoot(), g.getConfig("shiplist"), root.hostFile());
            for (size_t i = 0; i+1 < tools.size(); i += 2) {
                processDirectoryForTool(modifier, rater, state, root.toolRoot(), tools[i+1], root.hostFile());
            }
        }
        return formatRating(rater.getTotalRating() * modifier);
    }
    catch (afl::except::FileProblemException& e) {
        root.log().write(afl::sys::LogListener::Warn,
                         LOG_NAME,
                         afl::string::Format("game %d: error in rating computation", g.getId()),
                         e);
        return 100;
    }
}
