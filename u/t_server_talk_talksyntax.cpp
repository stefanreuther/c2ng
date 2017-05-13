/**
  *  \file u/t_server_talk_talksyntax.cpp
  *  \brief Test for server::talk::TalkSyntax
  */

#include <stdexcept>
#include "server/talk/talksyntax.hpp"

#include "t_server_talk.hpp"
#include "afl/data/vector.hpp"
#include "server/types.hpp"

/** Test TalkSyntax implementation. */
void
TestServerTalkTalkSyntax::testIt()
{
    util::syntax::KeywordTable table;
    table.add("k", "v");

    server::talk::TalkSyntax testee(table);
    TS_ASSERT_EQUALS(testee.get("k"), "v");
    TS_ASSERT_EQUALS(testee.get("K"), "v");
    TS_ASSERT_THROWS(testee.get("x"), std::exception);

    String_t ks[] = { "j", "k", "l" };
    afl::data::Vector::Ref_t result = testee.mget(ks);
    TS_ASSERT_EQUALS(result->size(), 3U);
    TS_ASSERT((*result)[0] == 0);
    TS_ASSERT((*result)[1] != 0);
    TS_ASSERT((*result)[2] == 0);
    TS_ASSERT_EQUALS(server::toString((*result)[1]), "v");
}

