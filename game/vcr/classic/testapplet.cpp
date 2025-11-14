/**
  *  \file game/vcr/classic/testapplet.cpp
  *  \brief Class game::vcr::classic::TestApplet
  */

#include "game/vcr/classic/testapplet.hpp"
#include "afl/base/optional.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/specificationloader.hpp"
#include "game/v3/rootloader.hpp"
#include "game/vcr/classic/algorithm.hpp"
#include "game/vcr/classic/database.hpp"
#include "game/vcr/classic/nullvisualizer.hpp"

using afl::base::Optional;
using afl::base::Ptr;
using afl::base::Ref;
using afl::io::FileSystem;
using afl::io::TextWriter;
using afl::string::Format;
using afl::sys::Environment;
using game::spec::ShipList;
using game::vcr::Object;
using game::vcr::classic::Database;

namespace {
    Ptr<Database> loadVcrs(util::Application& app, const String_t& name, game::Root& root)
    {
        afl::base::Ref<afl::io::Stream> file = app.fileSystem().openFile(name, FileSystem::OpenRead);
        Ptr<Database> db(new Database());

        afl::charset::CodepageCharset charset(afl::charset::g_codepageLatin1);
        db->load(*file, root.hostConfiguration(), charset);
        app.standardOutput().writeLine(Format("VCR file contains %d entries", db->getNumBattles()));
        return db;
    }

    String_t formatParticipant(const Object& obj)
    {
        return Format("  S:%3d  D:%3d  C:%3d  A:%3d",
                      obj.getShield(),
                      obj.getDamage(),
                      obj.getCrew(),
                      obj.getTorpedoType() != 0 ? obj.getNumTorpedoes() : obj.getNumFighters());
    }
}

int
game::vcr::classic::TestApplet::run(util::Application& app, afl::sys::Environment::CommandLine_t& cmdl)
{
    Optional<String_t> file;
    Optional<String_t> dir;
    Optional<int> repeatOption;

    String_t it;
    while (cmdl.getNextElement(it)) {
        if (!file.isValid()) {
            file = it;
        } else if (!dir.isValid()) {
            dir = it;
        } else if (!repeatOption.isValid()) {
            int n = 0;
            if (!afl::string::strToInteger(it, n) || n <= 0) {
                help(app);
            }
            repeatOption = n;
        } else {
            help(app);
        }
    }

    if (!file.isValid()) {
        help(app);
    }

    // Root loader
    Environment& env = app.environment();
    FileSystem& fs = app.fileSystem();
    afl::charset::Utf8Charset cs;
    game::v3::RootLoader loader(fs.openDirectory(fs.makePathName(fs.makePathName(env.getInstallationDirectoryName(), "share"), "specs")),
                                0, 0,
                                app.translator(), app.log(),
                                fs);

    // Root
    Ref<game::config::UserConfiguration> config = game::config::UserConfiguration::create();
    Ptr<Root> root(loader.load(fs.openDirectory(dir.orElse(".")), cs, *config, true));
    if (root.get() == 0) {
        app.errorOutput().writeLine("No game data found.");
        return 1;
    }

    // Specification
    ShipList shipList;
    root->specificationLoader().loadShipList(shipList, *root, std::auto_ptr<game::StatusTask_t>(StatusTask_t::makeNull()))->call();

    // Load VCRs
    Ptr<Database> vcrs = loadVcrs(app, *file.get(), *root);

    TextWriter& out = app.standardOutput();
    out.writeLine("--- Starting Playback ---");

    const int repeat = repeatOption.orElse(1);
    for (int rc = 0; rc < repeat; ++rc) {
        for(size_t i = 0; i < vcrs->getNumBattles(); ++i) {
            Battle* en = vcrs->getBattle(i);
            if (en == 0) {
                out.writeLine(Format("Record #%d does not exist?", (i+1)));
            } else {
                out.writeLine(Format("Record #%d:", (i+1)));

                // Create algo
                NullVisualizer vis;
                std::auto_ptr<Algorithm> algo(en->createAlgorithm(vis, root->hostConfiguration(), shipList));

                // Prepare capabilities
                if (!algo->setCapabilities(en->getCapabilities())) {
                    out.writeLine("\tinvalid (cap)");
                    continue;
                }

                // Prepare VCR
                Object left(*en->getObject(0, false));
                Object right(*en->getObject(1, false));
                uint16_t seed(en->getSeed());
                if (algo->checkBattle(left, right, seed)) {
                    out.writeLine("\tinvalid (content)");
                    continue;
                }

                // Play it
                algo->playBattle(left, right, seed);

                // Fetch result
                algo->doneBattle(left, right);
                BattleResult_t result = algo->getResult();
                Time_t time = algo->getTime();

                out.writeLine(Format("\tEnding time %d (%d:%02d)", time, time/60, time%60));
                if (result.contains(Invalid)) {
                    out.writeLine("\tinvalid (play)");
                } else {
                    String_t line;
                    char t = '\t';
                    if (result.contains(LeftDestroyed)) {
                        line += t;
                        line += "left-destroyed";
                        t = ' ';
                    }
                    if (result.contains(RightDestroyed)) {
                        line += t;
                        line += "right-destroyed";
                        t = ' ';
                    }
                    if (result.contains(LeftCaptured)) {
                        line += t;
                        line += "left-captured";
                        t = ' ';
                    }
                    if (result.contains(RightCaptured)) {
                        line += t;
                        line += "right-captured";
                        t = ' ';
                    }
                    if (result.contains(Timeout)) {
                        line += t;
                        line += "timeout";
                        t = ' ';
                    }
                    if (t == '\t') {
                        out.writeLine("\tnone");
                    } else {
                        out.writeLine(line);
                    }
                    out.writeLine(formatParticipant(left) + "   |   " + formatParticipant(right));
                }
            }
        }
    }
    return 0;
}

void
game::vcr::classic::TestApplet::help(util::Application& app)
{
    app.standardOutput().writeLine("Usage: playvcr vcrfile [rootdir [repeat]]");
    app.exit(1);
}
