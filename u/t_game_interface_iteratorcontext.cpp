/**
  *  \file u/t_game_interface_iteratorcontext.cpp
  *  \brief Test for game::interface::IteratorContext
  */

#include "game/interface/iteratorcontext.hpp"

#include "t_game_interface.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/internalsink.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
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
            {
                std::auto_ptr<afl::data::Value> result(call());
                m_assert.check("expect null", result.get() == 0);
            }

        void checkInteger(int value)
            {
                std::auto_ptr<afl::data::Value> result(call());
                m_assert.check("expect non-null", result.get() != 0);

                afl::data::IntegerValue* iv = dynamic_cast<afl::data::IntegerValue*>(result.get());
                m_assert.check("expect integer", iv != 0);
                m_assert.checkEqual("expect value", iv->getValue(), value);
            }

        void checkString(const String_t& value)
            {
                std::auto_ptr<afl::data::Value> result(call());
                m_assert.check("expect non-null", result.get() != 0);

                afl::data::StringValue* sv = dynamic_cast<afl::data::StringValue*>(result.get());
                m_assert.check("expect string", sv != 0);
                m_assert.checkEqual("expect value", sv->getValue(), value);
            }

     private:
        afl::test::Assert m_assert;
        afl::data::Segment m_parameters;
        std::auto_ptr<interpreter::IndexableValue> m_value;
    };
}

/** Test createObjectContext(), ship case. */
void
TestGameInterfaceIteratorContext::testCreateObjectShip()
{
    // Create session
    TestHarness h;

    // Create ship [must make it visible to be able to access properties]
    game::map::Ship* sh = h.session.getGame()->currentTurn().universe().ships().create(77);
    sh->addShipXYData(game::map::Point(1000, 1000), 3, 100, game::PlayerSet_t(4));
    sh->setName("Alice");
    sh->internalCheck(game::PlayerSet_t(4), 15);
    TS_ASSERT(sh->isVisible());

    // Test
    std::auto_ptr<Context> ctx(game::interface::createObjectContext(sh, h.session));

    // Verify
    TS_ASSERT(ctx.get() != 0);
    ContextVerifier verif(*ctx, "testCreateObjectShip");
    verif.verifyTypes();
    verif.verifyInteger("ID", 77);
    verif.verifyString("NAME", "Alice");
}

/** Test createObjectContext(), planet case. */
void
TestGameInterfaceIteratorContext::testCreateObjectPlanet()
{
    // Create session
    TestHarness h;

    // Create planet
    game::map::Planet* pl = createPlanet(h, 33, 1000, 1000);

    // Test
    std::auto_ptr<Context> ctx(game::interface::createObjectContext(pl, h.session));

    // Verify
    TS_ASSERT(ctx.get() != 0);
    ContextVerifier verif(*ctx, "testCreateObjectPlanet");
    verif.verifyTypes();
    verif.verifyInteger("ID", 33);
    verif.verifyString("NAME", "Bob");
}

/** Test createObjectContext(), minefield case. */
void
TestGameInterfaceIteratorContext::testCreateObjectMinefield()
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
    TS_ASSERT(ctx.get() != 0);
    ContextVerifier verif(*ctx, "testCreateObjectMinefield");
    verif.verifyTypes();
    verif.verifyInteger("ID", 22);
    verif.verifyInteger("UNITS", 5000);
}

/** Test createObjectContext(), ion storm case. */
void
TestGameInterfaceIteratorContext::testCreateObjectIonStorm()
{
    // Create session
    TestHarness h;

    // Create minefield
    game::map::IonStorm* ion = h.session.getGame()->currentTurn().universe().ionStorms().create(42);
    ion->setName("Baerbel");
    ion->setVoltage(10);
    TS_ASSERT(ion->isActive());

    // Test
    std::auto_ptr<Context> ctx(game::interface::createObjectContext(ion, h.session));

    // Verify
    TS_ASSERT(ctx.get() != 0);
    ContextVerifier verif(*ctx, "testCreateObjectIonStorm");
    verif.verifyTypes();
    verif.verifyInteger("ID", 42);
    verif.verifyString("NAME", "Baerbel");
}

/** Test IteratorContext basics. */
void
TestGameInterfaceIteratorContext::testIteratorContextBasics()
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
    ContextVerifier verif(ctx, "testIteratorContextBasics");
    verif.verifyTypes();
    verif.verifyInteger("SCREEN", 42);
    verif.verifyNull("CURRENTINDEX");
    verif.verifyNull("COUNT");

    TS_ASSERT_EQUALS(ctx.toString(true), "TestIteratorProvider");
    TS_ASSERT_DIFFERS(ctx.toString(false), "");

    TS_ASSERT(ctx.getObject() == 0);

    std::auto_ptr<Context> copy(ctx.clone());
    TS_ASSERT(copy.get() != 0);
    ContextVerifier(*copy, "testIteratorContextBasics::copy").verifyInteger("SCREEN", 42);

    interpreter::TagNode out;
    afl::io::InternalSink aux;
    interpreter::vmio::NullSaveContext saveContext;
    ctx.store(out, aux, saveContext);
    TS_ASSERT_EQUALS(out.tag, 0x2233U);
    TS_ASSERT_EQUALS(out.value, 0x77778888U);
    TS_ASSERT_EQUALS(aux.getContent().size(), 0U);
}

/** Test IteratorContext, native creation. */
void
TestGameInterfaceIteratorContext::testIteratorContextNativeCreate()
{
    // Environment
    TestHarness h;
    createPlanet(h, 100, 2000, 2000);

    // Create using makeIteratorValue
    std::auto_ptr<Context> ctx(game::interface::makeIteratorValue(h.session, Cursors::AllPlanets));
    TS_ASSERT(ctx.get() != 0);
    ContextVerifier verif(*ctx, "testIteratorContextNativeCreate");
    verif.verifyTypes();
    verif.verifyInteger("SCREEN", Cursors::AllPlanets);
    verif.verifyInteger("COUNT", 1);
}

/** Test IteratorContext, native creation, failure cases. */
void
TestGameInterfaceIteratorContext::testIteratorContextNativeCreateFail()
{
    // Environment
    TestHarness h;

    // Out-of-range
    TS_ASSERT(game::interface::makeIteratorValue(h.session, -1) == 0);

    // Empty session
    game::Session empty(h.tx, h.fs);
    TS_ASSERT(game::interface::makeIteratorValue(empty, Cursors::AllPlanets) == 0);
}

/** Test IteratorContext, script creation. */
void
TestGameInterfaceIteratorContext::testIteratorContextScriptCreate()
{
    // Environment
    TestHarness h;
    createPlanet(h, 100, 2000, 2000);

    // Create using IFIterator
    afl::data::Segment seg;
    seg.pushBackInteger(Cursors::AllPlanets);
    interpreter::Arguments args(seg, 0, 1);

    std::auto_ptr<afl::data::Value> p(game::interface::IFIterator(h.session, args));
    TS_ASSERT(p.get() != 0);

    Context* ctx = dynamic_cast<Context*>(p.get());
    TS_ASSERT(ctx != 0);
    ContextVerifier verif(*ctx, "testIteratorContextScriptCreate");
    verif.verifyTypes();
    verif.verifyInteger("SCREEN", Cursors::AllPlanets);
    verif.verifyInteger("COUNT", 1);
}

/** Test IteratorContext, script creation, failure. */
void
TestGameInterfaceIteratorContext::testIteratorContextScriptCreateFail()
{
    // Environment
    TestHarness h;

    // Out-of-range
    {
        afl::data::Segment seg;
        seg.pushBackInteger(-1);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFIterator(h.session, args), Error);
    }

    // Empty session
    {
        game::Session empty(h.tx, h.fs);
        afl::data::Segment seg;
        seg.pushBackInteger(Cursors::AllPlanets);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFIterator(empty, args), Error);
    }

    // Wrong number of parameters
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(game::interface::IFIterator(h.session, args), Error);
    }

    // Null parameter
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT(game::interface::IFIterator(h.session, args) == 0);
    }
}

/** Test IteratorContext properties. */
void
TestGameInterfaceIteratorContext::testIteratorContextProperties()
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
    TS_ASSERT(ctx.get() != 0);

    // Verify human-friendly stringification
    TS_ASSERT_EQUALS(ctx->toString(true), "Iterator(22)");

    // Serialisation
    interpreter::TagNode out;
    afl::io::InternalSink aux;
    interpreter::vmio::NullSaveContext saveContext;
    ctx->store(out, aux, saveContext);
    TS_ASSERT_EQUALS(out.tag, interpreter::TagNode::Tag_Iterator);
    TS_ASSERT_EQUALS(out.value, uint32_t(Cursors::AllPlanets));
    TS_ASSERT_EQUALS(aux.getContent().size(), 0U);

    // Verify scalars
    afl::test::Assert a("testIteratorContextProperties");
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
        TS_ASSERT(pa != 0);
        TS_ASSERT_THROWS(pa->set(idx, &iv), Error);
    }

    // Verify functions
    // - Id
    Call(a, verif, "ID").withInteger(10).checkInteger(10);
    Call(a, verif, "ID").withNull().checkNull();
    TS_ASSERT_THROWS(Call(a, verif, "ID").call(), Error);

    // - Index
    Call(a, verif, "INDEX").withInteger(10).checkInteger(10);
    Call(a, verif, "INDEX").withNull().checkNull();
    TS_ASSERT_THROWS(Call(a, verif, "INDEX").call(), Error);

    // - NearestIndex
    Call(a, verif, "NEARESTINDEX").withInteger(1010).withInteger(1290).checkInteger(30);
    Call(a, verif, "NEARESTINDEX").withInteger(1010).withNull().checkNull();
    TS_ASSERT_THROWS(Call(a, verif, "NEARESTINDEX").call(), Error);

    // - NextIndex
    Call(a, verif, "NEXTINDEX").withNull().checkNull();
    Call(a, verif, "NEXTINDEX").withInteger(0).checkInteger(10);
    Call(a, verif, "NEXTINDEX").withInteger(0).withString("M").checkInteger(30);
    Call(a, verif, "NEXTINDEX").withInteger(20).checkInteger(30);
    Call(a, verif, "NEXTINDEX").withInteger(50).checkInteger(0);
    Call(a, verif, "NEXTINDEX").withInteger(50).withString("W").checkInteger(10);
    TS_ASSERT_THROWS(Call(a, verif, "NEXTINDEX").call(), Error);

    // - NextIndexAt
    TS_ASSERT_THROWS(Call(a, verif, "NEXTINDEXAT").call(), Error);
    Call(a, verif, "NEXTINDEXAT").withNull().withNull().withNull().checkNull();
    Call(a, verif, "NEXTINDEXAT").withInteger(0).withInteger(1000).withInteger(1000).checkInteger(10);
    Call(a, verif, "NEXTINDEXAT").withInteger(10).withInteger(1000).withInteger(1000).checkInteger(40);
    Call(a, verif, "NEXTINDEXAT").withInteger(0).withInteger(1000).withInteger(1000).withString("M").checkInteger(40);
    Call(a, verif, "NEXTINDEXAT").withInteger(40).withInteger(1000).withInteger(1000).checkInteger(0);
    Call(a, verif, "NEXTINDEXAT").withInteger(40).withInteger(1000).withInteger(1000).withString("W").checkInteger(10);

    // - Object
    TS_ASSERT_THROWS(Call(a, verif, "OBJECT").call(), Error);
    {
        std::auto_ptr<afl::data::Value> p(Call(a, verif, "OBJECT").withInteger(20).call());
        TS_ASSERT(p.get() != 0);
        Context* objectContext = dynamic_cast<Context*>(p.get());
        TS_ASSERT(objectContext != 0);
        ContextVerifier objectVerif(*objectContext, a("OBJECT"));
        objectVerif.verifyInteger("ID", 20);
        objectVerif.verifyString("TYPE", "Planet");
    }

    // - PreviousIndex
    Call(a, verif, "PREVIOUSINDEX").withNull().checkNull();
    Call(a, verif, "PREVIOUSINDEX").withInteger(0).checkInteger(50);
    Call(a, verif, "PREVIOUSINDEX").withInteger(0).withString("M").checkInteger(40);
    Call(a, verif, "PREVIOUSINDEX").withInteger(30).checkInteger(20);
    Call(a, verif, "PREVIOUSINDEX").withInteger(10).checkInteger(0);
    Call(a, verif, "PREVIOUSINDEX").withInteger(10).withString("W").checkInteger(50);
    TS_ASSERT_THROWS(Call(a, verif, "PREVIOUSINDEX").call(), Error);

    // - PreviousIndexAt
    Call(a, verif, "PREVIOUSINDEXAT").withNull().withNull().withNull().checkNull();
    Call(a, verif, "PREVIOUSINDEXAT").withInteger(0).withInteger(1000).withInteger(1000).checkInteger(40);
    Call(a, verif, "PREVIOUSINDEXAT").withInteger(40).withInteger(1000).withInteger(1000).checkInteger(10);
    Call(a, verif, "PREVIOUSINDEXAT").withInteger(0).withInteger(1000).withInteger(1000).withString("M").checkInteger(40);
    Call(a, verif, "PREVIOUSINDEXAT").withInteger(10).withInteger(1000).withInteger(1000).checkInteger(0);
    Call(a, verif, "PREVIOUSINDEXAT").withInteger(10).withInteger(1000).withInteger(1000).withString("W").checkInteger(40);
    TS_ASSERT_THROWS(Call(a, verif, "PREVIOUSINDEXAT").call(), Error);
}

/** Test access to and manipulation of "Current". */
void
TestGameInterfaceIteratorContext::testIteratorContextCurrent()
{
    // Environment
    TestHarness h;

    // Create ion storms
    for (int i = 5; i <= 10; ++i) {
        game::map::IonStorm* ion = h.session.getGame()->currentTurn().universe().ionStorms().create(i);
        ion->setName("Baerbel");
        ion->setVoltage(10);
        TS_ASSERT(ion->isActive());
    }
    h.session.getGame()->currentTurn().universe().ionStormType().sig_setChange.raise(0);
    TS_ASSERT_EQUALS(h.session.getGame()->cursors().currentIonStorm().getCurrentIndex(), 5);

    // Object under test
    std::auto_ptr<Context> ctx(game::interface::makeIteratorValue(h.session, Cursors::IonStorms));
    TS_ASSERT(ctx.get() != 0);

    // Verify human-friendly stringification
    TS_ASSERT_EQUALS(ctx->toString(true), "Iterator(31)");

    // Initial value of Current
    ContextVerifier verif(*ctx, "testIteratorContextCurrent");
    verif.verifyInteger("CURRENTINDEX", 5);

    // Change current
    afl::data::IntegerValue iv(8);
    Context::PropertyIndex_t idx(0);
    Context::PropertyAccessor* pa = ctx->lookup("CURRENTINDEX", idx);
    TS_ASSERT(pa != 0);
    pa->set(idx, &iv);

    // Verify changed value
    TS_ASSERT_EQUALS(h.session.getGame()->cursors().currentIonStorm().getCurrentIndex(), 8);
    verif.verifyInteger("CURRENTINDEX", 8);

    // Assigning null is ignored
    pa->set(idx, 0);

    // Assigning out-of-range fails
    afl::data::IntegerValue iv2(100);
    TS_ASSERT_THROWS(pa->set(idx, &iv2), Error);

    // Assigning out-of-range fails
    afl::data::IntegerValue iv3(-1);
    TS_ASSERT_THROWS(pa->set(idx, &iv3), Error);

    // Assigning wrong type fails
    afl::data::StringValue sv("x");
    TS_ASSERT_THROWS(pa->set(idx, &sv), Error);

    // Value still unchanged
    verif.verifyInteger("CURRENTINDEX", 8);
}

/** Test IteratorContext, sorted iteration. */
void
TestGameInterfaceIteratorContext::testIteratorContextSorted()
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
    TS_ASSERT(ctx.get() != 0);

    // Verify
    afl::test::Assert a("testIteratorContextProperties");
    ContextVerifier verif(*ctx, a);

    // - NextIndexAt
    Call(a, verif, "NEXTINDEX").withInteger(0).withString("S").checkInteger(30);
    Call(a, verif, "NEXTINDEX").withInteger(30).withString("S").checkInteger(40);
    Call(a, verif, "NEXTINDEX").withInteger(40).withString("S").checkInteger(50);
    Call(a, verif, "NEXTINDEX").withInteger(50).withString("S").checkInteger(20);
    Call(a, verif, "NEXTINDEX").withInteger(20).withString("S").checkInteger(10);
    Call(a, verif, "NEXTINDEX").withInteger(10).withString("S").checkInteger(0);
}

