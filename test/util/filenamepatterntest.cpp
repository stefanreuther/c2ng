/**
  *  \file test/util/filenamepatterntest.cpp
  *  \brief Test for util::FileNamePattern
  */

#include "util/filenamepattern.hpp"

#include "afl/test/testrunner.hpp"
#include <stdexcept>

/*
 *  Wildcard tests.
 *  // ex IoGlobTestSuite::testGlob
 */

AFL_TEST("util.FileNamePattern:basic", a)
{
    util::FileNamePattern m("foo*.*");
    a.check("01", !m.match("foo"));
    a.check("02", !m.match("foo1"));
    a.check("03", m.match("foo."));
    a.check("04", m.match("FOO."));
    a.check("05", m.match("foobar.blub"));
    a.check("06", m.match("foo.bar"));
    a.check("07", m.hasWildcard());
    a.check("08", !m.getFileName().isValid());
    a.check("09", !m.empty());
}

AFL_TEST("util.FileNamePattern:multiple-stars", a)
{
    util::FileNamePattern m("*************************");
    a.check("01", m.match(""));
    a.check("02", m.match("xxxxxxxxxxxxxxxxxxxxxxxxxxx"));
    a.check("03", m.hasWildcard());
    a.check("04", !m.empty());
}

AFL_TEST("util.FileNamePattern:multiple-stars-suffix", a)
{
    util::FileNamePattern m("*************************x");
    a.check("01", !m.match(""));
    a.check("02", m.match("xxxxxxxxxxxxxxxxxxxxxxxxxxx"));
    /* The following will take a lot of time if we don't
       optimise multiple '*' */
    a.check("03", !m.match("yyyyyyyyyyyyyyyyyyyyyyyyyyy"));
    a.check("04", m.hasWildcard());
}

AFL_TEST("util.FileNamePattern:star", a)
{
    util::FileNamePattern m("*");
    a.check("01", m.match(""));
    a.check("02", m.match("a"));
    a.check("03", m.match("aaaaaaaaaaa"));
    a.check("04", m.match("*****"));
    a.check("05", m.hasWildcard());
}

AFL_TEST("util.FileNamePattern:suffix", a)
{
    util::FileNamePattern m("*a");
    a.check("01", !m.match(""));
    a.check("02", m.match("a"));
    a.check("03", m.match("aaaaaaaaaaa"));
    a.check("04", !m.match("*****"));
    a.check("05", m.hasWildcard());
}

AFL_TEST("util.FileNamePattern:stars-and-ques", a)
{
    util::FileNamePattern m("*?*?*?*?*?*?*?*?*?*?*?*?*?*");
    a.check("01", !m.match(""));
    a.check("02", !m.match("123456789ABC"));
    a.check("03", m.match("123456789ABCD"));
    a.check("04", m.match("123456789ABCDEFG"));
    a.check("05", m.hasWildcard());
}

AFL_TEST("util.FileNamePattern:stars-and-ques-suffix", a)
{
    util::FileNamePattern m("*?*?*?*?*?*?*?*?*?*?*?*?*?*x");
    a.check("01", !m.match(""));
    a.check("02", !m.match("123456789ABC"));
    a.check("03", !m.match("123456789ABCD"));
    a.check("04", !m.match("123456789ABCDEFG"));
    a.check("05", !m.match("123456789ABCx"));
    a.check("06", m.match("123456789ABCDx"));
    a.check("07", m.match("123456789ABCDEFGx"));
    a.check("08", m.hasWildcard());
}

AFL_TEST("util.FileNamePattern:quoted-star", a)
{
    util::FileNamePattern m("*\\**");
    a.check("01", !m.match("foo"));
    a.check("02", !m.match(""));
    a.check("03", m.match("foo*bar"));
    a.check("04", m.hasWildcard());
}

AFL_TEST("util.FileNamePattern:ques", a)
{
    util::FileNamePattern m("a?b");
    a.check("01", m.match("axb"));
    a.check("02", !m.match("ab"));
    a.check("03", !m.match("abc"));
    a.check("04", !m.match("axbc"));
    a.check("05", !m.match("axxb"));

    a.check("11", m.match("a\xc2\x80""b"));
    a.check("12", !m.match("ab"));
    a.check("13", !m.match("abc"));
    a.check("14", !m.match("a\xc2\x80""bc"));
    a.check("15", !m.match("a\xc2\x80\xc2\x80""b"));
    a.check("16. hasWildcard", m.hasWildcard());
}

AFL_TEST("util.FileNamePattern:ques-ques", a)
{
    util::FileNamePattern m("a??b");
    a.check("01", !m.match("axb"));
    a.check("02", !m.match("ab"));
    a.check("03", !m.match("abc"));
    a.check("04", !m.match("axbc"));
    a.check("05", m.match("axxb"));

    a.check("11", !m.match("a\xc2\x80""b"));
    a.check("12", !m.match("ab"));
    a.check("13", !m.match("abc"));
    a.check("14", !m.match("a\xc2\x80""bc"));
    a.check("15", m.match("a\xc2\x80\xc2\x80""b"));
    a.check("16. hasWildcard", m.hasWildcard());
}

AFL_TEST("util.FileNamePattern:empty", a)
{
    util::FileNamePattern m("");
    a.check("01. empty", m.empty());
    a.checkEqual("02. getFileName", m.getFileName().orElse("?"), "");
    a.check("03. hasWildcard", !m.hasWildcard());
}

/*
 *  Test failure (exception) cases.
 */

AFL_TEST("util.FileNamePattern:error:setPattern", a)
{
    util::FileNamePattern testee;
    AFL_CHECK_THROWS(a("t01"), testee.setPattern("\\"), std::runtime_error);
    AFL_CHECK_THROWS(a("t02"), testee.setPattern("a\\"), std::runtime_error);
}

AFL_TEST("util.FileNamePattern:error:constructor", a)
{
    AFL_CHECK_THROWS(a, util::FileNamePattern("a\\"), std::runtime_error);
}

/*
 *  Test literal patterns.
 */

AFL_TEST("util.FileNamePattern:literal", a)
{
    util::FileNamePattern m("foo");
    a.check("01", !m.match("fo"));
    a.check("02",  m.match("foo"));
    a.check("03", !m.match("fooo"));
    a.check("04", !m.hasWildcard());
    a.checkEqual("05", m.getFileName().orElse(""), "foo");
}

AFL_TEST("util.FileNamePattern:literal:quote-star", a)
{
    util::FileNamePattern m("a\\*b");
    a.check("01", m.match("a*b"));
    a.check("02", !m.hasWildcard());
    a.checkEqual("03", m.getFileName().orElse(""), "a*b");
}

AFL_TEST("util.FileNamePattern:literal:quote-ques", a)
{
    util::FileNamePattern m("a\\?b");
    a.check("01", m.match("a?b"));
    a.check("02", !m.hasWildcard());
    a.checkEqual("03", m.getFileName().orElse(""), "a?b");
}

/*
 *  Test copying patterns.
 */

AFL_TEST("util.FileNamePattern:copy", a)
{
    util::FileNamePattern orig("foo");
    util::FileNamePattern copy(orig);
    a.check("01", orig.match("foo"));
    a.check("02", copy.match("foo"));

    orig.setPattern("bar");
    a.check("11", orig.match("bar"));
    a.check("12", copy.match("foo"));

    copy = orig;
    a.check("21", orig.match("bar"));
    a.check("22", copy.match("bar"));
}

/*
 *  Test prepared patterns.
 */

AFL_TEST("util.FileNamePattern:getAllFilesPattern", a)
{
    util::FileNamePattern t(util::FileNamePattern::getAllFilesPattern());
    a.check("01", t.match(""));
    a.check("02", t.match("a"));
    a.check("03", t.match("aaaaaa"));
    a.check("04", t.match("a*a"));
}

AFL_TEST("util.FileNamePattern:getSingleFilePattern", a)
{
    util::FileNamePattern t(util::FileNamePattern::getSingleFilePattern("abc"));
    a.check("01", !t.match(""));
    a.check("02", !t.match("a"));
    a.check("03",  t.match("abc"));
    a.check("04", !t.match("a*c"));
    a.check("05", !t.match("abcde"));
}

AFL_TEST("util.FileNamePattern:getSingleFilePattern:star", a)
{
    util::FileNamePattern t(util::FileNamePattern::getSingleFilePattern("a*c"));
    a.check("11", !t.match(""));
    a.check("12", !t.match("a"));
    a.check("13", !t.match("abc"));
    a.check("14",  t.match("a*c"));
    a.check("15", !t.match("abbc"));
}

AFL_TEST("util.FileNamePattern:getAllFilesWithExtensionPattern", a)
{
    util::FileNamePattern t(util::FileNamePattern::getAllFilesWithExtensionPattern("qc"));
    a.check("01", !t.match("qc"));
    a.check("02",  t.match(".qc"));
    a.check("03",  t.match("f.qc"));
    a.check("04",  t.match("blaa.qc"));
    a.check("05", !t.match("x.qcc"));
}

AFL_TEST("util.FileNamePattern:getAllFilesWithExtensionPattern:star", a)
{
    util::FileNamePattern t(util::FileNamePattern::getAllFilesWithExtensionPattern("q*"));
    a.check("00", !t.match("qc"));
    a.check("01", !t.match(".qc"));
    a.check("02",  t.match(".q*"));
    a.check("03", !t.match("f.qc"));
    a.check("04",  t.match("f.q*"));
}
