/**
  *  \file test/server/host/rank/refereefilereadertest.cpp
  *  \brief Test for server::host::rank::RefereeFileReader
  */

#include "server/host/rank/refereefilereader.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/test/testrunner.hpp"

/** Test line processing. */
AFL_TEST("server.host.rank.RefereeFileReader:handleLine", a)
{
    // Referee file
    server::host::rank::RefereeFileReader rdr;
    a.check("01. isEnd", !rdr.isEnd());

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
    a.checkEqual("11", rdr.getRanks()[0], 99);
    a.checkEqual("12", rdr.getRanks()[1], 98);
    a.checkEqual("13", rdr.getRanks()[2], 97);
    a.checkEqual("14", rdr.getRanks()[3], 96);
    a.checkEqual("15", rdr.getRanks()[4], 95);
    a.checkEqual("16", rdr.getRanks()[5], 94);
    a.checkEqual("17", rdr.getRanks()[6], 93);
    a.checkEqual("18", rdr.getRanks()[7], 92);
    a.checkEqual("19", rdr.getRanks()[8], 91);
    a.checkEqual("20", rdr.getRanks()[9], 90);
    a.checkEqual("21", rdr.getRanks()[10], 89);
    a.check("22. isEnd", !rdr.isEnd());

    rdr.handleLine("<fn>", 0, "end=1");
    a.check("31. isEnd", rdr.isEnd());
}

/** Test reading a file.
    This tests the border cases. */
AFL_TEST("server.host.rank.RefereeFileReader:border-cases", a)
{
    afl::io::ConstMemoryStream ms(afl::string::toBytes("# comment\n"
                                                       "   \n"
                                                       "rank1   =   7\n"
                                                       "end\n"               // invalid line, no '='
                                                       "; rank1 = 2\n"));    // comment
    server::host::rank::RefereeFileReader rdr;
    rdr.parseFile(ms);
    a.checkEqual("01. getRanks", rdr.getRanks()[0], 7);
    a.checkEqual("02. isEnd", rdr.isEnd(), false);
}
