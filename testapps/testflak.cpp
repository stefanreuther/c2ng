/**
  *  \file testapps/testflak.cpp
  */

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/multidirectory.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/config/configurationparser.hpp"
#include "game/spec/shiplist.hpp"
#include "game/v3/specificationloader.hpp"
#include "game/vcr/flak/battle.hpp"
#include "game/vcr/flak/structures.hpp"
#include "game/vcr/flak/gameenvironment.hpp"
#include "afl/except/fileformatexception.hpp"
#include "game/vcr/flak/nullvisualizer.hpp"
#include "game/vcr/flak/algorithm.hpp"
#include "afl/charset/utf8charset.hpp"

namespace {
    const char* filename;
    const char* progname;

    void help()
    {
        std::fprintf(stderr, "usage: %s FILE [GAMEDIR [ROOTDIR]]", progname);
        std::exit(1);
    }

    void play(game::vcr::flak::Algorithm& b, const game::vcr::flak::Setup& s, const game::vcr::flak::Environment& env)
    {
        game::vcr::flak::NullVisualizer vis;
        std::printf("  Time according to header: %7d\n", int(s.getTotalTime()));
        b.init(env, vis);
        while (b.playCycle(env, vis))
            ;
        std::printf("  Real time taken:          %7d\n", b.getTime());
        for (size_t i = 0; i < b.getNumShips(); ++i) {
            std::printf("    Unit %3d (%-6s #%-3d): damage %3d, crew %4d, shield %3d, torps %3d, fighters %3d\n",
                        static_cast<int>(i),
                        b.isPlanet(i) ? "planet" : "ship",
                        b.getShipId(i),
                        b.getDamage(i),
                        b.getCrew(i),
                        b.getShield(i),
                        b.getNumTorpedoes(i),
                        b.getNumFighters(i));
        }
    }
}

int main(int /*argc*/, char** argv)
{
    // ex flak/flak-play.cc:main
    const char* gGameDirectory = ".";
    const char* gRootDirectory = ".";

    int repeat = 0;
    bool had_gamedir = false, had_rootdir = false;
    progname = argv[0];
    while (const char* p = std::strpbrk(progname, ":/\\")) {
        progname = p+1;
    }
    while (*++argv) {
        if (filename == 0) {
            filename = *argv;
        } else if (!had_gamedir) {
            gGameDirectory = *argv, had_gamedir = true;
        } else if (!had_rootdir) {
            gRootDirectory = *argv, had_rootdir = true;
        } else if (repeat == 0 && (repeat = atoi(*argv)) != 0) {
            // ok
        } else {
            help();
        }
    }
    if (!filename) {
        help();
    }

    // c2ng environment:
    afl::io::FileSystem& fs = afl::io::FileSystem::getInstance();
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::base::Ref<afl::io::MultiDirectory> specDir = afl::io::MultiDirectory::create();
    specDir->addDirectory(fs.openDirectory(gGameDirectory));
    specDir->addDirectory(fs.openDirectory(gRootDirectory));
    std::auto_ptr<afl::charset::Charset> charset(new afl::charset::CodepageCharset(afl::charset::g_codepageLatin1));
    
    // Spec:
    game::v3::SpecificationLoader specLoader(specDir, charset, tx, log);
    game::spec::ShipList list;
    specLoader.loadBeams(list, *specDir);
    specLoader.loadLaunchers(list, *specDir);

    // Config:
    game::config::HostConfiguration config;
    game::config::ConfigurationParser parser(log, tx, config, game::config::ConfigurationOption::Game);
    {
        afl::base::Ptr<afl::io::Stream> file = specDir->openFileNT("pconfig.src", afl::io::FileSystem::OpenRead);
        if (file.get() != 0) {
            parser.setSection("phost", true);
            parser.parseFile(*file);
        }
    }
    {
        afl::base::Ptr<afl::io::Stream> file = specDir->openFileNT("shiplist.txt", afl::io::FileSystem::OpenRead);
        if (file.get() != 0) {
            parser.setSection("phost", false);
            parser.parseFile(*file);
        }
    }
    game::vcr::flak::GameEnvironment env(config, list.beams(), list.launchers());

    try {
        /* Now read the input file */
        afl::base::Ref<afl::io::Stream> io = fs.openFile(filename, afl::io::FileSystem::OpenRead);

        game::vcr::flak::structures::Header header;
        io->fullRead(afl::base::fromObject(header));
        if (std::memcmp(header.magic, game::vcr::flak::structures::FLAK_MAGIC, sizeof(game::vcr::flak::structures::FLAK_MAGIC)) != 0) {
            throw afl::except::FileFormatException(*io, "File is missing required signature");
        }
        if (header.filefmt_version != 0) {
            throw afl::except::FileFormatException(*io, "Unsupported file format version");
        }

        if (repeat == 0) {
            repeat = 1;
        }
        for (int i = 0; i < header.num_battles; ++i) {
            /* read buffer */
            afl::base::GrowableBytes_t data;
            data.resize(4);
            io->fullRead(data);

            afl::bits::Value<afl::bits::UInt32LE> rawSize;
            afl::base::fromObject(rawSize).copyFrom(data);
            uint32_t size = rawSize;
           
            data.resize(size);
            io->fullRead(data.subrange(4));

            /* make battle */
            game::vcr::flak::Setup b;
            afl::charset::Utf8Charset cs; // FIXME
            b.load(filename, data, cs, tx);

            /* play it */
            std::printf("Battle %d...\n", i+1);
            for (int iter = 0; iter < repeat; ++iter) {
                game::vcr::flak::Algorithm algo(b, env);
                play(algo, b, env);
            }
        }
    }
    catch (std::exception& e) {
        std::fprintf(stderr, "Exception: %s\n", e.what());
        return 1;
    }
    catch (...) {
        std::fprintf(stderr, "Unknown exception\n");
        return 1;
    }
    return 0;
}
