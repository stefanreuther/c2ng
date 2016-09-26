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

game::nu::GameFolder::GameFolder(BrowserHandler& handler,
                                 game::browser::Account& acc,
                                 int32_t gameNr,
                                 size_t hint)
    : m_handler(handler),
      m_account(acc),
      m_gameNr(gameNr),
      m_state(new GameState(handler, acc, gameNr, hint))
{ }

void
game::nu::GameFolder::loadContent(afl::container::PtrVector<Folder>& /*result*/)
{
    // Nothing to load, there are no subfolders
}

afl::base::Ptr<game::Root>
game::nu::GameFolder::loadGameRoot()
{
    // Current data
    afl::data::Access a = m_state->loadGameListEntry();

    // Root
    afl::base::Ptr<Root> root = new Root(m_handler.getDefaultSpecificationDirectory(),
                                         afl::io::InternalDirectory::create("FIXME - nu game directory"),
                                         new SpecificationLoader(m_state, m_handler.translator(), m_handler.log()),
                                         HostVersion(HostVersion::NuHost, MKVERSION(3,2,0)),
                                         std::auto_ptr<game::RegistrationKey>(new RegistrationKey(a("player"))),
                                         std::auto_ptr<game::StringVerifier>(new StringVerifier()));

    // FIXME:
    // + hostConfiguration()
    // + userConfiguration

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
    root->setTurnLoader(new TurnLoader(m_state, m_handler.translator(), m_handler.log()));
    
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
