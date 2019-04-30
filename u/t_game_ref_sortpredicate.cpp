/**
  *  \file u/t_game_ref_sortpredicate.cpp
  *  \brief Test for game::ref::SortPredicate
  */

#include "game/ref/sortpredicate.hpp"

#include "t_game_ref.hpp"

/** Interface test. */
void
TestGameRefSortPredicate::testInterface()
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
void
TestGameRefSortPredicate::testThen()
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
    TS_ASSERT_EQUALS(Always(3, "x").then(Always(2, "y")).compare(r, r), 3);
    TS_ASSERT_EQUALS(Always(0, "x").then(Always(2, "y")).compare(r, r), 2);
    TS_ASSERT_EQUALS(Always(0, "x").then(Always(0, "y")).compare(r, r), 0);

    // Check .then().getClass()
    TS_ASSERT_EQUALS(Always(3, "x").then(Always(2, "y")).getClass(r), "x");
    TS_ASSERT_EQUALS(Always(0, "x").then(Always(2, "y")).getClass(r), "x");
    TS_ASSERT_EQUALS(Always(0, "x").then(Always(0, "y")).getClass(r), "x");
}

