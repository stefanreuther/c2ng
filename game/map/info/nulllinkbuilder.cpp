/**
  *  \file game/map/info/nulllinkbuilder.cpp
  *  \class game::map::info::NullLinkBuilder
  */

#include "game/map/info/nulllinkbuilder.hpp"

String_t
game::map::info::NullLinkBuilder::makePlanetLink(const Planet& /*pl*/) const
{
    return String_t();
}

String_t
game::map::info::NullLinkBuilder::makeSearchLink(const SearchQuery& /*q*/) const
{
    return String_t();
}
