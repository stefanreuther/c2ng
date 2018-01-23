/**
  *  \file u/t_server_interface_hostplayer.cpp
  *  \brief Test for server::interface::HostPlayer
  */

#include "server/interface/hostplayer.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceHostPlayer::testInterface()
{
    class Tester : public server::interface::HostPlayer {
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
    };
    Tester t;
}

/** Test format functions. */
void
TestServerInterfaceHostPlayer::testFormat()
{
    using server::interface::HostPlayer;
    TS_ASSERT_EQUALS(HostPlayer::formatFileStatus(HostPlayer::Stale),  "stale");
    TS_ASSERT_EQUALS(HostPlayer::formatFileStatus(HostPlayer::Allow),  "allow");
    TS_ASSERT_EQUALS(HostPlayer::formatFileStatus(HostPlayer::Turn),   "trn");
    TS_ASSERT_EQUALS(HostPlayer::formatFileStatus(HostPlayer::Refuse), "refuse");
}

/** Test parse functions. */
void
TestServerInterfaceHostPlayer::testParse()
{
    using server::interface::HostPlayer;

    HostPlayer::FileStatus fs;
    TS_ASSERT(HostPlayer::parseFileStatus("stale", fs));
    TS_ASSERT_EQUALS(fs, HostPlayer::Stale);
    TS_ASSERT(HostPlayer::parseFileStatus("allow", fs));
    TS_ASSERT_EQUALS(fs, HostPlayer::Allow);
    TS_ASSERT(HostPlayer::parseFileStatus("trn", fs));
    TS_ASSERT_EQUALS(fs, HostPlayer::Turn);
    TS_ASSERT(HostPlayer::parseFileStatus("refuse", fs));
    TS_ASSERT_EQUALS(fs, HostPlayer::Refuse);

    TS_ASSERT(!HostPlayer::parseFileStatus("TRN", fs));
    TS_ASSERT(!HostPlayer::parseFileStatus("turn", fs));
    TS_ASSERT(!HostPlayer::parseFileStatus("t", fs));
    TS_ASSERT(!HostPlayer::parseFileStatus("", fs));
}

/** Test initialisation. */
void
TestServerInterfaceHostPlayer::testInit()
{
    server::interface::HostPlayer::Info i;
    TS_ASSERT_EQUALS(i.longName, "");
    TS_ASSERT_EQUALS(i.shortName, "");
    TS_ASSERT_EQUALS(i.adjectiveName, "");
    TS_ASSERT_EQUALS(i.userIds.size(), 0U);
    TS_ASSERT_EQUALS(i.numEditable, 0);
    TS_ASSERT_EQUALS(i.joinable, false);
}

