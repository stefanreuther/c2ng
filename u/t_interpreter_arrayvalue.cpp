/**
  *  \file u/t_interpreter_arrayvalue.cpp
  *  \brief Test for interpreter::ArrayValue
  */

#include <memory>
#include "interpreter/arrayvalue.hpp"

#include "t_interpreter.hpp"
#include "afl/io/internalsink.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/arraydata.hpp"
#include "interpreter/error.hpp"
#include "interpreter/savecontext.hpp"
#include "interpreter/values.hpp"

/** Test basic operations on array. */
void
TestInterpreterArrayValue::testIt()
{
    // Create data object
    afl::base::Ref<interpreter::ArrayData> content = *new interpreter::ArrayData();
    content->addDimension(3);
    content->addDimension(5);

    // Create value
    interpreter::ArrayValue testee(content);

    // Verify dimensions
    TS_ASSERT_EQUALS(testee.getDimension(0), 2);
    TS_ASSERT_EQUALS(testee.getDimension(1), 3);
    TS_ASSERT_EQUALS(testee.getDimension(2), 5);

    // Context: not iterable
    TS_ASSERT_THROWS(testee.makeFirstContext(), interpreter::Error);

    // String
    TS_ASSERT_EQUALS(testee.toString(false).substr(0, 2), "#<");
    TS_ASSERT_EQUALS(testee.toString(true).substr(0, 2), "#<");

    // Clone
    std::auto_ptr<interpreter::ArrayValue> copy(testee.clone());
    TS_ASSERT_EQUALS(&*testee.getData(), &*copy->getData());
    TS_ASSERT_EQUALS(&*testee.getData(), &*content);

    // Access
    {
        // Fetch (0,3). Must be 0.
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        seg.pushBackInteger(3);
        interpreter::Arguments args(seg, 0, seg.size());
        TS_ASSERT(testee.get(args) == 0);
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
        TS_ASSERT(p.get() != 0);

        int32_t iv = 0;
        TS_ASSERT(interpreter::checkIntegerArg(iv, p.get()));
        TS_ASSERT_EQUALS(iv, 42);
    }

    // Some bogus accesses
    {
        // Out-of-range fetch (0,5) / store
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        seg.pushBackInteger(5);
        interpreter::Arguments args(seg, 0, seg.size());
        TS_ASSERT_THROWS(testee.get(args), interpreter::Error);
        TS_ASSERT_THROWS(testee.set(args, 0), interpreter::Error);
    }
    {
        // Out-of-range fetch (3,0)
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, seg.size());
        TS_ASSERT_THROWS(testee.get(args), interpreter::Error);
        TS_ASSERT_THROWS(testee.set(args, 0), interpreter::Error);
    }
    {
        // Bad arity
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, seg.size());
        TS_ASSERT_THROWS(testee.get(args), interpreter::Error);
        TS_ASSERT_THROWS(testee.set(args, 0), interpreter::Error);
    }
    {
        // Bad type
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        seg.pushBackString("x");
        interpreter::Arguments args(seg, 0, seg.size());
        TS_ASSERT_THROWS(testee.get(args), interpreter::Error);
        TS_ASSERT_THROWS(testee.set(args, 0), interpreter::Error);
    }

    // Serialize
    {
        class TestSaveContext : public interpreter::SaveContext {
         public:
            virtual uint32_t addBCO(const interpreter::BytecodeObject& /*bco*/)
                { TS_FAIL("addBCO unexpected"); return 0; }
            virtual uint32_t addHash(const afl::data::Hash& /*hash*/)
                { TS_FAIL("addHash unexpected"); return 0; }
            virtual uint32_t addArray(const interpreter::ArrayData& /*array*/)
                { return 222; }
            virtual uint32_t addStructureType(const interpreter::StructureTypeData& /*type*/)
                { TS_FAIL("addStructureType unexpected"); return 0; }
            virtual uint32_t addStructureValue(const interpreter::StructureValueData& /*value*/)
                { TS_FAIL("addStructureValue unexpected"); return 0; }
            virtual bool isCurrentProcess(const interpreter::Process* /*p*/)
                { return false; }
        };

        interpreter::TagNode out;
        afl::io::InternalSink aux;
        TestSaveContext ctx;
        TS_ASSERT_THROWS_NOTHING(testee.store(out, aux, ctx));
        TS_ASSERT_EQUALS(out.tag, interpreter::TagNode::Tag_Array);
        TS_ASSERT_EQUALS(out.value, 222U);
    }
}

