/**
  *  \file u/t_interpreter_hashvalue.cpp
  *  \brief Test for interpreter::HashValue
  */

#include <memory>
#include "interpreter/hashvalue.hpp"

#include "t_interpreter.hpp"
#include "afl/data/segment.hpp"
#include "interpreter/values.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/context.hpp"
#include "interpreter/error.hpp"
#include "u/helper/contextverifier.hpp"

/** Test basic operations on empty hash. */
void
TestInterpreterHashValue::testEmpty()
{
    // Create
    interpreter::HashValue testee(afl::data::Hash::create());

    // Verify dimensions: this is not an array, so dimensions are 0
    TS_ASSERT_EQUALS(testee.getDimension(0), 0);
    TS_ASSERT_EQUALS(testee.getDimension(1), 0);

    // Context: empty, does not create an iterator
    std::auto_ptr<interpreter::Context> p(testee.makeFirstContext());
    TS_ASSERT(p.get() == 0);

    // String
    TS_ASSERT(!testee.toString(false).empty());
    TS_ASSERT(!testee.toString(true).empty());

    // Clone
    std::auto_ptr<interpreter::HashValue> copy(testee.clone());
    TS_ASSERT_EQUALS(&*testee.getData(), &*copy->getData());

    // Inquiry
    {
        afl::data::Segment seg;
        seg.pushBackNew(interpreter::makeStringValue("A"));
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> v(testee.get(args));
        TS_ASSERT(v.get() == 0);
    }
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> v(testee.get(args));
        TS_ASSERT(v.get() == 0);
    }
}

/** Test basic operations on unit (one-element) hash. */
void
TestInterpreterHashValue::testUnit()
{
    // Create and populate
    interpreter::HashValue testee(afl::data::Hash::create());
    {
        afl::data::Segment seg;
        seg.pushBackNew(interpreter::makeStringValue("A"));
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> p(interpreter::makeIntegerValue(42));
        testee.set(args, p.get());
    }

    // Verify dimensions: this is not an array, so dimensions are 0
    TS_ASSERT_EQUALS(testee.getDimension(0), 0);
    TS_ASSERT_EQUALS(testee.getDimension(1), 0);

    // String
    TS_ASSERT(!testee.toString(false).empty());
    TS_ASSERT(!testee.toString(true).empty());

    // Clone
    std::auto_ptr<interpreter::HashValue> copy(testee.clone());
    TS_ASSERT_EQUALS(&*testee.getData(), &*copy->getData());

    // Context access
    std::auto_ptr<interpreter::Context> p(testee.makeFirstContext());
    TS_ASSERT(p.get() != 0);

    // - verify the context
    TS_ASSERT(p->getObject() == 0);

    std::auto_ptr<interpreter::Context> pClone(p->clone());
    TS_ASSERT(pClone.get() != 0);
    verifyTypes(*pClone);
    TS_ASSERT_EQUALS(pClone->toString(false), p->toString(false));
    TS_ASSERT_EQUALS(pClone->toString(true), p->toString(true));
    TS_ASSERT_DIFFERS(pClone->toString(false), "");
    TS_ASSERT_DIFFERS(pClone->toString(true), "");

    // - verify the properties published by this context
    interpreter::Context::PropertyIndex_t keyIndex;
    interpreter::Context* keyContext = p->lookup("KEY", keyIndex);
    TS_ASSERT(keyContext != 0);

    interpreter::Context::PropertyIndex_t valueIndex;
    interpreter::Context* valueContext = p->lookup("VALUE", valueIndex);
    TS_ASSERT(valueContext != 0);

    interpreter::Context::PropertyIndex_t otherIndex;
    interpreter::Context* otherContext = p->lookup("OTHER", otherIndex);
    TS_ASSERT(otherContext == 0);

    // - verify read access to the properties
    {
        std::auto_ptr<afl::data::Value> v(keyContext->get(keyIndex));
        TS_ASSERT(v.get() != 0);

        String_t sv;
        TS_ASSERT(interpreter::checkStringArg(sv, v.get()));
        TS_ASSERT_EQUALS(sv, "A");
    }
    {
        std::auto_ptr<afl::data::Value> v(valueContext->get(valueIndex));
        TS_ASSERT(v.get() != 0);

        int32_t iv;
        TS_ASSERT(interpreter::checkIntegerArg(iv, v.get()));
        TS_ASSERT_EQUALS(iv, 42);
    }

    // - verify write access to the properties
    {
        std::auto_ptr<afl::data::Value> v(interpreter::makeStringValue("B"));
        TS_ASSERT_THROWS(keyContext->set(keyIndex, v.get()), interpreter::Error);
    }
    {
        std::auto_ptr<afl::data::Value> v(interpreter::makeStringValue("nv"));
        TS_ASSERT_THROWS_NOTHING(valueContext->set(valueIndex, v.get()));
    }

    // - verify advance
    TS_ASSERT(!p->next());

    // Inquiry
    {
        // regular access
        afl::data::Segment seg;
        seg.pushBackNew(interpreter::makeStringValue("A"));
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> v(testee.get(args));
        TS_ASSERT(v.get() != 0);

        String_t sv;
        TS_ASSERT(interpreter::checkStringArg(sv, v.get()));
        TS_ASSERT_EQUALS(sv, "nv");
    }
    {
        // access to clone: clone has been modified by the above as well
        afl::data::Segment seg;
        seg.pushBackNew(interpreter::makeStringValue("A"));
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> v(copy->get(args));
        TS_ASSERT(v.get() != 0);

        String_t sv;
        TS_ASSERT(interpreter::checkStringArg(sv, v.get()));
        TS_ASSERT_EQUALS(sv, "nv");
    }
    {
        // case sensitive!
        afl::data::Segment seg;
        seg.pushBackNew(interpreter::makeStringValue("a"));
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> v(testee.get(args));
        TS_ASSERT(v.get() == 0);
    }
    {
        // null index
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> v(testee.get(args));
        TS_ASSERT(v.get() == 0);
    }
}

/** Test hash with multiple keys. */
void
TestInterpreterHashValue::testMulti()
{
    // Create and populate
    interpreter::HashValue testee(afl::data::Hash::create());
    {
        // Normal
        afl::data::Segment seg;
        seg.pushBackNew(interpreter::makeStringValue("A"));
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> p(interpreter::makeIntegerValue(42));
        testee.set(args, p.get());
    }
    {
        // Another
        afl::data::Segment seg;
        seg.pushBackNew(interpreter::makeStringValue("B"));
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> v(interpreter::makeStringValue("sv"));
        TS_ASSERT_THROWS_NOTHING(testee.set(args, v.get()));
    }
    {
        // Assigning null
        afl::data::Segment seg;
        seg.pushBackNew(interpreter::makeStringValue("C"));
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS_NOTHING(testee.set(args, 0));
    }
    {
        // Assigning to null key
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> v(interpreter::makeStringValue("null"));
        TS_ASSERT_THROWS(testee.set(args, v.get()), interpreter::Error);
    }

    // Iterate
    bool a = false, b = false, c = false;
    std::auto_ptr<interpreter::Context> p(testee.makeFirstContext());
    TS_ASSERT(p.get() != 0);
    do {
        // Get key
        interpreter::Context::PropertyIndex_t keyIndex;
        interpreter::Context* keyContext = p->lookup("KEY", keyIndex);
        TS_ASSERT(keyContext != 0);
        std::auto_ptr<afl::data::Value> keyValue(keyContext->get(keyIndex));
        TS_ASSERT(keyValue.get() != 0);

        // Get value
        interpreter::Context::PropertyIndex_t valueIndex;
        interpreter::Context* valueContext = p->lookup("VALUE", valueIndex);
        TS_ASSERT(valueContext != 0);
        std::auto_ptr<afl::data::Value> valueValue(valueContext->get(valueIndex));

        // Check
        String_t key;
        TS_ASSERT(interpreter::checkStringArg(key, keyValue.get()));
        if (key == "A") {
            int32_t iv;
            TS_ASSERT(!a);
            TS_ASSERT(interpreter::checkIntegerArg(iv, valueValue.get()));
            TS_ASSERT_EQUALS(iv, 42);
            a = true;
        } else if (key == "B") {
            String_t sv;
            TS_ASSERT(!b);
            TS_ASSERT(interpreter::checkStringArg(sv, valueValue.get()));
            TS_ASSERT_EQUALS(sv, "sv");
            b = true;
        } else if (key == "C") {
            TS_ASSERT(!c);
            TS_ASSERT(valueValue.get() == 0);
            c = true;
        } else {
            TS_ASSERT(!"unexpected key");
        }
    } while (p->next());
    TS_ASSERT(a);
    TS_ASSERT(b);
    TS_ASSERT(c);
}

