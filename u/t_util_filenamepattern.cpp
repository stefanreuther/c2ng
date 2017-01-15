/**
  *  \file u/t_util_filenamepattern.cpp
  *  \brief Test for util::FileNamePattern
  */

#include <stdexcept>
#include "util/filenamepattern.hpp"

#include "t_util.hpp"

/** Wildcard tests. */
void
TestUtilFileNamePattern::testIt()
{
    // ex IoGlobTestSuite::testGlob
    String_t tmp;
    {
        util::FileNamePattern m("foo*.*");
        TS_ASSERT(!m.match("foo"));
        TS_ASSERT(!m.match("foo1"));
        TS_ASSERT(m.match("foo."));
        TS_ASSERT(m.match("FOO."));
        TS_ASSERT(m.match("foobar.blub"));
        TS_ASSERT(m.match("foo.bar"));
        TS_ASSERT(m.hasWildcard());
        TS_ASSERT(!m.getFileName(tmp));
        TS_ASSERT(!m.empty());
    }

    {
        util::FileNamePattern m("*************************");
        TS_ASSERT(m.match(""));
        TS_ASSERT(m.match("xxxxxxxxxxxxxxxxxxxxxxxxxxx"));
        TS_ASSERT(m.hasWildcard());
        TS_ASSERT(!m.empty());
    }

    {
        util::FileNamePattern m("*************************x");
        TS_ASSERT(!m.match(""));
        TS_ASSERT(m.match("xxxxxxxxxxxxxxxxxxxxxxxxxxx"));
        /* The following will take a lot of time if we don't
           optimise multiple '*' */
        TS_ASSERT(!m.match("yyyyyyyyyyyyyyyyyyyyyyyyyyy"));
        TS_ASSERT(m.hasWildcard());
    }

    {
        util::FileNamePattern m("*");
        TS_ASSERT(m.match(""));
        TS_ASSERT(m.match("a"));
        TS_ASSERT(m.match("aaaaaaaaaaa"));
        TS_ASSERT(m.match("*****"));
        TS_ASSERT(m.hasWildcard());
    }

    {
        util::FileNamePattern m("*a");
        TS_ASSERT(!m.match(""));
        TS_ASSERT(m.match("a"));
        TS_ASSERT(m.match("aaaaaaaaaaa"));
        TS_ASSERT(!m.match("*****"));
        TS_ASSERT(m.hasWildcard());
    }

    {
        util::FileNamePattern m("*?*?*?*?*?*?*?*?*?*?*?*?*?*");
        TS_ASSERT(!m.match(""));
        TS_ASSERT(!m.match("123456789ABC"));
        TS_ASSERT(m.match("123456789ABCD"));
        TS_ASSERT(m.match("123456789ABCDEFG"));
        TS_ASSERT(m.hasWildcard());
    }

    {
        util::FileNamePattern m("*?*?*?*?*?*?*?*?*?*?*?*?*?*x");
        TS_ASSERT(!m.match(""));
        TS_ASSERT(!m.match("123456789ABC"));
        TS_ASSERT(!m.match("123456789ABCD"));
        TS_ASSERT(!m.match("123456789ABCDEFG"));
        TS_ASSERT(!m.match("123456789ABCx"));
        TS_ASSERT(m.match("123456789ABCDx"));
        TS_ASSERT(m.match("123456789ABCDEFGx"));
        TS_ASSERT(m.hasWildcard());
    }

    {
        util::FileNamePattern m("*\\**");
        TS_ASSERT(!m.match("foo"));
        TS_ASSERT(!m.match(""));
        TS_ASSERT(m.match("foo*bar"));
        TS_ASSERT(m.hasWildcard());
    }

    {
        util::FileNamePattern m("a?b");
        TS_ASSERT(m.match("axb"));
        TS_ASSERT(!m.match("ab"));
        TS_ASSERT(!m.match("abc"));
        TS_ASSERT(!m.match("axbc"));
        TS_ASSERT(!m.match("axxb"));

        TS_ASSERT(m.match("a\xc2\x80""b"));
        TS_ASSERT(!m.match("ab"));
        TS_ASSERT(!m.match("abc"));
        TS_ASSERT(!m.match("a\xc2\x80""bc"));
        TS_ASSERT(!m.match("a\xc2\x80\xc2\x80""b"));
        TS_ASSERT(m.hasWildcard());
    }

    {
        util::FileNamePattern m("a??b");
        TS_ASSERT(!m.match("axb"));
        TS_ASSERT(!m.match("ab"));
        TS_ASSERT(!m.match("abc"));
        TS_ASSERT(!m.match("axbc"));
        TS_ASSERT(m.match("axxb"));

        TS_ASSERT(!m.match("a\xc2\x80""b"));
        TS_ASSERT(!m.match("ab"));
        TS_ASSERT(!m.match("abc"));
        TS_ASSERT(!m.match("a\xc2\x80""bc"));
        TS_ASSERT(m.match("a\xc2\x80\xc2\x80""b"));
        TS_ASSERT(m.hasWildcard());
    }
    {
        String_t s = "hu?";
        util::FileNamePattern m("");
        TS_ASSERT(m.empty());
        TS_ASSERT(m.getFileName(s));
        TS_ASSERT(!m.hasWildcard());
        TS_ASSERT_EQUALS(s, "");
    }
}

/** Test failure (exception) cases. */
void
TestUtilFileNamePattern::testFail()
{
    {
        util::FileNamePattern testee;
        TS_ASSERT_THROWS(testee.setPattern("\\"), std::runtime_error);
        TS_ASSERT_THROWS(testee.setPattern("a\\"), std::runtime_error);
    }
    {
        TS_ASSERT_THROWS(util::FileNamePattern("a\\"), std::runtime_error);
    }
}

/** Test literal patterns. */
void
TestUtilFileNamePattern::testLiterals()
{
    String_t tmp;
    {
        util::FileNamePattern m("foo");
        TS_ASSERT(!m.match("fo"));
        TS_ASSERT( m.match("foo"));
        TS_ASSERT(!m.match("fooo"));
        TS_ASSERT(!m.hasWildcard());
        TS_ASSERT(m.getFileName(tmp));
        TS_ASSERT_EQUALS(tmp, "foo");
    }
    {
        util::FileNamePattern m("a\\*b");
        TS_ASSERT(m.match("a*b"));
        TS_ASSERT(!m.hasWildcard());
        TS_ASSERT(m.getFileName(tmp));
        TS_ASSERT_EQUALS(tmp, "a*b");
    }
    {
        util::FileNamePattern m("a\\?b");
        TS_ASSERT(m.match("a?b"));
        TS_ASSERT(!m.hasWildcard());
        TS_ASSERT(m.getFileName(tmp));
        TS_ASSERT_EQUALS(tmp, "a?b");
    }
}

/** Test copying patterns. */
void
TestUtilFileNamePattern::testCopy()
{
    util::FileNamePattern orig("foo");
    util::FileNamePattern copy(orig);
    TS_ASSERT(orig.match("foo"));
    TS_ASSERT(copy.match("foo"));

    orig.setPattern("bar");
    TS_ASSERT(orig.match("bar"));
    TS_ASSERT(copy.match("foo"));

    copy = orig;
    TS_ASSERT(orig.match("bar"));
    TS_ASSERT(copy.match("bar"));
}

/** Test prepared patterns. */
void
TestUtilFileNamePattern::testPrepared()
{
    {
        util::FileNamePattern t(util::FileNamePattern::getAllFilesPattern());
        TS_ASSERT(t.match(""));
        TS_ASSERT(t.match("a"));
        TS_ASSERT(t.match("aaaaaa"));
        TS_ASSERT(t.match("a*a"));
    }
    {
        util::FileNamePattern t(util::FileNamePattern::getSingleFilePattern("abc"));
        TS_ASSERT(!t.match(""));
        TS_ASSERT(!t.match("a"));
        TS_ASSERT( t.match("abc"));
        TS_ASSERT(!t.match("a*c"));
        TS_ASSERT(!t.match("abcde"));
    }
    {
        util::FileNamePattern t(util::FileNamePattern::getSingleFilePattern("a*c"));
        TS_ASSERT(!t.match(""));
        TS_ASSERT(!t.match("a"));
        TS_ASSERT(!t.match("abc"));
        TS_ASSERT( t.match("a*c"));
        TS_ASSERT(!t.match("abbc"));
    }
    {
        util::FileNamePattern t(util::FileNamePattern::getAllFilesWithExtensionPattern("qc"));
        TS_ASSERT(!t.match("qc"));
        TS_ASSERT( t.match(".qc"));
        TS_ASSERT( t.match("f.qc"));
        TS_ASSERT( t.match("blaa.qc"));
        TS_ASSERT(!t.match("x.qcc"));
    }
    {
        util::FileNamePattern t(util::FileNamePattern::getAllFilesWithExtensionPattern("q*"));
        TS_ASSERT(!t.match("qc"));
        TS_ASSERT(!t.match(".qc"));
        TS_ASSERT( t.match(".q*"));
        TS_ASSERT(!t.match("f.qc"));
        TS_ASSERT( t.match("f.q*"));
    }
}
