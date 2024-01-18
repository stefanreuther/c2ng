/**
  *  \file test/game/interface/iteratorcontexttest.cpp
  *  \brief Test for game::interface::IteratorContext
  */

#include "game/interface/iteratorcontext.hpp"

#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/internalsink.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/ref/configuration.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/vmio/nullsavecontext.hpp"

using interpreter::Error;
using interpreter::Context;
using interpreter::test::ContextVerifier;
using game::map::Cursors;

namespace {
    struct TestHarness {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;

        TestHarness()
            : tx(), fs(), session(tx, fs)
            {
                session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
                session.setGame(new game::Game());
                session.setShipList(new game::spec::ShipList());
            }
    };

    game::map::Planet* createPlanet(TestHarness& h, game::Id_t id, int x, int y)
    {
        game::map::Planet* pl = h.session.getGame()->currentTurn().universe().planets().create(id);
        pl->setPosition(game::map::Point(x, y));
        pl->setName("Bob");
        pl->internalCheck(h.session.getGame()->mapConfiguration(), game::PlayerSet_t(), 15, h.session.translator(), h.session.log());
        return pl;
    }

    class Call {
     public:
        Call(afl::test::Assert a, ContextVerifier& verif, const char* name)
            : m_assert(a(name)),
              m_parameters(),
              m_value()
            {
                afl::data::Value* p = verif.getValue(name);
                m_value.reset(dynamic_cast<interpreter::IndexableValue*>(p));
                if (m_value.get() == 0) {
                    delete p;
                    m_assert.fail("expect indexable");
                }
            }

        Call& withInteger(int i)
            { m_parameters.pushBackInteger(i); return *this; }

        Call& withString(const String_t& s)
            { m_parameters.pushBackString(s); return *this; }

        Call& withNull()
            { m_parameters.pushBackNew(0); return *this; }

        afl::data::Value* call()
            {
                interpreter::Arguments args(m_parameters, 0, m_parameters.size());
                return m_value->get(args);
            }

        void checkNull()
            { interpreter::test::verifyNewNull(m_assert, call()); }

        void checkInteger(int value)
            { interpreter::test::verifyNewInteger(m_assert, call(), value); }
        void checkString(const String_t& value)
            { interpreter::test::verifyNewString(m_assert, call(), value.c_str()); }

     private:
        afl::test::Assert m_assert;
        afl::data::Segment m_parameters;
        std::auto_ptr<interpreter::IndexableValue> m_value;
    };
}

/** Test createObjectContext(), ship case. */
AFL_TEST("game.interface.IteratorContext:createObjectContext:ship", a)
{
    // Create session
    TestHarness h;

    // Create ship [must make it visible to be able to access properties]
    game::map::Ship* sh = h.session.getGame()->currentTurn().universe().ships().create(77);
    sh->addShipXYData(game::map::Point(1000, 1000), 3, 100, game::PlayerSet_t(4));
    sh->setName("Alice");
    sh->internalCheck(game::PlayerSet_t(4), 15);
    a.check("01. isVisible", sh->isVisible());

    // Test
    std::auto_ptr<Context> ctx(game::interface::createObjectContext(sh, h.session));

    // Verify
    a.checkNonNull("11. ctx", ctx.get());
    ContextVerifier verif(*ctx, a);
    verif.verifyTypes();
    verif.verifyInteger("ID", 77);
    verif.verifyString("NAME", "Alice");
}

/** Test createObjectContext(), planet case. */
AFL_TEST("game.interface.IteratorContext:createObjectContext:planet", a)
{
    // Create session
    TestHarness h;

    // Create planet
    game::map::Planet* pl = createPlanet(h, 33, 1000, 1000);

    // Test
    std::auto_ptr<Context> ctx(game::interface::createObjectContext(pl, h.session));

    // Verify
    a.checkNonNull("01. get", ctx.get());
    ContextVerifier verif(*ctx, a);
    verif.verifyTypes();
    verif.verifyInteger("ID", 33);
    verif.verifyString("NAME", "Bob");
}

/** Test createObjectContext(), minefield case. */
AFL_TEST("game.interface.IteratorContext:createObjectContext:minefield", a)
{
    // Create session
    TestHarness h;

    // Create minefield
    game::map::Minefield* mf = h.session.getGame()->currentTurn().universe().minefields().create(22);
    mf->addReport(game::map::Point(1000, 2000), 3, game::map::Minefield::IsMine,
                  game::map::Minefield::UnitsKnown, 5000, 50, game::map::Minefield::MinefieldScanned);
    mf->internalCheck(50, h.session.getRoot()->hostVersion(), h.session.getRoot()->hostConfiguration());

    // Test
    std::auto_ptr<Context> ctx(game::interface::createObjectContext(mf, h.session));

    // Verify
    a.checkNonNull("01. get", ctx.get());
    ContextVerifier verif(*ctx, a);
    verif.verifyTypes();
    verif.verifyInteger("ID", 22);
    verif.verifyInteger("UNITS", 5000);
}

/** Test createObjectContext(), ion storm case. */
AFL_TEST("game.interface.IteratorContext:createObjectContext:ionstorm", a)
{
    // Create session
    TestHarness h;

    // Create storm
    game::map::IonStorm* ion = h.session.getGame()->currentTurn().universe().ionStorms().create(42);
    ion->setName("Baerbel");
    ion->setVoltage(10);
    a.check("01", ion->isActive());

    // Test
    std::auto_ptr<Context> ctx(game::interface::createObjectContext(ion, h.session));

    // Verify
    a.checkNonNull("11. get", ctx.get());
    ContextVerifier verif(*ctx, a);
    verif.verifyTypes();
    verif.verifyInteger("ID", 42);
    verif.verifyString("NAME", "Baerbel");
}

/** Test IteratorContext basics. */
AFL_TEST("game.interface.IteratorContext:basics", a)
{
    // A minimal IteratorProvider
    class TestIteratorProvider : public game::interface::IteratorProvider {
     public:
        TestIteratorProvider(game::Session& s)
            : m_session(s)
            { }
        virtual game::map::ObjectCursor* getCursor()
            { return 0; }
        virtual game::map::ObjectType* getType()
            { return 0; }
        virtual int getCursorNumber()
            { return 42; }
        virtual game::Session& getSession()
            { return m_session; }
        virtual void store(interpreter::TagNode& out)
            { out.tag = 0x2233; out.value = 0x77778888; }
        virtual String_t toString()
            { return "TestIteratorProvider"; }
     private:
        game::Session& m_session;
    };

    // Create environment
    TestHarness h;

    // Create testee
    game::interface::IteratorContext ctx(*new TestIteratorProvider(h.session));

    // Verify
    ContextVerifier verif(ctx, a);
    verif.verifyTypes();
    verif.verifyInteger("SCREEN", 42);
    verif.verifyNull("CURRENTINDEX");
    verif.verifyNull("COUNT");

    a.checkEqual("01. toString", ctx.toString(true), "TestIteratorProvider");
    a.checkDifferent("02. toString", ctx.toString(false), "");

    a.checkNull("11. get", ctx.getObject());

    std::auto_ptr<Context> copy(ctx.clone());
    a.checkNonNull("21. clone", copy.get());
    ContextVerifier(*copy, a("clone")).verifyInteger("SCREEN", 42);

    interpreter::TagNode out;
    afl::io::InternalSink aux;
    interpreter::vmio::NullSaveContext saveContext;
    ctx.store(out, aux, saveContext);
    a.checkEqual("31. tag", out.tag, 0x2233U);
    a.checkEqual("32. value", out.value, 0x77778888U);
    a.checkEqual("33. content", aux.getContent().size(), 0U);
}

/** Test IteratorContext, native creation. */
AFL_TEST("game.interface.IteratorContext:makeIteratorValue:success", a)
{
    // Environment
    TestHarness h;
    createPlanet(h, 100, 2000, 2000);

    // Create using makeIteratorValue
    std::auto_ptr<Context> ctx(game::interface::makeIteratorValue(h.session, Cursors::AllPlanets));
    a.checkNonNull("01. get", ctx.get());
    ContextVerifier verif(*ctx, a);
    verif.verifyTypes();
    verif.verifyInteger("SCREEN", Cursors::AllPlanets);
    verif.verifyInteger("COUNT", 1);
}

/** Test IteratorContext, native creation, failure cases. */
AFL_TEST("game.interface.IteratorContext:makeIteratorValue:error:range", a)
{
    // Out-of-range
    TestHarness h;
    a.checkNull("", game::interface::makeIteratorValue(h.session, -1));
}

AFL_TEST("game.interface.IteratorContext:makeIteratorValue:error:empty-session", a)
{
    // Empty session
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session empty(tx, fs);
    a.checkNull("", game::interface::makeIteratorValue(empty, Cursors::AllPlanets));
}

/** Test IteratorContext, script creation. */
AFL_TEST("game.interface.IteratorContext:IFIterator:success", a)
{
    // Environment
    TestHarness h;
    createPlanet(h, 100, 2000, 2000);

    // Create using IFIterator
    afl::data::Segment seg;
    seg.pushBackInteger(Cursors::AllPlanets);
    interpreter::Arguments args(seg, 0, 1);

    std::auto_ptr<afl::data::Value> p(game::interface::IFIterator(h.session, args));
    a.checkNonNull("01. result", p.get());

    Context* ctx = dynamic_cast<Context*>(p.get());
    a.checkNonNull("11. ctx", ctx);
    ContextVerifier verif(*ctx, a);
    verif.verifyTypes();
    verif.verifyInteger("SCREEN", Cursors::AllPlanets);
    verif.verifyInteger("COUNT", 1);
}

/** Test IteratorContext, script creation, failure. */
// Out-of-range
AFL_TEST("game.interface.IteratorContext:IFIterator:error:range", a)
{
    TestHarness h;
    afl::data::Segment seg;
    seg.pushBackInteger(-1);
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFIterator(h.session, args), Error);
}

// Empty session
AFL_TEST("game.interface.IteratorContext:IFIterator:error:empty", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session empty(tx, fs);
    afl::data::Segment seg;
    seg.pushBackInteger(Cursors::AllPlanets);
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFIterator(empty, args), Error);
}

// Wrong number of parameters
AFL_TEST("game.interface.IteratorContext:IFIterator:error:arity", a)
{
    TestHarness h;
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, game::interface::IFIterator(h.session, args), Error);
}

// Null parameter
AFL_TEST("game.interface.IteratorContext:IFIterator:null", a)
{
    TestHarness h;
    afl::data::Segment seg;
    seg.pushBackNew(0);
    interpreter::Arguments args(seg, 0, 1);
    a.checkNull("", game::interface::IFIterator(h.session, args));
}

/** Test IteratorContext properties. */
AFL_TEST("game.interface.IteratorContext:properties", a)
{
    // Environment
    TestHarness h;
    createPlanet(h, 10, 1000, 1000);
    createPlanet(h, 20, 1000, 1200);
    createPlanet(h, 30, 1000, 1300)->setIsMarked(true);
    createPlanet(h, 40, 1000, 1000)->setIsMarked(true);
    createPlanet(h, 50, 1000, 1400);

    // Object under test
    std::auto_ptr<Context> ctx(game::interface::makeIteratorValue(h.session, Cursors::AllPlanets));
    a.checkNonNull("01. get", ctx.get());

    // Verify human-friendly stringification
    a.checkEqual("11. toString", ctx->toString(true), "Iterator(22)");

    // Serialisation
    interpreter::TagNode out;
    afl::io::InternalSink aux;
    interpreter::vmio::NullSaveContext saveContext;
    ctx->store(out, aux, saveContext);
    a.checkEqual("21. tag", out.tag, interpreter::TagNode::Tag_Iterator);
    a.checkEqual("22. value", out.value, uint32_t(Cursors::AllPlanets));
    a.checkEqual("23. content", aux.getContent().size(), 0U);

    // Verify scalars
    ContextVerifier verif(*ctx, a);
    verif.verifyTypes();
    verif.verifyInteger("COUNT", 5);
    verif.verifyNull("CURRENTINDEX");
    verif.verifyInteger("SCREEN", Cursors::AllPlanets);

    // Cannot assign current because there's no cursor behind
    {
        afl::data::IntegerValue iv(20);
        Context::PropertyIndex_t idx(0);
        Context::PropertyAccessor* pa = ctx->lookup("CURRENTINDEX", idx);
        a.checkNonNull("31. CURRENTINDEX", pa);
        AFL_CHECK_THROWS(a("32. set"), pa->set(idx, &iv), Error);
    }

    // Verify functions
    // - Id
    Call(a("41. ID(10)"),   verif, "ID").withInteger(10).checkInteger(10);
    Call(a("42. ID(null)"), verif, "ID").withNull().checkNull();
    AFL_CHECK_THROWS(a("43. ID()"), Call(a, verif, "ID").call(), Error);

    // - Index
    Call(a("51. INDEX(10)"),   verif, "INDEX").withInteger(10).checkInteger(10);
    Call(a("52. INDEX(null)"), verif, "INDEX").withNull().checkNull();
    AFL_CHECK_THROWS(a("53. INDEX()"), Call(a, verif, "INDEX").call(), Error);

    // - NearestIndex
    Call(a("61. NEARESTINDEX(x,y)"),    verif, "NEARESTINDEX").withInteger(1010).withInteger(1290).checkInteger(30);
    Call(a("62. NEARESTINDEX(x,null)"), verif, "NEARESTINDEX").withInteger(1010).withNull().checkNull();
    AFL_CHECK_THROWS(a("63. NEARESTINDEX()"), Call(a, verif, "NEARESTINDEX").call(), Error);

    // - NextIndex
    Call(a("71. NEXTINDEX(null)"), verif, "NEXTINDEX").withNull().checkNull();
    Call(a("72. NEXTINDEX(0)"),    verif, "NEXTINDEX").withInteger(0).checkInteger(10);
    Call(a("73. NEXTINDEX(0,M)"),  verif, "NEXTINDEX").withInteger(0).withString("M").checkInteger(30);
    Call(a("74. NEXTINDEX(20)"),   verif, "NEXTINDEX").withInteger(20).checkInteger(30);
    Call(a("75. NEXTINDEX(50)"),   verif, "NEXTINDEX").withInteger(50).checkInteger(0);
    Call(a("76. NEXTINDEX(50,W)"), verif, "NEXTINDEX").withInteger(50).withString("W").checkInteger(10);
    AFL_CHECK_THROWS(a("77. NEXTINDEX()"), Call(a, verif, "NEXTINDEX").call(), Error);

    // - NextIndexAt
    AFL_CHECK_THROWS(a("81. NEXTINDEXAT()"), Call(a, verif, "NEXTINDEXAT").call(), Error);
    Call(a("82. NEXTINDEXAT(null,null)"),      verif, "NEXTINDEXAT").withNull().withNull().withNull().checkNull();
    Call(a("83. NEXTINDEXAT(0,1000,1000)"),    verif, "NEXTINDEXAT").withInteger(0).withInteger(1000).withInteger(1000).checkInteger(10);
    Call(a("84. NEXTINDEXAT(10,1000,1000)"),   verif, "NEXTINDEXAT").withInteger(10).withInteger(1000).withInteger(1000).checkInteger(40);
    Call(a("85. NEXTINDEXAT(0,1000,1000,M)"),  verif, "NEXTINDEXAT").withInteger(0).withInteger(1000).withInteger(1000).withString("M").checkInteger(40);
    Call(a("86. NEXTINDEXAT(40,1000,1000)"),   verif, "NEXTINDEXAT").withInteger(40).withInteger(1000).withInteger(1000).checkInteger(0);
    Call(a("87. NEXTINDEXAT(40,1000,1000,W)"), verif, "NEXTINDEXAT").withInteger(40).withInteger(1000).withInteger(1000).withString("W").checkInteger(10);

    // - Object
    AFL_CHECK_THROWS(a("91. OBJECT()"), Call(a, verif, "OBJECT").call(), Error);
    {
        std::auto_ptr<afl::data::Value> p(Call(a, verif, "OBJECT").withInteger(20).call());
        a.checkNonNull("92. get", p.get());
        Context* objectContext = dynamic_cast<Context*>(p.get());
        a.checkNonNull("93. Context", objectContext);
        ContextVerifier objectVerif(*objectContext, a("OBJECT"));
        objectVerif.verifyInteger("ID", 20);
        objectVerif.verifyString("TYPE", "Planet");
    }

    // - PreviousIndex
    Call(a("101. PREVIOUSINDEX(null)"), verif, "PREVIOUSINDEX").withNull().checkNull();
    Call(a("102. PREVIOUSINDEX(0)"),    verif, "PREVIOUSINDEX").withInteger(0).checkInteger(50);
    Call(a("103. PREVIOUSINDEX(0,M)"),  verif, "PREVIOUSINDEX").withInteger(0).withString("M").checkInteger(40);
    Call(a("104. PREVIOUSINDEX(30)"),   verif, "PREVIOUSINDEX").withInteger(30).checkInteger(20);
    Call(a("105. PREVIOUSINDEX(10)"),   verif, "PREVIOUSINDEX").withInteger(10).checkInteger(0);
    Call(a("106. PREVIOUSINDEX(10,W)"), verif, "PREVIOUSINDEX").withInteger(10).withString("W").checkInteger(50);
    AFL_CHECK_THROWS(a("107. PREVIOUSINDEX()"), Call(a, verif, "PREVIOUSINDEX").call(), Error);

    // - PreviousIndexAt
    Call(a("111. PREVIOUSINDEXAT(null,null,null)"), verif, "PREVIOUSINDEXAT").withNull().withNull().withNull().checkNull();
    Call(a("112. PREVIOUSINDEXAT(0,1000,1000)"),    verif, "PREVIOUSINDEXAT").withInteger(0).withInteger(1000).withInteger(1000).checkInteger(40);
    Call(a("113. PREVIOUSINDEXAT(40,1000,1000)"),   verif, "PREVIOUSINDEXAT").withInteger(40).withInteger(1000).withInteger(1000).checkInteger(10);
    Call(a("114. PREVIOUSINDEXAT(0,1000,1000,M)"),  verif, "PREVIOUSINDEXAT").withInteger(0).withInteger(1000).withInteger(1000).withString("M").checkInteger(40);
    Call(a("115. PREVIOUSINDEXAT(0,1000,1000)"),    verif, "PREVIOUSINDEXAT").withInteger(10).withInteger(1000).withInteger(1000).checkInteger(0);
    Call(a("116. PREVIOUSINDEXAT(10,1000,1000)"),   verif, "PREVIOUSINDEXAT").withInteger(10).withInteger(1000).withInteger(1000).withString("W").checkInteger(40);
    AFL_CHECK_THROWS(a("117. PREVIOUSINDEXAT()"), Call(a, verif, "PREVIOUSINDEXAT").call(), Error);
}

/** Test access to and manipulation of "Current". */
AFL_TEST("game.interface.IteratorContext:Current", a)
{
    // Environment
    TestHarness h;

    // Create ion storms
    for (int i = 5; i <= 10; ++i) {
        game::map::IonStorm* ion = h.session.getGame()->currentTurn().universe().ionStorms().create(i);
        ion->setName("Baerbel");
        ion->setVoltage(10);
        a.check("01. isActive", ion->isActive());
    }
    h.session.getGame()->currentTurn().universe().ionStormType().sig_setChange.raise(0);
    a.checkEqual("02. getCurrentIndex", h.session.getGame()->cursors().currentIonStorm().getCurrentIndex(), 5);

    // Object under test
    std::auto_ptr<Context> ctx(game::interface::makeIteratorValue(h.session, Cursors::IonStorms));
    a.checkNonNull("11. get", ctx.get());

    // Verify human-friendly stringification
    a.checkEqual("21. toString", ctx->toString(true), "Iterator(31)");

    // Initial value of Current
    ContextVerifier verif(*ctx, a);
    verif.verifyInteger("CURRENTINDEX", 5);

    // Change current
    afl::data::IntegerValue iv(8);
    Context::PropertyIndex_t idx(0);
    Context::PropertyAccessor* pa = ctx->lookup("CURRENTINDEX", idx);
    a.checkNonNull("31. PropertyAccessor", pa);
    pa->set(idx, &iv);

    // Verify changed value
    a.checkEqual("41. getCurrentIndex", h.session.getGame()->cursors().currentIonStorm().getCurrentIndex(), 8);
    verif.verifyInteger("CURRENTINDEX", 8);

    // Assigning null is ignored
    pa->set(idx, 0);

    // Assigning out-of-range fails
    afl::data::IntegerValue iv2(100);
    AFL_CHECK_THROWS(a("51. out-of-range"), pa->set(idx, &iv2), Error);

    // Assigning out-of-range fails
    afl::data::IntegerValue iv3(-1);
    AFL_CHECK_THROWS(a("61. out-of-range"), pa->set(idx, &iv3), Error);

    // Assigning wrong type fails
    afl::data::StringValue sv("x");
    AFL_CHECK_THROWS(a("71. type-error"), pa->set(idx, &sv), Error);

    // Value still unchanged
    verif.verifyInteger("CURRENTINDEX", 8);
}

/** Test IteratorContext, sorted iteration. */
AFL_TEST("game.interface.IteratorContext:sorted-iteration", a)
{
    // Environment
    TestHarness h;
    createPlanet(h, 10, 1000, 1000)->setName("e");
    createPlanet(h, 20, 1000, 1200)->setName("d");
    createPlanet(h, 30, 1000, 1300)->setName("a");
    createPlanet(h, 40, 1000, 1000)->setName("b");
    createPlanet(h, 50, 1000, 1400)->setName("c");

    h.session.getRoot()->userConfiguration()[game::config::UserConfiguration::Sort_Ship].set(game::ref::ConfigSortByName);

    // Object under test
    std::auto_ptr<Context> ctx(game::interface::makeIteratorValue(h.session, Cursors::AllPlanets));
    a.checkNonNull("01. get", ctx.get());

    // Verify
    ContextVerifier verif(*ctx, a);

    // - NextIndex
    Call(a("NEXTINDEX(0)"),  verif, "NEXTINDEX").withInteger(0).withString("S").checkInteger(30);
    Call(a("NEXTINDEX(30)"), verif, "NEXTINDEX").withInteger(30).withString("S").checkInteger(40);
    Call(a("NEXTINDEX(40)"), verif, "NEXTINDEX").withInteger(40).withString("S").checkInteger(50);
    Call(a("NEXTINDEX(50)"), verif, "NEXTINDEX").withInteger(50).withString("S").checkInteger(20);
    Call(a("NEXTINDEX(20)"), verif, "NEXTINDEX").withInteger(20).withString("S").checkInteger(10);
    Call(a("NEXTINDEX(10)"), verif, "NEXTINDEX").withInteger(10).withString("S").checkInteger(0);
}
