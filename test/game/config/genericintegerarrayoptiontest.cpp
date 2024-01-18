/**
  *  \file test/game/config/genericintegerarrayoptiontest.cpp
  *  \brief Test for game::config::GenericIntegerArrayOption
  */

#include "game/config/genericintegerarrayoption.hpp"

#include "afl/test/testrunner.hpp"
#include "game/config/integervalueparser.hpp"

namespace {
    const game::config::IntegerValueParser g_valueParser;
}

/** Test regular behaviour. */
AFL_TEST("game.config.GenericIntegerArrayOption", a)
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
    a.check("01. isAllTheSame", t.isAllTheSame());
    a.checkEqual("02. index", t(0), 1);
    a.checkEqual("03. index", t(1), 1);
    a.checkEqual("04. index", t(10), 1);
    a.checkEqual("05. index", t(100), 1);
    a.checkEqual("06. toString", t.toString(), "1,1,1");

    // Set single element
    t.set(2, 9);
    a.checkEqual("11. index", t(0), 1);
    a.checkEqual("12. index", t(1), 1);
    a.checkEqual("13. index", t(2), 9);
    a.checkEqual("14. index", t(3), 1);
    a.checkEqual("15. index", t(4), 1);
    a.checkEqual("16. toString", t.toString(), "1,9,1");
    a.check("17. isAllTheSame", !t.isAllTheSame());

    t.set(3, 7);
    a.checkEqual("21. index", t(0), 7);
    a.checkEqual("22. index", t(1), 1);
    a.checkEqual("23. index", t(2), 9);
    a.checkEqual("24. index", t(3), 7);
    a.checkEqual("25. index", t(4), 7);
    a.checkEqual("26. toString", t.toString(), "1,9,7");
    a.check("27. isAllTheSame", !t.isAllTheSame());

    // Set more
    t.set(1, 7);
    t.set(2, 7);
    a.check("31. isAllTheSame", t.isAllTheSame());
    a.checkEqual("32. toString", t.toString(), "7,7,7");

    t.set(99);
    a.check("41. isAllTheSame", t.isAllTheSame());
    a.checkEqual("42. toString", t.toString(), "99,99,99");
}

/** Test behaviour with zero-element array. */
AFL_TEST("game.config.GenericIntegerArrayOption:zero-length", a)
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
    a.check("01. isAllTheSame", t.isAllTheSame());
    a.checkEqual("02. index", t(0), 0);
    a.checkEqual("03. index", t(1), 0);
    a.checkEqual("04. index", t(10), 0);
    a.checkEqual("05. index", t(100), 0);
    a.checkEqual("06. toString", t.toString(), "");

    // Set-element does not change anything
    t.set(9);
    a.checkEqual("11. index", t(0), 0);
    a.checkEqual("12. index", t(1), 0);
    a.checkEqual("13. index", t(10), 0);
    a.checkEqual("14. index", t(100), 0);
    a.checkEqual("15. toString", t.toString(), "");

    // Set-individual does not change anything
    t.set(1, 10);
    a.checkEqual("21. index", t(0), 0);
    a.checkEqual("22. index", t(1), 0);
    a.checkEqual("23. index", t(10), 0);
    a.checkEqual("24. index", t(100), 0);
    a.checkEqual("25. toString", t.toString(), "");
}
