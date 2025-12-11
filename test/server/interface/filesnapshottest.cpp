/**
  *  \file test/server/interface/filesnapshottest.cpp
  *  \brief Test for server::interface::FileSnapshot
  */

#include "server/interface/filesnapshot.hpp"

#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.interface.FileSnapshot")
{
    class Tester : public server::interface::FileSnapshot {
     public:
        virtual void createSnapshot(String_t /*name*/)
            { }
        virtual void copySnapshot(String_t /*oldName*/, String_t /*newName*/)
            { }
        virtual void removeSnapshot(String_t /*name*/)
            { }
        virtual void listSnapshots(afl::data::StringList_t& /*out*/)
            { }
    };
    Tester t;
}
