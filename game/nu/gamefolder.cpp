/**
  *  \file game/nu/gamefolder.cpp
  *  \brief Class game::nu::GameFolder
  */

#include "game/nu/gamefolder.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/string/format.hpp"
#include "game/hostversion.hpp"
#include "game/nu/browserhandler.hpp"
#include "game/nu/gamestate.hpp"
#include "game/nu/registrationkey.hpp"
#include "game/nu/specificationloader.hpp"
#include "game/nu/stringverifier.hpp"
#include "game/nu/turnloader.hpp"

using afl::string::Format;
using afl::sys::LogListener;
using game::browser::Browser;
using game::config::ConfigurationOption;
using game::config::UserConfiguration;

namespace {
    const char*const LOG_NAME = "game.nu";

    String_t getGameIdAsString(int32_t gameNr)
    {
        return Format("%d", gameNr);
    }

    const String_t* getGameFolderName(const game::browser::Account& account, int32_t gameNr)
    {
        return account.getGameFolderName(getGameIdAsString(gameNr));
    }
}

game::nu::GameFolder::GameFolder(BrowserHandler& handler,
                                 const afl::base::Ref<game::browser::Account>& acc,
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
    if (const String_t* pFolderName = getGameFolderName()) {
        Browser& b = m_handler.browser();
        config.loadGameConfiguration(*b.fileSystem().openDirectory(b.expandGameDirectoryName(*pFolderName)), b.log(), b.translator());
    }
    config[UserConfiguration::Game_Type].set(m_account->getType());
    config[UserConfiguration::Game_Type].setSource(ConfigurationOption::Game);
    config[UserConfiguration::Game_User].set(m_account->getUser());
    config[UserConfiguration::Game_User].setSource(ConfigurationOption::Game);
    config[UserConfiguration::Game_Host].set(m_account->getHost());
    config[UserConfiguration::Game_Host].setSource(ConfigurationOption::Game);
    config[UserConfiguration::Game_Id].set(getGameIdAsString());
    config[UserConfiguration::Game_Id].setSource(ConfigurationOption::Game);
    config[UserConfiguration::Game_Finished].set(m_state->loadGameListEntryPreAuthenticated()("game")("status").toInteger() == 3);
    config[UserConfiguration::Game_Finished].setSource(ConfigurationOption::Game);
    return true;
}

void
game::nu::GameFolder::saveConfiguration(const game::config::UserConfiguration& config)
{
    if (const String_t* pFolderName = getGameFolderName()) {
        Browser& b = m_handler.browser();
        config.saveGameConfiguration(*b.fileSystem().openDirectory(b.expandGameDirectoryName(*pFolderName)), b.log(), b.translator());
    }
}

bool
game::nu::GameFolder::setLocalDirectoryName(String_t directoryName)
{
    m_account->setGameFolderName(getGameIdAsString(), directoryName);
    return true;
}

std::auto_ptr<game::Task_t>
game::nu::GameFolder::loadGameRoot(const game::config::UserConfiguration& config, std::auto_ptr<game::browser::LoadGameRootTask_t> then)
{
    class Task : public Task_t {
     public:
        Task(BrowserHandler& handler, const afl::base::Ref<game::browser::Account>& account, int32_t gameNr, afl::base::Ref<GameState> state,
             const UserConfiguration& config, std::auto_ptr<game::browser::LoadGameRootTask_t>& then)
            : m_handler(handler), m_account(account), m_gameNr(gameNr), m_state(state), m_config(config), m_then(then)
            { }
        virtual void call()
            {
                m_handler.log().write(LogListener::Trace, LOG_NAME, "Task: GameFolder.loadGameRoot");
                afl::base::Ptr<Root> result;
                try {
                    // Current data
                    afl::data::Access a = m_state->loadGameListEntryPreAuthenticated();
                    afl::data::Access ai = m_handler.getAccountInfoPreAuthenticated(m_account);

                    // Actions
                    Root::Actions_t actions;

                    // Game directory
                    afl::base::Ptr<afl::io::Directory> dir;
                    if (const String_t* pFolderName = ::getGameFolderName(*m_account, m_gameNr)) {
                        try {
                            afl::base::Ref<afl::io::Directory> p = m_handler.browser().fileSystem().openDirectory(m_handler.browser().expandGameDirectoryName(*pFolderName));
                            p->getDirectoryEntries();
                            dir = p.asPtr();
                            actions += Root::aLoadEditable;
                        }
                        catch (std::exception& e) {
                            m_handler.log().write(LogListener::Warn, LOG_NAME, m_handler.translator()("Game directory lost"), e);
                            m_account->setGameFolderName(::getGameIdAsString(m_gameNr), String_t());
                        }
                    }
                    if (dir.get() == 0) {
                        dir = afl::io::InternalDirectory::create("<Internal>").asPtr();
                    }
                    actions += Root::aLocalSetup;
                    actions += Root::aConfigureReadOnly;

                    // Root
                    afl::base::Ptr<Root> root =
                        new Root(*dir,
                                 *new SpecificationLoader(m_handler.getDefaultSpecificationDirectory(), m_state, m_handler.translator(), m_handler.log()),
                                 HostVersion(HostVersion::NuHost, MKVERSION(3,2,0)),
                                 std::auto_ptr<game::RegistrationKey>(new RegistrationKey(ai)),
                                 std::auto_ptr<game::StringVerifier>(new StringVerifier()),
                                 std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()),
                                 actions);

                    root->userConfiguration().loadUserConfiguration(m_handler.browser().profile(), m_handler.log(), m_handler.translator());
                    root->userConfiguration().merge(m_config);

                    // Host configuration loaded by SpecificationLoader

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
                    root->setTurnLoader(new TurnLoader(m_state, m_handler.browser().profile(), m_handler.getDefaultSpecificationDirectory()));

                    result = root;
                }
                catch (std::exception& e) {
                    m_handler.log().write(LogListener::Warn, LOG_NAME, String_t(), e);
                }
                m_then->call(result);
            }
     private:
        BrowserHandler& m_handler;
        afl::base::Ref<game::browser::Account> m_account;
        int32_t m_gameNr;
        afl::base::Ref<GameState> m_state;
        const UserConfiguration& m_config;
        std::auto_ptr<game::browser::LoadGameRootTask_t> m_then;
    };
    return m_handler.login(m_account, std::auto_ptr<Task_t>(new Task(m_handler, m_account, m_gameNr, m_state, config, then)));
}

String_t
game::nu::GameFolder::getName() const
{
    afl::data::Access a = m_state->loadGameListEntryPreAuthenticated();
    return afl::string::Format("%s (%d)", a("game")("name").toString(), a("game")("id").toInteger());
}

util::rich::Text
game::nu::GameFolder::getDescription() const
{
    return m_state->loadGameListEntryPreAuthenticated()("game")("description").toString();
}

bool
game::nu::GameFolder::isSame(const Folder& other) const
{
    const GameFolder* p = dynamic_cast<const GameFolder*>(&other);
    return p != 0
        && &*p->m_account == &*m_account
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
    return ::getGameFolderName(*m_account, m_gameNr);
}

String_t
game::nu::GameFolder::getGameIdAsString() const
{
    return ::getGameIdAsString(m_gameNr);
}
