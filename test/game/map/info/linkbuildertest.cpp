/**
  *  \file test/game/map/info/linkbuildertest.cpp
  *  \brief Test for game::map::info::LinkBuilder
  */

#include "game/map/info/linkbuilder.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.map.info.LinkBuilder")
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
