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
game::Root::Root(afl::base::Ref<afl::io::Directory> gameDirectory,
                 afl::base::Ref<SpecificationLoader> specLoader,
                 game::HostVersion hostVersion,
                 std::auto_ptr<game::RegistrationKey> registrationKey,
                 std::auto_ptr<StringVerifier> stringVerifier,
                 std::auto_ptr<afl::charset::Charset> charset,
                 Actions_t actions)
    : m_gameDirectory(gameDirectory),
      m_specificationLoader(specLoader),
      m_hostVersion(hostVersion),
      m_hostConfiguration(game::config::HostConfiguration::create()),
      m_flakConfiguration(),
      m_userConfiguration(game::config::UserConfiguration::create()),
      m_playerList(),
      m_registrationKey(registrationKey),
      m_stringVerifier(stringVerifier),
      m_charset(charset),
      m_turnLoader(),
      m_actions(actions)
{ }

// Destructor.
game::Root::~Root()
{ }

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

game::Root::Actions_t
game::Root::getPossibleActions() const
{
    return m_actions;
}

// Notify listeners.
void
game::Root::notifyListeners()
{
    // m_gameDirectory: does not change
    // m_specificationLoader: does not change

    // m_hostVersion: ?

    m_hostConfiguration->notifyListeners();
    m_userConfiguration->notifyListeners();
    m_playerList.notifyListeners();

    // m_registrationKey: ?

    // m_turnLoader: does not have user-visible properties
}
