/**
  *  \file game/map/messagelink.cpp
  *  \brief Class game::map::MessageLink
  */

#include "game/map/messagelink.hpp"


game::map::MessageLink::MessageLink()
    : m_data()
{ }

void
game::map::MessageLink::add(size_t nr)
{
    // This is the same logic as in PCC2, assuming that we're parsing forward
    if (m_data.empty() || m_data.back() != nr) {
        m_data.push_back(nr);
    }
}

bool
game::map::MessageLink::empty() const
{
    return m_data.empty();
}

const
game::map::MessageLink::Vector_t&
game::map::MessageLink::get() const
{
    return m_data;
}
