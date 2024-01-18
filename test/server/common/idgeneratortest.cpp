/**
  *  \file test/server/common/idgeneratortest.cpp
  *  \brief Test for server::common::IdGenerator
  */

#include "server/common/idgenerator.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.common.IdGenerator")
{
    class Tester : public server::common::IdGenerator {
     public:
        virtual String_t createId()
            { return String_t(); }
    };
    Tester t;
}
