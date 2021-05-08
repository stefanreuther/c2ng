/**
  *  \file game/vcr/nulldatabase.cpp
  *  \brief Class game::vcr::NullDatabase
  */

#include "game/vcr/nulldatabase.hpp"

size_t
game::vcr::NullDatabase::getNumBattles() const
{
    return 0;
}

game::vcr::Battle*
game::vcr::NullDatabase::getBattle(size_t /*nr*/)
{
    return 0;
}
