/**
  *  \file game/test/specificationloader.hpp
  *  \brief Class game::test::SpecificationLoader
  */
#ifndef C2NG_GAME_TEST_SPECIFICATIONLOADER_HPP
#define C2NG_GAME_TEST_SPECIFICATIONLOADER_HPP

#include "game/specificationloader.hpp"

namespace game { namespace test {

    /** Test support: SpecificationLoader.
        This SpecificationLoader just does nothing. */
    class SpecificationLoader : public game::SpecificationLoader {
     public:
        virtual std::auto_ptr<Task_t> loadShipList(game::spec::ShipList& list, game::Root& root, std::auto_ptr<StatusTask_t> then);
    };

} }

#endif
