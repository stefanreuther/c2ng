/**
  *  \file test/game/specificationloadertest.cpp
  *  \brief Test for game::SpecificationLoader
  */

#include "game/specificationloader.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.SpecificationLoader")
{
    class Tester : public game::SpecificationLoader {
     public:
        virtual std::auto_ptr<game::Task_t> loadShipList(game::spec::ShipList& /*list*/, game::Root& /*root*/, std::auto_ptr<game::StatusTask_t> /*then*/)
            { return std::auto_ptr<game::Task_t>(); }
        virtual afl::base::Ref<afl::io::Stream> openSpecificationFile(const String_t& /*fileName*/)
            { throw std::runtime_error("not supported"); }
    };
    Tester t;
}
