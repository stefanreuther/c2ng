/**
  *  \file test/server/file/ca/referenceupdatertest.cpp
  *  \brief Test for server::file::ca::ReferenceUpdater
  */

#include "server/file/ca/referenceupdater.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.file.ca.ReferenceUpdater")
{
    class Tester : public server::file::ca::ReferenceUpdater {
     public:
        virtual void updateDirectoryReference(const String_t& /*name*/, const server::file::ca::ObjectId& /*newId*/)
            { }
    };
    Tester t;
}
