/**
  *  \file u/t_game_config_genericintegerarrayoption.cpp
  *  \brief Test for game::config::GenericIntegerArrayOption
  */

#include "game/config/genericintegerarrayoption.hpp"

#include "t_game_config.hpp"
#include "game/config/integervalueparser.hpp"

namespace {
    const game::config::IntegerValueParser g_valueParser;
}

/** Test regular behaviour. */
void
TestGameConfigGenericIntegerArrayOption::testIt()
{
    class Tester : public game::config::GenericIntegerArrayOption {
     public:
        Tester()
            : GenericIntegerArrayOption(g_valueParser)
            {
                getArray().fill(1);
            }
        virtual afl::base::Memory<int32_t> getArray()
            { return afl::base::Memory<int32_t>(m_array); }
        virtual String_t toString() const
            { return parser().toStringArray(GenericIntegerArrayOption::getArray()); }
     private:
        int32_t m_array[3];
    };

    Tester t;

    // Initial state
    TS_ASSERT(t.isAllTheSame());
    TS_ASSERT_EQUALS(t(0), 1);
    TS_ASSERT_EQUALS(t(1), 1);
    TS_ASSERT_EQUALS(t(10), 1);
    TS_ASSERT_EQUALS(t(100), 1);
    TS_ASSERT_EQUALS(t.toString(), "1,1,1");

    // Set single element
    t.set(2, 9);
    TS_ASSERT_EQUALS(t(0), 1);
    TS_ASSERT_EQUALS(t(1), 1);
    TS_ASSERT_EQUALS(t(2), 9);
    TS_ASSERT_EQUALS(t(3), 1);
    TS_ASSERT_EQUALS(t(4), 1);
    TS_ASSERT_EQUALS(t.toString(), "1,9,1");
    TS_ASSERT(!t.isAllTheSame());

    t.set(3, 7);
    TS_ASSERT_EQUALS(t(0), 7)
    TS_ASSERT_EQUALS(t(1), 1);
    TS_ASSERT_EQUALS(t(2), 9);
    TS_ASSERT_EQUALS(t(3), 7);
    TS_ASSERT_EQUALS(t(4), 7);
    TS_ASSERT_EQUALS(t.toString(), "1,9,7");
    TS_ASSERT(!t.isAllTheSame());

    // Set more
    t.set(1, 7);
    t.set(2, 7);
    TS_ASSERT(t.isAllTheSame());
    TS_ASSERT_EQUALS(t.toString(), "7,7,7");

    t.set(99);
    TS_ASSERT(t.isAllTheSame());
    TS_ASSERT_EQUALS(t.toString(), "99,99,99");
}

/** Test behaviour with zero-element array. */
void
TestGameConfigGenericIntegerArrayOption::testZero()
{
    class Tester : public game::config::GenericIntegerArrayOption {
     public:
        Tester()
            : GenericIntegerArrayOption(g_valueParser)
            { }
        virtual afl::base::Memory<int32_t> getArray()
            { return afl::base::Memory<int32_t>(); }
        virtual String_t toString() const
            { return parser().toStringArray(GenericIntegerArrayOption::getArray()); }
    };
    Tester t;

    // Initial state
    TS_ASSERT(t.isAllTheSame());
    TS_ASSERT_EQUALS(t(0), 0);
    TS_ASSERT_EQUALS(t(1), 0);
    TS_ASSERT_EQUALS(t(10), 0);
    TS_ASSERT_EQUALS(t(100), 0);
    TS_ASSERT_EQUALS(t.toString(), "");

    // Set-element does not change anything
    t.set(9);
    TS_ASSERT_EQUALS(t(0), 0);
    TS_ASSERT_EQUALS(t(1), 0);
    TS_ASSERT_EQUALS(t(10), 0);
    TS_ASSERT_EQUALS(t(100), 0);
    TS_ASSERT_EQUALS(t.toString(), "");

    // Set-individual does not change anything
    t.set(1, 10);
    TS_ASSERT_EQUALS(t(0), 0);
    TS_ASSERT_EQUALS(t(1), 0);
    TS_ASSERT_EQUALS(t(10), 0);
    TS_ASSERT_EQUALS(t(100), 0);
    TS_ASSERT_EQUALS(t.toString(), "");
}
