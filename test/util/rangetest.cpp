/**
  *  \file test/util/rangetest.cpp
  *  \brief Test for util::Range
  */

#include "util/range.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("util.Range:init;empty", a)
{
    util::Range<int> ra;
    a.check("01. empty", ra.empty());
    a.check("02. contains", !ra.contains(0));
    a.check("03. contains", !ra.contains(1));
}

AFL_TEST("util.Range:init:range", a)
{
    util::Range<int> rb(3, 10);
    a.check("01. empty", !rb.empty());
    a.checkEqual("02. min", rb.min(), 3);
    a.checkEqual("03. max", rb.max(), 10);
    a.check("04. contains", rb.contains(3));
    a.check("05. contains", rb.contains(10));
    a.check("06. contains", !rb.contains(2));
    a.check("07. contains", !rb.contains(11));
    a.check("08. isUnit",   !rb.isUnit());
}

AFL_TEST("util.Range:init:fromValue", a)
{
    util::Range<int> rc = util::Range<int>::fromValue(2);
    a.check("01. empty", !rc.empty());
    a.checkEqual("02. min", rc.min(), 2);
    a.checkEqual("03. max", rc.max(), 2);
    a.check("04. contains", rc.contains(2));
    a.check("05. contains", !rc.contains(1));
    a.check("06. contains", !rc.contains(3));
    a.check("07. isUnit",   rc.isUnit());

    rc.clear();
    a.check("11. empty", rc.empty());
    a.check("12. contains", !rc.contains(2));
}

/*
 *  include
 */

AFL_TEST("util.Range:include:set+set", a)
{
    util::Range<int> ra(10, 20);
    ra.include(util::Range<int>(30, 40));
    a.checkEqual("min", ra.min(), 10);
    a.checkEqual("max", ra.max(), 40);
}

AFL_TEST("util.Range:include:set+empty", a)
{
    util::Range<int> rb(10, 20);
    rb.include(util::Range<int>());
    a.checkEqual("min", rb.min(), 10);
    a.checkEqual("max", rb.max(), 20);
}

AFL_TEST("util.Range:include:empty+set", a)
{
    util::Range<int> rc;
    rc.include(util::Range<int>(5, 8));
    a.checkEqual("min", rc.min(), 5);
    a.checkEqual("max", rc.max(), 8);
}

AFL_TEST("util.Range:include:overlapping-sets", a)
{
    util::Range<int> rd(10, 20);
    rd.include(util::Range<int>(5, 15));
    a.checkEqual("min", rd.min(), 5);
    a.checkEqual("max", rd.max(), 20);
}

/*
 *  intersect
 */

AFL_TEST("util.Range:intersect:disjoint-sets", a)
{
    util::Range<int> ra(10, 20);
    ra.intersect(util::Range<int>(30, 40));
    a.check("empty", ra.empty());
}

AFL_TEST("util.Range:intersect:set+empty", a)
{
    util::Range<int> rb(10, 20);
    rb.intersect(util::Range<int>());
    a.check("empty", rb.empty());
}

AFL_TEST("util.Range:intersect:empty+set", a)
{
    util::Range<int> rc;
    rc.intersect(util::Range<int>(5, 8));
    a.check("empty", rc.empty());
}

AFL_TEST("util.Range:intersect:overlapping-sets", a)
{
    util::Range<int> rd(10, 20);
    rd.intersect(util::Range<int>(5, 15));
    a.checkEqual("min", rd.min(), 10);
    a.checkEqual("max", rd.max(), 15);
}

/*
 *  Operators
 */

AFL_TEST("util.Range:op", a)
{
    util::Range<int> ra(5, 10);
    ra += util::Range<int>(2, 6);
    a.checkEqual("01. min", ra.min(), 7);
    a.checkEqual("02. max", ra.max(), 16);

    ra -= util::Range<int>(2, 6);
    a.checkEqual("11. min", ra.min(), 1);
    a.checkEqual("12. max", ra.max(), 14);
}

AFL_TEST("util.Range:op:empty+set", a)
{
    util::Range<int> rb;
    rb += util::Range<int>(1, 9);
    a.check("empty", rb.empty());
}

AFL_TEST("util.Range:op:set+empty", a)
{
    util::Range<int> rc(5, 10);
    rc += util::Range<int>();
    a.check("empty", rc.empty());
}

/*
 *  toString
 */

AFL_TEST("util.Range:toString", a)
{
    util::Range<int> max(1, 10000);
    util::NumberFormatter fmt(true, true);
    afl::string::NullTranslator tx;

    a.checkEqual("01", toString(util::Range<int>(5, 9000),     max, true, fmt, tx), "5 to 9,000");
    a.checkEqual("02", toString(util::Range<int>(1, 1000),     max, true, fmt, tx), "up to 1,000");
    a.checkEqual("03", toString(util::Range<int>(5000, 10000), max, true, fmt, tx), "5,000 or more");
    a.checkEqual("04", toString(util::Range<int>(7777, 7777),  max, true, fmt, tx), "7,777");
    a.checkEqual("05", toString(util::Range<int>(),            max, true, fmt, tx), "none");

    a.checkEqual("11", toString(util::Range<int>(5, 9000),     max, false, fmt, tx), "5" UTF_EN_DASH "9,000");
    a.checkEqual("12", toString(util::Range<int>(1, 1000),     max, false, fmt, tx), UTF_LEQ " 1,000");
    a.checkEqual("13", toString(util::Range<int>(5000, 10000), max, false, fmt, tx), UTF_GEQ " 5,000");
    a.checkEqual("14", toString(util::Range<int>(7777, 7777),  max, false, fmt, tx), "7,777");
    a.checkEqual("15", toString(util::Range<int>(),            max, false, fmt, tx), "-");
}
