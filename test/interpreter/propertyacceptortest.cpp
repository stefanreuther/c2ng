/**
  *  \file test/interpreter/propertyacceptortest.cpp
  *  \brief Test for interpreter::PropertyAcceptor
  */

#include "interpreter/propertyacceptor.hpp"

#include "afl/data/namemap.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("interpreter.PropertyAcceptor", a)
{
    // Test implementation
    class Tester : public interpreter::PropertyAcceptor {
     public:
        virtual void addProperty(const String_t& name, interpreter::TypeHint /*th*/)
            {
                if (!m_result.empty()) {
                    m_result += ",";
                }
                m_result += name;
            }
        String_t get() const
            { return m_result; }
     private:
        String_t m_result;
    };

    // Test with a NameMap
    {
        afl::data::NameMap m;
        m.add("A");
        m.add("B");
        m.add("X");

        Tester t;
        t.enumNames(m);
        a.checkEqual("01. get", t.get(), "A,B,X");
    }

    // Test with a table
    {
        static const interpreter::NameTable tab[] = {
            { "FIRST",  0, 0, interpreter::thInt },
            { "SECOND", 0, 0, interpreter::thString },
        };

        Tester t;
        t.enumTable(tab);
        a.checkEqual("11. get", t.get(), "FIRST,SECOND");
    }
}
