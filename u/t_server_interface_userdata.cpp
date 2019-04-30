/**
  *  \file u/t_server_interface_userdata.cpp
  *  \brief Test for server::interface::UserData
  */

#include "server/interface/userdata.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceUserData::testInterface()
{
    class Tester : public server::interface::UserData {
     public:
        virtual void set(String_t /*userId*/, String_t /*key*/, String_t /*value*/)
            { }
        virtual String_t get(String_t /*userId*/, String_t /*key*/)
            { return String_t(); }
    };
    Tester t;
}

