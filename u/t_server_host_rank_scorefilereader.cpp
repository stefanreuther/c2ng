/**
  *  \file u/t_server_host_rank_scorefilereader.cpp
  *  \brief Test for server::host::rank::ScoreFileReader
  */

#include "server/host/rank/scorefilereader.hpp"

#include "t_server_host_rank.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/io/constmemorystream.hpp"

/** Test packScore(). */
void
TestServerHostRankScoreFileReader::testPackScore()
{
    server::host::rank::Score_t score = { 1, 2, 3, 4, 5, 6, 7, 8, 256, 65536, 16777216 };
    String_t packed = server::host::rank::packScore(score);

    static const uint8_t expected[44] = {
        1,0,0,0, 2,0,0,0, 3,0,0,0, 4,0,0,0, 5,0,0,0, 6,0,0,0, 7,0,0,0, 8,0,0,0,
        0,1,0,0, 0,0,1,0, 0,0,0,1
    };

    TS_ASSERT_EQUALS(packed.size(), 44U);
    TS_ASSERT_SAME_DATA(packed.data(), expected, sizeof(expected));
}

/** Test ScoreFileReader::handleLine(). */
void
TestServerHostRankScoreFileReader::testParse()
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
    TS_ASSERT_EQUALS(HashKey(db, "desc").stringField("foo").get(), "foo score");
    TS_ASSERT_EQUALS(HashKey(db, "desc").stringField("bar").get(), "bar score");

    String_t foo = HashKey(db, "score").stringField("foo").get();
    String_t bar = HashKey(db, "score").stringField("bar").get();
    TS_ASSERT_EQUALS(foo.size(), 44U);
    TS_ASSERT_EQUALS(bar.size(), 44U);

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
    TS_ASSERT_SAME_DATA(foo.data(), fooExpected, 44);
    TS_ASSERT_SAME_DATA(bar.data(), barExpected, 44);
}

/** Test ScoreFileReader::parseFile().
    This tests some border cases. */
void
TestServerHostRankScoreFileReader::testFile()
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
    TS_ASSERT_EQUALS(HashKey(db, "desc").stringField("new").get(), "");  // no description given!
    TS_ASSERT_EQUALS(HashKey(db, "score").stringField("new").get().substr(0, 8), String_t("\5\0\0\0\3\0\0\0", 8));
    TS_ASSERT_EQUALS(HashKey(db, "desc").size(), 1);
    TS_ASSERT_EQUALS(HashKey(db, "score").size(), 1);
}

