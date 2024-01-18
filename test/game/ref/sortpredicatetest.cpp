/**
  *  \file test/game/ref/sortpredicatetest.cpp
  *  \brief Test for game::ref::SortPredicate
  */

#include "game/ref/sortpredicate.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.ref.SortPredicate:interface")
{
    class Tester : public game::ref::SortPredicate {
     public:
        virtual int compare(const game::Reference& /*a*/, const game::Reference& /*b*/) const
            { return 0; }
        virtual String_t getClass(const game::Reference& /*a*/) const
            { return String_t(); }
    };
    Tester t;
}

/** Test SortPredicate::then(). */
AFL_TEST("game.ref.SortPredicate:then", a)
{
    class Always : public game::ref::SortPredicate {
     public:
        Always(int value, String_t name)
            : m_value(value), m_name(name)
            { }
        virtual int compare(const game::Reference& /*a*/, const game::Reference& /*b*/) const
            { return m_value; }
        virtual String_t getClass(const game::Reference& /*a*/) const
            { return m_name; }
     private:
        int m_value;
        String_t m_name;
    };

    game::Reference r;

    // Check .then().compare()
    a.checkEqual("01", Always(3, "x").then(Always(2, "y")).compare(r, r), 3);
    a.checkEqual("02", Always(0, "x").then(Always(2, "y")).compare(r, r), 2);
    a.checkEqual("03", Always(0, "x").then(Always(0, "y")).compare(r, r), 0);

    // Check .then().getClass()
    a.checkEqual("11", Always(3, "x").then(Always(2, "y")).getClass(r), "x");
    a.checkEqual("12", Always(0, "x").then(Always(2, "y")).getClass(r), "x");
    a.checkEqual("13", Always(0, "x").then(Always(0, "y")).getClass(r), "x");
}
