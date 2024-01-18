/**
  *  \file test/server/format/utilstest.cpp
  *  \brief Test for server::format::Utils
  */

#include "server/format/utils.hpp"

#include "afl/data/access.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/value.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/test/testrunner.hpp"
#include "game/v3/structures.hpp"
#include <memory>

/*
 *  Test packCost.
 */

// With a vector
AFL_TEST("server.format.Utils:packCost:vector", a)
{
    afl::data::Vector::Ref_t v = afl::data::Vector::create();
    v->pushBackString("T");
    v->pushBackInteger(114);
    v->pushBackString("M");
    v->pushBackInteger(113);
    v->pushBackString("MC");
    v->pushBackInteger(112);
    v->pushBackString("D");
    v->pushBackString("111");

    std::auto_ptr<afl::data::Value> vv(new afl::data::VectorValue(v));

    game::v3::structures::Cost c;
    server::format::packCost(c, vv);

    a.checkEqual("money",      c.money, 112);
    a.checkEqual("tritanium",  c.tritanium, 114);
    a.checkEqual("duranium",   c.duranium, 111);
    a.checkEqual("molybdenum", c.molybdenum, 113);
}

// With a hash
AFL_TEST("server.format.Utils:packCost:hash", a)
{
    afl::data::Hash::Ref_t v = afl::data::Hash::create();
    v->setNew("T", new afl::data::IntegerValue(222));
    v->setNew("D", new afl::data::IntegerValue(333));
    v->setNew("M", new afl::data::IntegerValue(444));
    v->setNew("MC", new afl::data::IntegerValue(555));

    std::auto_ptr<afl::data::Value> vv(new afl::data::HashValue(v));

    game::v3::structures::Cost c;
    server::format::packCost(c, vv);

    a.checkEqual("money",      c.money, 555);
    a.checkEqual("tritanium",  c.tritanium, 222);
    a.checkEqual("duranium",   c.duranium, 333);
    a.checkEqual("molybdenum", c.molybdenum, 444);
}

// With null
AFL_TEST("server.format.Utils:packCost:null", a)
{
    game::v3::structures::Cost c = {{7,7},{7,7},{7,7},{7,7}}; // we're initializing with bytes!
    server::format::packCost(c, 0);

    a.checkEqual("money",      c.money, 0);
    a.checkEqual("tritanium",  c.tritanium, 0);
    a.checkEqual("duranium",   c.duranium, 0);
    a.checkEqual("molybdenum", c.molybdenum, 0);
}

/*
 *  Test unpackCost.
 */
AFL_TEST("server.format.Utils:unpackCost", a)
{
    game::v3::structures::Cost c;
    c.money = 999;
    c.tritanium = 111;
    c.duranium = 222;
    c.molybdenum = 333;

    std::auto_ptr<afl::data::Value> v(server::format::unpackCost(c));
    afl::data::Access ap(v);
    a.checkEqual("01", ap("MC").toInteger(), 999);
    a.checkEqual("02", ap("T").toInteger(), 111);
    a.checkEqual("03", ap("D").toInteger(), 222);
    a.checkEqual("04", ap("M").toInteger(), 333);
}
