/**
  *  \file test/util/messagematchertest.cpp
  *  \brief Test for util::MessageMatcher
  */

#include "util/messagematcher.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/sys/loglistener.hpp"
#include "afl/test/testrunner.hpp"
#include <stdexcept>

/** Test error cases. */
AFL_TEST("util.MessageMatcher:errors", a)
{
    util::MessageMatcher t;
    afl::string::NullTranslator tx;

    // Missing anything
    AFL_CHECK_THROWS(a("01. missing config"), t.setConfiguration("x", tx), std::runtime_error);

    // Missing log level
    AFL_CHECK_THROWS(a("11. missing level"), t.setConfiguration("x:=", tx), std::runtime_error);
    AFL_CHECK_THROWS(a("12. missing level"), t.setConfiguration("x@=", tx), std::runtime_error);

    // Missing action
    AFL_CHECK_THROWS(a("21. missing action"), t.setConfiguration("x@info:y@info=a", tx), std::runtime_error);

    // Misplaced backslash
    AFL_CHECK_THROWS(a("31. backslash"), t.setConfiguration("\\=foo", tx), std::runtime_error);
}

/** Test matches. */
AFL_TEST("util.MessageMatcher:match", a)
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
        a.check("01", t.match(warn, r));
        a.checkEqual("02", r, "ok");
    }

    // Match level and below
    {
        util::MessageMatcher t;
        String_t r;
        t.setConfiguration("a*@-Warn=ok", tx);
        a.check("11", t.match(warn, r));
        a.checkEqual("12", r, "ok");
    }

    // Match level and up
    {
        util::MessageMatcher t;
        String_t r;
        t.setConfiguration("a*@Warn+=ok", tx);
        a.check("21", t.match(warn, r));
        a.checkEqual("22", r, "ok");
    }

    // Match direct level
    {
        util::MessageMatcher t;
        String_t r;
        t.setConfiguration("a*@Info=ok", tx);
        a.check("31", !t.match(warn, r));
    }

    // Match level and below
    {
        util::MessageMatcher t;
        String_t r;
        t.setConfiguration("a*@-Info=ok", tx);
        a.check("41", !t.match(warn, r));
    }

    // Match level and up
    {
        util::MessageMatcher t;
        String_t r;
        t.setConfiguration("a*@Info+=ok", tx);
        a.check("51", t.match(warn, r));
        a.checkEqual("52", r, "ok");
    }

    // Multiple expressions
    {
        util::MessageMatcher t;
        String_t r;
        t.setConfiguration("b@Trace=x:a@Warn=y:a*@Info+=z", tx);
        a.check("61", t.match(warn, r));
        a.checkEqual("62", r, "z");
    }

    // No level limit
    {
        util::MessageMatcher t;
        String_t r;
        t.setConfiguration("a*=hi", tx);
        a.check("71", t.match(warn, r));
        a.checkEqual("72", r, "hi");
    }

    // Multiple expressions, example from docs
    {
        util::MessageMatcher t;
        String_t r;
        t.setConfiguration("*@Info+=show:*@Trace=drop:*=hide", tx);
        a.check("81", t.match(warn, r));
        a.checkEqual("82", r, "show");
        a.check("83", t.match(trace, r));
        a.checkEqual("84", r, "drop");
        a.check("85", t.match(debug, r));
        a.checkEqual("86", r, "hide");
    }

    // Empty result
    {
        util::MessageMatcher t;
        String_t r;
        t.setConfiguration("a*=:aha=foo", tx);
        a.check("91", t.match(warn, r));
        a.checkEqual("92", r, "");
    }
}
