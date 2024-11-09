/**
  *  \file game/spec/advantagelist.cpp
  *  \brief Class game::spec::AdvantageList
  */

#include "game/spec/advantagelist.hpp"

struct game::spec::AdvantageList::Item {
    int id;
    String_t name;
    String_t description;
    PlayerSet_t players;

    Item(int id)
        : id(id), name(), description(), players()
        { }
};

game::spec::AdvantageList::AdvantageList()
    : m_data()
{ }

game::spec::AdvantageList::~AdvantageList()
{ }

game::spec::AdvantageList::Item*
game::spec::AdvantageList::add(int id)
{
    if (Item* p = find(id)) {
        return p;
    } else {
        return m_data.pushBackNew(new Item(id));
    }
}

game::spec::AdvantageList::Item*
game::spec::AdvantageList::find(int id) const
{
    for (size_t i = 0; i < m_data.size(); ++i) {
        if (m_data[i]->id == id) {
            return m_data[i];
        }
    }
    return 0;
}

game::spec::AdvantageList::Item*
game::spec::AdvantageList::getAdvantageByIndex(size_t index) const
{
    if (index < m_data.size()) {
        return m_data[index];
    } else {
        return 0;
    }
}

size_t
game::spec::AdvantageList::getNumAdvantages() const
{
    return m_data.size();
}

void
game::spec::AdvantageList::setName(Item* p, const String_t& name)
{
    if (p != 0) {
        p->name = name;
    }
}

void
game::spec::AdvantageList::setDescription(Item* p, const String_t& description)
{
    if (p != 0) {
        p->description = description;
    }
}

void
game::spec::AdvantageList::addPlayer(Item* p, int player)
{
    if (p != 0) {
        p->players += player;
    }
}

int
game::spec::AdvantageList::getId(const Item* p) const
{
    return p != 0
        ? p->id
        : 0;
}

String_t
game::spec::AdvantageList::getName(const Item* p) const
{
    return p != 0
        ? p->name
        : String_t();
}

String_t
game::spec::AdvantageList::getDescription(const Item* p) const
{
    return p != 0
        ? p->description
        : String_t();
}

game::PlayerSet_t
game::spec::AdvantageList::getPlayers(const Item* p) const
{
    return p != 0
        ? p->players
        : PlayerSet_t();
}
