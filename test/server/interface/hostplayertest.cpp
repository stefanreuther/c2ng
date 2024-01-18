/**
  *  \file test/server/interface/hostplayertest.cpp
  *  \brief Test for server::interface::HostPlayer
  */

#include "server/interface/hostplayer.hpp"
#include "afl/test/testrunner.hpp"

using server::interface::HostPlayer;

/** Interface test. */
AFL_TEST_NOARG("server.interface.HostPlayer:interface")
{
    class Tester : public HostPlayer {
     public:
        virtual void join(int32_t /*gameId*/, int32_t /*slot*/, String_t /*userId*/)
            { }
        virtual void substitute(int32_t /*gameId*/, int32_t /*slot*/, String_t /*userId*/)
            { }
        virtual void resign(int32_t /*gameId*/, int32_t /*slot*/, String_t /*userId*/)
            { }
        virtual void add(int32_t /*gameId*/, String_t /*userId*/)
            { }
        virtual void list(int32_t /*gameId*/, bool /*all*/, std::map<int,Info>& /*result*/)
            { }
        virtual Info getInfo(int32_t /*gameId*/, int32_t /*slot*/)
            { return Info(); }
        virtual void setDirectory(int32_t /*gameId*/, String_t /*userId*/, String_t /*dirName*/)
            { }
        virtual String_t getDirectory(int32_t /*gameId*/, String_t /*userId*/)
            { return String_t(); }
        virtual FileStatus checkFile(int32_t /*gameId*/, String_t /*userId*/, String_t /*fileName*/, afl::base::Optional<String_t> /*dirName*/)
            { return FileStatus(); }
        virtual void set(int32_t /*gameId*/, String_t /*userId*/, String_t /*key*/, String_t /*value*/)
            { }
        virtual String_t get(int32_t /*gameId*/, String_t /*userId*/, String_t /*key*/)
            { return String_t(); }
    };
    Tester t;
}

/** Test format functions. */
AFL_TEST("server.interface.HostPlayer:formatFileStatus", a)
{
    a.checkEqual("01", HostPlayer::formatFileStatus(HostPlayer::Stale),  "stale");
    a.checkEqual("02", HostPlayer::formatFileStatus(HostPlayer::Allow),  "allow");
    a.checkEqual("03", HostPlayer::formatFileStatus(HostPlayer::Turn),   "trn");
    a.checkEqual("04", HostPlayer::formatFileStatus(HostPlayer::Refuse), "refuse");
}

/** Test parse functions. */
AFL_TEST("server.interface.HostPlayer:parseFileStatus", a)
{
    HostPlayer::FileStatus fs;
    a.check("01", HostPlayer::parseFileStatus("stale", fs));
    a.checkEqual("02", fs, HostPlayer::Stale);
    a.check("03", HostPlayer::parseFileStatus("allow", fs));
    a.checkEqual("04", fs, HostPlayer::Allow);
    a.check("05", HostPlayer::parseFileStatus("trn", fs));
    a.checkEqual("06", fs, HostPlayer::Turn);
    a.check("07", HostPlayer::parseFileStatus("refuse", fs));
    a.checkEqual("08", fs, HostPlayer::Refuse);

    a.check("11", !HostPlayer::parseFileStatus("TRN", fs));
    a.check("12", !HostPlayer::parseFileStatus("turn", fs));
    a.check("13", !HostPlayer::parseFileStatus("t", fs));
    a.check("14", !HostPlayer::parseFileStatus("", fs));
}

/** Test initialisation. */
AFL_TEST("server.interface.HostPlayer:init:Info", a)
{
    HostPlayer::Info i;
    a.checkEqual("01. longName",      i.longName, "");
    a.checkEqual("02. shortName",     i.shortName, "");
    a.checkEqual("03. adjectiveName", i.adjectiveName, "");
    a.checkEqual("04. userIds",       i.userIds.size(), 0U);
    a.checkEqual("05. numEditable",   i.numEditable, 0);
    a.checkEqual("06. joinable",      i.joinable, false);
}
