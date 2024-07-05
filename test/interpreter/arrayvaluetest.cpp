/**
  *  \file test/interpreter/arrayvaluetest.cpp
  *  \brief Test for interpreter::ArrayValue
  */

#include "interpreter/arrayvalue.hpp"

#include <memory>
#include <stdexcept>
#include "afl/io/internalsink.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/arraydata.hpp"
#include "interpreter/error.hpp"
#include "interpreter/savecontext.hpp"
#include "interpreter/values.hpp"

/** Test basic operations on array. */
AFL_TEST("interpreter.ArrayValue", a)
{
    // Create data object
    afl::base::Ref<interpreter::ArrayData> content = *new interpreter::ArrayData();
    content->addDimension(3);
    content->addDimension(5);

    // Create value
    interpreter::ArrayValue testee(content);

    // Verify dimensions
    a.checkEqual("01. getDimension", testee.getDimension(0), 2);
    a.checkEqual("02. getDimension", testee.getDimension(1), 3);
    a.checkEqual("03. getDimension", testee.getDimension(2), 5);

    // Context: not iterable
    AFL_CHECK_THROWS(a("11. makeFirstContext"), testee.makeFirstContext(), interpreter::Error);

    // String
    a.checkEqual("21. toString", testee.toString(false).substr(0, 2), "#<");
    a.checkEqual("22. toString", testee.toString(true).substr(0, 2), "#<");

    // Clone
    std::auto_ptr<interpreter::ArrayValue> copy(testee.clone());
    a.checkEqual("31. getData", &*testee.getData(), &*copy->getData());
    a.checkEqual("32. getData", &*testee.getData(), &*content);

    // Access
    {
        // Fetch (0,3). Must be 0.
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, seg.size());
        a.checkNull("41. get", testee.get(args));
    }
    {
        // Store 42 at (0,3)
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, seg.size());
        std::auto_ptr<afl::data::Value> value(interpreter::makeIntegerValue(42));
        testee.set(args, value.get());
    }
    {
        // Read (0,3) again.
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, seg.size());
        std::auto_ptr<afl::data::Value> p(testee.get(args));
        a.checkNonNull("42. get", p.get());

        int32_t iv = 0;
        a.check("51. checkIntegerArg", interpreter::checkIntegerArg(iv, p.get()));
        a.checkEqual("52. value", iv, 42);
    }
    {
        // Fetch (null,3). Must be null.
        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, seg.size());
        a.checkNull("53. get", testee.get(args));
    }

    // Some bogus accesses
    {
        // Out-of-range fetch (0,5) / store
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        seg.pushBackInteger(5);
        interpreter::Arguments args(seg, 0, seg.size());
        AFL_CHECK_THROWS(a("61. out-of-range get"), testee.get(args), interpreter::Error);
        AFL_CHECK_THROWS(a("62. out-of-range set"), testee.set(args, 0), interpreter::Error);
    }
    {
        // Out-of-range fetch (3,0)
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, seg.size());
        AFL_CHECK_THROWS(a("63. out-of-range get"), testee.get(args), interpreter::Error);
        AFL_CHECK_THROWS(a("64. out-of-range set"), testee.set(args, 0), interpreter::Error);
    }
    {
        // Bad arity
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, seg.size());
        AFL_CHECK_THROWS(a("65. bad arity get"), testee.get(args), interpreter::Error);
        AFL_CHECK_THROWS(a("66. bad arity set"), testee.set(args, 0), interpreter::Error);
    }
    {
        // Bad type
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        seg.pushBackString("x");
        interpreter::Arguments args(seg, 0, seg.size());
        AFL_CHECK_THROWS(a("67. type error get"), testee.get(args), interpreter::Error);
        AFL_CHECK_THROWS(a("68. type error set"), testee.set(args, 0), interpreter::Error);
    }
    {
        // Null index
        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, seg.size());
        std::auto_ptr<afl::data::Value> value(interpreter::makeIntegerValue(42));
        AFL_CHECK_THROWS(a("69. store null index"), testee.set(args, value.get()), interpreter::Error);
    }

    // Serialize
    {
        class TestSaveContext : public interpreter::SaveContext {
         public:
            virtual uint32_t addBCO(const interpreter::BytecodeObject& /*bco*/)
                { throw std::runtime_error("addBCO unexpected"); }
            virtual uint32_t addHash(const afl::data::Hash& /*hash*/)
                { throw std::runtime_error("addHash unexpected"); }
            virtual uint32_t addArray(const interpreter::ArrayData& /*array*/)
                { return 222; }
            virtual uint32_t addStructureType(const interpreter::StructureTypeData& /*type*/)
                { throw std::runtime_error("addStructureType unexpected"); }
            virtual uint32_t addStructureValue(const interpreter::StructureValueData& /*value*/)
                { throw std::runtime_error("addStructureValue unexpected"); }
            virtual bool isCurrentProcess(const interpreter::Process* /*p*/)
                { return false; }
        };

        interpreter::TagNode out;
        afl::io::InternalSink aux;
        TestSaveContext ctx;
        AFL_CHECK_SUCCEEDS(a("71. store"), testee.store(out, aux, ctx));
        a.checkEqual("72. tag", out.tag, interpreter::TagNode::Tag_Array);
        a.checkEqual("73. value", out.value, 222U);
    }
}

/** Test toString on empty 1-D array. */
AFL_TEST("interpreter.ArrayValue:toString:empty", a)
{
    // Create data object
    afl::base::Ref<interpreter::ArrayData> content = *new interpreter::ArrayData();
    content->addDimension(3);

    // Create value
    interpreter::ArrayValue testee(content);

    // String
    a.checkEqual("01. toString", testee.toString(false), "Array(Z(0),Z(0),Z(0))");
    a.checkEqual("02. toString", testee.toString(true),  "Array(Z(0),Z(0),Z(0))");
}

/** Test toString on populated 1-D array. */
AFL_TEST("interpreter.ArrayValue:toString:1d:small", a)
{
    // Create data object
    afl::base::Ref<interpreter::ArrayData> content = *new interpreter::ArrayData();
    content->addDimension(3);
    content->content().pushBackInteger(32);
    content->content().pushBackInteger(16);
    content->content().pushBackInteger(8);

    // Create value
    interpreter::ArrayValue testee(content);

    // String
    a.checkEqual("01. toString", testee.toString(false), "Array(32,16,8)");
    a.checkEqual("02. toString", testee.toString(true),  "Array(32,16,8)");
}

/** Test toString on overlong 1-D array: too long, falls back to default. */
AFL_TEST("interpreter.ArrayValue:toString:1d:big", a)
{
    // Create data object
    afl::base::Ref<interpreter::ArrayData> content = *new interpreter::ArrayData();
    content->addDimension(500);

    // Create value
    interpreter::ArrayValue testee(content);

    // String
    a.checkEqual("21. toString", testee.toString(false).substr(0, 2), "#<");
    a.checkEqual("22. toString", testee.toString(true).substr(0, 2), "#<");
}

/** Test toString on populated 2-D array. Those are not stringified */
AFL_TEST("interpreter.ArrayValue:toString:2d:small", a)
{
    // Create data object
    afl::base::Ref<interpreter::ArrayData> content = *new interpreter::ArrayData();
    content->addDimension(2);
    content->addDimension(2);
    content->content().pushBackInteger(32);
    content->content().pushBackInteger(16);
    content->content().pushBackInteger(8);
    content->content().pushBackInteger(89);

    // Create value
    interpreter::ArrayValue testee(content);

    // String
    a.checkEqual("21. toString", testee.toString(false).substr(0, 2), "#<");
    a.checkEqual("22. toString", testee.toString(true).substr(0, 2), "#<");
}
