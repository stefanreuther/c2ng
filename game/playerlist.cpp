/**
  *  \file game/playerlist.cpp
  */

#include "game/playerlist.hpp"
#include "util/string.hpp"

game::PlayerList::PlayerList()
    : m_players()
{
    if (Player* p = create(0)) {
        p->initUnowned();
    }
}

game::PlayerList::~PlayerList()
{ }

game::Player*
game::PlayerList::create(int id)
{
    if (id >= 0) {
        // FIXME: give PtrVector a resize()?
        while (id >= static_cast<int>(m_players.size())) {
            m_players.pushBackNew(0);
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

game::Player*
game::PlayerList::get(int id) const
{
    if (id >= 0 && id < static_cast<int>(m_players.size())) {
        return m_players[id];
    } else {
        return 0;
    }
}

void
game::PlayerList::clear()
{
    // FIXME: slot 0?
    m_players.clear();
}

int
game::PlayerList::size() const
{
    return m_players.size();
}

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

char
game::PlayerList::getCharacterFromPlayer(int id) const
{
    // FIXME: might be better in class Player? util?
    if (id >= 0 && id < 10) {
        return '0' + id;
    } else if (id >= 10 && id < 36) {
        return 'A' + (id-10);
    } else {
        return '\0';
    }
}

// /** Expand race names.
//     "%N" expands to race N's short name.
//     "%-N" expands to race N's adjective.
//     "%%" is a percent sign. */
String_t
game::PlayerList::expandNames(const String_t tpl) const
{
    // ex GRaceNameList::expandNames
    String_t result;
    String_t::size_type pos = 0;
    String_t::size_type n;

    while ((n = tpl.find('%', pos)) != tpl.npos) {
        result.append(tpl, pos, n - pos);
        ++n;

        Player::Name which = Player::ShortName;
        if (n < tpl.size() && tpl[n] == '-') {
            which = Player::AdjectiveName;
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

game::Player*
game::PlayerList::getFirstPlayer() const
{
    return findNextPlayer(0);
}

game::Player*
game::PlayerList::getNextPlayer(Player* p) const
{
    return p
        ? findNextPlayer(p->getId())
        : 0;
}

game::Player*
game::PlayerList::getNextPlayer(int n) const
{
    return findNextPlayer(n);
}

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
    while (nr < limit) {
        ++nr;
        if (Player* p = get(nr)) {
            return p;
        }
    }
    return 0;
}
