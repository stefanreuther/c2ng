/**
  *  \file u/t_interpreter_keywords.cpp
  *  \brief Test for interpreter::Keywords
  */

#include "interpreter/keywords.hpp"

#include "t_interpreter.hpp"
#include "interpreter/propertyacceptor.hpp"

/** Test enumKeywords. */
void
TestInterpreterKeywords::testEnum()
{
    class Tester : public interpreter::PropertyAcceptor {
     public:
        Tester()
            : m_seenAbort(false),
              m_seenWith(false),
              m_seenRedim(false)
            { }
        virtual void addProperty(const String_t& name, interpreter::TypeHint th)
            {
                // Validate type hint
                TS_ASSERT_EQUALS(th, interpreter::thNone);

                // Validate that resolving the keyword back works
                TS_ASSERT_DIFFERS(interpreter::lookupKeyword(name), interpreter::kwNone);

                // Make sure we see some specific keywords (but only once)
                if (name == "ABORT") {
                    TS_ASSERT(!m_seenAbort);
                    m_seenAbort = true;
                }
                if (name == "WITH") {
                    TS_ASSERT(!m_seenWith);
                    m_seenWith = true;
                }
                if (name == "REDIM") {
                    TS_ASSERT(!m_seenRedim);
                    m_seenRedim = true;
                }
            }

        bool m_seenAbort, m_seenWith, m_seenRedim;
    };
    Tester t;
    interpreter::enumKeywords(t);
    TS_ASSERT(t.m_seenAbort);
    TS_ASSERT(t.m_seenWith);
    TS_ASSERT(t.m_seenRedim);
}

/** Test lookupKeyword. */
void
TestInterpreterKeywords::testLookup()
{
    // Some successful lookups
    TS_ASSERT_EQUALS(interpreter::lookupKeyword("ABORT"), interpreter::kwAbort);
    TS_ASSERT_EQUALS(interpreter::lookupKeyword("BIND"), interpreter::kwBind);
    TS_ASSERT_EQUALS(interpreter::lookupKeyword("END"), interpreter::kwEnd);
    TS_ASSERT_EQUALS(interpreter::lookupKeyword("ENDSTRUCT"), interpreter::kwEndStruct);
    TS_ASSERT_EQUALS(interpreter::lookupKeyword("WITH"), interpreter::kwWith);

    // Case sensitive!
    TS_ASSERT_EQUALS(interpreter::lookupKeyword("with"), interpreter::kwNone);

    // Boundary cases
    TS_ASSERT_EQUALS(interpreter::lookupKeyword(""), interpreter::kwNone);
    TS_ASSERT_EQUALS(interpreter::lookupKeyword("WITH "), interpreter::kwNone);
    TS_ASSERT_EQUALS(interpreter::lookupKeyword("WI"), interpreter::kwNone);
    TS_ASSERT_EQUALS(interpreter::lookupKeyword("ABORTED"), interpreter::kwNone);
    TS_ASSERT_EQUALS(interpreter::lookupKeyword("ENDF"), interpreter::kwNone);
}
