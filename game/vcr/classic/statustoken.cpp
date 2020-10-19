/**
  *  \file game/vcr/classic/statustoken.cpp
  *  \brief Base class game::vcr::classic::StatusToken
  */

#include "game/vcr/classic/statustoken.hpp"

game::vcr::classic::StatusToken::StatusToken(Time_t time)
    : m_time(time)
{ }

game::vcr::classic::Time_t
game::vcr::classic::StatusToken::getTime() const
{
    return m_time;
}
