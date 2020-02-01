/**
  *  \file u/t_server_common_idgenerator.cpp
  *  \brief Test for server::common::IdGenerator
  */

#include "server/common/idgenerator.hpp"

#include "t_server_common.hpp"

/** Interface test. */
void
TestServerCommonIdGenerator::testInterface()
{
    class Tester : public server::common::IdGenerator {
     public:
        virtual String_t createId()
            { return String_t(); }
    };
    Tester t;
}

