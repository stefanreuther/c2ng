/**
  *  \file test/game/interface/basetaskbuildcommandparsertest.cpp
  *  \brief Test for game::interface::BaseTaskBuildCommandParser
  */

#include "game/interface/basetaskbuildcommandparser.hpp"

#include "afl/test/testrunner.hpp"
#include "game/spec/shiplist.hpp"

using game::spec::ShipList;

namespace {
    void prepare(ShipList& shipList)
    {
        for (int i = 1; i <= 5; ++i) {
            shipList.launchers().create(i);
        }
        for (int i = 1; i <= 7; ++i) {
            shipList.beams().create(i);
        }
        for (int i = 1; i <= 9; ++i) {
            shipList.engines().create(i);
        }
        for (int i = 1; i <= 11; ++i) {
            game::spec::Hull* h = shipList.hulls().create(i);
            h->setMaxLaunchers(4);
            h->setMaxBeams(6);
        }
    }
}

// Test: BuildShip command
AFL_TEST("game.interface.BaseTaskBuildCommandParser:BuildShip", a)
{
    ShipList shipList;
    prepare(shipList);

    game::interface::BaseTaskBuildCommandParser p(shipList);
    p.predictStatement("BuildShip 10, 5, 3, -1, 2, -1");
    a.checkEqual("getVerb",         p.getVerb(), "BUILDSHIP");
    a.checkEqual("getHullIndex",    p.getOrder().getHullIndex(), 10);
    a.checkEqual("getEngineType",   p.getOrder().getEngineType(), 5);
    a.checkEqual("getBeamType",     p.getOrder().getBeamType(), 3);
    a.checkEqual("getNumBeams",     p.getOrder().getNumBeams(), 6);
    a.checkEqual("getTorpedoType",  p.getOrder().getTorpedoType(), 2);
    a.checkEqual("getNumLaunchers", p.getOrder().getNumLaunchers(), 4);
}

// Test: EnqueueShip command
AFL_TEST("game.interface.BaseTaskBuildCommandParser:EnqueueShip", a)
{
    ShipList shipList;
    prepare(shipList);

    game::interface::BaseTaskBuildCommandParser p(shipList);
    p.predictStatement("EnqueueShip 5, 2");
    a.checkEqual("getVerb",         p.getVerb(), "ENQUEUESHIP");
    a.checkEqual("getHullIndex",    p.getOrder().getHullIndex(), 5);
    a.checkEqual("getEngineType",   p.getOrder().getEngineType(), 2);
    a.checkEqual("getBeamType",     p.getOrder().getBeamType(), 0);
    a.checkEqual("getNumBeams",     p.getOrder().getNumBeams(), 0);
    a.checkEqual("getTorpedoType",  p.getOrder().getTorpedoType(), 0);
    a.checkEqual("getNumLaunchers", p.getOrder().getNumLaunchers(), 0);
}

// Test: other command
AFL_TEST("game.interface.BaseTaskBuildCommandParser:other-command", a)
{
    ShipList shipList;
    prepare(shipList);

    game::interface::BaseTaskBuildCommandParser p(shipList);
    p.predictStatement("GotoShip 5, 2");
    a.checkEqual("getVerb", p.getVerb(), "");
}

// Test: cancel build order
AFL_TEST("game.interface.BaseTaskBuildCommandParser:cancel-build", a)
{
    ShipList shipList;
    prepare(shipList);

    game::interface::BaseTaskBuildCommandParser p(shipList);
    p.predictStatement("BuildShip 0");
    a.checkEqual("getVerb", p.getVerb(), "BUILDSHIP");
    a.checkEqual("getHullIndex", p.getOrder().getHullIndex(), 0);
}

// Test: hull out of range
//   Exceptions are swallowed by predictStatement().
//   We therefore just do not recognize the command.
AFL_TEST("game.interface.BaseTaskBuildCommandParser:error:hull-out-of-range", a)
{
    ShipList shipList;
    prepare(shipList);

    game::interface::BaseTaskBuildCommandParser p(shipList);
    p.predictStatement("BuildShip 12");
    a.checkEqual("getVerb", p.getVerb(), "");
}

// Test: engine out of range
AFL_TEST("game.interface.BaseTaskBuildCommandParser:error:engine-out-of-range", a)
{
    ShipList shipList;
    prepare(shipList);

    game::interface::BaseTaskBuildCommandParser p(shipList);
    p.predictStatement("BuildShip 5, 0");
    a.checkEqual("getVerb", p.getVerb(), "");
}

// Test: arity error
AFL_TEST("game.interface.BaseTaskBuildCommandParser:error:missing-args", a)
{
    ShipList shipList;
    prepare(shipList);

    game::interface::BaseTaskBuildCommandParser p(shipList);
    p.predictStatement("BuildShip");
    a.checkEqual("getVerb", p.getVerb(), "");
}
