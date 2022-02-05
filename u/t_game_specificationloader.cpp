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
        virtual std::auto_ptr<game::Task_t> loadShipList(game::spec::ShipList& /*list*/, game::Root& /*root*/, std::auto_ptr<game::StatusTask_t> /*then*/)
            { return std::auto_ptr<game::Task_t>(); }
    };
    Tester t;
}

