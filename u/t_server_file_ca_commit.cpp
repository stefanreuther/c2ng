/**
  *  \file u/t_server_file_ca_commit.cpp
  *  \brief Test for server::file::ca::Commit
  */

#include "server/file/ca/commit.hpp"

#include "t_server_file_ca.hpp"

/** Test store(). */
void
TestServerFileCaCommit::testStore()
{
    // Create commit and verify it
    const server::file::ca::ObjectId id = {{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20}};
    server::file::ca::Commit testee(id);

    TS_ASSERT_EQUALS(testee.getTreeId(), id);

    // Store and verify result
    afl::base::GrowableMemory<uint8_t> out;
    testee.store(out);

    TS_ASSERT_EQUALS(afl::string::fromBytes(out.subrange(0, 46)), "tree 0102030405060708090a0b0c0d0e0f1011121314\n");
}

/** Test parse(). */
void
TestServerFileCaCommit::testParse()
{
    // Valid
    const server::file::ca::ObjectId id = {{49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,32,33,34,35,36}};
    {
        server::file::ca::Commit testee;
        TS_ASSERT(testee.parse(afl::string::toBytes("tree 3132333435363738393a3b3c3d3e3f2021222324\nauthor ....")));
        TS_ASSERT_EQUALS(testee.getTreeId(), id);
    }

    {
        server::file::ca::Commit testee;
        TS_ASSERT(testee.parse(afl::string::toBytes("tree 3132333435363738393a3b3c3d3e3f2021222324")));
        TS_ASSERT_EQUALS(testee.getTreeId(), id);
    }

    // Invalid
    {
        // - too short
        TS_ASSERT(!server::file::ca::Commit().parse(afl::string::toBytes("tree 3132333435363738393a3b3c3d3e3f202122232")));

        // - wrong case
        TS_ASSERT(!server::file::ca::Commit().parse(afl::string::toBytes("tree 3132333435363738393A3B3C3D3E3F2021222324")));

        // - totally not hex
        TS_ASSERT(!server::file::ca::Commit().parse(afl::string::toBytes("tree 3132333435363738393a3b3c3d3e3f3g3h3i3j3k")));

        // - bad header
        TS_ASSERT(!server::file::ca::Commit().parse(afl::string::toBytes("fork 3132333435363738393a3b3c3d3e3f2021222324")));

        // - totally too short
        TS_ASSERT(!server::file::ca::Commit().parse(afl::string::toBytes("tree ")));
        TS_ASSERT(!server::file::ca::Commit().parse(afl::string::toBytes("tree")));
        TS_ASSERT(!server::file::ca::Commit().parse(afl::string::toBytes("t")));
        TS_ASSERT(!server::file::ca::Commit().parse(afl::string::toBytes("")));
    }
}
