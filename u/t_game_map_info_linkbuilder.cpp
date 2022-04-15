/**
  *  \file u/t_game_map_info_linkbuilder.cpp
  *  \brief Test for game::map::info::LinkBuilder
  */

#include "game/map/info/linkbuilder.hpp"

#include "t_game_map_info.hpp"

/** Interface test. */
void
TestGameMapInfoLinkBuilder::testInterface()
{
    class Tester : public game::map::info::LinkBuilder {
     public:
        virtual String_t makePlanetLink(const game::map::Planet& /*pl*/) const
            { return String_t(); }
        virtual String_t makeSearchLink(const game::SearchQuery& /*q*/) const
            { return String_t(); }
    };
    Tester t;
}

