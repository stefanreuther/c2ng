/**
  *  \file u/t_game_specificationloader.cpp
  *  \brief Test for game::SpecificationLoader
  */

#include "game/specificationloader.hpp"

#include "t_game.hpp"

/** Interface test. */
void
TestGameSpecificationLoader::testIt()
{
    class Tester : public game::SpecificationLoader {
     public:
        virtual void loadShipList(game::spec::ShipList& /*list*/, game::Root& /*root*/)
            { }
    };
    Tester t;
}

