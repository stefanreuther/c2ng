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

void
game::vcr::NullDatabase::save(afl::io::Stream& /*out*/, size_t /*first*/, size_t /*num*/, const game::config::HostConfiguration& /*config*/, afl::charset::Charset& /*cs*/)
{ }
