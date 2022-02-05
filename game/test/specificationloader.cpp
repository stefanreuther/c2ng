/**
  *  \file game/test/specificationloader.cpp
  *  \brief Class game::test::SpecificationLoader
  */

#include "game/test/specificationloader.hpp"

std::auto_ptr<game::Task_t>
game::test::SpecificationLoader::loadShipList(game::spec::ShipList& /*list*/, game::Root& /*root*/, std::auto_ptr<StatusTask_t> then)
{
    return makeConfirmationTask(true, then);
}
