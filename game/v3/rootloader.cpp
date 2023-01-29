/**
  *  \file game/v3/rootloader.cpp
  */

#include "game/v3/rootloader.hpp"
#include "afl/io/multidirectory.hpp"
#include "game/v3/directoryloader.hpp"
#include "game/v3/loader.hpp"
#include "game/v3/registrationkey.hpp"
#include "game/v3/resultloader.hpp"
#include "game/v3/specificationloader.hpp"
#include "game/v3/stringverifier.hpp"
#include "game/v3/utils.hpp"

game::v3::RootLoader::RootLoader(afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory,
                                 util::ProfileDirectory* pProfile,
                                 game::browser::UserCallback* pCallback,
                                 afl::string::Translator& tx,
                                 afl::sys::LogListener& log,
                                 afl::io::FileSystem& fs)
    : m_defaultSpecificationDirectory(defaultSpecificationDirectory),
      m_pProfile(pProfile),
      m_pCallback(pCallback),
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
    m_scanner.scan(*gameDirectory, charset, DirectoryScanner::UnpackedThenResult);

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
            actions += Root::aUnpack;
            if (m_scanner.getDirectoryFlags().contains(DirectoryScanner::HaveNewResult)) {
                actions += Root::aSuggestUnpack;
            }
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
        // @change PCC2 originally loaded some files (pconfig, hconfig) from the spec directory, others from game directory.
        // We now load everything from gameDirectory; there isn't supposed to be a config file in the system spec directory.
        // This is the same behaviour as PCC1, PHost.
        loadConfiguration(*result, *gameDirectory, charset);

        // Race names
        loadRaceNames(result->playerList(), *spec, charset);

        // Preferences
        if (m_pProfile != 0) {
            result->userConfiguration().loadUserConfiguration(*m_pProfile, m_log, m_translator);
        }
        result->userConfiguration().merge(config);

        // Turn loader
        if (m_scanner.getDirectoryFlags().contains(DirectoryScanner::HaveUnpacked)) {
            result->setTurnLoader(new DirectoryLoader(spec, m_defaultSpecificationDirectory, std::auto_ptr<afl::charset::Charset>(charset.clone()), m_translator, m_log, m_scanner, m_fileSystem, m_pProfile, m_pCallback));
        } else if (m_scanner.getDirectoryFlags().contains(DirectoryScanner::HaveResult)) {
            result->setTurnLoader(new ResultLoader(spec, m_defaultSpecificationDirectory, std::auto_ptr<afl::charset::Charset>(charset.clone()), m_translator, m_log, m_scanner, m_fileSystem, m_pProfile, m_pCallback));
        } else {
            // nothing loadable
        }
    }
    return result;
}

void
game::v3::RootLoader::loadConfiguration(Root& root, afl::io::Directory& dir, afl::charset::Charset& charset)
{
    Loader(charset, m_translator, m_log).loadConfiguration(root, dir);
}
