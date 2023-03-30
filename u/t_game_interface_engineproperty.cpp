/**
  *  \file u/t_game_interface_engineproperty.cpp
  *  \brief Test for game::interface::EngineProperty
  */

#include "game/interface/engineproperty.hpp"

#include <memory>
#include "t_game_interface.hpp"
#include "afl/data/access.hpp"
#include "afl/data/integervalue.hpp"
#include "game/spec/shiplist.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/tagnode.hpp"
#include "interpreter/test/valueverifier.hpp"

/** Test getEngineProperty. */
void
TestGameInterfaceEngineProperty::testGet()
{
    // Configure an engine.
    // This is the PList enerpsi drive.
    game::spec::Engine e(6);
    e.setFuelFactor(1, 51);
    e.setFuelFactor(2, 232);
    e.setFuelFactor(3, 585);
    e.setFuelFactor(4, 1152);
    e.setFuelFactor(5, 1975);
    e.setFuelFactor(6, 3096);
    e.setFuelFactor(7, 4557);
    e.setFuelFactor(8, 6400);
    e.setFuelFactor(9, 16200);

    TS_ASSERT_EQUALS(e.getMaxEfficientWarp(), 8);

    // Check iepEfficientWarp
    std::auto_ptr<afl::data::Value> p(game::interface::getEngineProperty(e, game::interface::iepEfficientWarp));
    TS_ASSERT_EQUALS(afl::data::Access(p.get()).toInteger(), 8);

    // Check iepFuelFactor
    p.reset(game::interface::getEngineProperty(e, game::interface::iepFuelFactor));
    interpreter::IndexableValue* iv = dynamic_cast<interpreter::IndexableValue*>(p.get());
    TS_ASSERT(iv != 0);

    // - self-description
    TS_ASSERT_EQUALS(iv->getDimension(0), 1);
    TS_ASSERT_EQUALS(iv->getDimension(1), e.MAX_WARP+1);
    TS_ASSERT_EQUALS(iv->toString(false), "#<array>");

    // - not iterable, not serializable
    TS_ASSERT_THROWS(iv->makeFirstContext(), interpreter::Error);
    interpreter::test::ValueVerifier(*iv, "testGet").verifyNotSerializable();

    // - accessing values
    {
        // index 0
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> value(iv->get(args));
        TS_ASSERT(value.get() != 0);
        TS_ASSERT_EQUALS(afl::data::Access(value.get()).toInteger(), 0);
    }
    {
        // index 8
        afl::data::Segment seg;
        seg.pushBackInteger(8);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> value(iv->get(args));
        TS_ASSERT(value.get() != 0);
        TS_ASSERT_EQUALS(afl::data::Access(value.get()).toInteger(), 6400);
    }
    {
        // null
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> value(iv->get(args));
        TS_ASSERT(value.get() == 0);
    }
    {
        // wrong number of parameters
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(iv->get(args), interpreter::Error);
    }
    {
        // set
        afl::data::Segment seg;
        seg.pushBackInteger(8);
        interpreter::Arguments args(seg, 0, 1);
        afl::data::IntegerValue nv(5000);
        TS_ASSERT_THROWS(iv->set(args, &nv), interpreter::Error);
    }
}

/** Test setEngineProperty. */
void
TestGameInterfaceEngineProperty::testSet()
{
    game::spec::ShipList list;
    game::spec::Engine& e = *list.engines().create(6);

    // Set iepEfficientWarp
    {
        afl::data::IntegerValue iv(5);
        setEngineProperty(e, game::interface::iepEfficientWarp, &iv, list);
        TS_ASSERT_EQUALS(e.getMaxEfficientWarp(), 5);
    }

    // Set iepEfficientWarp out of range
    {
        afl::data::IntegerValue iv(10);
        TS_ASSERT_THROWS(setEngineProperty(e, game::interface::iepEfficientWarp, &iv, list), interpreter::Error);
    }

    // Set iepEfficientWarp null
    {
        setEngineProperty(e, game::interface::iepEfficientWarp, 0, list);
        TS_ASSERT_EQUALS(e.getMaxEfficientWarp(), 5);
    }

    // Set iepFuelFactor
    {
        TS_ASSERT_THROWS(setEngineProperty(e, game::interface::iepFuelFactor, 0, list), interpreter::Error);
    }
}

