/**
  *  \file test/server/file/ca/referencecountertest.cpp
  *  \brief Test for server::file::ca::ReferenceCounter
  */

#include "server/file/ca/referencecounter.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.file.ca.ReferenceCounter")
{
    class Tester : public server::file::ca::ReferenceCounter {
     public:
        virtual void set(const server::file::ca::ObjectId& /*id*/, int32_t /*value*/)
            { }
        virtual bool modify(const server::file::ca::ObjectId& /*id*/, int32_t /*delta*/, int32_t& /*result*/)
            { return false; }
    };
    Tester t;
}
