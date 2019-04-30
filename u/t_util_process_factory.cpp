/**
  *  \file u/t_util_process_factory.cpp
  *  \brief Test for util::process::Factory
  */

#include "util/process/factory.hpp"

#include "t_util_process.hpp"

/** Interface test. */
void
TestUtilProcessFactory::testInterface()
{
    class Tester : public util::process::Factory {
     public:
        virtual util::process::Subprocess* createNewProcess()
            { return 0; }
    };
    Tester t;
}

