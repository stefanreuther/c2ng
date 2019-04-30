/**
  *  \file u/t_server_common_randomidgenerator.cpp
  *  \brief Test for server::common::RandomIdGenerator
  */

#include "server/common/randomidgenerator.hpp"

#include "t_server_common.hpp"
#include "afl/io/nullfilesystem.hpp"

/** Simple test.
    Even without a file system, we need to be able to construct a RandomIdGenerator
    and obtain Ids of a usable quality. */
void
TestServerCommonRandomIdGenerator::testIt()
{
    afl::io::NullFileSystem fs;
    server::common::RandomIdGenerator testee(fs);

    String_t a = testee.createId();
    String_t b = testee.createId();
    TS_ASSERT_LESS_THAN(15U, a.size());
    TS_ASSERT_LESS_THAN(15U, b.size());
    TS_ASSERT_DIFFERS(a, b);
}

