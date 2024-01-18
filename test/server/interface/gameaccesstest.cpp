/**
  *  \file test/server/interface/gameaccesstest.cpp
  *  \brief Test for server::interface::GameAccess
  */

#include "server/interface/gameaccess.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.interface.GameAccess")
{
    class Tester : public server::interface::GameAccess {
     public:
        virtual void save()
            { }
        virtual String_t getStatus()
            { return String_t(); }
        virtual server::Value_t* get(String_t /*objName*/)
            { return 0; }
        virtual server::Value_t* post(String_t /*objName*/, const server::Value_t* /*value*/)
            { return 0; }
    };
    Tester t;
}
