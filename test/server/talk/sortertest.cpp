/**
  *  \file test/server/talk/sortertest.cpp
  *  \brief Test for server::talk::Sorter
  */

#include "server/talk/sorter.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.talk.Sorter")
{
    class Tester : public server::talk::Sorter {
     public:
        virtual void applySortKey(afl::net::redis::SortOperation& /*op*/, const String_t& /*keyName*/) const
            { }
    };
    Tester t;
}
