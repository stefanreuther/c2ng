/**
  *  \file u/t_util_messagematcher.cpp
  *  \brief Test for util::MessageMatcher
  */

#include <stdexcept>
#include "util/messagematcher.hpp"

#include "t_util.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/loglistener.hpp"

/** Test error cases. */
void
TestUtilMessageMatcher::testErrors()
{
    util::MessageMatcher t;
    afl::string::NullTranslator tx;

    // Missing anything
    TS_ASSERT_THROWS(t.setConfiguration("x", tx), std::runtime_error);

    // Missing log level
    TS_ASSERT_THROWS(t.setConfiguration("x:=", tx), std::runtime_error);
    TS_ASSERT_THROWS(t.setConfiguration("x@=", tx), std::runtime_error);

    // Missing action
    TS_ASSERT_THROWS(t.setConfiguration("x@info:y@info=a", tx), std::runtime_error);

    // Misplaced backslash
    TS_ASSERT_THROWS(t.setConfiguration("\\=foo", tx), std::runtime_error);
}

/** Test matches. */
void
TestUtilMessageMatcher::testMatch()
{
    const afl::sys::LogListener::Message warn  = { afl::sys::Time(), afl::sys::LogListener::Warn, "aha", "msg" };
    const afl::sys::LogListener::Message trace = { afl::sys::Time(), afl::sys::LogListener::Trace, "aha", "msg" };
    const afl::sys::LogListener::Message debug = { afl::sys::Time(), afl::sys::LogListener::Debug, "aha", "msg" };
    afl::string::NullTranslator tx;

    // Match direct level
    {
        util::MessageMatcher t;
        String_t r;
        t.setConfiguration("a*@Warn=ok", tx);
        TS_ASSERT(t.match(warn, r));
        TS_ASSERT_EQUALS(r, "ok");
    }

    // Match level and below
    {
        util::MessageMatcher t;
        String_t r;
        t.setConfiguration("a*@-Warn=ok", tx);
        TS_ASSERT(t.match(warn, r));
        TS_ASSERT_EQUALS(r, "ok");
    }

    // Match level and up
    {
        util::MessageMatcher t;
        String_t r;
        t.setConfiguration("a*@Warn+=ok", tx);
        TS_ASSERT(t.match(warn, r));
        TS_ASSERT_EQUALS(r, "ok");
    }

    // Match direct level
    {
        util::MessageMatcher t;
        String_t r;
        t.setConfiguration("a*@Info=ok", tx);
        TS_ASSERT(!t.match(warn, r));
    }

    // Match level and below
    {
        util::MessageMatcher t;
        String_t r;
        t.setConfiguration("a*@-Info=ok", tx);
        TS_ASSERT(!t.match(warn, r));
    }

    // Match level and up
    {
        util::MessageMatcher t;
        String_t r;
        t.setConfiguration("a*@Info+=ok", tx);
        TS_ASSERT(t.match(warn, r));
        TS_ASSERT_EQUALS(r, "ok");
    }

    // Multiple expressions
    {
        util::MessageMatcher t;
        String_t r;
        t.setConfiguration("b@Trace=x:a@Warn=y:a*@Info+=z", tx);
        TS_ASSERT(t.match(warn, r));
        TS_ASSERT_EQUALS(r, "z");
    }

    // No level limit
    {
        util::MessageMatcher t;
        String_t r;
        t.setConfiguration("a*=hi", tx);
        TS_ASSERT(t.match(warn, r));
        TS_ASSERT_EQUALS(r, "hi");
    }

    // Multiple expressions, example from docs
    {
        util::MessageMatcher t;
        String_t r;
        t.setConfiguration("*@Info+=show:*@Trace=drop:*=hide", tx);
        TS_ASSERT(t.match(warn, r));
        TS_ASSERT_EQUALS(r, "show");
        TS_ASSERT(t.match(trace, r));
        TS_ASSERT_EQUALS(r, "drop");
        TS_ASSERT(t.match(debug, r));
        TS_ASSERT_EQUALS(r, "hide");
    }

    // Empty result
    {
        util::MessageMatcher t;
        String_t r;
        t.setConfiguration("a*=:aha=foo", tx);
        TS_ASSERT(t.match(warn, r));
        TS_ASSERT_EQUALS(r, "");
    }
}
