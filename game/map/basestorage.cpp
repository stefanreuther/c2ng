/**
  *  \file game/map/basestorage.cpp
  */

#include "game/map/basestorage.hpp"

game::map::BaseStorage::BaseStorage()
    : m_content()
{ }

game::map::BaseStorage::~BaseStorage()
{ }

void
game::map::BaseStorage::set(int slot, IntegerProperty_t amount)
{
    if (slot > 0 && int(size_t(slot)) == slot) {
        if (m_content.size() < size_t(slot)) {
            m_content.resize(slot);
        }
        m_content[slot-1] = amount;
    }
}

game::IntegerProperty_t
game::map::BaseStorage::get(int slot) const
{
    if (const IntegerProperty_t* p = at(slot)) {
        return *p;
    } else {
        return IntegerProperty_t();
    }
}

game::IntegerProperty_t*
game::map::BaseStorage::at(int slot)
{
    if (slot > 0 && int(size_t(slot)) == slot && size_t(slot) <= m_content.size()) {
        return &m_content[slot-1];
    } else {
        return 0;
    }
}

const game::IntegerProperty_t*
game::map::BaseStorage::at(int slot) const
{
    if (slot > 0 && int(size_t(slot)) == slot && size_t(slot) <= m_content.size()) {
        return &m_content[slot-1];
    } else {
        return 0;
    }
}

bool
game::map::BaseStorage::isValid() const
{
    for (size_t i = 0, n = m_content.size(); i < n; ++i) {
        if (m_content[i].isValid()) {
            return true;
        }
    }
    return false;
}

void
game::map::BaseStorage::clear()
{
    m_content.clear();
}
