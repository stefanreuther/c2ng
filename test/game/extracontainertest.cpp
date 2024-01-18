/**
  *  \file test/game/extracontainertest.cpp
  *  \brief Test for game::ExtraContainer
  */

#include "game/extracontainer.hpp"
#include "afl/test/testrunner.hpp"

namespace {
    class MyExtra1 : public game::Extra { };
    class MyExtra2 : public game::Extra { };
}

/** Test basic operations. */
AFL_TEST("game.ExtraContainer", a)
{
    static const game::ExtraIdentifier<int,MyExtra1> def1 = {{}};
    static const game::ExtraIdentifier<int,MyExtra2> def2 = {{}};

    game::ExtraContainer<int> testee;
    a.checkNull("01. get", testee.get(def1));
    a.checkNull("02. get", testee.get(def2));

    MyExtra1* p = new MyExtra1();
    testee.setNew(def1, p);
    a.check("11. get", testee.get(def1) == p);
    a.checkNull("12. get", testee.get(def2));

    a.check("21. create", &testee.create(def1) == p);
    a.checkNonNull("22. create", &testee.create(def2));

    a.check("31. get", testee.get(def1) == p);
    a.checkNonNull("32. get", testee.get(def2));
}
