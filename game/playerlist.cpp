/**
  *  \file game/playerlist.cpp
  *  \brief Class game::PlayerList
  */

#include "game/playerlist.hpp"
#include "util/string.hpp"
#include "afl/string/format.hpp"
#include "util/translation.hpp"

// FIXME: simplification: refuse player numbers > MAX_PLAYERS. Keep m_players at MAX_PLAYERS+1 slots all the time.

// Default constructor.
game::PlayerList::PlayerList()
    : m_players()
{
    clear();
}

// Destructor.
game::PlayerList::~PlayerList()
{ }

// Create a player slot.
game::Player*
game::PlayerList::create(int id)
{
    if (id >= 0) {
        size_t requiredSize = static_cast<size_t>(id)+1;
        if (m_players.size() < requiredSize) {
            m_players.resize(requiredSize);
        }
        if (m_players[id] == 0) {
            m_players.replaceElementNew(id, new Player(id));
            m_players[id]->markChanged();
        }
        return m_players[id];
    } else {
        return 0;
    }
}

// Get player slot.
game::Player*
game::PlayerList::get(int id) const
{
    if (id >= 0 && id < static_cast<int>(m_players.size())) {
        return m_players[id];
    } else {
        return 0;
    }
}

// Reset this object.
void
game::PlayerList::clear()
{
    m_players.clear();
    if (Player* p = create(0)) {
        p->initUnowned();
    }
}

// Get size.
int
game::PlayerList::size() const
{
    return int(m_players.size());
}

// Get player object, given a player character.
game::Player*
game::PlayerList::getPlayerFromCharacter(char ch) const
{
    int nr;
    if (util::parsePlayerCharacter(ch, nr)) {
        return get(nr);
    } else {
        return 0;
    }
}

// Get character from player number.
char
game::PlayerList::getCharacterFromPlayer(int id)
{
    // ex game/player.h:getPlayerChar
    // FIXME: might be better in class Player? util?
    if (id >= 0 && id < 10) {
        return char('0' + id);
    } else if (id >= 10 && id < 36) {
        return char('A' + (id-10));
    } else {
        return '\0';
    }
}

// Expand names in string template.
String_t
game::PlayerList::expandNames(const String_t tpl, bool useOriginalNames) const
{
    // ex GRaceNameList::expandNames, ccmain.pas:ReplaceRaces
    String_t result;
    String_t::size_type pos = 0;
    String_t::size_type n;

    while ((n = tpl.find('%', pos)) != tpl.npos) {
        result.append(tpl, pos, n - pos);
        ++n;

        Player::Name which = useOriginalNames ? Player::OriginalShortName : Player::ShortName;
        if (n < tpl.size() && tpl[n] == '-') {
            which = useOriginalNames ? Player::OriginalAdjectiveName : Player::AdjectiveName;
            ++n;
        }
        if (n < tpl.size()) {
            if (tpl[n] == '%') {
                result += '%';
                ++n;
            } else if (Player* pl = getPlayerFromCharacter(tpl[n])) {
                result += pl->getName(which);
                ++n;
            } else {
                // ignore; next iteration will append this character
            }
        }
        pos = n;
    }
    result.append(tpl, pos, tpl.size() - pos);
    return result;
}

// Get set of all players.
game::PlayerSet_t
game::PlayerList::getAllPlayers() const
{
    PlayerSet_t result;
    for (int i = 0, n = size(); i < n; ++i) {
        if (const Player* pl = get(i)) {
            if (pl->isReal()) {
                result += i;
            }
        }
    }
    return result;
}

// Get first player.
game::Player*
game::PlayerList::getFirstPlayer() const
{
    return findNextPlayer(0);
}

// Get next player.
game::Player*
game::PlayerList::getNextPlayer(const Player* p) const
{
    return p
        ? findNextPlayer(p->getId())
        : 0;
}

// Get next player.
game::Player*
game::PlayerList::getNextPlayer(int id) const
{
    return findNextPlayer(id);
}

// Get name of a player.
String_t
game::PlayerList::getPlayerName(int id, Player::Name which) const
{
    String_t result;
    if (const Player* p = get(id)) {
        result = p->getName(which);
    }
    if (result.empty()) {
        switch (which) {
         case Player::ShortName:
         case Player::AdjectiveName:
         case Player::LongName:
         case Player::OriginalShortName:
         case Player::OriginalAdjectiveName:
         case Player::OriginalLongName:
            result = afl::string::Format(_("Player %d").c_str(), id);
            break;
         case Player::UserName:
         case Player::NickName:
         case Player::EmailAddress:
            break;
        }
    }
    return result;
}

// Notify listeners.
void
game::PlayerList::notifyListeners()
{
    bool needed = false;
    for (Player* p = getFirstPlayer(); p != 0; p = getNextPlayer(p)) {
        if (p->isChanged()) {
            p->markChanged(false);
            needed = true;
        }
    }
    if (needed) {
        sig_change.raise();
    }
}

game::Player*
game::PlayerList::findNextPlayer(int nr) const
{
    int limit = size();

    // If input is negative, fast-forward to 0. This avoids that a loop starting at -bignum takes forever.
    if (nr < 0) {
        nr = 0;
    }
    while (nr < limit) {
        ++nr;
        if (Player* p = get(nr)) {
            return p;
        }
    }
    return 0;
}
