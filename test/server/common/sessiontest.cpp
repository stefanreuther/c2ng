/**
  *  \file test/server/common/sessiontest.cpp
  *  \brief Test for server::common::Session
  */

#include "server/common/session.hpp"
#include "afl/test/testrunner.hpp"

using server::common::Session;

/** Simple test. */
AFL_TEST("server.common.Session:basics", a)
{
    Session testee;

    // Test initial state
    a.check           ("01. isAdmin",       testee.isAdmin());
    a.checkEqual      ("02. getUser",       testee.getUser(), "");
    AFL_CHECK_THROWS  (a("03. checkUser"),  testee.checkUser(), std::exception);
    AFL_CHECK_SUCCEEDS(a("04. checkAdmin"), testee.checkAdmin());

    // Test user
    testee.setUser("1003");
    a.check           ("11. isAdmin",      !testee.isAdmin());
    a.checkEqual      ("12. getUser",       testee.getUser(), "1003");
    AFL_CHECK_SUCCEEDS(a("13. checkUser"),  testee.checkUser());
    AFL_CHECK_THROWS  (a("14. checkAdmin"), testee.checkAdmin(), std::exception);
    a.checkEqual      ("15. option",        testee.checkUserOption(String_t("1003")), "1003");
    a.checkEqual      ("16. option",        testee.checkUserOption(afl::base::Nothing), "1003");
    AFL_CHECK_THROWS  (a("17. option"),     testee.checkUserOption(String_t("1004")), std::exception);
    AFL_CHECK_THROWS  (a("18. option"),     testee.checkUserOption(String_t("")), std::exception);

    // Reset to admin
    testee.setUser("");
    a.check           ("21. isAdmin",       testee.isAdmin());
    a.checkEqual      ("22. getUser",       testee.getUser(), "");
    AFL_CHECK_THROWS  (a("23. checkUser"),  testee.checkUser(), std::exception);
    AFL_CHECK_SUCCEEDS(a("24. checkAdmin"), testee.checkAdmin());
    a.checkEqual      ("25. option",        testee.checkUserOption(String_t("1003")), "1003");
    AFL_CHECK_THROWS  (a("26. option"),     testee.checkUserOption(afl::base::Nothing), std::exception);
    a.checkEqual      ("27. option",        testee.checkUserOption(String_t("1004")), "1004");
    AFL_CHECK_THROWS  (a("28. option"),     testee.checkUserOption(String_t("")), std::exception);
}

/** Test formatWord(). */
AFL_TEST("server.common.Session:formatWord", a)
{
    // Empty
    a.checkEqual("01", Session::formatWord("", false), "''");
    a.checkEqual("02", Session::formatWord("", true), "''");

    // Placeholder trigger
    // - spaces
    a.checkEqual("11", Session::formatWord(" ", false), "...");
    // - special characters
    a.checkEqual("12", Session::formatWord("[foo]", false), "...");
    a.checkEqual("13", Session::formatWord("a\nb", false), "...");
    a.checkEqual("14", Session::formatWord("''", false), "...");
    // - too long
    a.checkEqual("15", Session::formatWord(String_t(200, 'x'), false), "...");

    // Censor
    a.checkEqual("21", Session::formatWord("x", true), "...");

    // Normal: we want to pass...
    // - normal words
    a.checkEqual("31", Session::formatWord("x", false), "x");
    a.checkEqual("32", Session::formatWord("x_y", false), "x_y");
    // - file names
    a.checkEqual("33", Session::formatWord("a/b/c.dat", false), "a/b/c.dat");
    // - permission strings
    a.checkEqual("34", Session::formatWord("g:1,g:2", false), "g:1,g:2");
    a.checkEqual("35", Session::formatWord("-all", false), "-all");
    // - wildcards
    a.checkEqual("36", Session::formatWord("xy*", false), "xy*");
}
