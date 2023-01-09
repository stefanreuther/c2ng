/**
  *  \file client/widgets/playersetselector.cpp
  *  \brief Class client::widgets::PlayerSetSelector
  */

#include "client/widgets/playersetselector.hpp"
#include "afl/string/char.hpp"
#include "afl/string/format.hpp"
#include "game/playerlist.hpp"

namespace {
    /*
     *  Resource Ids for the checkboxes
     */
    const char*const RESID_CHECKED   = "ui.cb1";
    const char*const RESID_UNCHECKED = "ui.cb0";

    /*
     *  Default size limit
     */
    const int MAX_PREFERRED_SIZE = 15;
}


client::widgets::PlayerSetSelector::PlayerSetSelector(ui::Root& root, const game::PlayerArray<String_t>& names, game::PlayerSet_t set, afl::string::Translator& tx)
    : CheckboxListbox(root, SingleLine),
      m_selectedPlayers(),
      m_selectablePlayers(set)
{
    init(names, set, tx);
    sig_checkboxClick.add(this, &PlayerSetSelector::togglePlayer);
}

client::widgets::PlayerSetSelector::~PlayerSetSelector()
{ }

void
client::widgets::PlayerSetSelector::setSelectedPlayers(game::PlayerSet_t set)
{
    // ex WPlayerSetSelector::setSelectedPlayers
    set &= m_selectablePlayers;
    if (set != m_selectedPlayers) {
        m_selectedPlayers = set;
        updateAllImageNames();
        sig_setChange.raise();
    }
}

void
client::widgets::PlayerSetSelector::setSelectablePlayers(game::PlayerSet_t set)
{
    // ex WPlayerSetSelector::setSelectablePlayers
    m_selectablePlayers = set;
    for (size_t i = 0, n = getNumItems(); i < n; ++i) {
        Item* p = getItemByIndex(i);
        setItemAccessible(p, set.contains(getItemId(p)));
    }
    setSelectedPlayers(m_selectedPlayers & set);
}

game::PlayerSet_t
client::widgets::PlayerSetSelector::getSelectedPlayers() const
{
    return m_selectedPlayers;
}

void
client::widgets::PlayerSetSelector::togglePlayer(int player)
{
    // ex WPlayerSetSelector::togglePlayer
    setSelectedPlayers(getSelectedPlayers() ^ player);
}

void
client::widgets::PlayerSetSelector::togglePlayers(game::PlayerSet_t set)
{
    // ex WPlayerSetSelector::togglePlayers
    set &= m_selectablePlayers;
    if ((getSelectedPlayers() & set) == set) {
        setSelectedPlayers(getSelectedPlayers() - set);
    } else {
        setSelectedPlayers(getSelectedPlayers() | set);
    }
}

void
client::widgets::PlayerSetSelector::toggleAll()
{
    togglePlayers(m_selectablePlayers);
}

void
client::widgets::PlayerSetSelector::init(const game::PlayerArray<String_t>& names, game::PlayerSet_t set, afl::string::Translator& tx)
{
    for (int playerNr = 1; playerNr <= game::MAX_PLAYERS; ++playerNr) {
        if (set.contains(playerNr)) {
            char key = game::PlayerList::getCharacterFromPlayer(playerNr);
            Item* p = addItem(playerNr, afl::string::Format("%c - %s", key, names.get(playerNr)));
            setItemKey(p, afl::string::charToLower(key));
            updateImageName(p);
        }
    }
    if (set.contains(0)) {
        Item* p = addItem(0, afl::string::Format("%c - %s", 'X', tx("Host")));
        setItemKey(p, 'x');
        updateImageName(p);
    }
    if (getNumItems() > MAX_PREFERRED_SIZE) {
        setPreferredHeight(MAX_PREFERRED_SIZE);
    }
}

void
client::widgets::PlayerSetSelector::updateImageName(Item* p)
{
    setItemImageName(p, m_selectedPlayers.contains(getItemId(p)) ? RESID_CHECKED : RESID_UNCHECKED);
}

void
client::widgets::PlayerSetSelector::updateAllImageNames()
{
    for (size_t i = 0, n = getNumItems(); i < n; ++i) {
        Item* p = getItemByIndex(i);
        updateImageName(p);
    }
}
