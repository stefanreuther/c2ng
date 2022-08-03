/**
  *  \file test_apps/testvcr.cpp
  */

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <iomanip>
#include "afl/except/fileproblemexception.hpp"
#include "util/consolelogger.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/v3/rootloader.hpp"
#include "game/vcr/classic/database.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "game/vcr/classic/nullvisualizer.hpp"
#include "game/vcr/classic/algorithm.hpp"
#include "afl/sys/environment.hpp"
#include "afl/charset/utf8charset.hpp"
#include "game/specificationloader.hpp"

namespace {
    void help()
    {
        std::cout << "Usage: playvcr vcrfile [rootdir [repeat]]\n";
        std::exit(1);
    }

    afl::base::Ptr<game::vcr::classic::Database> loadVcrs(const char* name, game::Root& root)
    {
        afl::base::Ref<afl::io::Stream> file = afl::io::FileSystem::getInstance().openFile(name, afl::io::FileSystem::OpenRead);
        afl::base::Ptr<game::vcr::classic::Database> db(new game::vcr::classic::Database());

        afl::charset::CodepageCharset charset(afl::charset::g_codepageLatin1);
        db->load(*file, root.hostConfiguration(), charset);
        std::cout << "VCR file contains " << db->getNumBattles() << " entries\n";
        return db;
    }

}

int
main(int, char** argv)
{
    const char* file = 0;
    const char* dir = 0;
    bool repeat_set = false;
    int repeat = 1;

    while (const char* p = *++argv) {
        if (!file) {
            file = p;
        } else if (!dir) {
            dir = p;
        } else if (!repeat_set) {
            repeat_set = true;
            repeat = std::atoi(*argv);
            if (repeat <= 0) {
                help();
            }
        } else {
            help();
        }
    }

    if (!file) {
        help();
    }

    try {
        // Root loader
        afl::sys::Environment& env = afl::sys::Environment::getInstance(argv);
        afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
        afl::string::NullTranslator tx;
        util::ConsoleLogger logger;
        afl::charset::Utf8Charset cs;
        game::v3::RootLoader loader(fs.openDirectory(fs.makePathName(fs.makePathName(env.getInstallationDirectoryName(), "share"), "specs")),
                                    0, 0,
                                    tx, logger,
                                    fs);

        // Root
        afl::base::Ptr<game::Root> root(loader.load(fs.openDirectory(dir ? dir : "."),
                                                    cs,
                                                    game::config::UserConfiguration(),
                                                    true));
        if (root.get() == 0) {
            std::cerr << "No game data found.\n";
            return 1;
        }

        // Specification
        game::spec::ShipList shipList;
        root->specificationLoader().loadShipList(shipList, *root, std::auto_ptr<game::StatusTask_t>(game::StatusTask_t::makeNull()))->call();

        // Load VCRs
        afl::base::Ptr<game::vcr::classic::Database> vcrs = loadVcrs(file, *root);

        std::cout << "--- Starting Playback ---" << std::endl;

        for (int rc = 0; rc < repeat; ++rc) {
            for(size_t i = 0; i < vcrs->getNumBattles(); ++i) {
                game::vcr::classic::Battle* en = vcrs->getBattle(i);
                if (en == 0) {
                    std::cout << "Record #" << (i+1) << " does not exist?\n";
                } else {
                    std::cout << "Record #" << (i+1) << ":\n";

                    // Create algo
                    game::vcr::classic::NullVisualizer vis;
                    std::auto_ptr<game::vcr::classic::Algorithm> algo(en->createAlgorithm(vis, root->hostConfiguration(), shipList));

                    // Prepare capabilities
                    if (!algo->setCapabilities(en->getCapabilities())) {
                        std::cout << "\tinvalid (cap)\n";
                        continue;
                    }

                    // Prepare VCR
                    game::vcr::Object left(*en->getObject(0, false));
                    game::vcr::Object right(*en->getObject(1, false));
                    uint16_t seed(en->getSeed());
                    if (algo->checkBattle(left, right, seed)) {
                        std::cout << "\tinvalid (content)\n";
                        continue;
                    }

                    // Play it
                    algo->playBattle(left, right, seed);

                    // Fetch result
                    algo->doneBattle(left, right);
                    game::vcr::classic::BattleResult_t result = algo->getResult();
                    game::vcr::classic::Time_t time = algo->getTime();

                    std::cout << "\tEnding time " << time
                              << " (" << time/60 << ":" << std::setw(2) << std::setfill('0') << time%60
                              << ")\n";
                    if (result.contains(game::vcr::classic::Invalid)) {
                        std::cout << "\tinvalid (play)\n";
                    } else {
                        char t = '\t';
                        if (result.contains(game::vcr::classic::LeftDestroyed)) {
                            std::cout << t << "left-destroyed";
                            t = ' ';
                        }
                        if (result.contains(game::vcr::classic::RightDestroyed)) {
                            std::cout << t << "right-destroyed";
                            t = ' ';
                        }
                        if (result.contains(game::vcr::classic::LeftCaptured)) {
                            std::cout << t << "left-captured";
                            t = ' ';
                        }
                        if (result.contains(game::vcr::classic::RightCaptured)) {
                            std::cout << t << "right-captured";
                            t = ' ';
                        }
                        if (result.contains(game::vcr::classic::Timeout)) {
                            std::cout << t << "timeout";
                            t = ' ';
                        }
                        if (t == '\t') {
                            std::cout << "\tnone\n";
                        } else {
                            std::cout << "\n";
                        }
#define VALUE(v,x) "  " << v << ":" << std::setw(3) << x
#define AMMO(t) (t.getTorpedoType() ? t.getNumTorpedoes() : t.getNumFighters())
                        std::cout << std::setfill(' ')
                                  << VALUE("S", left.getShield())
                                  << VALUE("D", left.getDamage())
                                  << VALUE("C", left.getCrew())
                                  << VALUE("A", AMMO(left))
                                  << "   |   "
                                  << VALUE("S", right.getShield())
                                  << VALUE("D", right.getDamage())
                                  << VALUE("C", right.getCrew())
                                  << VALUE("A", AMMO(right))
                                  << "\n";
#undef AMMO
#undef VALUE
                    }
                }
            }
        }
    }
    catch (afl::except::FileProblemException& e) {
        std::cout << "Exception: " << e.getFileName() << ": " << e.what() << "\n";
    }
    catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
}
