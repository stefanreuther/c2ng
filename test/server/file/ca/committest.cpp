/**
  *  \file test/server/file/ca/committest.cpp
  *  \brief Test for server::file::ca::Commit
  */

#include "server/file/ca/commit.hpp"
#include "afl/test/testrunner.hpp"

/** Test store(). */
AFL_TEST("server.file.ca.Commit:store", a)
{
    // Create commit and verify it
    const server::file::ca::ObjectId id = {{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20}};
    server::file::ca::Commit testee(id);

    a.checkEqual("01. getTreeId", testee.getTreeId(), id);

    // Store and verify result
    afl::base::GrowableMemory<uint8_t> out;
    testee.store(out);

    a.checkEqual("11. content", afl::string::fromBytes(out.subrange(0, 46)), "tree 0102030405060708090a0b0c0d0e0f1011121314\n");
}

/** Test parse(). */
AFL_TEST("server.file.ca.Commit:parse", a)
{
    // Valid
    const server::file::ca::ObjectId id = {{49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,32,33,34,35,36}};
    {
        server::file::ca::Commit testee;
        a.check("01. parse", testee.parse(afl::string::toBytes("tree 3132333435363738393a3b3c3d3e3f2021222324\nauthor ....")));
        a.checkEqual("02. getTreeId", testee.getTreeId(), id);
    }

    {
        server::file::ca::Commit testee;
        a.check("11. parse", testee.parse(afl::string::toBytes("tree 3132333435363738393a3b3c3d3e3f2021222324")));
        a.checkEqual("12. getTreeId", testee.getTreeId(), id);
    }

    // Invalid
    {
        // - too short
        a.check("21. too short", !server::file::ca::Commit().parse(afl::string::toBytes("tree 3132333435363738393a3b3c3d3e3f202122232")));

        // - wrong case
        a.check("31. wrong case", !server::file::ca::Commit().parse(afl::string::toBytes("tree 3132333435363738393A3B3C3D3E3F2021222324")));

        // - totally not hex
        a.check("41. not hex", !server::file::ca::Commit().parse(afl::string::toBytes("tree 3132333435363738393a3b3c3d3e3f3g3h3i3j3k")));

        // - bad header
        a.check("51. header", !server::file::ca::Commit().parse(afl::string::toBytes("fork 3132333435363738393a3b3c3d3e3f2021222324")));

        // - totally too short
        a.check("61. too short", !server::file::ca::Commit().parse(afl::string::toBytes("tree ")));
        a.check("62. too short", !server::file::ca::Commit().parse(afl::string::toBytes("tree")));
        a.check("63. too short", !server::file::ca::Commit().parse(afl::string::toBytes("t")));
        a.check("64. too short", !server::file::ca::Commit().parse(afl::string::toBytes("")));
    }
}
