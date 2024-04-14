/**
  *  \file test/game/interface/missionlistcontexttest.cpp
  *  \brief Test for game::interface::MissionListContext
  */

#include "game/interface/missionlistcontext.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/interface/missioncontext.hpp"
#include "game/spec/mission.hpp"
#include "game/spec/missionlist.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/context.hpp"
#include "interpreter/error.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/process.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"
#include "interpreter/world.hpp"

using afl::base::Ref;
using afl::data::Segment;
using afl::data::Value;
using game::interface::MissionContext;
using game::interface::MissionListContext;
using game::spec::Mission;
using game::spec::MissionList;
using interpreter::Arguments;
using interpreter::CallableValue;
using interpreter::Context;
using interpreter::IndexableValue;
using interpreter::test::ContextVerifier;
using interpreter::test::ValueVerifier;

// Basics
AFL_TEST("game.interface.MissionListContext:basics", a)
{
    Ref<MissionList> list = MissionList::create();
    list->addMission(Mission(10, ",one"));
    list->addMission(Mission(20, ",two"));

    // Verify general operations
    MissionListContext testee(list);
    ContextVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyNotSerializable();
    verif.verifyTypes();
    a.checkNull("01. getObject", testee.getObject());
    a.checkEqual("02. mission", &testee.missions(), &*list);
}

// MissionList().Mission() function, normal case
AFL_TEST("game.interface.MissionListContext:Mission", a)
{
    Ref<MissionList> list = MissionList::create();
    list->addMission(Mission(10, ",one"));
    list->addMission(Mission(20, ",two"));

    // "Mission" attribute must be retrievable
    MissionListContext testee(list);
    std::auto_ptr<Value> val(ContextVerifier(testee, a).getValue("MISSION"));
    IndexableValue* fcn = dynamic_cast<IndexableValue*>(val.get());
    a.checkNonNull("01. IndexableValue", fcn);

    // Values
    ValueVerifier verif(*fcn, a("11. ValueVerifier"));
    verif.verifyBasics();
    verif.verifyNotSerializable();

    // Dimension
    a.checkEqual("21. dim 0", fcn->getDimension(0), 1);
    a.checkEqual("22. dim 1", fcn->getDimension(1), 2);

    // Iteration
    {
        std::auto_ptr<Context> ctx(fcn->makeFirstContext());
        a.checkNonNull("31. makeFirstContext", ctx.get());
        ContextVerifier(*ctx, a("32. ContextVerifier"))
            .verifyString("NAME", "one");
    }

    // Get element, in range
    {
        Segment seg;
        seg.pushBackInteger(1);
        Arguments args(seg, 0, 1);
        std::auto_ptr<Value> val(fcn->get(args));
        Context* ctx = dynamic_cast<Context*>(val.get());
        a.checkNonNull("41. get", ctx);
        ContextVerifier(*ctx, a("42. ContextVerifier"))
            .verifyString("NAME", "two");
    }

    // Get element, arity error
    {
        Segment seg;
        Arguments args(seg, 0, 0);
        AFL_CHECK_THROWS(a("51. arity error"), fcn->get(args), interpreter::Error);
    }

    // Get element, range error
    {
        Segment seg;
        seg.pushBackInteger(2);
        Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("61. range error"), fcn->get(args), interpreter::Error);
    }

    // Get element, type error
    {
        Segment seg;
        seg.pushBackString("huh");
        Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("71. type error"), fcn->get(args), interpreter::Error);
    }

    // Get element, null
    {
        Segment seg;
        Arguments args(seg, 0, 1);
        a.checkNull("81. null", fcn->get(args));
    }

    // Set element
    {
        Segment seg;
        seg.pushBackInteger(1);
        Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("91. set"), fcn->set(args, seg[0]), interpreter::Error);
    }
}

// MissionList().Mission() function, empty list
AFL_TEST("game.interface.MissionListContext:Mission:empty", a)
{
    Ref<MissionList> list = MissionList::create();

    // "Mission" attribute must be retrievable
    MissionListContext testee(list);
    std::auto_ptr<Value> val(ContextVerifier(testee, a).getValue("MISSION"));
    IndexableValue* fcn = dynamic_cast<IndexableValue*>(val.get());
    a.checkNonNull("01. IndexableValue", fcn);

    // Values
    ValueVerifier verif(*fcn, a("11. ValueVerifier"));
    verif.verifyBasics();
    verif.verifyNotSerializable();

    // Dimension
    a.checkEqual("21. dim 0", fcn->getDimension(0), 1);
    a.checkEqual("22. dim 1", fcn->getDimension(1), 0);

    // Iteration
    {
        std::auto_ptr<Context> ctx(fcn->makeFirstContext());
        a.checkNull("31. makeFirstContext", ctx.get());
    }
}

// MissionList().AddMission command
AFL_TEST("game.interface.MissionListContext:AddMission", a)
{
    Ref<MissionList> list = MissionList::create();
    list->addMission(Mission(10, ",one"));

    // "AddMission" attribute must be retrievable
    MissionListContext testee(list);
    std::auto_ptr<Value> val(ContextVerifier(testee, a).getValue("ADDMISSION"));
    CallableValue* cv = dynamic_cast<CallableValue*>(val.get());
    a.checkNonNull("01. CallableValue", cv);

    // Values
    ValueVerifier verif(*cv, a("11. ValueVerifier"));
    verif.verifyBasics();
    verif.verifyNotSerializable();

    // Process environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);
    interpreter::Process proc(world, "proc", 99);

    // Call with mission argument
    {
        Ref<MissionList> other = MissionList::create();
        other->addMission(Mission(11, ",two"));

        Segment seg;
        seg.pushBackNew(new MissionContext(0, other));
        cv->call(proc, seg, false);
        a.checkEqual("21. after first add", list->size(), 2U);
    }

    // Call with string argument
    {
        Segment seg;
        seg.pushBackString("20,,twenty\n21,,twenty-one\nc=cond");
        cv->call(proc, seg, false);
        a.checkEqual("31. after second add", list->size(), 4U);
    }

    // Error: too few args
    {
        Segment seg;
        AFL_CHECK_THROWS(a("41. too few"), cv->call(proc, seg, false), interpreter::Error);
    }

    // Error: too many args
    {
        Segment seg;
        seg.pushBackString("");
        seg.pushBackString("");
        AFL_CHECK_THROWS(a("51. too many"), cv->call(proc, seg, false), interpreter::Error);
    }

    // Final verification
    a.checkEqual("61. final size", list->size(), 4U);
    a.checkEqual("62. index 0 getName", list->at(0)->getName(), "one");
    a.checkEqual("63. index 1 getName", list->at(1)->getName(), "two");
    a.checkEqual("64. index 2 getName", list->at(2)->getName(), "twenty");
    a.checkEqual("65. index 3 getName", list->at(3)->getName(), "twenty-one");
    a.checkEqual("65. index 3 getCond", list->at(3)->getConditionExpression(), "cond");
}

// Factory function
AFL_TEST("game.interface.MissionListContext:factory", a)
{
    Segment seg;
    Arguments args(seg, 0, 0);
    std::auto_ptr<Value> val(game::interface::IFMissionList(args));

    Context* ctx = dynamic_cast<Context*>(val.get());
    a.checkNonNull("01. Context", ctx);

    std::auto_ptr<Value> val2(ContextVerifier(*ctx, a).getValue("MISSION"));
    IndexableValue* fcn = dynamic_cast<IndexableValue*>(val2.get());
    a.checkNonNull("11. IndexableValue", fcn);
    a.checkEqual("12. dim 1", fcn->getDimension(1), 0);
}

// Factory function, arity error
AFL_TEST("game.interface.MissionListContext:factory:error", a)
{
    Segment seg;
    Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFMissionList(args), interpreter::Error);
}
