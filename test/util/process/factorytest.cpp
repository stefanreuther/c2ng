/**
  *  \file test/util/process/factorytest.cpp
  *  \brief Test for util::process::Factory
  */

#include "util/process/factory.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("util.process.Factory")
{
    class Tester : public util::process::Factory {
     public:
        virtual util::process::Subprocess* createNewProcess()
            { return 0; }
    };
    Tester t;
}
