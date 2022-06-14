/**
  *  \file game/ref/typeadaptor.cpp
  *  \brief Class game::ref::TypeAdaptor
  */

#include "game/ref/typeadaptor.hpp"
#include "game/ref/list.hpp"

game::ref::TypeAdaptor::TypeAdaptor(const List& list, game::map::Universe& univ)
    : m_list(list),
      m_universe(univ)
{ }

game::map::Object*
game::ref::TypeAdaptor::getObjectByIndex(Id_t index)
{
    // m_list will do range-checking, so this is simple:
    return m_universe.getObject(m_list[index-1]);
}

game::Id_t
game::ref::TypeAdaptor::getNextIndex(Id_t index) const
{
    // GObjectList::getNextIndex
    if (index < Id_t(m_list.size())) {
        return index+1;
    } else {
        return 0;
    }
}

game::Id_t
game::ref::TypeAdaptor::getPreviousIndex(Id_t index) const
{
    // GObjectList::getPreviousIndex
    if (index == 0) {
        return Id_t(m_list.size());
    } else {
        return index-1;
    }
}
