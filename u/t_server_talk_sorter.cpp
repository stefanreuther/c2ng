/**
  *  \file u/t_server_talk_sorter.cpp
  *  \brief Test for server::talk::Sorter
  */

#include "server/talk/sorter.hpp"

#include "t_server_talk.hpp"

/** Interface test. */
void
TestServerTalkSorter::testInterface()
{
    class Tester : public server::talk::Sorter {
     public:
        virtual void applySortKey(afl::net::redis::SortOperation& /*op*/, const String_t& /*keyName*/) const
            { }
    };
    Tester t;
}

