/**
  *  \file u/t_server_format_utils.cpp
  *  \brief Test for server/format/utils.hpp
  */

#include <memory>
#include "server/format/utils.hpp"

#include "t_server_format.hpp"
#include "game/v3/structures.hpp"
#include "afl/data/value.hpp"
#include "afl/data/access.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/integervalue.hpp"

/** Test packCost. */
void
TestServerFormatutils::testPackCost()
{
    // With a vector
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

        TS_ASSERT_EQUALS(c.money, 112);
        TS_ASSERT_EQUALS(c.tritanium, 114);
        TS_ASSERT_EQUALS(c.duranium, 111);
        TS_ASSERT_EQUALS(c.molybdenum, 113);
    }

    // With a hash
    {
        afl::data::Hash::Ref_t v = afl::data::Hash::create();
        v->setNew("T", new afl::data::IntegerValue(222));
        v->setNew("D", new afl::data::IntegerValue(333));
        v->setNew("M", new afl::data::IntegerValue(444));
        v->setNew("MC", new afl::data::IntegerValue(555));

        std::auto_ptr<afl::data::Value> vv(new afl::data::HashValue(v));

        game::v3::structures::Cost c;
        server::format::packCost(c, vv);

        TS_ASSERT_EQUALS(c.money, 555);
        TS_ASSERT_EQUALS(c.tritanium, 222);
        TS_ASSERT_EQUALS(c.duranium, 333);
        TS_ASSERT_EQUALS(c.molybdenum, 444);
    }

    // With null
    {
        game::v3::structures::Cost c = {{7,7},{7,7},{7,7},{7,7}}; // we're initializing with bytes!
        server::format::packCost(c, 0);

        TS_ASSERT_EQUALS(c.money, 0);
        TS_ASSERT_EQUALS(c.tritanium, 0);
        TS_ASSERT_EQUALS(c.duranium, 0);
        TS_ASSERT_EQUALS(c.molybdenum, 0);
    }
}

/** Test unpackCost. */
void
TestServerFormatutils::testUnpackCost()
{
    game::v3::structures::Cost c;
    c.money = 999;
    c.tritanium = 111;
    c.duranium = 222;
    c.molybdenum = 333;

    std::auto_ptr<afl::data::Value> v(server::format::unpackCost(c));
    afl::data::Access a(v);
    TS_ASSERT_EQUALS(a("MC").toInteger(), 999);
    TS_ASSERT_EQUALS(a("T").toInteger(), 111);
    TS_ASSERT_EQUALS(a("D").toInteger(), 222);
    TS_ASSERT_EQUALS(a("M").toInteger(), 333);
}
