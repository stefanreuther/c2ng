/**
  *  \file test/server/common/randomidgeneratortest.cpp
  *  \brief Test for server::common::RandomIdGenerator
  */

#include "server/common/randomidgenerator.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/test/testrunner.hpp"
#include <set>

/** Simple test.
    Even without a file system, we need to be able to construct a RandomIdGenerator
    and obtain Ids of a usable quality. */
AFL_TEST("server.common.RandomIdGenerator:basics", a)
{
    afl::io::NullFileSystem fs;
    server::common::RandomIdGenerator testee(fs);

    String_t sa = testee.createId();
    String_t sb = testee.createId();
    a.checkLessThan("01. min size", 15U, sa.size());
    a.checkLessThan("02. min size", 15U, sb.size());
    a.checkDifferent("03. different", sa, sb);
}

/** Test that we can generate many Ids. */
AFL_TEST("server.common.RandomIdGenerator:loop", a)
{
    afl::io::NullFileSystem fs;
    server::common::RandomIdGenerator testee(fs);

    std::set<String_t> set;
    for (size_t i = 0; i < 1000; ++i) {
        String_t id = testee.createId();
        a.check("01. unique", set.find(id) == set.end());
        set.insert(id);
    }
}
