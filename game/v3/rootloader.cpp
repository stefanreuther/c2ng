/**
  *  \file game/v3/rootloader.cpp
  */

#include "game/v3/rootloader.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/io/multidirectory.hpp"
#include "afl/string/format.hpp"
#include "game/config/configurationparser.hpp"
#include "game/v3/registrationkey.hpp"
#include "game/v3/specificationloader.hpp"
#include "game/v3/resultloader.hpp"
#include "game/v3/stringverifier.hpp"
#include "game/v3/hconfig.hpp"
#include "game/v3/utils.hpp"
#include "game/v3/directoryloader.hpp"

namespace gt = game::v3::structures;
using afl::io::FileSystem;
using game::config::ConfigurationOption;

namespace {
    const int DEFAULT_PHOST_VERSION = MKVERSION(4,1,0);
    const int DEFAULT_HOST_VERSION = MKVERSION(3,22,26);

    const char LOG_NAME[] = "game.v3.rootloader";
}

game::v3::RootLoader::RootLoader(afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory,
                                 util::ProfileDirectory* pProfile,
                                 afl::string::Translator& tx,
                                 afl::sys::LogListener& log,
                                 afl::io::FileSystem& fs)
    : m_defaultSpecificationDirectory(defaultSpecificationDirectory),
      m_pProfile(pProfile),
      m_translator(tx),
      m_log(log),
      m_fileSystem(fs),
      m_scanner(*m_defaultSpecificationDirectory, tx, log)
{ }

afl::base::Ptr<game::Root>
game::v3::RootLoader::load(afl::base::Ref<afl::io::Directory> gameDirectory,
                           afl::charset::Charset& charset,
                           const game::config::UserConfiguration& config,
                           bool forceEmpty)
{
    m_scanner.clear();
    m_scanner.scan(*gameDirectory, charset);

    afl::base::Ptr<Root> result;
    if (!m_scanner.getDirectoryFlags().empty() || forceEmpty) {
        // Specification directory
        afl::base::Ref<afl::io::MultiDirectory> spec = afl::io::MultiDirectory::create();
        spec->addDirectory(gameDirectory);
        spec->addDirectory(m_defaultSpecificationDirectory);

        // Registration key
        std::auto_ptr<RegistrationKey> key(new RegistrationKey(std::auto_ptr<afl::charset::Charset>(charset.clone())));
        key->initFromDirectory(*gameDirectory, m_log, m_translator);

        // Specification loader
        afl::base::Ref<SpecificationLoader> specLoader(*new SpecificationLoader(spec, std::auto_ptr<afl::charset::Charset>(charset.clone()), m_translator, m_log));

        // Actions
        Root::Actions_t actions;
        actions += Root::aLoadEditable;
        actions += Root::aConfigureCharset;
        actions += Root::aConfigureFinished;
        actions += Root::aConfigureReadOnly;
        actions += Root::aSweep;
        if (m_scanner.getDirectoryFlags().containsAnyOf(DirectoryScanner::PlayerFlags_t() + DirectoryScanner::HaveResult + DirectoryScanner::HaveNewResult + DirectoryScanner::HaveOtherResult)) {
            // FIXME: add a bit "suggest unpack" for HaveNewResult?
            actions += Root::aUnpack;
        }
        if (m_scanner.getDirectoryFlags().contains(DirectoryScanner::HaveUnpacked)) {
            actions += Root::aMaketurn;
        }

        // Produce result
        result = new Root(gameDirectory, specLoader,
                          m_scanner.getDirectoryHostVersion(),
                          std::auto_ptr<game::RegistrationKey>(key),
                          std::auto_ptr<game::StringVerifier>(new StringVerifier(std::auto_ptr<afl::charset::Charset>(charset.clone()))),
                          std::auto_ptr<afl::charset::Charset>(charset.clone()),
                          Root::Actions_t(actions));

        // Configuration
        loadConfiguration(*result, *spec, charset);
        loadRaceNames(result->playerList(), *spec, charset);

        // Preferences
        if (m_pProfile != 0) {
            result->userConfiguration().loadUserConfiguration(*m_pProfile, m_log, m_translator);
        }
        result->userConfiguration().merge(config);

        // Turn loader
        if (m_scanner.getDirectoryFlags().contains(DirectoryScanner::HaveUnpacked)) {
            result->setTurnLoader(new DirectoryLoader(spec, m_defaultSpecificationDirectory, std::auto_ptr<afl::charset::Charset>(charset.clone()), m_translator, m_log, m_scanner, m_fileSystem, m_pProfile));
        } else if (m_scanner.getDirectoryFlags().contains(DirectoryScanner::HaveResult)) {
            result->setTurnLoader(new ResultLoader(spec, m_defaultSpecificationDirectory, std::auto_ptr<afl::charset::Charset>(charset.clone()), m_translator, m_log, m_scanner, m_fileSystem, m_pProfile));
        } else {
            // nothing loadable
        }
    }
    return result;
}

void
game::v3::RootLoader::loadConfiguration(Root& root, afl::io::Directory& dir, afl::charset::Charset& charset)
{
    // ex game/config.cc:initConfig
    game::config::HostConfiguration& config = root.hostConfiguration();
    config.setDefaultValues();

    // FIXME: PCC1 shows warning if fewer than 70 pconfig keys
    // FIXME: PCC1 shows warning if both PCONFIG.SRC and FRIDAY.DAT

    // Check pconfig.src
    // FIXME: do we really want to load these from specificationDirectory()?
    afl::base::Ptr<afl::io::Stream> file = dir.openFileNT("pconfig.src", FileSystem::OpenRead);
    if (file.get() != 0) {
        // OK, PHost
        loadPConfig(root,
                    file,
                    dir.openFileNT("shiplist.txt", FileSystem::OpenRead),
                    ConfigurationOption::Game,
                    charset);
    } else {
        // SRace
        file = root.gameDirectory().openFileNT("friday.dat", FileSystem::OpenRead);
        if (file.get() != 0) {
            loadRaceMapping(root, *file, ConfigurationOption::Game);
        }

        // Regular host config
        file = dir.openFileNT("hconfig.hst", FileSystem::OpenRead);
        if (file.get() != 0) {
            loadHConfig(root, *file, ConfigurationOption::Game);
        } else {
            m_log.write(m_log.Warn, LOG_NAME, m_translator.translateString("No host configuration file found, using defaults"));
        }
    }

    root.hostVersion().setImpliedHostConfiguration(config);

    // FLAK
    game::vcr::flak::loadConfiguration(root.flakConfiguration(), dir, m_log, m_translator);
}

/** Load PCONFIG.SRC.
    \param pconf pconfig.src file
    \param shiplist shiplist.txt file, may be null. */
void
game::v3::RootLoader::loadPConfig(Root& root,
                                  afl::base::Ptr<afl::io::Stream> pconfig,
                                  afl::base::Ptr<afl::io::Stream> shiplist,
                                  game::config::ConfigurationOption::Source source,
                                  afl::charset::Charset& charset)
{
    // ex game/config.cc:loadPConfig
    // Configure parser
    game::config::ConfigurationParser parser(m_log, m_translator, root.hostConfiguration(), source);
    parser.setCharsetNew(charset.clone());

    // Load pconfig.src (mandatory)
    m_log.write(m_log.Info, LOG_NAME, afl::string::Format(m_translator("Reading configuration from %s..."), pconfig->getName()));
    parser.setSection("phost", true);
    parser.parseFile(*pconfig);

    // Load shiplist.txt (optional)
    if (shiplist.get() != 0) {
        m_log.write(m_log.Info, LOG_NAME, afl::string::Format(m_translator("Reading configuration from %s..."), shiplist->getName()));
        parser.setSection("phost", false);
        parser.parseFile(*shiplist);
    }

    // Postprocess
    root.hostConfiguration().setDependantOptions();

    // Update host version guess
    HostVersion& host = root.hostVersion();
    if (host.getKind() == HostVersion::Unknown) {
        host.set(HostVersion::PHost, DEFAULT_PHOST_VERSION);
        m_log.write(m_log.Info, LOG_NAME, afl::string::Format(m_translator("Host version not known, assuming %s"), host.toString(m_translator)));
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
