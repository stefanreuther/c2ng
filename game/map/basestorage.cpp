/**
  *  \file game/map/basestorage.cpp
  *  \brief Class game::map::BaseStorage
  */

#include "game/map/basestorage.hpp"

bool
game::map::BaseStorage::isValid() const
{
    for (int i = 0, n = size(); i < n; ++i) {
        if (get(i).isValid()) {
            return true;
        }
    }
    return false;
}
