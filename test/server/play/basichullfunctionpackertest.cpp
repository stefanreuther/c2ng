/**
  *  \file test/server/play/basichullfunctionpackertest.cpp
  *  \brief Test for server::play::BasicHullFunctionPacker
  */

#include "server/play/basichullfunctionpacker.hpp"

#include "afl/data/access.hpp"
#include "afl/test/testrunner.hpp"
#include "game/spec/shiplist.hpp"

/** Simple functionality test. */
AFL_TEST("server.play.BasicHullFunctionPacker", a)
{
    // Populate session
    afl::base::Ref<game::spec::ShipList> sl(*new game::spec::ShipList());
    game::spec::BasicHullFunctionList& funcs = sl->basicHullFunctions();
    game::spec::BasicHullFunction* f1 = funcs.addFunction(9, "Eat");
    f1->setDescription("quarterpounder with cheese");

    game::spec::BasicHullFunction* f2 = funcs.addFunction(12, "Sleep");
    f2->setPictureName("zzz.gif");
    f2->setCode("Z");

    // Testee
    server::play::BasicHullFunctionPacker testee(*sl);
    a.checkEqual("01. getName", testee.getName(), "zab");

    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access ap(value.get());

    // Must produce two elements (not indexed by Id!)
    a.checkEqual("11. getArraySize", ap.getArraySize(), 2U);
    a.checkEqual("12", ap[0]("ID").toInteger(), 9);
    a.checkEqual("13", ap[0]("NAME").toString(), "Eat");
    a.checkEqual("14", ap[0]("INFO").toString(), "quarterpounder with cheese");
    a.checkEqual("15", ap[1]("ID").toInteger(), 12);
    a.checkEqual("16", ap[1]("NAME").toString(), "Sleep");
    a.checkEqual("17", ap[1]("IMAGE").toString(), "zzz.gif");
    a.checkEqual("18", ap[1]("CODE").toString(), "Z");
}
