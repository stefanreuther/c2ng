/**
  *  \file test/server/host/rank/scorefilereadertest.cpp
  *  \brief Test for server::host::rank::ScoreFileReader
  */

#include "server/host/rank/scorefilereader.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/test/testrunner.hpp"

/** Test packScore(). */
AFL_TEST("server.host.rank.ScoreFileReader:packScore", a)
{
    server::host::rank::Score_t score = { 1, 2, 3, 4, 5, 6, 7, 8, 256, 65536, 16777216 };
    String_t packed = server::host::rank::packScore(score);

    static const uint8_t expected[44] = {
        1,0,0,0, 2,0,0,0, 3,0,0,0, 4,0,0,0, 5,0,0,0, 6,0,0,0, 7,0,0,0, 8,0,0,0,
        0,1,0,0, 0,0,1,0, 0,0,0,1
    };

    a.checkEqualContent<uint8_t>("packed", afl::string::toBytes(packed), expected);
}

/** Test ScoreFileReader::handleLine(). */
AFL_TEST("server.host.rank.ScoreFileReader:handleLine", a)
{
    using afl::net::redis::HashKey;
    afl::net::redis::InternalDatabase db;

    // Process a file
    server::host::rank::ScoreFileReader testee(HashKey(db, "score"), HashKey(db, "desc"));
    testee.handleLine("", 0, "score1=1");
    testee.handleLine("", 0, "%foo");
    testee.handleLine("", 0, "score1=2");
    testee.handleLine("", 0, " score2=3");
    testee.handleLine("", 0, "score3 =4");
    testee.handleLine("", 0, "score4= 5");
    testee.handleLine("", 0, "description=foo score");
    testee.handleLine("", 0, "");
    testee.handleLine("", 0, "%bar");
    testee.handleLine("", 0, "score4=7");
    testee.handleLine("", 0, "description=bar score");
    testee.flush();

    // Verify
    a.checkEqual("01. db desc", HashKey(db, "desc").stringField("foo").get(), "foo score");
    a.checkEqual("02. db desc", HashKey(db, "desc").stringField("bar").get(), "bar score");
    a.checkEqual("03. db desc", HashKey(db, "desc").size(), 2);

    String_t foo = HashKey(db, "score").stringField("foo").get();
    String_t bar = HashKey(db, "score").stringField("bar").get();
    a.checkEqual("11. size", foo.size(), 44U);
    a.checkEqual("12. size", bar.size(), 44U);

    static const uint8_t fooExpected[44] = {
        2,0,0,0, 3,0,0,0, 4,0,0,0, 5,0,0,0,
        255,255,255,255, 255,255,255,255, 255,255,255,255, 255,255,255,255,
        255,255,255,255, 255,255,255,255, 255,255,255,255
    };
    static const uint8_t barExpected[44] = {
        255,255,255,255, 255,255,255,255, 255,255,255,255, 7,0,0,0,
        255,255,255,255, 255,255,255,255, 255,255,255,255, 255,255,255,255,
        255,255,255,255, 255,255,255,255, 255,255,255,255
    };
    a.checkEqualContent<uint8_t>("21. foo", afl::string::toBytes(foo), fooExpected);
    a.checkEqualContent<uint8_t>("21. bar", afl::string::toBytes(bar), barExpected);
}

/** Test ScoreFileReader::parseFile().
    This tests some border cases. */
AFL_TEST("server.host.rank.ScoreFileReader:parseFile", a)
{
    using afl::net::redis::HashKey;
    afl::net::redis::InternalDatabase db;

    afl::io::ConstMemoryStream ms(afl::string::toBytes("# scores\n"
                                                       "\n"
                                                       "score1=9\n"      // ignored, no section defined
                                                       "%new\n"
                                                       "score1=5\n"
                                                       "score1\n"
                                                       "; comment\n"
                                                       "score2=3"));

    // Process a file
    server::host::rank::ScoreFileReader testee(HashKey(db, "score"), HashKey(db, "desc"));
    testee.parseFile(ms);
    testee.flush();          // required!

    // Verify
    a.checkEqual("01. db desc",  HashKey(db, "desc").stringField("new").get(), "");  // no description given!
    a.checkEqual("02. db score", HashKey(db, "score").stringField("new").get().substr(0, 8), String_t("\5\0\0\0\3\0\0\0", 8));
    a.checkEqual("03. db desc",  HashKey(db, "desc").size(), 1);
    a.checkEqual("04. db score", HashKey(db, "score").size(), 1);
}
