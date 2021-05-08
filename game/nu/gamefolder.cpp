/**
  *  \file game/nu/gamefolder.cpp
  */

#include "game/nu/gamefolder.hpp"
#include "afl/string/format.hpp"
#include "game/nu/browserhandler.hpp"
#include "afl/io/internaldirectory.hpp"
#include "game/nu/specificationloader.hpp"
#include "game/nu/gamestate.hpp"
#include "game/hostversion.hpp"
#include "game/nu/registrationkey.hpp"
#include "game/nu/stringverifier.hpp"
#include "game/nu/turnloader.hpp"
#include "afl/charset/utf8charset.hpp"
#include "util/translation.hpp"

namespace {
    using afl::string::Format;

    const char* LOG_NAME = "game.nu";
}

game::nu::GameFolder::GameFolder(BrowserHandler& handler,
                                 game::browser::Account& acc,
                                 int32_t gameNr,
                                 size_t hint)
    : m_handler(handler),
      m_account(acc),
      m_gameNr(gameNr),
      m_state(*new GameState(handler, acc, gameNr, hint))
{ }

void
game::nu::GameFolder::loadContent(afl::container::PtrVector<Folder>& /*result*/)
{
    // Nothing to load, there are no subfolders
}

bool
game::nu::GameFolder::loadConfiguration(game::config::UserConfiguration& config)
{
    using game::config::UserConfiguration;
    if (const String_t* pFolderName = getGameFolderName()) {
        game::browser::Browser& b = m_handler.browser();
        config.loadGameConfiguration(*b.fileSystem().openDirectory(b.expandGameDirectoryName(*pFolderName)), b.log(), b.translator());
    }
    config[UserConfiguration::Game_Type].set(m_account.getType());
    config[UserConfiguration::Game_Type].setSource(game::config::ConfigurationOption::Game);
    config[UserConfiguration::Game_User].set(m_account.getUser());
    config[UserConfiguration::Game_User].setSource(game::config::ConfigurationOption::Game);
    config[UserConfiguration::Game_Host].set(m_account.getHost());
    config[UserConfiguration::Game_Host].setSource(game::config::ConfigurationOption::Game);
    config[UserConfiguration::Game_Id].set(getGameIdAsString());
    config[UserConfiguration::Game_Id].setSource(game::config::ConfigurationOption::Game);
    config[UserConfiguration::Game_Finished].set(m_state->loadGameListEntry()("game")("status").toInteger() == 3);
    config[UserConfiguration::Game_Finished].setSource(game::config::ConfigurationOption::Game);
    return true;
}

void
game::nu::GameFolder::saveConfiguration(const game::config::UserConfiguration& config)
{
    using game::config::UserConfiguration;
    if (const String_t* pFolderName = getGameFolderName()) {
        game::browser::Browser& b = m_handler.browser();
        config.saveGameConfiguration(*b.fileSystem().openDirectory(b.expandGameDirectoryName(*pFolderName)), b.log(), b.translator());
    }
}

bool
game::nu::GameFolder::setLocalDirectoryName(String_t directoryName)
{
    if (directoryName.empty()) {
        m_account.removeGameFolderName(getGameIdAsString());
    } else {
        m_account.setGameFolderName(getGameIdAsString(), directoryName);
    }
    return true;
}

afl::base::Ptr<game::Root>
game::nu::GameFolder::loadGameRoot(const game::config::UserConfiguration& config)
{
    // Current data
    afl::data::Access a = m_state->loadGameListEntry();

    // Actions
    Root::Actions_t actions;

    // Game directory
    afl::base::Ptr<afl::io::Directory> dir;
    if (const String_t* pFolderName = getGameFolderName()) {
        try {
            afl::base::Ref<afl::io::Directory> p = m_handler.browser().fileSystem().openDirectory(m_handler.browser().expandGameDirectoryName(*pFolderName));
            p->getDirectoryEntries();
            dir = p.asPtr();
            actions += Root::aLoadEditable;
        }
        catch (std::exception& e) {
            m_handler.log().write(afl::sys::Log::Warn, LOG_NAME, _("Game directory lost"), e);
            m_account.removeGameFolderName(getGameIdAsString());
        }
    }
    if (dir.get() == 0) {
        dir = afl::io::InternalDirectory::create("<Internal>").asPtr();
    }
    actions += Root::aLocalSetup;
    actions += Root::aConfigureReadOnly;

    // Root
    afl::base::Ptr<Root> root = new Root(*dir,
                                         *new SpecificationLoader(m_state, m_handler.translator(), m_handler.log()),
                                         HostVersion(HostVersion::NuHost, MKVERSION(3,2,0)),
                                         std::auto_ptr<game::RegistrationKey>(new RegistrationKey(a("player"))),
                                         std::auto_ptr<game::StringVerifier>(new StringVerifier()),
                                         std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()),
                                         actions);

    // FIXME -> root->userConfiguration().loadUserConfiguration(m_profile, m_log, m_translator);
    root->userConfiguration().merge(config);

    // FIXME:
    // + hostConfiguration()

    // Player list: from the game list entry, we know
    // - how many players there are (.game.slots)
    // - the player's slot (.player.id)
    // - the player's race (.player.raceid)
    PlayerList& players = root->playerList();
    int thisPlayer = a("player")("id").toInteger();
    int thisRace   = a("player")("raceid").toInteger();
    for (int player = 1, n = a("game")("slots").toInteger(); player <= n; ++player) {
        if (Player* pl = players.create(player)) {
            if (player == thisPlayer && GameState::setRaceName(*pl, thisRace)) {
                // ok
            } else {
                String_t pseudo = afl::string::Format("#%d", player);
                pl->setName(Player::LongName, pseudo);
                pl->setName(Player::ShortName, pseudo);
                pl->setName(Player::AdjectiveName, pseudo);
            }
            pl->setOriginalNames();
            pl->setIsReal(true);
        }
    }

    // Turn loader
    root->setTurnLoader(new TurnLoader(m_state, m_handler.translator(), m_handler.log(), m_handler.browser().profile(), m_handler.getDefaultSpecificationDirectory()));
    
    // + playerList
    return root;
}

String_t
game::nu::GameFolder::getName() const
{
    afl::data::Access a = m_state->loadGameListEntry();
    return afl::string::Format("%s (%d)", a("game")("name").toString(), a("game")("id").toInteger());
}

util::rich::Text
game::nu::GameFolder::getDescription() const
{
    return m_state->loadGameListEntry()("game")("description").toString();
}

bool
game::nu::GameFolder::isSame(const Folder& other) const
{
    const GameFolder* p = dynamic_cast<const GameFolder*>(&other);
    return p != 0
        && &p->m_account == &m_account
        && p->m_gameNr == m_gameNr;
}

bool
game::nu::GameFolder::canEnter() const
{
    return false;
}

game::browser::Folder::Kind
game::nu::GameFolder::getKind() const
{
    return kGame;
}

const String_t*
game::nu::GameFolder::getGameFolderName() const
{
    return m_account.getGameFolderName(getGameIdAsString());
}

String_t
game::nu::GameFolder::getGameIdAsString() const
{
    return Format("%d", m_gameNr);
}
