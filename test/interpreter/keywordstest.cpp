/**
  *  \file test/interpreter/keywordstest.cpp
  *  \brief Test for interpreter::Keywords
  */

#include "interpreter/keywords.hpp"

#include "afl/test/testrunner.hpp"
#include "interpreter/propertyacceptor.hpp"

/** Test enumKeywords. */
AFL_TEST("interpreter.Keywords:enumKeywords", a)
{
    class Tester : public interpreter::PropertyAcceptor {
     public:
        Tester(afl::test::Assert a)
            : m_assert(a),
              m_seenAbort(false),
              m_seenWith(false),
              m_seenRedim(false)
            { }
        virtual void addProperty(const String_t& name, interpreter::TypeHint th)
            {
                // Validate type hint
                m_assert.checkEqual("01. type hint", th, interpreter::thNone);

                // Validate that resolving the keyword back works
                m_assert.checkDifferent("11. lookup", interpreter::lookupKeyword(name), interpreter::kwNone);

                // Make sure we see some specific keywords (but only once)
                if (name == "ABORT") {
                    m_assert.check("21. m_seenAbort", !m_seenAbort);
                    m_seenAbort = true;
                }
                if (name == "WITH") {
                    m_assert.check("22. m_seenWith", !m_seenWith);
                    m_seenWith = true;
                }
                if (name == "REDIM") {
                    m_assert.check("23. m_seenRedim", !m_seenRedim);
                    m_seenRedim = true;
                }
            }

        afl::test::Assert m_assert;
        bool m_seenAbort, m_seenWith, m_seenRedim;
    };
    Tester t(a);
    interpreter::enumKeywords(t);
    a.check("31. m_seenAbort", t.m_seenAbort);
    a.check("32. m_seenWith", t.m_seenWith);
    a.check("33. m_seenRedim", t.m_seenRedim);
}

/** Test lookupKeyword. */
AFL_TEST("interpreter.Keywords:lookupKeyword", a)
{
    // Some successful lookups
    a.checkEqual("01", interpreter::lookupKeyword("ABORT"), interpreter::kwAbort);
    a.checkEqual("02", interpreter::lookupKeyword("BIND"), interpreter::kwBind);
    a.checkEqual("03", interpreter::lookupKeyword("END"), interpreter::kwEnd);
    a.checkEqual("04", interpreter::lookupKeyword("ENDSTRUCT"), interpreter::kwEndStruct);
    a.checkEqual("05", interpreter::lookupKeyword("WITH"), interpreter::kwWith);

    // Case sensitive!
    a.checkEqual("11", interpreter::lookupKeyword("with"), interpreter::kwNone);

    // Boundary cases
    a.checkEqual("21", interpreter::lookupKeyword(""), interpreter::kwNone);
    a.checkEqual("22", interpreter::lookupKeyword("WITH "), interpreter::kwNone);
    a.checkEqual("23", interpreter::lookupKeyword("WI"), interpreter::kwNone);
    a.checkEqual("24", interpreter::lookupKeyword("ABORTED"), interpreter::kwNone);
    a.checkEqual("25", interpreter::lookupKeyword("ENDF"), interpreter::kwNone);
}
