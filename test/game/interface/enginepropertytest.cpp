/**
  *  \file test/game/interface/enginepropertytest.cpp
  *  \brief Test for game::interface::EngineProperty
  */

#include "game/interface/engineproperty.hpp"

#include "afl/data/access.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/test/testrunner.hpp"
#include "game/spec/shiplist.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/tagnode.hpp"
#include "interpreter/test/valueverifier.hpp"
#include <memory>

/** Test getEngineProperty. */
AFL_TEST("game.interface.EngineProperty:get", a)
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

    a.checkEqual("01. getMaxEfficientWarp", e.getMaxEfficientWarp(), 8);

    // Check iepEfficientWarp
    std::auto_ptr<afl::data::Value> p(game::interface::getEngineProperty(e, game::interface::iepEfficientWarp));
    a.checkEqual("11. iepEfficientWarp", afl::data::Access(p.get()).toInteger(), 8);

    // Check iepFuelFactor
    p.reset(game::interface::getEngineProperty(e, game::interface::iepFuelFactor));
    interpreter::IndexableValue* iv = dynamic_cast<interpreter::IndexableValue*>(p.get());
    a.checkNonNull("21. iepFuelFactor", iv);

    // - self-description
    a.checkEqual("31. iepFuelFactor dim(0)",   iv->getDimension(0), 1);
    a.checkEqual("32. iepFuelFactor dim(1)",   iv->getDimension(1), e.MAX_WARP+1);
    a.checkEqual("33. iepFuelFactor toString", iv->toString(false), "#<array>");

    // - not iterable, not serializable
    AFL_CHECK_THROWS(a("41. iepFuelFactor makeFirstContext"), iv->makeFirstContext(), interpreter::Error);
    interpreter::test::ValueVerifier(*iv, a("42. iepFuelFactor")).verifyNotSerializable();

    // - accessing values
    {
        // index 0
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> value(iv->get(args));
        a.checkNonNull("51. iepFuelFactor index 0", value.get());
        a.checkEqual("52. iepFuelFactor valie 0", afl::data::Access(value.get()).toInteger(), 0);
    }
    {
        // index 8
        afl::data::Segment seg;
        seg.pushBackInteger(8);
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> value(iv->get(args));
        a.checkNonNull("53. iepFuelFactor index 0", value.get());
        a.checkEqual("54. iepFuelFactor value 0", afl::data::Access(value.get()).toInteger(), 6400);
    }
    {
        // null
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> value(iv->get(args));
        a.check("55. iepFuelFactor index null", value.get() == 0);
    }
    {
        // wrong number of parameters
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 2);
        AFL_CHECK_THROWS(a("56. iepFuelFactor arity error"), iv->get(args), interpreter::Error);
    }
    {
        // set
        afl::data::Segment seg;
        seg.pushBackInteger(8);
        interpreter::Arguments args(seg, 0, 1);
        afl::data::IntegerValue nv(5000);
        AFL_CHECK_THROWS(a("57. iepFuelFactor set"), iv->set(args, &nv), interpreter::Error);
    }
}

/** Test setEngineProperty. */
AFL_TEST("game.interface.EngineProperty:setEngineProperty", a)
{
    game::spec::ShipList list;
    game::spec::Engine& e = *list.engines().create(6);

    // Set iepEfficientWarp
    {
        afl::data::IntegerValue iv(5);
        setEngineProperty(e, game::interface::iepEfficientWarp, &iv, list);
        a.checkEqual("01. getMaxEfficientWarp", e.getMaxEfficientWarp(), 5);
    }

    // Set iepEfficientWarp out of range
    {
        afl::data::IntegerValue iv(10);
        AFL_CHECK_THROWS(a("11. iepEfficientWarp range"), setEngineProperty(e, game::interface::iepEfficientWarp, &iv, list), interpreter::Error);
    }

    // Set iepEfficientWarp null
    {
        setEngineProperty(e, game::interface::iepEfficientWarp, 0, list);
        a.checkEqual("21. iepEfficientWarp null", e.getMaxEfficientWarp(), 5);
    }

    // Set iepFuelFactor
    {
        AFL_CHECK_THROWS(a("31. iepFuelFactor"), setEngineProperty(e, game::interface::iepFuelFactor, 0, list), interpreter::Error);
    }
}
