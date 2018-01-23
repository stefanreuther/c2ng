/**
  *  \file game/root.cpp
  *  \brief Class game::Root
  */

#include "game/root.hpp"
#include "game/registrationkey.hpp"
#include "game/specificationloader.hpp"
#include "game/stringverifier.hpp"
#include "game/turnloader.hpp"

// Constructor.
game::Root::Root(afl::base::Ref<afl::io::Directory> specificationDirectory,
                 afl::base::Ref<afl::io::Directory> gameDirectory,
                 afl::base::Ref<SpecificationLoader> specLoader,
                 game::HostVersion hostVersion,
                 std::auto_ptr<game::RegistrationKey> registrationKey,
                 std::auto_ptr<StringVerifier> stringVerifier)
    : m_specificationDirectory(specificationDirectory),
      m_gameDirectory(gameDirectory),
      m_specificationLoader(specLoader),
      m_hostVersion(hostVersion),
      m_hostConfiguration(),
      m_userConfiguration(),
      m_playerList(),
      m_registrationKey(registrationKey),
      m_stringVerifier(stringVerifier),
      m_turnLoader()
{ }

// Destructor.
game::Root::~Root()
{ }

// Access specification directory.
afl::io::Directory&
game::Root::specificationDirectory() const
{
    return *m_specificationDirectory;
}

// Access game directory.
afl::io::Directory&
game::Root::gameDirectory() const
{
    return *m_gameDirectory;
}

// Access specification loader.
game::SpecificationLoader&
game::Root::specificationLoader()
{
    return *m_specificationLoader;
}

// Access host version.
game::HostVersion&
game::Root::hostVersion()
{
    return m_hostVersion;
}

// Access host version.
const game::HostVersion&
game::Root::hostVersion() const
{
    return m_hostVersion;
}

// Access host configuration.
game::config::HostConfiguration&
game::Root::hostConfiguration()
{
    return m_hostConfiguration;
}

const game::config::HostConfiguration&
game::Root::hostConfiguration() const
{
    return m_hostConfiguration;
}

// Access user configuration (preferences).
game::config::UserConfiguration&
game::Root::userConfiguration()
{
    return m_userConfiguration;
}

const game::config::UserConfiguration&
game::Root::userConfiguration() const
{
    return m_userConfiguration;
}

// Access player list.
game::PlayerList&
game::Root::playerList()
{
    return m_playerList;
}

const game::PlayerList&
game::Root::playerList() const
{
    return m_playerList;
}

// Access registration status.
game::RegistrationKey&
game::Root::registrationKey()
{
    return *m_registrationKey;
}

const game::RegistrationKey&
game::Root::registrationKey() const
{
    return *m_registrationKey;
}

// Access string verifier.
game::StringVerifier&
game::Root::stringVerifier()
{
    return *m_stringVerifier;
}

// Set turn loader
void
game::Root::setTurnLoader(afl::base::Ptr<TurnLoader> turnLoader)
{
    m_turnLoader = turnLoader;
}

// Get turn loader.
afl::base::Ptr<game::TurnLoader>
game::Root::getTurnLoader() const
{
    return m_turnLoader;
}

// Notify listeners.
void
game::Root::notifyListeners()
{
    // m_specificationDirectory: does not change
    // m_gameDirectory: does not change
    // m_specificationLoader: does not change

    // m_hostVersion: ?

    m_hostConfiguration.notifyListeners();
    m_userConfiguration.notifyListeners();
    m_playerList.notifyListeners();

    // m_registrationKey: ?

    // m_turnLoader: does not have user-visible properties
}
