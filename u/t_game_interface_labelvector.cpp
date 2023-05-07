/**
  *  \file u/t_game_interface_labelvector.cpp
  *  \brief Test for game::interface::LabelVector
  */

#include "game/interface/labelvector.hpp"

#include "t_game_interface.hpp"
#include "game/map/object.hpp"
#include "game/map/objectvector.hpp"
#include "game/map/objectvectortype.hpp"
#include "interpreter/process.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/sys/log.hpp"
#include "afl/string/nulltranslator.hpp"
#include "interpreter/world.hpp"

namespace {
    /* Test implementation of map::Object. Just the minimum to get an object with Id. */
    class TestObject : public game::map::Object {
     public:
        TestObject(int id)
            : Object(id)
            { }

        // Object:
        virtual String_t getName(game::ObjectName /*which*/, afl::string::Translator& /*tx*/, const game::InterpreterInterface& /*iface*/) const
            { return "obj"; }
        virtual afl::base::Optional<int> getOwner() const
            { return 0; }
        virtual afl::base::Optional<game::map::Point> getPosition() const
            { return afl::base::Nothing; }
    };

    /* ObjectVectorType implementation */
    class TestObjectType : public game::map::ObjectVectorType<TestObject> {
     public:
        TestObjectType(game::map::ObjectVector<TestObject>& vec)
            : game::map::ObjectVectorType<TestObject>(vec)
            { }
        virtual bool isValid(const TestObject&) const
            { return true; }
    };

    /* Dummy callable */
    class DummyCallable : public interpreter::CallableValue {
     public:
        virtual void call(interpreter::Process& proc, afl::data::Segment& /*args*/, bool want_result)
            {
                if (want_result) {
                    proc.pushNewValue(0);
                }
            }
        virtual bool isProcedureCall() const
            { return false; }
        virtual int32_t getDimension(int32_t /*which*/) const
            { return 0; }
        virtual interpreter::Context* makeFirstContext()
            { return 0; }
        virtual DummyCallable* clone() const
            { return new DummyCallable(); }
        virtual String_t toString(bool /*readable*/) const
            { return "#<dummy>"; }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
            { }
    };
}

/** Test label storage: updateLabel(), getLastError(), status bits. */
void
TestGameInterfaceLabelVector::testStorage()
{
    // Testee
    game::interface::LabelVector testee;

    // Verify initial state
    TS_ASSERT_EQUALS(testee.hasDirtyLabels(), false);
    TS_ASSERT_EQUALS(testee.hasUpdatingLabels(), false);
    TS_ASSERT_EQUALS(testee.hasChangedLabels(), false);
    TS_ASSERT_EQUALS(testee.hasError(), false);
    TS_ASSERT_EQUALS(testee.getLastError(), "");
    TS_ASSERT_EQUALS(testee.getLabel(1), "");
    TS_ASSERT_EQUALS(testee.getLabel(100), "");
    TS_ASSERT_EQUALS(testee.getExpression(), "");

    // Set error
    testee.updateLabel(1, false, "err");
    TS_ASSERT_EQUALS(testee.hasChangedLabels(), false); // This is not a change
    TS_ASSERT_EQUALS(testee.hasError(), true);          // All calls until now are errors
    TS_ASSERT_EQUALS(testee.getLastError(), "err");
    TS_ASSERT_EQUALS(testee.getLabel(1), "");

    // Set success
    testee.updateLabel(2, true, "ok");
    TS_ASSERT_EQUALS(testee.hasChangedLabels(), true);  // Label changed
    TS_ASSERT_EQUALS(testee.hasError(), false);         // We had a successful call
    TS_ASSERT_EQUALS(testee.getLabel(2), "ok");

    // Reset/set change marker
    testee.markLabelsUnchanged();
    TS_ASSERT_EQUALS(testee.hasChangedLabels(), false);
    testee.updateLabel(2, true, "ok");
    TS_ASSERT_EQUALS(testee.hasChangedLabels(), false);
    testee.updateLabel(2, true, "x");
    TS_ASSERT_EQUALS(testee.hasChangedLabels(), true);
    TS_ASSERT_EQUALS(testee.getLabel(1), "");
    TS_ASSERT_EQUALS(testee.getLabel(2), "x");

    // Clear
    testee.clear();
    TS_ASSERT_EQUALS(testee.getLabel(1), "");
    TS_ASSERT_EQUALS(testee.getLabel(2), "");
}

/** Test status management: checkObjects(), compileUpdater(), updateLabel(), finishUpdate(). */
void
TestGameInterfaceLabelVector::testStatus()
{
    // Some objects
    game::map::ObjectVector<TestObject> container;
    container.create(1);
    container.create(2);
    container.create(3);
    container.create(4);
    TestObjectType type(container);
    TS_ASSERT(!container.get(1)->isDirty());

    // Testee
    game::interface::LabelVector testee;

    // Objects are clean, so this doesn't do anything
    testee.checkObjects(type);
    TS_ASSERT_EQUALS(testee.hasDirtyLabels(), false);
    TS_ASSERT_EQUALS(testee.hasUpdatingLabels(), false);
    TS_ASSERT_EQUALS(testee.hasChangedLabels(), false);

    // Mark one dirty: this sets hasDirtyLabels(), but does not modify the object (still dirty)
    container.get(3)->markDirty();
    testee.checkObjects(type);
    TS_ASSERT_EQUALS(testee.hasDirtyLabels(), true);
    TS_ASSERT_EQUALS(testee.hasUpdatingLabels(), false);
    TS_ASSERT_EQUALS(testee.hasChangedLabels(), false);
    TS_ASSERT(container.get(3)->isDirty());

    // Generate code: this sets hasUpdatingLabels(), clears hasDirtyLabels()
    interpreter::BytecodeObject bco;
    DummyCallable dc;
    TS_ASSERT_EQUALS(testee.compileUpdater(bco, dc, dc), 1);
    TS_ASSERT_EQUALS(testee.hasDirtyLabels(), false);
    TS_ASSERT_EQUALS(testee.hasUpdatingLabels(), true);
    TS_ASSERT_EQUALS(testee.hasChangedLabels(), false);

    // Checking again does not change anything
    testee.checkObjects(type);
    TS_ASSERT_EQUALS(testee.hasDirtyLabels(), false);
    TS_ASSERT_EQUALS(testee.hasUpdatingLabels(), true);
    TS_ASSERT_EQUALS(testee.hasChangedLabels(), false);

    // Produce an update
    testee.updateLabel(3, true, "x");
    TS_ASSERT_EQUALS(testee.hasDirtyLabels(), false);
    TS_ASSERT_EQUALS(testee.hasUpdatingLabels(), true);
    TS_ASSERT_EQUALS(testee.hasChangedLabels(), true);

    // Complete the update cycle
    testee.finishUpdate();
    TS_ASSERT_EQUALS(testee.hasDirtyLabels(), false);
    TS_ASSERT_EQUALS(testee.hasUpdatingLabels(), false);
    TS_ASSERT_EQUALS(testee.hasChangedLabels(), true);
}

/** Test status management: markObjects(), compileUpdater(), updateLabel(), clearErrorStatus(). */
void
TestGameInterfaceLabelVector::testStatus2()
{
    // Environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    // Some objects
    game::map::ObjectVector<TestObject> container;
    container.create(1);
    container.create(2);
    container.create(3);
    container.create(4);
    TestObjectType type(container);

    // Testee; set an expression just for coverage
    game::interface::LabelVector testee;
    testee.setExpression("1", world);

    // Force everything
    testee.markObjects(type);
    TS_ASSERT_EQUALS(testee.hasDirtyLabels(), true);
    TS_ASSERT_EQUALS(testee.hasUpdatingLabels(), false);
    TS_ASSERT_EQUALS(testee.hasChangedLabels(), false);

    // Generate code: this sets hasUpdatingLabels(), clears hasDirtyLabels()
    interpreter::BytecodeObject bco;
    DummyCallable dc;
    TS_ASSERT_EQUALS(testee.compileUpdater(bco, dc, dc), 4);
    TS_ASSERT_EQUALS(testee.hasDirtyLabels(), false);
    TS_ASSERT_EQUALS(testee.hasUpdatingLabels(), true);
    TS_ASSERT_EQUALS(testee.hasChangedLabels(), false);

    // Generate error
    testee.updateLabel(1, false, "ee");
    testee.updateLabel(2, false, "ff");
    TS_ASSERT_EQUALS(testee.hasError(), true);
    TS_ASSERT_EQUALS(testee.getLastError(), "ff");

    // Discard error
    testee.clearErrorStatus();
    TS_ASSERT_EQUALS(testee.hasError(), false);
}

/** Test status management: markObjects(), markClean(). */
void
TestGameInterfaceLabelVector::testStatus3()
{
    // Environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    // Some objects
    game::map::ObjectVector<TestObject> container;
    container.create(1);
    container.create(2);
    container.create(3);
    container.create(4);
    TestObjectType type(container);

    // Testee
    game::interface::LabelVector testee;

    // Force everything
    testee.markObjects(type);
    TS_ASSERT_EQUALS(testee.hasDirtyLabels(), true);
    TS_ASSERT_EQUALS(testee.hasUpdatingLabels(), false);
    TS_ASSERT_EQUALS(testee.hasChangedLabels(), false);

    // Discard changes
    testee.markClean();
    TS_ASSERT_EQUALS(testee.hasDirtyLabels(), false);
    TS_ASSERT_EQUALS(testee.hasUpdatingLabels(), false);
    TS_ASSERT_EQUALS(testee.hasChangedLabels(), false);
}

/** Test compilation of expressions: setExpression(), getExpression(), hasError(). */
void
TestGameInterfaceLabelVector::testCompile()
{
    // Environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    // Initial state with no expression
    game::interface::LabelVector testee;
    TS_ASSERT_EQUALS(testee.getExpression(), "");
    TS_ASSERT_EQUALS(testee.hasError(), false);

    // Valid expression
    testee.setExpression("1", world);
    TS_ASSERT_EQUALS(testee.getExpression(), "1");
    TS_ASSERT_EQUALS(testee.hasError(), false);

    // Invalid expression
    testee.setExpression("1+", world);
    TS_ASSERT_EQUALS(testee.getExpression(), "1+");
    TS_ASSERT_EQUALS(testee.hasError(), true);
    TS_ASSERT_DIFFERS(testee.getLastError(), "");

    // Also invalid
    testee.setExpression("1)", world);
    TS_ASSERT_EQUALS(testee.getExpression(), "1)");
    TS_ASSERT_EQUALS(testee.hasError(), true);
    TS_ASSERT_DIFFERS(testee.getLastError(), "");

    // Valid again
    testee.setExpression("2", world);
    TS_ASSERT_EQUALS(testee.getExpression(), "2");
    TS_ASSERT_EQUALS(testee.hasError(), false);

    // Empty again
    testee.setExpression("", world);
    TS_ASSERT_EQUALS(testee.getExpression(), "");
    TS_ASSERT_EQUALS(testee.hasError(), false);
}

