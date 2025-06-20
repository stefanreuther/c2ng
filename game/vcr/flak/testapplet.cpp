/**
  *  \file game/vcr/flak/testapplet.cpp
  *  \brief Class game::vcr::flak::TestApplet
  */

#include "game/vcr/flak/testapplet.hpp"

#include "afl/base/optional.hpp"
#include "afl/charset/charset.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/io/multidirectory.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "game/config/configurationparser.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/spec/shiplist.hpp"
#include "game/v3/specificationloader.hpp"
#include "game/vcr/flak/algorithm.hpp"
#include "game/vcr/flak/gameenvironment.hpp"
#include "game/vcr/flak/nullvisualizer.hpp"
#include "game/vcr/flak/setup.hpp"
#include "game/vcr/flak/structures.hpp"

using afl::base::Optional;
using afl::base::Ptr;
using afl::base::Ref;
using afl::bits::UInt32LE;
using afl::bits::Value;
using afl::charset::Charset;
using afl::charset::CodepageCharset;
using afl::except::FileFormatException;
using afl::io::FileSystem;
using afl::io::MultiDirectory;
using afl::io::Stream;
using afl::io::TextWriter;
using afl::string::Format;
using afl::string::Translator;
using afl::sys::LogListener;
using game::config::ConfigurationOption;
using game::config::ConfigurationParser;
using game::config::HostConfiguration;
using game::spec::ShipList;

namespace st = game::vcr::flak::structures;

namespace {
    void help(util::Application& app)
    {
        app.errorOutput().writeLine("Usage: flak FILE [GAMEDIR [ROOTDIR [REPEAT]]]");
        app.exit(1);
    }

    void play(TextWriter& out, game::vcr::flak::Algorithm& b, const game::vcr::flak::Setup& s, const game::vcr::flak::Environment& env)
    {
        game::vcr::flak::NullVisualizer vis;
        out.writeLine(Format("  Time according to header: %7d", s.getTotalTime()));
        b.init(env, vis);
        while (b.playCycle(env, vis))
            ;
        out.writeLine(Format("  Real time taken:          %7d", b.getTime()));
        for (size_t i = 0; i < b.getNumShips(); ++i) {
            out.writeLine(Format("    Unit %3d (%-6s #%-3d): damage %3d, crew %4d, shield %3d, torps %3d, fighters %3d")
                          << i
                          << (b.isPlanet(i) ? "planet" : "ship")
                          << b.getShipId(i)
                          << b.getDamage(i)
                          << b.getCrew(i)
                          << b.getShield(i)
                          << b.getNumTorpedoes(i)
                          << b.getNumFighters(i));
        }
    }
}

int
game::vcr::flak::TestApplet::run(util::Application& app, afl::sys::Environment::CommandLine_t& cmdl)
{
    Optional<String_t> gGameDirectory;
    Optional<String_t> gRootDirectory;
    Optional<String_t> fileName;
    Optional<int> repeatOption;

    String_t it;
    while (cmdl.getNextElement(it)) {
        if (!fileName.isValid()) {
            fileName = it;
        } else if (!gGameDirectory.isValid()) {
            gGameDirectory = it;
        } else if (!gRootDirectory.isValid()) {
            gRootDirectory = it;
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
    if (!fileName.isValid()) {
        help(app);
    }

    // c2ng environment:
    FileSystem& fs = app.fileSystem();
    LogListener& log = app.log();
    Translator& tx = app.translator();
    Ref<MultiDirectory> specDir = MultiDirectory::create();
    specDir->addDirectory(fs.openDirectory(gGameDirectory.orElse(".")));
    specDir->addDirectory(fs.openDirectory(gRootDirectory.orElse(".")));
    std::auto_ptr<Charset> charset(new CodepageCharset(afl::charset::g_codepageLatin1));

    // Spec:
    game::v3::SpecificationLoader specLoader(specDir, charset, tx, log);
    ShipList list;
    specLoader.loadBeams(list, *specDir);
    specLoader.loadLaunchers(list, *specDir);

    // Config:
    HostConfiguration config;
    ConfigurationParser parser(log, tx, config, ConfigurationOption::Game);
    {
        Ptr<Stream> file = specDir->openFileNT("pconfig.src", FileSystem::OpenRead);
        if (file.get() != 0) {
            parser.setSection("phost", true);
            parser.parseFile(*file);
        }
    }
    {
        Ptr<Stream> file = specDir->openFileNT("shiplist.txt", FileSystem::OpenRead);
        if (file.get() != 0) {
            parser.setSection("phost", false);
            parser.parseFile(*file);
        }
    }
    GameEnvironment env(config, list.beams(), list.launchers());

    /* Now read the input file */
    Ref<Stream> io = fs.openFile(*fileName.get(), FileSystem::OpenRead);

    st::Header header;
    io->fullRead(afl::base::fromObject(header));
    if (std::memcmp(header.magic, st::FLAK_MAGIC, sizeof(st::FLAK_MAGIC)) != 0) {
        throw FileFormatException(*io, "File is missing required signature");
    }
    if (header.filefmt_version != 0) {
        throw FileFormatException(*io, "Unsupported file format version");
    }

    const int repeat = repeatOption.orElse(1);
    TextWriter& out = app.standardOutput();
    for (int i = 0; i < header.num_battles; ++i) {
        /* read buffer */
        afl::base::GrowableBytes_t data;
        data.resize(4);
        io->fullRead(data);

        Value<UInt32LE> rawSize;
        afl::base::fromObject(rawSize).copyFrom(data);
        uint32_t size = rawSize;

        data.resize(size);
        io->fullRead(data.subrange(4));

        /* make battle */
        Setup b;
        CodepageCharset cs(afl::charset::g_codepageLatin1);
        b.load(*fileName.get(), data, cs, tx);

        /* play it */
        out.writeLine(Format("Battle %d...", i+1));
        for (int iter = 0; iter < repeat; ++iter) {
            Algorithm algo(b, env);
            play(out, algo, b, env);
        }
    }
    return 0;
}
