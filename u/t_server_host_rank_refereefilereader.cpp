/**
  *  \file u/t_server_host_rank_refereefilereader.cpp
  *  \brief Test for server::host::rank::RefereeFileReader
  */

#include "server/host/rank/refereefilereader.hpp"

#include "t_server_host_rank.hpp"
#include "afl/io/constmemorystream.hpp"

/** Test line processing. */
void
TestServerHostRankRefereeFileReader::testIt()
{
    // Referee file
    server::host::rank::RefereeFileReader rdr;
    TS_ASSERT(!rdr.isEnd());

    rdr.handleLine("<fn>", 0, "rank1=99");
    rdr.handleLine("<fn>", 0, "rank2=98");
    rdr.handleLine("<fn>", 0, "rank3=97");
    rdr.handleLine("<fn>", 0, "rank4=96");
    rdr.handleLine("<fn>", 0, " RANK5=95");
    rdr.handleLine("<fn>", 0, "Rank6 =94");
    rdr.handleLine("<fn>", 0, "rank7= 93");
    rdr.handleLine("<fn>", 0, "RANK8=92");
    rdr.handleLine("<fn>", 0, "rank9=91");
    rdr.handleLine("<fn>", 0, "rank10=90");
    rdr.handleLine("<fn>", 0, "rank11=89");

    rdr.handleLine("<fn>", 0, "rank0=42");
    TS_ASSERT_EQUALS(rdr.getRanks()[0], 99);
    TS_ASSERT_EQUALS(rdr.getRanks()[1], 98);
    TS_ASSERT_EQUALS(rdr.getRanks()[2], 97);
    TS_ASSERT_EQUALS(rdr.getRanks()[3], 96);
    TS_ASSERT_EQUALS(rdr.getRanks()[4], 95);
    TS_ASSERT_EQUALS(rdr.getRanks()[5], 94);
    TS_ASSERT_EQUALS(rdr.getRanks()[6], 93);
    TS_ASSERT_EQUALS(rdr.getRanks()[7], 92);
    TS_ASSERT_EQUALS(rdr.getRanks()[8], 91);
    TS_ASSERT_EQUALS(rdr.getRanks()[9], 90);
    TS_ASSERT_EQUALS(rdr.getRanks()[10], 89);
    TS_ASSERT(!rdr.isEnd());

    rdr.handleLine("<fn>", 0, "end=1");
    TS_ASSERT(rdr.isEnd());
}

/** Test reading a file.
    This tests the border cases. */
void
TestServerHostRankRefereeFileReader::testFile()
{
    afl::io::ConstMemoryStream ms(afl::string::toBytes("# comment\n"
                                                       "   \n"
                                                       "rank1   =   7\n"
                                                       "end\n"               // invalid line, no '='
                                                       "; rank1 = 2\n"));    // comment
    server::host::rank::RefereeFileReader rdr;
    rdr.parseFile(ms);
    TS_ASSERT_EQUALS(rdr.getRanks()[0], 7);
    TS_ASSERT_EQUALS(rdr.isEnd(), false);
}

