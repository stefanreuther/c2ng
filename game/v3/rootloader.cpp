/**
  *  \file game/v3/rootloader.cpp
  */

#include "game/v3/rootloader.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/io/multidirectory.hpp"
#include "afl/string/format.hpp"
#include "game/config/configurationparser.hpp"
#include "game/v3/registrationkey.hpp"
#include "game/v3/specificationloader.hpp"
#include "game/v3/resultloader.hpp"
#include "game/v3/stringverifier.hpp"
#include "game/v3/hconfig.hpp"

namespace gt = game::v3::structures;
using afl::io::FileSystem;
using game::config::ConfigurationOption;

namespace {
    const int DEFAULT_PHOST_VERSION = MKVERSION(4,1,0);
    const int DEFAULT_HOST_VERSION = MKVERSION(3,22,26);

    const char LOG_NAME[] = "game.v3.rootloader";
}

game::v3::RootLoader::RootLoader(afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory,
                                 util::ProfileDirectory& profile,
                                 afl::string::Translator& tx,
                                 afl::sys::LogListener& log,
                                 afl::io::FileSystem& fs)
    : m_defaultSpecificationDirectory(defaultSpecificationDirectory),
      m_profile(profile),
      m_translator(tx),
      m_log(log),
      m_fileSystem(fs),
      m_scanner(*m_defaultSpecificationDirectory, tx, log),
      m_charset(new afl::charset::CodepageCharset(afl::charset::g_codepageLatin1))
{ }

void
game::v3::RootLoader::setCharsetNew(afl::charset::Charset* p)
{
    if (p) {
        m_charset.reset(p);
    }
}

afl::base::Ptr<game::Root>
game::v3::RootLoader::load(afl::base::Ref<afl::io::Directory> gameDirectory, bool forceEmpty)
{
    m_scanner.clear();
    m_scanner.scan(*gameDirectory, *m_charset);

    afl::base::Ptr<Root> result;
    if (!m_scanner.getDirectoryFlags().empty() || forceEmpty) {
        // Specification directory
        afl::base::Ref<afl::io::MultiDirectory> spec = afl::io::MultiDirectory::create();
        spec->addDirectory(gameDirectory);
        spec->addDirectory(m_defaultSpecificationDirectory);

        // Registration key
        std::auto_ptr<RegistrationKey> key(new RegistrationKey(*m_charset));
        key->initFromDirectory(*gameDirectory, m_log);

        // Specification loader
        afl::base::Ref<SpecificationLoader> specLoader(*new SpecificationLoader(*m_charset, m_translator, m_log));

        // Produce result
        result = new Root(spec, gameDirectory, specLoader,
                          m_scanner.getDirectoryHostVersion(),
                          std::auto_ptr<game::RegistrationKey>(key),
                          std::auto_ptr<game::StringVerifier>(new StringVerifier(std::auto_ptr<afl::charset::Charset>(m_charset->clone()))));

        // Configuration
        loadConfiguration(*result);
        loadRaceNames(result->playerList(), *spec);

        // Preferences
        result->userConfiguration().loadUserConfiguration(m_profile, m_log, m_translator);
        result->userConfiguration().loadGameConfiguration(*gameDirectory, m_log, m_translator);

        // Turn loader
        if (m_scanner.getDirectoryFlags().contains(DirectoryScanner::HaveResult)) {
            result->setTurnLoader(new ResultLoader(*m_charset, m_translator, m_log, m_scanner, m_fileSystem));
        }
    }
    return result;
}

void
game::v3::RootLoader::loadConfiguration(Root& root)
{
    // ex game/config.cc:initConfig
    game::config::HostConfiguration& config = root.hostConfiguration();
    config.setDefaultValues();

    // Check pconfig.src
    // FIXME: do we really want to load these from specificationDirectory()?
    afl::base::Ptr<afl::io::Stream> file = root.specificationDirectory().openFileNT("pconfig.src", FileSystem::OpenRead);
    if (file.get() != 0) {
        // OK, PHost
        loadPConfig(root,
                    file,
                    root.specificationDirectory().openFileNT("shiplist.txt", FileSystem::OpenRead),
                    ConfigurationOption::Game);
    } else {
        // SRace
        file = root.gameDirectory().openFileNT("friday.dat", FileSystem::OpenRead);
        if (file.get() != 0) {
            loadRaceMapping(root, *file, ConfigurationOption::Game);
        }

        // Regular host config
        file = root.specificationDirectory().openFileNT("hconfig.hst", FileSystem::OpenRead);
        if (file.get() != 0) {
            loadHConfig(root, *file, ConfigurationOption::Game);
        } else {
            m_log.write(m_log.Warn, LOG_NAME, m_translator.translateString("No host configuration file found, using defaults"));
        }
    }

    // Set Tim-Host defaults
    // FIXME: this belongs either in HostConfiguration, or in HostVersion
    HostVersion& host = root.hostVersion();
    if (host.getKind() != HostVersion::Host) {
        config[config.RoundGravityWells].set(1);
        config[config.CPEnableRemote].set(0);
    }
}

/** Load PCONFIG.SRC.
    \param pconf pconfig.src file
    \param shiplist shiplist.txt file, may be null. */
void
game::v3::RootLoader::loadPConfig(Root& root,
                                  afl::base::Ptr<afl::io::Stream> pconfig,
                                  afl::base::Ptr<afl::io::Stream> shiplist,
                                  game::config::ConfigurationOption::Source source)
{
    // ex game/config.cc:loadPConfig
    // Configure parser
    game::config::ConfigurationParser parser(m_log, root.hostConfiguration(), source);
    parser.setCharsetNew(m_charset->clone());

    // Load pconfig.src (mandatory)
    m_log.write(m_log.Info, LOG_NAME, afl::string::Format(m_translator.translateString("Reading configuration from %s...").c_str(), pconfig->getName()));
    parser.setSection("phost", true);
    parser.parseFile(*pconfig);

    // Load shiplist.txt (optional)
    if (shiplist.get() != 0) {
        m_log.write(m_log.Info, LOG_NAME, afl::string::Format(m_translator.translateString("Reading configuration from %s...").c_str(), shiplist->getName()));
        parser.setSection("phost", false);
        parser.parseFile(*shiplist);
    }

    // Postprocess
    root.hostConfiguration().setDependantOptions();

    // Update host version guess
    HostVersion& host = root.hostVersion();
    if (host.getKind() == HostVersion::Unknown) {
        host.set(HostVersion::PHost, DEFAULT_PHOST_VERSION);
        m_log.write(m_log.Info, LOG_NAME, afl::string::Format(m_translator.translateString("Host version not known, assuming %s").c_str(), host.toString(m_translator)));
    }
}

void
game::v3::RootLoader::loadHConfig(Root& root,
                                  afl::io::Stream& hconfig,
                                  game::config::ConfigurationOption::Source source)
{
    // ex game/config.cc:loadHConfig, Config::assignFromHConfigImage
    // FIXME: do host version guessing in this function
    if (hconfig.getSize() > 10*sizeof(gt::HConfig)) {
        // FIXME: log only?
        throw afl::except::FileFormatException(hconfig, m_translator.translateString("File has invalid size"));
    }

    // Read hconfig
    m_log.write(m_log.Info, LOG_NAME, afl::string::Format(m_translator.translateString("Reading configuration from %s...").c_str(), hconfig.getName()));

    gt::HConfig image;
    size_t size = hconfig.read(afl::base::fromObject(image));
    unpackHConfig(image, size, root.hostConfiguration(), source);

    // Postprocess
    root.hostConfiguration().setDependantOptions();

    // Update host version guess
    HostVersion& host = root.hostVersion();
    if (host.getKind() == HostVersion::Unknown) {
        host.set(HostVersion::Host, DEFAULT_HOST_VERSION);
        m_log.write(m_log.Info, LOG_NAME, afl::string::Format(m_translator.translateString("Host version not known, assuming %s").c_str(), host.toString(m_translator)));
    }
}

void
game::v3::RootLoader::loadRaceMapping(Root& root, afl::io::Stream& file, game::config::ConfigurationOption::Source source)
{
    gt::Int16_t mapping[gt::NUM_PLAYERS];
    if (file.read(afl::base::fromObject(mapping)) == sizeof(mapping)) {
        // Load configuration option
        game::config::HostConfiguration& config = root.hostConfiguration();
        for (int i = 1; i <= gt::NUM_PLAYERS; ++i) {
            config[config.PlayerRace].set(i, mapping[i-1]);
        }
        config[config.PlayerSpecialMission].copyFrom(config[config.PlayerRace]);
        config[config.PlayerRace].setSource(source);
        config[config.PlayerSpecialMission].setSource(source);

        // Update host version guess
        HostVersion& host = root.hostVersion();
        if (host.getKind() == HostVersion::Unknown) {
            host.set(HostVersion::SRace, DEFAULT_HOST_VERSION);
            m_log.write(m_log.Info, LOG_NAME, afl::string::Format(m_translator.translateString("Host version not known, assuming %s").c_str(), host.toString(m_translator)));
        }
    }
}

void
game::v3::RootLoader::loadRaceNames(PlayerList& list, afl::io::Directory& dir)
{
    // ex GRaceNameList::load
    list.clear();

    // Load the file
    afl::base::Ref<afl::io::Stream> file = dir.openFile("race.nm", FileSystem::OpenRead);
    gt::RaceNames in;
    file->fullRead(afl::base::fromObject(in));
    for (int player = 0; player < gt::NUM_PLAYERS; ++player) {
        if (Player* out = list.create(player+1)) {
            out->setName(Player::ShortName,     m_charset->decode(in.shortNames[player]));
            out->setName(Player::LongName,      m_charset->decode(in.longNames[player]));
            out->setName(Player::AdjectiveName, m_charset->decode(in.adjectiveNames[player]));
            out->setOriginalNames();
        }
    }

    // Create aliens
    if (Player* aliens = list.create(gt::NUM_PLAYERS+1)) {
        aliens->initAlien();
    }
}

// FIXME: delete?
// /** Save race names to specified 'race.nm' file. */
// void
// GRaceNameList::save(Stream& s) const
// {
//     char buffer[RN_FILE_SIZE];
//     for(int i = 1; i <= 11; ++i) {
//         storeBasicStringN(&buffer[RN_FULL_ORIG  + 30*(i-1)], 30, convertUtf8ToGame(full_names[i]));
//         storeBasicStringN(&buffer[RN_SHORT_ORIG + 20*(i-1)], 20, convertUtf8ToGame(short_names[i]));
//         storeBasicStringN(&buffer[RN_ADJ_ORIG   + 12*(i-1)], 12, convertUtf8ToGame(adjectives[i]));
//     }
//     s.writeT(buffer, sizeof(buffer));
// }
