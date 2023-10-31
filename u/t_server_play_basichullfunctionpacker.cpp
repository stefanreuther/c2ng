/**
  *  \file u/t_server_play_basichullfunctionpacker.cpp
  *  \brief Test for server::play::BasicHullFunctionPacker
  */

#include "server/play/basichullfunctionpacker.hpp"

#include "t_server_play.hpp"
#include "afl/data/access.hpp"
#include "game/spec/shiplist.hpp"

/** Simple functionality test. */
void
TestServerPlayBasicHullFunctionPacker::testIt()
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
    TS_ASSERT_EQUALS(testee.getName(), "zab");

    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access a(value.get());

    // Must produce two elements (not indexed by Id!)
    TS_ASSERT_EQUALS(a.getArraySize(), 2U);
    TS_ASSERT_EQUALS(a[0]("ID").toInteger(), 9);
    TS_ASSERT_EQUALS(a[0]("NAME").toString(), "Eat");
    TS_ASSERT_EQUALS(a[0]("INFO").toString(), "quarterpounder with cheese");
    TS_ASSERT_EQUALS(a[1]("ID").toInteger(), 12);
    TS_ASSERT_EQUALS(a[1]("NAME").toString(), "Sleep");
    TS_ASSERT_EQUALS(a[1]("IMAGE").toString(), "zzz.gif");
    TS_ASSERT_EQUALS(a[1]("CODE").toString(), "Z");
}

