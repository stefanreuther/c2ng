/**
  *  \file u/t_interpreter_mutexcontext.cpp
  *  \brief Test for interpreter::MutexContext
  */

#include "interpreter/mutexcontext.hpp"

#include "t_interpreter.hpp"
#include "afl/io/internalsink.hpp"
#include "interpreter/mutexlist.hpp"
#include "interpreter/propertyacceptor.hpp"
#include "interpreter/savecontext.hpp"
#include "interpreter/tagnode.hpp"

namespace {
    class MySaveContext : public interpreter::SaveContext {
     public:
        virtual uint32_t addBCO(const interpreter::BytecodeObject& /*bco*/)
            { TS_FAIL("addBCO"); return 0; }
        virtual uint32_t addHash(const afl::data::Hash& /*hash*/)
            { TS_FAIL("addHash"); return 0; }
        virtual uint32_t addArray(const interpreter::ArrayData& /*array*/)
            { TS_FAIL("addArray"); return 0; }
        virtual uint32_t addStructureType(const interpreter::StructureTypeData& /*type*/)
            { TS_FAIL("addStructureType"); return 0; }
        virtual uint32_t addStructureValue(const interpreter::StructureValueData& /*value*/)
            { TS_FAIL("addStructureValue"); return 0; }
        virtual bool isCurrentProcess(const interpreter::Process* /*p*/)
            { return false; }
    };
}


/** Test saving a mutex.
    A: set up a mutex and save it
    E: correct serialisation format */
void
TestInterpreterMutexContext::testSave()
{
    interpreter::MutexList list;
    interpreter::MutexContext testee("NAME", "long info");

    // Save it
    interpreter::TagNode tag;
    afl::io::InternalSink aux;
    MySaveContext sc;

    TS_ASSERT_THROWS_NOTHING(testee.store(tag, aux, sc));

    TS_ASSERT_EQUALS(tag.tag, interpreter::TagNode::Tag_Mutex);
    TS_ASSERT_EQUALS(tag.value, 0U);

    const uint8_t EXPECTED_AUX[] = {
        4,0,0,0,                              // length of name
        9,0,0,0,                              // length of info
        'N','A','M','E',                      // name
        'l','o','n','g',' ','i','n','f','o',  // info
    };

    TS_ASSERT_EQUALS(aux.getContent().size(), sizeof(EXPECTED_AUX));
    TS_ASSERT_SAME_DATA(aux.getContent().unsafeData(), EXPECTED_AUX, sizeof(EXPECTED_AUX));
}

/** Test basics.
    A: set up a mutex, call basic functions on it.
    E: correct results */
void
TestInterpreterMutexContext::testBasics()
{
    interpreter::MutexList list;
    interpreter::MutexContext testee("NAME", "long info");

    // lookup: always fails
    interpreter::Context::PropertyIndex_t x;
    TS_ASSERT(testee.lookup("FOO", x) == 0);
    TS_ASSERT(testee.lookup("", x) == 0);
    TS_ASSERT(testee.lookup("NAME", x) == 0);

    // next: no next object
    TS_ASSERT_EQUALS(testee.next(), false);

    // getObject: no embedded object
    TS_ASSERT(testee.getObject() == 0);

    // enumProperties: none
    class MyPropertyAcceptor : public interpreter::PropertyAcceptor {
     public:
        virtual void addProperty(const String_t& /*name*/, interpreter::TypeHint /*th*/)
            { TS_FAIL("addProperty"); }
    };
    MyPropertyAcceptor pa;
    testee.enumProperties(pa);

    // toString
    TS_ASSERT_EQUALS(testee.toString(false), "#<lock>");
    TS_ASSERT_EQUALS(testee.toString(true), "Lock(\"NAME\",\"long info\")");
}

