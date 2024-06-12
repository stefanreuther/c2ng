/**
  *  \file test/interpreter/hashvaluetest.cpp
  *  \brief Test for interpreter::HashValue
  */

#include "interpreter/hashvalue.hpp"

#include "afl/data/segment.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/context.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/values.hpp"
#include <memory>

using interpreter::test::ContextVerifier;

/** Test basic operations on empty hash. */
AFL_TEST("interpreter.HashValue:empty", a)
{
    // Create
    interpreter::HashValue testee(afl::data::Hash::create());

    // Verify dimensions: this is not an array, so dimensions are 0
    a.checkEqual("01. getDimension", testee.getDimension(0), 0);
    a.checkEqual("02. getDimension", testee.getDimension(1), 0);

    // Context: empty, does not create an iterator
    std::auto_ptr<interpreter::Context> p(testee.makeFirstContext());
    a.checkNull("11. makeFirstContext", p.get());

    // String
    a.check("21. toString", !testee.toString(false).empty());
    a.check("22. toString", !testee.toString(true).empty());

    // Clone
    std::auto_ptr<interpreter::HashValue> copy(testee.clone());
    a.checkEqual("31. clone", &*testee.getData(), &*copy->getData());

    // Inquiry
    {
        afl::data::Segment seg;
        seg.pushBackNew(interpreter::makeStringValue("A"));
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> v(testee.get(args));
        a.checkNull("41. get('A')", v.get());
    }
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> v(testee.get(args));
        a.checkNull("42. get(null)'", v.get());
    }
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        AFL_CHECK_THROWS(a("43. get()"), testee.get(args), interpreter::Error);
    }
    {
        afl::data::Segment seg;
        seg.pushBackString("x");
        seg.pushBackString("y");
        interpreter::Arguments args(seg, 0, 2);
        AFL_CHECK_THROWS(a("44. get()"), testee.get(args), interpreter::Error);
    }
}

/** Test basic operations on unit (one-element) hash. */
AFL_TEST("interpreter.HashValue:unit", a)
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
    a.checkEqual("01. getDimension", testee.getDimension(0), 0);
    a.checkEqual("02. getDimension", testee.getDimension(1), 0);

    // String
    a.check("11. toString", !testee.toString(false).empty());
    a.check("12. toString", !testee.toString(true).empty());

    // Clone
    std::auto_ptr<interpreter::HashValue> copy(testee.clone());
    a.checkEqual("21. clone", &*testee.getData(), &*copy->getData());

    // Context access
    std::auto_ptr<interpreter::Context> p(testee.makeFirstContext());
    a.checkNonNull("31. makeFirstContext", p.get());
    ContextVerifier(*p, a("32. basics")).verifyBasics();
    ContextVerifier(*p, a("33. serializable")).verifyNotSerializable();

    // - verify the context
    a.checkNull("41. getObject", p->getObject());

    std::auto_ptr<interpreter::Context> pClone(p->clone());
    a.checkNonNull("51. clone", pClone.get());
    ContextVerifier(*pClone, a("52. clone")).verifyTypes();
    a.checkEqual("53. toString", pClone->toString(false), p->toString(false));
    a.checkEqual("54. toString", pClone->toString(true), p->toString(true));
    a.checkDifferent("55. toString", pClone->toString(false), "");
    a.checkDifferent("56. toString", pClone->toString(true), "");

    // - verify the properties published by this context
    interpreter::Context::PropertyIndex_t keyIndex;
    interpreter::Context::PropertyAccessor* keyContext = p->lookup("KEY", keyIndex);
    a.checkNonNull("61. key", keyContext);

    interpreter::Context::PropertyIndex_t valueIndex;
    interpreter::Context::PropertyAccessor* valueContext = p->lookup("VALUE", valueIndex);
    a.checkNonNull("71. value", valueContext);

    interpreter::Context::PropertyIndex_t otherIndex;
    interpreter::Context::PropertyAccessor* otherContext = p->lookup("OTHER", otherIndex);
    a.checkNull("81. other", otherContext);

    // - verify read access to the properties
    {
        std::auto_ptr<afl::data::Value> v(keyContext->get(keyIndex));
        a.checkNonNull("91. get key", v.get());

        String_t sv;
        a.check("101. key string", interpreter::checkStringArg(sv, v.get()));
        a.checkEqual("102. key", sv, "A");
    }
    {
        std::auto_ptr<afl::data::Value> v(valueContext->get(valueIndex));
        a.checkNonNull("103. get value", v.get());

        int32_t iv;
        a.check("111. value int", interpreter::checkIntegerArg(iv, v.get()));
        a.checkEqual("112. value", iv, 42);
    }

    // - verify write access to the properties
    {
        std::auto_ptr<afl::data::Value> v(interpreter::makeStringValue("B"));
        AFL_CHECK_THROWS(a("121. set key"), keyContext->set(keyIndex, v.get()), interpreter::Error);
    }
    {
        std::auto_ptr<afl::data::Value> v(interpreter::makeStringValue("nv"));
        AFL_CHECK_SUCCEEDS(a("122. set value"), valueContext->set(valueIndex, v.get()));
    }

    // - verify advance
    a.check("131. next", !p->next());

    // Inquiry
    {
        // regular access
        afl::data::Segment seg;
        seg.pushBackNew(interpreter::makeStringValue("A"));
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> v(testee.get(args));
        a.checkNonNull("141. get('A')", v.get());

        String_t sv;
        a.check("151. checkStringArg", interpreter::checkStringArg(sv, v.get()));
        a.checkEqual("152. string value", sv, "nv");
    }
    {
        // access to clone: clone has been modified by the above as well
        afl::data::Segment seg;
        seg.pushBackNew(interpreter::makeStringValue("A"));
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> v(copy->get(args));
        a.checkNonNull("153. get('A') clone", v.get());

        String_t sv;
        a.check("161. checkStringArg", interpreter::checkStringArg(sv, v.get()));
        a.checkEqual("162. string value", sv, "nv");
    }
    {
        // case sensitive!
        afl::data::Segment seg;
        seg.pushBackNew(interpreter::makeStringValue("a"));
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> v(testee.get(args));
        a.checkNull("163. get('a')", v.get());
    }
    {
        // null index
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> v(testee.get(args));
        a.checkNull("164. get('null')", v.get());
    }
}

/** Test hash with multiple keys. */
AFL_TEST("interpreter.HashValue:multi", a)
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
        AFL_CHECK_SUCCEEDS(a("01. set"), testee.set(args, v.get()));
    }
    {
        // Assigning null
        afl::data::Segment seg;
        seg.pushBackNew(interpreter::makeStringValue("C"));
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_SUCCEEDS(a("02. set null"), testee.set(args, 0));
    }
    {
        // Assigning to null key
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        std::auto_ptr<afl::data::Value> v(interpreter::makeStringValue("null"));
        AFL_CHECK_THROWS(a("03. set null key"), testee.set(args, v.get()), interpreter::Error);
    }

    // Iterate
    bool aflag = false, bflag = false, cflag = false;
    std::auto_ptr<interpreter::Context> p(testee.makeFirstContext());
    a.checkNonNull("11. makeFirstContext", p.get());
    do {
        // Get key
        interpreter::Context::PropertyIndex_t keyIndex;
        interpreter::Context::PropertyAccessor* keyContext = p->lookup("KEY", keyIndex);
        a.checkNonNull("12. key", keyContext);
        std::auto_ptr<afl::data::Value> keyValue(keyContext->get(keyIndex));
        a.checkNonNull("13. key value", keyValue.get());

        // Get value
        interpreter::Context::PropertyIndex_t valueIndex;
        interpreter::Context::PropertyAccessor* valueContext = p->lookup("VALUE", valueIndex);
        a.checkNonNull("21. value", valueContext);
        std::auto_ptr<afl::data::Value> valueValue(valueContext->get(valueIndex));

        // Check
        String_t key;
        a.check("31. key string", interpreter::checkStringArg(key, keyValue.get()));
        if (key == "A") {
            int32_t iv;
            a.check("32. a", !aflag);
            a.check("33. a value", interpreter::checkIntegerArg(iv, valueValue.get()));
            a.checkEqual("34. a value", iv, 42);
            aflag = true;
        } else if (key == "B") {
            String_t sv;
            a.check("35. b", !bflag);
            a.check("36. b value", interpreter::checkStringArg(sv, valueValue.get()));
            a.checkEqual("37. b value", sv, "sv");
            bflag = true;
        } else if (key == "C") {
            a.check("38. c value", !cflag);
            a.checkNull("39. c value", valueValue.get());
            cflag = true;
        } else {
            a.fail("40. unexpected key");
        }
    } while (p->next());
    a.check("41. a", aflag);
    a.check("42. b", bflag);
    a.check("43. c", cflag);
}
