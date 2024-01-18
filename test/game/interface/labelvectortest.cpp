/**
  *  \file test/game/interface/labelvectortest.cpp
  *  \brief Test for game::interface::LabelVector
  */

#include "game/interface/labelvector.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/object.hpp"
#include "game/map/objectvector.hpp"
#include "game/map/objectvectortype.hpp"
#include "interpreter/process.hpp"
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
AFL_TEST("game.interface.LabelVector:storage", a)
{
    // Testee
    game::interface::LabelVector testee;

    // Verify initial state
    a.checkEqual("01. hasDirtyLabels",    testee.hasDirtyLabels(), false);
    a.checkEqual("02. hasUpdatingLabels", testee.hasUpdatingLabels(), false);
    a.checkEqual("03. hasChangedLabels",  testee.hasChangedLabels(), false);
    a.checkEqual("04. hasError",          testee.hasError(), false);
    a.checkEqual("05. getLastError",      testee.getLastError(), "");
    a.checkEqual("06. getLabel",          testee.getLabel(1), "");
    a.checkEqual("07. getLabel",          testee.getLabel(100), "");
    a.checkEqual("08. getExpression",     testee.getExpression(), "");

    // Set error
    testee.updateLabel(1, false, "err");
    a.checkEqual("11. hasChangedLabels", testee.hasChangedLabels(), false); // This is not a change
    a.checkEqual("12. hasError",         testee.hasError(), true);          // All calls until now are errors
    a.checkEqual("13. getLastError",     testee.getLastError(), "err");
    a.checkEqual("14. getLabel",         testee.getLabel(1), "");

    // Set success
    testee.updateLabel(2, true, "ok");
    a.checkEqual("21. hasChangedLabels", testee.hasChangedLabels(), true);  // Label changed
    a.checkEqual("22. hasError",         testee.hasError(), false);         // We had a successful call
    a.checkEqual("23. getLabel",         testee.getLabel(2), "ok");

    // Reset/set change marker
    testee.markLabelsUnchanged();
    a.checkEqual("31. hasChangedLabels", testee.hasChangedLabels(), false);
    testee.updateLabel(2, true, "ok");
    a.checkEqual("32. hasChangedLabels", testee.hasChangedLabels(), false);
    testee.updateLabel(2, true, "x");
    a.checkEqual("33. hasChangedLabels", testee.hasChangedLabels(), true);
    a.checkEqual("34. getLabel", testee.getLabel(1), "");
    a.checkEqual("35. getLabel", testee.getLabel(2), "x");

    // Clear
    testee.clear();
    a.checkEqual("41. getLabel", testee.getLabel(1), "");
    a.checkEqual("42. getLabel", testee.getLabel(2), "");
}

/** Test status management: checkObjects(), compileUpdater(), updateLabel(), finishUpdate(). */
AFL_TEST("game.interface.LabelVector:status", a)
{
    // Some objects
    game::map::ObjectVector<TestObject> container;
    container.create(1);
    container.create(2);
    container.create(3);
    container.create(4);
    TestObjectType type(container);
    a.check("01. isDirty", !container.get(1)->isDirty());

    // Testee
    game::interface::LabelVector testee;

    // Objects are clean, so this doesn't do anything
    testee.checkObjects(type);
    a.checkEqual("11. hasDirtyLabels",    testee.hasDirtyLabels(), false);
    a.checkEqual("12. hasUpdatingLabels", testee.hasUpdatingLabels(), false);
    a.checkEqual("13. hasChangedLabels",  testee.hasChangedLabels(), false);

    // Mark one dirty: this sets hasDirtyLabels(), but does not modify the object (still dirty)
    container.get(3)->markDirty();
    testee.checkObjects(type);
    a.checkEqual("21. hasDirtyLabels",    testee.hasDirtyLabels(), true);
    a.checkEqual("22. hasUpdatingLabels", testee.hasUpdatingLabels(), false);
    a.checkEqual("23. hasChangedLabels",  testee.hasChangedLabels(), false);
    a.check("24. isDirty", container.get(3)->isDirty());

    // Generate code: this sets hasUpdatingLabels(), clears hasDirtyLabels()
    interpreter::BytecodeObject bco;
    DummyCallable dc;
    a.checkEqual("31. compileUpdater",    testee.compileUpdater(bco, dc, dc), 1);
    a.checkEqual("32. hasDirtyLabels",    testee.hasDirtyLabels(), false);
    a.checkEqual("33. hasUpdatingLabels", testee.hasUpdatingLabels(), true);
    a.checkEqual("34. hasChangedLabels",  testee.hasChangedLabels(), false);

    // Checking again does not change anything
    testee.checkObjects(type);
    a.checkEqual("41. hasDirtyLabels",    testee.hasDirtyLabels(), false);
    a.checkEqual("42. hasUpdatingLabels", testee.hasUpdatingLabels(), true);
    a.checkEqual("43. hasChangedLabels",  testee.hasChangedLabels(), false);

    // Produce an update
    testee.updateLabel(3, true, "x");
    a.checkEqual("51. hasDirtyLabels",    testee.hasDirtyLabels(), false);
    a.checkEqual("52. hasUpdatingLabels", testee.hasUpdatingLabels(), true);
    a.checkEqual("53. hasChangedLabels",  testee.hasChangedLabels(), true);

    // Complete the update cycle
    testee.finishUpdate();
    a.checkEqual("61. hasDirtyLabels",    testee.hasDirtyLabels(), false);
    a.checkEqual("62. hasUpdatingLabels", testee.hasUpdatingLabels(), false);
    a.checkEqual("63. hasChangedLabels",  testee.hasChangedLabels(), true);
}

/** Test status management: markObjects(), compileUpdater(), updateLabel(), clearErrorStatus(). */
AFL_TEST("game.interface.LabelVector:status:2", a)
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
    a.checkEqual("01. hasDirtyLabels",    testee.hasDirtyLabels(), true);
    a.checkEqual("02. hasUpdatingLabels", testee.hasUpdatingLabels(), false);
    a.checkEqual("03. hasChangedLabels",  testee.hasChangedLabels(), false);

    // Generate code: this sets hasUpdatingLabels(), clears hasDirtyLabels()
    interpreter::BytecodeObject bco;
    DummyCallable dc;
    a.checkEqual("11. compileUpdater",    testee.compileUpdater(bco, dc, dc), 4);
    a.checkEqual("12. hasDirtyLabels",    testee.hasDirtyLabels(), false);
    a.checkEqual("13. hasUpdatingLabels", testee.hasUpdatingLabels(), true);
    a.checkEqual("14. hasChangedLabels",  testee.hasChangedLabels(), false);

    // Generate error
    testee.updateLabel(1, false, "ee");
    testee.updateLabel(2, false, "ff");
    a.checkEqual("21. hasError",     testee.hasError(), true);
    a.checkEqual("22. getLastError", testee.getLastError(), "ff");

    // Discard error
    testee.clearErrorStatus();
    a.checkEqual("31. hasError", testee.hasError(), false);
}

/** Test status management: markObjects(), markClean(). */
AFL_TEST("game.interface.LabelVector:status:3", a)
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
    a.checkEqual("01. hasDirtyLabels",    testee.hasDirtyLabels(), true);
    a.checkEqual("02. hasUpdatingLabels", testee.hasUpdatingLabels(), false);
    a.checkEqual("03. hasChangedLabels",  testee.hasChangedLabels(), false);

    // Discard changes
    testee.markClean();
    a.checkEqual("11. hasDirtyLabels",    testee.hasDirtyLabels(), false);
    a.checkEqual("12. hasUpdatingLabels", testee.hasUpdatingLabels(), false);
    a.checkEqual("13. hasChangedLabels",  testee.hasChangedLabels(), false);
}

/** Test compilation of expressions: setExpression(), getExpression(), hasError(). */
AFL_TEST("game.interface.LabelVector:setExpression", a)
{
    // Environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    // Initial state with no expression
    game::interface::LabelVector testee;
    a.checkEqual("01. getExpression", testee.getExpression(), "");
    a.checkEqual("02. hasError", testee.hasError(), false);

    // Valid expression
    testee.setExpression("1", world);
    a.checkEqual("11. getExpression", testee.getExpression(), "1");
    a.checkEqual("12. hasError", testee.hasError(), false);

    // Invalid expression
    testee.setExpression("1+", world);
    a.checkEqual("21. getExpression", testee.getExpression(), "1+");
    a.checkEqual("22. hasError", testee.hasError(), true);
    a.checkDifferent("23. getLastError", testee.getLastError(), "");

    // Also invalid
    testee.setExpression("1)", world);
    a.checkEqual("31. getExpression", testee.getExpression(), "1)");
    a.checkEqual("32. hasError", testee.hasError(), true);
    a.checkDifferent("33. getLastError", testee.getLastError(), "");

    // Valid again
    testee.setExpression("2", world);
    a.checkEqual("41. getExpression", testee.getExpression(), "2");
    a.checkEqual("42. hasError", testee.hasError(), false);

    // Empty again
    testee.setExpression("", world);
    a.checkEqual("51. getExpression", testee.getExpression(), "");
    a.checkEqual("52. hasError", testee.hasError(), false);
}
