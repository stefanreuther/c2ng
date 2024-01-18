/**
  *  \file test/server/talk/talksyntaxtest.cpp
  *  \brief Test for server::talk::TalkSyntax
  */

#include "server/talk/talksyntax.hpp"

#include "afl/data/vector.hpp"
#include "afl/test/testrunner.hpp"
#include "server/types.hpp"
#include <stdexcept>

/** Test TalkSyntax implementation. */
AFL_TEST("server.talk.TalkSyntax", a)
{
    util::syntax::KeywordTable table;
    table.add("k", "v");

    server::talk::TalkSyntax testee(table);
    a.checkEqual("01. get", testee.get("k"), "v");
    a.checkEqual("02. get", testee.get("K"), "v");
    AFL_CHECK_THROWS(a("03. get"), testee.get("x"), std::exception);

    String_t ks[] = { "j", "k", "l" };
    afl::data::Vector::Ref_t result = testee.mget(ks);

    a.checkEqual  ("11. size", result->size(), 3U);
    a.checkNull   ("12. result", (*result)[0]);
    a.checkNonNull("13. result", (*result)[1]);
    a.checkNull   ("14. result", (*result)[2]);
    a.checkEqual  ("15. result", server::toString((*result)[1]), "v");
}
