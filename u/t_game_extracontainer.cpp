/**
  *  \file u/t_game_extracontainer.cpp
  *  \brief Test for game::ExtraContainer
  */

#include "game/extracontainer.hpp"

#include "t_game.hpp"

namespace {
    class MyExtra1 : public game::Extra { };
    class MyExtra2 : public game::Extra { };
}

/** Test basic operations. */
void
TestGameExtraContainer::testIt()
{
    static const game::ExtraIdentifier<int,MyExtra1> def1 = {};
    static const game::ExtraIdentifier<int,MyExtra2> def2 = {};

    game::ExtraContainer<int> testee;
    TS_ASSERT(testee.get(def1) == 0);
    TS_ASSERT(testee.get(def2) == 0);

    MyExtra1* p = new MyExtra1();
    testee.setNew(def1, p);
    TS_ASSERT(testee.get(def1) == p);
    TS_ASSERT(testee.get(def2) == 0);

    TS_ASSERT(&testee.create(def1) == p);
    TS_ASSERT(&testee.create(def2) != 0);

    TS_ASSERT(testee.get(def1) == p);
    TS_ASSERT(testee.get(def2) != 0);
}
