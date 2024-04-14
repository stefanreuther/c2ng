/**
  *  \file test/game/test/specificationloadertest.cpp
  *  \brief Test for game::test::SpecificationLoader
  */

#include "game/test/specificationloader.hpp"

#include "afl/except/fileproblemexception.hpp"
#include "afl/test/testrunner.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"

AFL_TEST("game.test.SpecificationLoader:loadShipList", a)
{
    game::spec::ShipList shipList;
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion()));

    game::test::SpecificationLoader testee;

    bool result = false;
    testee.loadShipList(shipList, *root, game::makeResultTask(result))->call();

    a.check("result", result);
}

AFL_TEST("game.test.SpecificationLoader:openSpecificationFile", a)
{
    game::test::SpecificationLoader testee;
    AFL_CHECK_THROWS(a, testee.openSpecificationFile("race.nm"), afl::except::FileProblemException);
}
