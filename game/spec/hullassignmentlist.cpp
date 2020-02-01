/**
  *  \file game/spec/hullassignmentlist.cpp
  *  \brief Class game::spec::HullAssignmentList
  */

#include "game/spec/hullassignmentlist.hpp"

// Default constructor.
game::spec::HullAssignmentList::HullAssignmentList()
    : m_mode(PlayerIndexed),
      m_mapping()
{ }

// Destructor.
game::spec::HullAssignmentList::~HullAssignmentList()
{ }

// Clear.
void
game::spec::HullAssignmentList::clear()
{
    m_mapping.clear();
}

// Set access mode.
void
game::spec::HullAssignmentList::setMode(Mode mode)
{
    m_mode = mode;
}

// Add a mapping.
void
game::spec::HullAssignmentList::add(int player, int position, int hullNr)
{
    // FIXME: do we need the ability to set a hullNr to 0?
    if (player > 0 && position > 0 && hullNr > 0) {
        if (int(m_mapping.size()) <= player) {
            m_mapping.resize(player+1);
        }
        std::vector<int>& row = m_mapping[player];
        if (int(row.size()) <= position) {
            row.resize(position+1);
        }
        row[position] = hullNr;
    }
}

// Clear a player slot.
void
game::spec::HullAssignmentList::clearPlayer(int player)
{
    if (player != 0 && player < int(m_mapping.size())) {
        m_mapping[player].clear();
    }
}

// Get index, given a hull.
int
game::spec::HullAssignmentList::getIndexFromHull(const game::config::HostConfiguration& config, int player, int hullNr) const
{
    // ex game/spec.cc:getTruehullSlot
    // ex planint.pas:TruehullSlot
    int mappedPlayer = mapPlayer(config, player);
    if (mappedPlayer < 0 || mappedPlayer >= int(m_mapping.size())) {
        // player does not exist
        return 0;
    } else {
        for (size_t i = 0, n = m_mapping[mappedPlayer].size(); i < n; ++i) {
            if (m_mapping[mappedPlayer][i] == hullNr) {
                return int(i);
            }
        }
        return 0;
    }
}

// Get hull, given an index.
int
game::spec::HullAssignmentList::getHullFromIndex(const game::config::HostConfiguration& config, int player, int index) const
{
    // ex game/spec.cc:getTruehull
    int mappedPlayer = mapPlayer(config, player);
    if (mappedPlayer < 0 || mappedPlayer >= int(m_mapping.size())) {
        // player does not exist
        return 0;
    } else if (index < 0 || index >= int(m_mapping[mappedPlayer].size())) {
        // slot does not exist
        return 0;
    } else {
        return m_mapping[mappedPlayer][index];
    }
}

// Get maximum index.
int
game::spec::HullAssignmentList::getMaxIndex(const game::config::HostConfiguration& config, int player) const
{
    int mappedPlayer = mapPlayer(config, player);
    if (mappedPlayer < 0 || mappedPlayer >= int(m_mapping.size())) {
        return 0;
    } else if (m_mapping[mappedPlayer].size() <= 1) {
        return 0;
    } else {
        return int(m_mapping[mappedPlayer].size() - 1);
    }
}

int
game::spec::HullAssignmentList::mapPlayer(const game::config::HostConfiguration& config, int player) const
{
    switch (m_mode) {
     case PlayerIndexed:
        return player;

     case RaceIndexed:
        return config.getPlayerRaceNumber(player);
    }
    return 0;
}
