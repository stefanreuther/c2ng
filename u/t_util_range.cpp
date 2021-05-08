/**
  *  \file u/t_util_range.cpp
  *  \brief Test for util::Range
  */

#include "util/range.hpp"

#include "t_util.hpp"
#include "afl/string/nulltranslator.hpp"

void
TestUtilRange::testInit()
{
    util::Range<int> a;
    TS_ASSERT(a.empty());
    TS_ASSERT(!a.contains(0));
    TS_ASSERT(!a.contains(1));

    util::Range<int> b(3, 10);
    TS_ASSERT(!b.empty());
    TS_ASSERT_EQUALS(b.min(), 3);
    TS_ASSERT_EQUALS(b.max(), 10);
    TS_ASSERT(b.contains(3));
    TS_ASSERT(b.contains(10));
    TS_ASSERT(!b.contains(2));
    TS_ASSERT(!b.contains(11));
    TS_ASSERT(!b.isUnit());

    util::Range<int> c = util::Range<int>::fromValue(2);
    TS_ASSERT(!c.empty());
    TS_ASSERT_EQUALS(c.min(), 2);
    TS_ASSERT_EQUALS(c.max(), 2);
    TS_ASSERT(c.contains(2));
    TS_ASSERT(!c.contains(1));
    TS_ASSERT(!c.contains(3));
    TS_ASSERT(c.isUnit());

    c.clear();
    TS_ASSERT(c.empty());
    TS_ASSERT(!c.contains(2));
}

void
TestUtilRange::testInclude()
{
    util::Range<int> a(10, 20);
    a.include(util::Range<int>(30, 40));
    TS_ASSERT_EQUALS(a.min(), 10);
    TS_ASSERT_EQUALS(a.max(), 40);

    util::Range<int> b(10, 20);
    b.include(util::Range<int>());
    TS_ASSERT_EQUALS(b.min(), 10);
    TS_ASSERT_EQUALS(b.max(), 20);

    util::Range<int> c;
    c.include(util::Range<int>(5, 8));
    TS_ASSERT_EQUALS(c.min(), 5);
    TS_ASSERT_EQUALS(c.max(), 8);

    util::Range<int> d(10, 20);
    d.include(util::Range<int>(5, 15));
    TS_ASSERT_EQUALS(d.min(), 5);
    TS_ASSERT_EQUALS(d.max(), 20);
}

void
TestUtilRange::testIntersect()
{
    util::Range<int> a(10, 20);
    a.intersect(util::Range<int>(30, 40));
    TS_ASSERT(a.empty());

    util::Range<int> b(10, 20);
    b.intersect(util::Range<int>());
    TS_ASSERT(b.empty());

    util::Range<int> c;
    c.intersect(util::Range<int>(5, 8));
    TS_ASSERT(c.empty());

    util::Range<int> d(10, 20);
    d.intersect(util::Range<int>(5, 15));
    TS_ASSERT_EQUALS(d.min(), 10);
    TS_ASSERT_EQUALS(d.max(), 15);
}

void
TestUtilRange::testOp()
{
    util::Range<int> a(5, 10);
    a += util::Range<int>(2, 6);
    TS_ASSERT_EQUALS(a.min(), 7);
    TS_ASSERT_EQUALS(a.max(), 16);

    a -= util::Range<int>(2, 6);
    TS_ASSERT_EQUALS(a.min(), 1);
    TS_ASSERT_EQUALS(a.max(), 14);

    util::Range<int> b;
    b += util::Range<int>(1, 9);
    TS_ASSERT(b.empty());

    util::Range<int> c(5, 10);
    c += util::Range<int>();
    TS_ASSERT(c.empty());
}

void
TestUtilRange::testFormat()
{
    util::Range<int> max(1, 10000);
    util::NumberFormatter fmt(true, true);
    afl::string::NullTranslator tx;

    TS_ASSERT_EQUALS(toString(util::Range<int>(5, 9000),     max, true, fmt, tx), "5 to 9,000");
    TS_ASSERT_EQUALS(toString(util::Range<int>(1, 1000),     max, true, fmt, tx), "up to 1,000");
    TS_ASSERT_EQUALS(toString(util::Range<int>(5000, 10000), max, true, fmt, tx), "5,000 or more");
    TS_ASSERT_EQUALS(toString(util::Range<int>(7777, 7777),  max, true, fmt, tx), "7,777");
    TS_ASSERT_EQUALS(toString(util::Range<int>(),            max, true, fmt, tx), "none");

    TS_ASSERT_EQUALS(toString(util::Range<int>(5, 9000),     max, false, fmt, tx), "5" UTF_EN_DASH "9,000");
    TS_ASSERT_EQUALS(toString(util::Range<int>(1, 1000),     max, false, fmt, tx), UTF_LEQ " 1,000");
    TS_ASSERT_EQUALS(toString(util::Range<int>(5000, 10000), max, false, fmt, tx), UTF_GEQ " 5,000");
    TS_ASSERT_EQUALS(toString(util::Range<int>(7777, 7777),  max, false, fmt, tx), "7,777");
    TS_ASSERT_EQUALS(toString(util::Range<int>(),            max, false, fmt, tx), "-");
}

