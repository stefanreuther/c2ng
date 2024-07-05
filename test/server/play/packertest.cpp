/**
  *  \file test/server/play/packertest.cpp
  *  \brief Test for server::play::Packer
  */

#include <memory>
#include "server/play/packer.hpp"
#include "afl/base/ref.hpp"
#include "afl/data/access.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/interface/referencecontext.hpp"
#include "game/map/point.hpp"
#include "game/reference.hpp"
#include "game/session.hpp"
#include "interpreter/arraydata.hpp"
#include "interpreter/arrayvalue.hpp"

using afl::base::Ref;
using afl::data::Access;
using afl::data::IntegerValue;
using afl::data::StringValue;
using afl::data::Value;
using afl::io::NullFileSystem;
using afl::string::NullTranslator;
using game::Reference;
using game::Session;
using game::interface::ReferenceContext;
using game::map::Point;
using interpreter::ArrayData;
using interpreter::ArrayValue;
using server::play::Packer;

/** Interface test. */
AFL_TEST_NOARG("server.play.Packer")
{
    class Tester : public Packer {
     public:
        virtual server::Value_t* buildValue() const
            { return 0; }
        virtual String_t getName() const
            { return String_t(); }
    };
    Tester t;
}

/** Test flattenNew, null. */
AFL_TEST("server.play.Packer:flattenNew:null", a)
{
    std::auto_ptr<Value> p(Packer::flattenNew(0));
    a.checkNull("01", p.get());
}

/** Test flattenNew, integer. */
AFL_TEST("server.play.Packer:flattenNew:int", a)
{
    IntegerValue* iv = new IntegerValue(42);
    std::auto_ptr<Value> p(Packer::flattenNew(iv));

    a.checkEqual("01", Access(p.get()).toInteger(), 42);
}

/** Test flattenNew, string. */
AFL_TEST("server.play.Packer:flattenNew:str", a)
{
    StringValue* sv = new StringValue("xyz");
    std::auto_ptr<Value> p(Packer::flattenNew(sv));

    a.checkEqual("01", Access(p.get()).toString(), "xyz");
}

/** Test flattenNew, ship reference (as specimen for object reference). */
AFL_TEST("server.play.Packer:flattenNew:ref:ship", a)
{
    NullTranslator tx;
    NullFileSystem fs;
    Session session(tx, fs);

    ReferenceContext* rv = new ReferenceContext(Reference(Reference::Ship, 54), session);
    std::auto_ptr<Value> p(Packer::flattenNew(rv));

    a.checkEqual("01", Access(p.get()).getArraySize(), 2U);
    a.checkEqual("02", Access(p.get())[0].toString(), "ship");
    a.checkEqual("03", Access(p.get())[1].toInteger(), 54);
}

/** Test flattenNew, location reference. */
AFL_TEST("server.play.Packer:flattenNew:ref:xy", a)
{
    NullTranslator tx;
    NullFileSystem fs;
    Session session(tx, fs);

    ReferenceContext* rv = new ReferenceContext(Reference(Point(2000,1500)), session);
    std::auto_ptr<Value> p(Packer::flattenNew(rv));

    a.checkEqual("01", Access(p.get()).getArraySize(), 3U);
    a.checkEqual("02", Access(p.get())[0].toString(), "location");
    a.checkEqual("03", Access(p.get())[1].toInteger(), 2000);
    a.checkEqual("04", Access(p.get())[2].toInteger(), 1500);
}

/** Test flattenNew, one-dimensional array. */
AFL_TEST("server.play.Packer:flattenNew:array:1d", a)
{
    Ref<ArrayData> ad = *new ArrayData();
    ad->addDimension(4);
    ad->content().pushBackInteger(7);
    ad->content().pushBackInteger(8);
    ad->content().pushBackInteger(3);
    ad->content().pushBackInteger(4);
    std::auto_ptr<Value> p(Packer::flattenNew(new ArrayValue(ad)));

    a.checkEqual("01", Access(p.get()).getArraySize(), 4U);
    a.checkEqual("02", Access(p.get())[0].toInteger(), 7);
    a.checkEqual("03", Access(p.get())[1].toInteger(), 8);
    a.checkEqual("04", Access(p.get())[2].toInteger(), 3);
    a.checkEqual("05", Access(p.get())[3].toInteger(), 4);
}

/** Test flattenNew, two-dimensional array. */
AFL_TEST("server.play.Packer:flattenNew:array:2d", a)
{
    Ref<ArrayData> ad = *new ArrayData();
    ad->addDimension(3);
    ad->addDimension(2);
    ad->content().pushBackInteger(7);
    ad->content().pushBackInteger(8);
    ad->content().pushBackInteger(3);
    ad->content().pushBackInteger(4);
    ad->content().pushBackInteger(1);
    ad->content().pushBackInteger(9);
    std::auto_ptr<Value> p(Packer::flattenNew(new ArrayValue(ad)));

    a.checkEqual("01", Access(p.get()).getArraySize(), 3U);
    a.checkEqual("02", Access(p.get())[0].getArraySize(), 2U);
    a.checkEqual("03", Access(p.get())[0][0].toInteger(), 7);
    a.checkEqual("04", Access(p.get())[0][1].toInteger(), 8);
    a.checkEqual("05", Access(p.get())[1].getArraySize(), 2U);
    a.checkEqual("06", Access(p.get())[1][0].toInteger(), 3);
    a.checkEqual("07", Access(p.get())[1][1].toInteger(), 4);
    a.checkEqual("08", Access(p.get())[2].getArraySize(), 2U);
    a.checkEqual("09", Access(p.get())[2][0].toInteger(), 1);
    a.checkEqual("10", Access(p.get())[2][1].toInteger(), 9);
}

/** Test flattenNew, two-dimensional array. */
AFL_TEST("server.play.Packer:flattenNew:array:2d-gap", a)
{
    Ref<ArrayData> ad = *new ArrayData();
    ad->addDimension(2);
    ad->addDimension(2);
    ad->content().pushBackNew(0);
    ad->content().pushBackNew(0);
    ad->content().pushBackNew(0);
    ad->content().pushBackInteger(8);
    std::auto_ptr<Value> p(Packer::flattenNew(new ArrayValue(ad)));

    a.checkEqual("01", Access(p.get()).getArraySize(), 2U);
    a.checkEqual("02", Access(p.get())[0].getArraySize(), 2U);
    a.checkNull ("03", Access(p.get())[0][0].getValue());
    a.checkNull ("04", Access(p.get())[0][1].getValue());
    a.checkEqual("05", Access(p.get())[1].getArraySize(), 2U);
    a.checkNull ("06", Access(p.get())[1][0].getValue());
    a.checkEqual("07", Access(p.get())[1][1].toInteger(), 8);
}

/** Test flattenNew, four-dimensional array. */
AFL_TEST("server.play.Packer:flattenNew:array:4d", a)
{
    Ref<ArrayData> ad = *new ArrayData();
    ad->addDimension(1);
    ad->addDimension(1);
    ad->addDimension(1);
    ad->addDimension(1);
    ad->content().pushBackInteger(42);
    std::auto_ptr<Value> p(Packer::flattenNew(new ArrayValue(ad)));

    a.checkEqual("01", Access(p.get()).getArraySize(), 1U);
    a.checkEqual("02", Access(p.get())[0].getArraySize(), 1U);
    a.checkEqual("03", Access(p.get())[0][0].getArraySize(), 1U);
    a.checkEqual("04", Access(p.get())[0][0][0].getArraySize(), 1U);
    a.checkEqual("05", Access(p.get())[0][0][0][0].toInteger(), 42);
}

/** Test flattenNew, array of references (demonstrates nested flattening). */
AFL_TEST("server.play.Packer:flattenNew:array:ref", a)
{
    NullTranslator tx;
    NullFileSystem fs;
    Session session(tx, fs);

    Ref<ArrayData> ad = *new ArrayData();
    ad->addDimension(2);
    ad->pushBackNew(new ReferenceContext(Reference(Point(2000,1500)), session));
    ad->pushBackNew(0);
    std::auto_ptr<Value> p(Packer::flattenNew(new ArrayValue(ad)));

    a.checkEqual("01", Access(p.get()).getArraySize(), 2U);
    a.checkEqual("02", Access(p.get())[0].getArraySize(), 3U);
    a.checkEqual("03", Access(p.get())[0][0].toString(), "location");
    a.checkEqual("04", Access(p.get())[0][1].toInteger(), 2000);
    a.checkEqual("05", Access(p.get())[0][2].toInteger(), 1500);
}
