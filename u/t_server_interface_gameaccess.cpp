/**
  *  \file u/t_server_interface_gameaccess.cpp
  *  \brief Test for server::interface::GameAccess
  */

#include "server/interface/gameaccess.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceGameAccess::testInterface()
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

