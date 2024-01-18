/**
  *  \file test/interpreter/mutexcontexttest.cpp
  *  \brief Test for interpreter::MutexContext
  */

#include "interpreter/mutexcontext.hpp"

#include "afl/io/internalsink.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/mutexlist.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/savecontext.hpp"
#include "interpreter/tagnode.hpp"

namespace {
    class MySaveContext : public interpreter::SaveContext {
     public:
        virtual uint32_t addBCO(const interpreter::BytecodeObject& /*bco*/)
            { throw std::runtime_error("addBCO"); }
        virtual uint32_t addHash(const afl::data::Hash& /*hash*/)
            { throw std::runtime_error("addHash"); }
        virtual uint32_t addArray(const interpreter::ArrayData& /*array*/)
            { throw std::runtime_error("addArray"); }
        virtual uint32_t addStructureType(const interpreter::StructureTypeData& /*type*/)
            { throw std::runtime_error("addStructureType"); }
        virtual uint32_t addStructureValue(const interpreter::StructureValueData& /*value*/)
            { throw std::runtime_error("addStructureValue"); }
        virtual bool isCurrentProcess(const interpreter::Process* /*p*/)
            { return false; }
    };
}


/** Test saving a mutex.
    A: set up a mutex and save it
    E: correct serialisation format */
AFL_TEST("interpreter.MutexContext:save", a)
{
    interpreter::MutexList list;
    interpreter::MutexContext testee("NAME", "long info");

    // Save it
    interpreter::TagNode tag;
    afl::io::InternalSink aux;
    MySaveContext sc;

    AFL_CHECK_SUCCEEDS(a("01. store"), testee.store(tag, aux, sc));

    a.checkEqual("11. tag", tag.tag, interpreter::TagNode::Tag_Mutex);
    a.checkEqual("12. value", tag.value, 0U);

    const uint8_t EXPECTED_AUX[] = {
        4,0,0,0,                              // length of name
        9,0,0,0,                              // length of info
        'N','A','M','E',                      // name
        'l','o','n','g',' ','i','n','f','o',  // info
    };

    a.checkEqualContent<uint8_t>("21. content", aux.getContent(), EXPECTED_AUX);
}

/** Test basics.
    A: set up a mutex, call basic functions on it.
    E: correct results */
AFL_TEST("interpreter.MutexContext:basics", a)
{
    interpreter::MutexList list;
    interpreter::MutexContext testee("NAME", "long info");

    // lookup: always fails
    interpreter::Context::PropertyIndex_t x;
    a.checkNull("01. lookup", testee.lookup("FOO", x));
    a.checkNull("02. lookup", testee.lookup("", x));
    a.checkNull("03. lookup", testee.lookup("NAME", x));

    // next: no next object
    a.checkEqual("11. next", testee.next(), false);

    // getObject: no embedded object
    a.checkNull("21. getObject", testee.getObject());

    // enumProperties: none
    class MyPropertyAcceptor : public interpreter::PropertyAcceptor {
     public:
        virtual void addProperty(const String_t& /*name*/, interpreter::TypeHint /*th*/)
            { throw std::runtime_error("addProperty"); }
    };
    MyPropertyAcceptor pa;
    testee.enumProperties(pa);

    // toString
    a.checkEqual("31. toString", testee.toString(false), "#<lock>");
    a.checkEqual("32. toString", testee.toString(true), "Lock(\"NAME\",\"long info\")");
}
